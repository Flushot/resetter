#include "arp_mitm.h"
#include "listener.h"

static void on_arp_packet_captured(
        resetter_context_t *,
        const struct pcap_pkthdr *,
        const u_char *);

static int init_libnet(resetter_context_t *ctx) {
    int injection_type = LIBNET_LINK; // Layer 2 (link)

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

static void *arp_mitm_thread(void *vargp) {
    thread_node *thread = (thread_node *)vargp;
    resetter_context_t *ctx = &thread->ctx;

    if (listener_start(ctx, on_arp_packet_captured) != 0) {
        core_cleanup(ctx);
        // return EXIT_FAILURE;
        return NULL;
    }

    return NULL;
}

int start_arp_mitm_thread(thread_node *thread, char *device) {
    resetter_context_t *ctx = &thread->ctx;
    memset(ctx, 0, sizeof(resetter_context_t));
    strcpy(ctx->filter_string, "arp");
    ctx->device = device;

    printf("Monitoring ARP traffic on %s ( %s )...\n", device, ctx->filter_string);

    if (init_libnet(ctx) != 0) {
        return -1;
    }

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
