#include <stdio.h>
#include <unistd.h> // getopt

#include <pcap.h>

#include "resetter.h"

static resetter_context_t ctx;

void on_signal_trapped(int);

int main(int argc, char **argv) {
    if (resetter_init(&ctx) != 0) {
        return EXIT_FAILURE;
    }

    signal(SIGINT, on_signal_trapped); // Trap ^C

    char *filter_string;
    if (argc > 1) {
        // User-defined
        filter_string = argv[1];
    } else {
        // All HTTP/SSL traffic
        filter_string = "tcp[tcpflags] & tcp-ack != 0 && ( port 80 or port 443 )";
    }

    if (resetter_start(&ctx, filter_string) != 0) {
        resetter_cleanup(&ctx);
        return EXIT_FAILURE;
    }

    resetter_cleanup(&ctx);

    printf("Exiting...\n");
    return EXIT_SUCCESS;
}

void on_signal_trapped(int signum) {
    if (signum == SIGINT) {
        resetter_stop(&ctx);
    }
}
