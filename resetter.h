#ifndef __RESETTER_H__
#define __RESETTER_H__

#include "context.h"
#include "thread_mgr.h"

/**
 * Send a RST packet (spoofed from source client) to dest server.
 *
 * Should be sent to dest after SYN-ACK sent from dest -> source (so that we can use its ack#
 * as the next seq#).
 *
 * @param saddr client IP to spoof.
 * @param sport client port (in network byte order).
 * @param daddr server to send RST to.
 * @param dport server port (in network byte order).
 * @param ack ack# from previous SYN-ACK packet (in network byte order).
 * @return 0 if successful, otherwise -1.
 */
int send_reset_packet(
    resetter_context_t *ctx,
    struct sockaddr_in saddr,
    uint16_t sport,
    struct sockaddr_in daddr,
    uint16_t dport,
    uint32_t ack
);

/**
 *
 * @param thread
 * @param device
 * @param target_ip
 * @param target_port
 * @return
 */
int start_resetter_thread(
    thread_node *thread,
    char *device,
    char *target_ip,
    uint16_t target_port
);

#endif
