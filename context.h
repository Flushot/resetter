#ifndef __CORE_H__
#define __CORE_H__

#include <stdio.h>
#include <stdio.h>
#include <inttypes.h> // PRId64

#include <pcap.h>
#include <pthread.h>
#include <libnet.h>

#include "utils/hash_table.h"

/**
 * Data structure with all handles
 * (so that cleanup can be simplified).
 */
typedef struct _resetter_context_t {
    /**
     * Device name to capture and inject packets on
     * (e.g. "en0").
     *
     * If blank on startup, this will be automatically detected.
     */
    char* device;

    /**
     * [Optional] Target IP address to block.
     * If unspecified, then blocks all IPs.
     */
    struct sockaddr_in target_addr;

    /**
     * [Optional] Target TCP port to block.
     * If 0, then blocks all ports.
     */
    uint16_t target_port;

    /**
     * pcap: handle.
     */
    pcap_t* pcap;

    /**
     * pcap: BPF filter string.
     */
    char filter_string[1000];

    /**
     * Is ARP poisoning mode enabled?
     */
    int arp_poisoning;

    /**
     * Poisoned ARP table: IP -> original MAC address.
     * Also used for unpoisoning.
     */
    hash_table* arp_table;

    /**
     * libnet: handle.
     */
    libnet_t* libnet;

    /**
     * libnet: Last time stats were printed to the console.
     */
    u_long libnet_last_stats_at;

    /**
     * Cleanup function to call before program is terminated.
     */
    void (*cleanup)(struct _resetter_context_t*);
} resetter_context_t;

#endif
