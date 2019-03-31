#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "context.h"

typedef void (*listener_callback)(
        resetter_context_t *ctx,
        const struct pcap_pkthdr *cap_header,
        const u_char *packet);

int is_listener_started(resetter_context_t *);

int listener_start(resetter_context_t *, listener_callback);

void listener_stop(resetter_context_t *);

#endif
