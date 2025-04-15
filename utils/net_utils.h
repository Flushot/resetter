#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

#include "../context.h"

/**
 * Convert an IPv4 address string into a long
 *
 * @param ip_addr IPv4 address string to convert
 * @return Long representation of IP address (or -1 if failed)
 */
uint32_t net_utils_ip2long(char* ip_addr);

/**
 * Convert a long into an IPv4 address string
 *
 * @param long_addr Long to convert to IPv4 address
 * @param ip_addr_out Output buffer for IPv4 address string
 */
char* net_utils_long2ip(uint32_t long_addr, char* ip_addr_out);

/**
 * Matches IP addresses using net_bits significant bits.
 *
 * @param test_ip_addr IPv4 address to test
 * @param match_ip_addr Matching IPv4 address or network
 * @param net_bits Number of network bits to use when matching against match_ip_addr (0-32)
 * @return 1 if match, or 0 if no match
 */
int net_utils_ip_matches(char* test_ip_addr, char* match_ip_addr, uint8_t net_bits);

/**
 * Convert ethernet address to human-readable string.
 *
 * @param ether_addr ethernet address.
 * @return human-readable string.
 */
char* net_utils_ether_ntoa(const uint8_t* ether_addr);

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
