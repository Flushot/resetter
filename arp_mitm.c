#include "arp_mitm.h"
#include "listener.h"

#define IP_ADDR_LEN 4

/*
 * ARP payload
 */
typedef struct _arp_payload_t {
    u_char ar_sha[ETHER_ADDR_LEN]; // Sender HW addr
    u_char ar_spa[IP_ADDR_LEN];    // Sender IP addr
    u_char ar_tha[ETHER_ADDR_LEN]; // Target HW addr
    u_char ar_tpa[IP_ADDR_LEN];    // Target IP addr
} arp_payload_t;

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

static char *ether_ntoa(uint8_t *ether_addr) {
    static char addr_buf[18];
    addr_buf[17] = 0;

    snprintf(addr_buf, sizeof(addr_buf),
             "%02x:%02x:%02x:%02x:%02x:%02x",
             ether_addr[0],
             ether_addr[1],
             ether_addr[2],
             ether_addr[3],
             ether_addr[4],
             ether_addr[5]);

    return addr_buf;
}

static void on_arp_packet_captured(
        resetter_context_t *ctx,
        const struct pcap_pkthdr *cap_header,
        const u_char *packet) {
    // Resolve local MAC address
    static struct libnet_ether_addr *local_mac_addr = NULL;
    if (local_mac_addr == NULL) {
        local_mac_addr = libnet_get_hwaddr(ctx->libnet);
        if (local_mac_addr == NULL) {
            fprintf(stderr, "Error getting local MAC address: %s\n",
                    libnet_geterror(ctx->libnet));
            listener_stop(ctx);
            return;
        }

        printf("Local MAC address: %s\n", ether_ntoa(local_mac_addr->ether_addr_octet));
    }

    struct libnet_ethernet_hdr *eth_hdr = (struct libnet_ethernet_hdr *)packet;
    if (htons(eth_hdr->ether_type) != ETHERTYPE_ARP) {
        // Type must be ARP
        return;
    }

    struct libnet_arp_hdr *arp_hdr = (struct libnet_arp_hdr *)(packet + LIBNET_ETH_H);
    if (htons(arp_hdr->ar_hrd) != ARPHRD_ETHER || htons(arp_hdr->ar_pro) != 0x0800) {
        // HW address format must be ethernet and ARP proto must be IPv4
        return;
    }

    arp_payload_t *arp_payload = (arp_payload_t *)(packet + LIBNET_ETH_H + LIBNET_ARP_H);

    struct sockaddr_in saddr, daddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    memset(&daddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_addr.s_addr = *((uint32_t *)&(arp_payload->ar_spa));
    daddr.sin_addr.s_addr = *((uint32_t *)&(arp_payload->ar_tpa));

    switch (htons(arp_hdr->ar_op)) {
        case ARPOP_REQUEST:
            // req to resolve address
            printf("arp %s -> ", ether_ntoa(eth_hdr->ether_shost));
            printf("%s who-has %s ", ether_ntoa(eth_hdr->ether_dhost), inet_ntoa(daddr.sin_addr));
            printf("tell %s (%s)\n", inet_ntoa(saddr.sin_addr), ether_ntoa(arp_payload->ar_sha));
            break;

        case ARPOP_REPLY:
            // resp to previous request
            printf("arp %s -> ", ether_ntoa(eth_hdr->ether_shost));
            printf("%s reply %s is-at ", ether_ntoa(eth_hdr->ether_dhost), inet_ntoa(saddr.sin_addr));
            printf("%s\n", ether_ntoa(arp_payload->ar_sha));
            break;
    }
}
