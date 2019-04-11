#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

/**
 * Convert ethernet address to human-readable string.
 *
 * @param ether_addr ethernet address.
 * @return human-readable string.
 */
char *ether_ntoa(uint8_t *ether_addr);

#endif
