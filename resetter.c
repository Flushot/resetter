#include <stdio.h>
#include <inttypes.h> // PRId64

#include "resetter.h"

static int init_zeromq(resetter_context_t *ctx) {
#ifdef USE_ZEROMQ
    if (ctx->zmq_port == 0) {
        // Disable zmq: No port specified
        return 0;
    }

    ctx->zmq_ctx = zmq_ctx_new();
    if (ctx->zmq_ctx == NULL) {
        perror("zmq_ctx_new() failed");
        return -1;
    }

    ctx->zmq_pub = zmq_socket(ctx->zmq_ctx, ZMQ_PUB);
    if (ctx->zmq_pub == NULL) {
        perror("zmq_socket() failed");
        return -1;
    }

    char zmq_url[500];
    snprintf(zmq_url, sizeof(zmq_url) / sizeof(char) - 1, "tcp://*:%d", ctx->zmq_port);

    if (zmq_bind(ctx->zmq_pub, zmq_url) == -1) {
        perror("zmq_bind() failed");
        return -1;
    }

    printf("ZeroMQ listening at: %s\n", zmq_url);
#endif

    return 0;
}

static int init_libnet(resetter_context_t *ctx) {
    int injection_type = LIBNET_RAW4; // Lowest layer is IPv4

    char errbuf[LIBNET_ERRBUF_SIZE];

    ctx->libnet = libnet_init(injection_type, ctx->device, errbuf);
    if (ctx->libnet == NULL) {
        fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
        return -1;
    }

    if (libnet_seed_prand(ctx->libnet) == -1) {
        fprintf(stderr, "libnet_seed_prand() failed: %s\n",libnet_geterror(ctx->libnet));
        return -1;
    }

    return 0;
}

/**
 * Compile and set pcap packet filter, which will determine what gets sent to
 * the on_packet_captured function.
 *
 * @param ctx
 * @param filter_string BPF filter string http://biot.com/capstats/bpf.html
 * @return 0 if successful, otherwise -1.
 */
static int init_pcap(resetter_context_t *ctx) {
    const int snapshot_length = 2048;
    const int promiscuous = 1; // Promiscuous mode
    const int timeout = 1; // Packet buffer timeout https://www.tcpdump.org/manpages/pcap.3pcap.html

    char errbuf[PCAP_ERRBUF_SIZE];

    if (ctx->device == NULL) {
        ctx->device = pcap_lookupdev(errbuf);
    }

    if (ctx->device == NULL) {
        fprintf(stderr, "pcap_lookupdev() failed: %s\n", errbuf);
        return -1;
    }

    ctx->pcap = pcap_open_live(ctx->device, snapshot_length, promiscuous, timeout, errbuf);
    if (ctx->pcap == NULL) {
        fprintf(stderr, "pcap_open_live() failed: %s\n", errbuf);
        return -1;
    }

    return 0;
}

static int set_pcap_filter(resetter_context_t *ctx, char *filter_suffix) {
    // Always prepend with this condition to just include SYN-ACK packets
    char *filter_prefix = "( tcp[tcpflags] & tcp-ack != 0 ) && ";
    char *filter_string = (char *)malloc(sizeof(char) * strlen(filter_prefix) + strlen(filter_suffix) + 1);
    if (filter_string == NULL) {
        perror("malloc() failed");
        return -1;
    }

    filter_string[0] = 0;
    strcat(filter_string, filter_prefix);
    strcat(filter_string, filter_suffix);

    printf("Filter: %s\n", filter_string);

    // Compile BPF filter
    struct bpf_program filter;
    if (pcap_compile(ctx->pcap, &filter, filter_string, 0, PCAP_NETMASK_UNKNOWN) == PCAP_ERROR) {
        pcap_perror(ctx->pcap, "pcap_compile() failed");
        free(filter_string);
        return -1;
    }

    free(filter_string);

    // Use compiled filter
    if (pcap_setfilter(ctx->pcap, &filter) == PCAP_ERROR) {
        pcap_perror(ctx->pcap, "pcap_setfilter() failed");
        return -1;
    }

    return 0;
}

int send_reset_packet(
        resetter_context_t *ctx,
        struct sockaddr_in saddr, uint16_t sport,
        struct sockaddr_in daddr, uint16_t dport,
        uint32_t ack) {
    static libnet_ptag_t tcp_tag = LIBNET_PTAG_INITIALIZER;
    static libnet_ptag_t ip_tag = LIBNET_PTAG_INITIALIZER;

    // inet_ntoa stores results in a static buffer that gets overwritten with every call
    char saddr_str[16], daddr_str[16];
    strncpy(saddr_str, inet_ntoa(saddr.sin_addr), sizeof(saddr_str));
    strncpy(daddr_str, inet_ntoa(daddr.sin_addr), sizeof(daddr_str));

    printf("Resetting TCP connection from %s:%d <-> %s:%d\n",
           saddr_str, sport,
           daddr_str, dport);

    // Build TCP header
    tcp_tag = libnet_build_tcp(
            sport,                           // Source TCP port (pretend we are dst)
            dport,                           // Destination TCP port (send back to src)
            ack,                             // Sequence number (use previous ack)
            (uint32_t)libnet_get_prand(LIBNET_PRu32), // Acknowledgement number
            TH_RST,                          // Control flags (RST flag set only)
            (uint16_t)libnet_get_prand(LIBNET_PRu16), // Window size
            0,                               // Checksum
            0,                               // Urgent pointer
            LIBNET_TCP_H,                    // Packet size (so far)
            NULL,                            // Payload (none)
            0,                               // Payload length
            ctx->libnet,
            tcp_tag);
    if (tcp_tag == -1) {
        fprintf(stderr, "Error building TCP header: %s\n", libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Build IP header
    ip_tag = libnet_build_ipv4(
            LIBNET_IPV4_H + LIBNET_TCP_H,    // Packet size (so far)
            IPTOS_LOWDELAY,                  // TOS
            (uint16_t)libnet_get_prand(LIBNET_PRu16), // IP ID
            0,                               // IP frag flags/offset
            64,                              // TTL
            IPPROTO_TCP,                     // Transport protocol
            0,                               // Checksum (auto calculate)
            saddr.sin_addr.s_addr,           // Source IP (pretend we are dst)
            daddr.sin_addr.s_addr,           // Destination IP (send back to src)
            NULL,                            // Payload (none)
            0,                               // Payload length
            ctx->libnet,
            ip_tag);
    if (ip_tag == -1) {
        fprintf(stderr, "Error building IP header: %s\n", libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Write packet
    int bytes_written = libnet_write(ctx->libnet);
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Passing ptag to reuse packet instead of clearing (which is less CPU efficient)
    // libnet_clear_packet(ctx.libnet);

#ifdef USE_ZEROMQ
    // Publish zeromq message.
    if (ctx->zmq_pub != NULL) {
        char queue_message[500];
        snprintf(queue_message, sizeof(queue_message) / sizeof(char) - 1,
                 "reset %s:%d %s:%d", saddr_str, sport, daddr_str, dport);
        if (zmq_send(ctx->zmq_pub, queue_message, strlen(queue_message), 0) == -1) {
            perror("zmq_send() failed");
        }
    }
#endif

    // Occasionally report packet sent/errors stats
    u_long curr_time = (u_long)time(0);
    if (ctx->libnet_last_stats_at == 0) {
        ctx->libnet_last_stats_at = curr_time;
    } else if (curr_time - ctx->libnet_last_stats_at > 10) {
        struct libnet_stats stat;
        libnet_stats(ctx->libnet, &stat);
        printf("Packets sent:  %" PRId64 " (%" PRId64 " bytes)\n"
               "Packet errors: %" PRId64 "\n",
               stat.packets_sent,
               stat.bytes_written,
               stat.packet_errors);
        ctx->libnet_last_stats_at = curr_time;
    }

    return 0;
}

/**
 * Packet captured by pcap.
 *
 * @param user_args
 * @param cap_header
 * @param packet raw packet data.
 */
static void on_packet_captured(u_char *user_args, const struct pcap_pkthdr *cap_header, const u_char *packet) {
    resetter_context_t *ctx = (resetter_context_t *)user_args;
    struct libnet_ipv4_hdr *ip_hdr = (struct libnet_ipv4_hdr *)(packet + LIBNET_ETH_H);
    struct libnet_tcp_hdr *tcp_hdr = (struct libnet_tcp_hdr *)(packet + LIBNET_ETH_H + LIBNET_TCP_H);

    // Get saddr/daddr from IPv4 header
    struct sockaddr_in saddr, daddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    memset(&daddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_addr.s_addr = *((uint32_t *)&(ip_hdr->ip_src));
    daddr.sin_addr.s_addr = *((uint32_t *)&(ip_hdr->ip_dst));

    if (tcp_hdr->th_flags & (TH_SYN | TH_ACK)) {
        // SYN-ACK packet sent from dest to source, so send a RST packet
        // back to dest (spoofed as source).
        send_reset_packet(
                ctx,
                daddr, htons(tcp_hdr->th_dport),
                saddr, htons(tcp_hdr->th_sport),
                htonl(tcp_hdr->th_ack));
    }
}

int resetter_init(resetter_context_t *ctx) {
    if (init_zeromq(ctx) != 0 || init_libnet(ctx) != 0 || init_pcap(ctx) != 0) {
        resetter_cleanup(ctx);
        return -1;
    }

    return 0;
}

int resetter_start(resetter_context_t *ctx, char *filter_string) {
    if (set_pcap_filter(ctx, filter_string) != 0) {
        return -1;
    }

    // Listen for packets
    printf("Listening for packets on %s...\n", ctx->device);
    pcap_loop(ctx->pcap, -1, on_packet_captured, (u_char *)ctx);

    return 0;
}

void resetter_stop(resetter_context_t *ctx) {
    if (ctx->pcap != NULL) {
        pcap_breakloop(ctx->pcap);
    }

    // pcap must be closed before pcap_loop() is exited
    resetter_cleanup(ctx);
}

void resetter_cleanup(resetter_context_t *ctx) {
    if (ctx->pcap != NULL) {
        pcap_close(ctx->pcap);
        ctx->pcap = NULL;
    }

    if (ctx->libnet != NULL) {
        libnet_destroy(ctx->libnet);
        ctx->libnet = NULL;
    }

#ifdef USE_ZEROMQ
    if (ctx->zmq_pub != NULL) {
        zmq_close(ctx->zmq_pub);
        ctx->zmq_pub = NULL;
    }

    if (ctx->zmq_ctx != NULL) {
        zmq_ctx_destroy(ctx->zmq_ctx);
        ctx->zmq_ctx = NULL;
    }
#endif
}
