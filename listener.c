#include "listener.h"

typedef struct packet_capture_args {
    resetter_context* ctx;
    listener_callback callback;
} packet_capture_args;

static void on_packet_captured(
    u_char* user_args,
    const struct pcap_pkthdr* cap_header,
    const u_char* packet) {
    const packet_capture_args* args = (packet_capture_args *)user_args;
    (*args->callback)(args->ctx, cap_header, packet);
}

static int listener_init(resetter_context* ctx) {
    const int snapshot_length = 2048;
    const int promiscuous = 1; // Promiscuous mode
    const int timeout = 1; // Packet buffer timeout https://www.tcpdump.org/manpages/pcap.3pcap.html
    char errbuf[PCAP_ERRBUF_SIZE];

    if (ctx->device == NULL) {
        fprintf(stderr, "Interface is required\n");
        return -1;
    }

    ctx->pcap = pcap_open_live(ctx->device, snapshot_length, promiscuous, timeout, errbuf);
    if (ctx->pcap == NULL) {
        fprintf(stderr, "pcap_open_live() failed: %s\n", errbuf);
        return -1;
    }

    return 0;
}

static int set_pcap_filter(const resetter_context* ctx) {
    struct bpf_program filter;

    // Compile BPF filter
    if (pcap_compile(ctx->pcap, &filter, ctx->filter_string, 0, PCAP_NETMASK_UNKNOWN) == PCAP_ERROR) {
        pcap_perror(ctx->pcap, "pcap_compile() failed");
        return -1;
    }

    // Use compiled filter
    if (pcap_setfilter(ctx->pcap, &filter) == PCAP_ERROR) {
        pcap_perror(ctx->pcap, "pcap_setfilter() failed");
        return -1;
    }

    return 0;
}

int is_listener_started(const resetter_context* ctx) {
    return ctx->pcap != NULL;
}

int listener_start(resetter_context* ctx, const listener_callback callback) {
    packet_capture_args args;

    if (listener_init(ctx) != 0) {
        return -1;
    }

    if (set_pcap_filter(ctx) != 0) {
        return -1;
    }

    args.ctx = ctx;
    args.callback = callback;

    // Listen for packets
    pcap_loop(ctx->pcap, -1, on_packet_captured, (u_char *)&args);

    return 0;
}

void listener_stop(resetter_context* ctx) {
    if (ctx->pcap == NULL) {
        return;
    }

    pcap_breakloop(ctx->pcap);

    // pcap must be closed before pcap_loop() is exited
    pcap_close(ctx->pcap);
    ctx->pcap = NULL;
}
