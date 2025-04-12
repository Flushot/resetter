#include <stdio.h>
#include <inttypes.h>

#include "../context.h"
#include "net_utils.h"

char* ether_ntoa(const uint8_t* ether_addr) {
    static char addr_buf[18];
    addr_buf[17] = 0;

    snprintf(addr_buf, sizeof(addr_buf),
             "%02x:%02x:%02x:%02x:%02x:%02x",
             ether_addr[0],
             ether_addr[1],
             ether_addr[2],
             ether_addr[3],
             ether_addr[4],
             ether_addr[5]);

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
