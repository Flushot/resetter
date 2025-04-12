#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

/**
 * Convert ethernet address to human-readable string.
 *
 * @param ether_addr ethernet address.
 * @return human-readable string.
 */
char* ether_ntoa(const uint8_t* ether_addr);

/**
 *
 * @param ctx
 * @param packet_type
 */
void maybe_print_libnet_stats(
    resetter_context* ctx,
    char* packet_type
);

#endif
