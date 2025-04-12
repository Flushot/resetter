#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "context.h"

/**
 *
 */
typedef void (*listener_callback)(
    resetter_context* ctx,
    const struct pcap_pkthdr* cap_header,
    const u_char* packet
);

/**
 *
 * @param ctx
 * @return
 */
int is_listener_started(const resetter_context* ctx);

/**
 *
 * @param ctx
 * @param callback
 * @return
 */
int listener_start(
    resetter_context* ctx,
    listener_callback callback
);

/**
 *
 * @param ctx
 */
void listener_stop(resetter_context* ctx);

#endif
