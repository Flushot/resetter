#ifndef __ARP_MITM_H__
#define __ARP_MITM_H__

#include "context.h"
#include "thread_mgr.h"

int send_arp_reply_packet(resetter_context_t *, struct sockaddr_in, uint8_t *);

int send_arp_request_packet(resetter_context_t *, struct sockaddr_in);

int start_arp_mitm_thread(thread_node *, char *);

#endif
