#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

/**
 * Convert ethernet address to human-readable string.
 *
 * @param ether_addr ethernet address.
 * @return human-readable string.
 */
char *ether_ntoa(uint8_t *ether_addr);

/**
 *
 * @param ctx
 * @param packet_type
 */
void maybe_print_libnet_stats(resetter_context_t *ctx, char *packet_type);

#endif
