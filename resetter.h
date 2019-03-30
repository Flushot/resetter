#ifndef __RESETTER_H__
#define __RESETTER_H__

#include "core.h"

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
        resetter_context_t *,
        struct sockaddr_in,
        uint16_t,
        struct sockaddr_in,
        uint16_t,
        uint32_t);

int start_resetter_thread(thread_node *, char *, char *, uint16_t);

#endif
