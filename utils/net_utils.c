#include <stdio.h>
#include <inttypes.h>

#include "net_utils.h"

char *ether_ntoa(uint8_t *ether_addr) {
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
