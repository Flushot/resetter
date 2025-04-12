#ifndef __ARP_MITM_H__
#define __ARP_MITM_H__

#include "context.h"
#include "thread_mgr.h"

/**
 *
 * @param ctx
 * @param addr
 * @param victim_eth_addr
 * @return
 */
int send_arp_reply_packet(
    resetter_context* ctx,
    struct sockaddr_in addr,
    const uint8_t* victim_eth_addr
);

/**
 *
 * @param ctx
 * @param addr
 * @return
 */
int send_arp_request_packet(
    resetter_context* ctx,
    struct sockaddr_in addr
);

/**
 *
 * @param thread
 * @param device
 * @return
 */
int start_arp_mitm_thread(thread_node* thread, char* device);

#endif
