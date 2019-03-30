#include "arp_mitm.h"
#include "listener.h"

static void on_arp_packet_captured(
        resetter_context_t *,
        const struct pcap_pkthdr *,
        const u_char *);

static void *arp_mitm_thread(void *vargp) {
    thread_node *thread = (thread_node *)vargp;
    resetter_context_t ctx = thread->ctx;

    if (core_init(&ctx) != 0) {
        // return EXIT_FAILURE;
        return NULL;
    }

    if (listener_start(&ctx, on_arp_packet_captured) != 0) {
        core_cleanup(&ctx);
        // return EXIT_FAILURE;
        return NULL;
    }

    return NULL;
}

int start_arp_mitm_thread(thread_node *thread) {
    printf("Starting ARP MITM thread...\n");

    resetter_context_t ctx;
    memset(&ctx, 0, sizeof(resetter_context_t));

    strcpy(ctx.filter_string, "arp");
    thread->ctx = ctx;

    if (pthread_create(&thread->thread_id, NULL, arp_mitm_thread, (void *)thread) != 0) {
        perror("pthread_create() failed");
        return -1;
    }

    return 0;
}

static void on_arp_packet_captured(
        resetter_context_t *ctx,
        const struct pcap_pkthdr *cap_header,
        const u_char *packet) {

    // TODO: process arp packet
    printf("ARP packet captured!\n");
}
