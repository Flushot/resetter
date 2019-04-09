#ifndef __CORE_H__
#define __CORE_H__

#include <stdio.h>
#include <stdio.h>
#include <inttypes.h> // PRId64

#include <pcap.h>
#include <pthread.h>
#include <libnet.h>

/**
 * Data structure with all handles
 * (so that cleanup can be simplified).
 */
typedef struct _resetter_context_t {
    char *device; // pcap capture and libnet device (e.g. "en0"); leave blank to detect.

    struct sockaddr_in target_addr;
    uint16_t target_port;

    pcap_t *pcap; // pcap handle
    char filter_string[1000];
    int arp_poisoning;

    libnet_t *libnet; // libnet handle
    u_long libnet_last_stats_at; // last time libnet stats were reported

    void (*cleanup)(struct _resetter_context_t *);
} resetter_context_t;

#endif