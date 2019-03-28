#include <stdio.h>
#include <unistd.h> // getopt

#include <pcap.h>

#include "resetter.h"

static resetter_context_t ctx;

void on_signal_trapped(int);

int main(int argc, char **argv) {
    if (resetter_init(&ctx) != 0) {
        fprintf(stderr, "failed to init resetter");
        return EXIT_FAILURE;
    }

    signal(SIGINT, on_signal_trapped); // Trap ^C

    if (resetter_start(&ctx, "tcp[tcpflags] & tcp-ack != 0 && ( port 80 or port 443 )") != 0) {
        fprintf(stderr, "failed to start resetter");
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
