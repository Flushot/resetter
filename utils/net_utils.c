#include <stdio.h>
#include <inttypes.h>

#include "net_utils.h"

uint32_t net_utils_ip2long(char* ip_addr) {
    uint32_t long_addr = 0;

    char* ip_buffer = strdup(ip_addr);
    if (ip_buffer == NULL) {
        perror("net_utils_ip2long: strdup() failed");
        return -1;
    }

    char* token = strtok(ip_buffer, ".");
    if (token == NULL) {
        return -1;
    }

    while (token != NULL) {
        long_addr <<= 8;
        long_addr |= atoi(token) & 0xFF;

        token = strtok(NULL, "."); // get next token
    }

    free(ip_buffer);

    return long_addr;
}

char* net_utils_long2ip(const uint32_t long_addr, char* ip_addr_out) {
    char octet[4];

    ip_addr_out[0] = 0;

    for (uint8_t i = 0; i < 4; ++i) {
        const int ret = snprintf(
            (char *)&octet, 4, "%d",
            (long_addr >> (24 - 8 * i)) & 0xFF
        );

        if (ret < 0) {
            perror("net_utils_long2ip: snprintf() failed");
            return NULL;
        }

        strcat(ip_addr_out, (char *)&octet);

        if (i < 3) {
            strcat(ip_addr_out, ".");
        }
    }

    return ip_addr_out;
}

int net_utils_ip_matches(char* test_ip_addr, char* match_ip_addr, const uint8_t net_bits) {
    const uint32_t mask = ~0UL << (32 - net_bits);
    return (net_utils_ip2long(test_ip_addr) & mask) == (net_utils_ip2long(match_ip_addr) & mask);
}

char* net_utils_ether_ntoa(const uint8_t* ether_addr) {
    static char addr_buf[18];
    addr_buf[17] = 0;

    snprintf(
        addr_buf,
        sizeof(addr_buf),
        "%02x:%02x:%02x:%02x:%02x:%02x",
        ether_addr[0],
        ether_addr[1],
        ether_addr[2],
        ether_addr[3],
        ether_addr[4],
        ether_addr[5]
    );

    return addr_buf;
}

void maybe_print_libnet_stats(
    resetter_context* ctx,
    char* packet_type
) {
    struct libnet_stats stat;

    // Occasionally report packet sent/errors stats
    u_long curr_time = time(0);
    if (ctx->libnet_last_stats_at == 0) {
        ctx->libnet_last_stats_at = curr_time;
    }
    else if (curr_time - ctx->libnet_last_stats_at > 10) {
        libnet_stats(ctx->libnet, &stat);
        printf("%s packets sent:  %" PRId64 " (%" PRId64 " bytes)\n"
               "%s packet errors: %" PRId64 "\n",
               packet_type,
               stat.packets_sent,
               stat.bytes_written,
               packet_type,
               stat.packet_errors);
        ctx->libnet_last_stats_at = curr_time;
    }
}
