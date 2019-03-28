#include <libnet.h>
#include <pcap.h>
#include <zmq.h>

/**
 * Data structure with all handles
 * (so that cleanup can be simplified).
 */
typedef struct _resetter_context_t {
    char *device; // pcap capture and libnet device (e.g. "en0"); leave blank to detect.

    pcap_t *pcap; // pcap handle

    libnet_t *libnet; // libnet handle
    u_long libnet_last_stats_at; // last time libnet stats were reported

#ifdef USE_ZEROMQ
    void *zmq_ctx;  // zeromq context
    void *zmq_pub;  // zeromq socket
#endif
} resetter_context_t;

/**
 * Initialize context.
 *
 * @return 0 if successful, otherwise -1.
 */
int resetter_init(resetter_context_t *);

int resetter_start(resetter_context_t *, char *);

void resetter_stop(resetter_context_t *);

/**
 * Clean up: Close all handles.
 *
 * This function can safely be called multiple times.
 */
void resetter_cleanup(resetter_context_t *);

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
