#include <stdio.h>
#include <unistd.h> // getopt

#include "arp_mitm.h"
#include "listener.h"
#include "resetter.h"

static resetter_context_t ctx;

extern void testfunc();

static void on_signal_trapped(int signum) {
    if (signum == SIGINT) {
        resetter_stop(&ctx);
    }
}

void usage(char **argv) {
    printf("resetter (c)2019 Chris Lyon\n\n"
           "usage: %s [ -f bpf_filter ] [ -z zmq_listen_port ]\n",
           argv[0]);
}

int main(int argc, char **argv) {
    memset(&ctx, 0, sizeof(resetter_context_t));

    int flag;
    char *filter = NULL;

    while ((flag = getopt(argc, argv, "hf:z:")) != -1) {
        switch (flag) {
            case 'f':
                // Filter expr
                filter = optarg;
                break;

            case 'z':
                // ZeroMQ port
#ifdef USE_ZEROMQ
                ctx.zmq_port = strtol(optarg, NULL, 10);
#else
                fprintf(stderr, "zeromq is not enabled in this build\n");
                usage(argv);
#endif
                break;

            case 'h':
                // Help
                usage(argv);
                return EXIT_SUCCESS;

            default:
                // Unknown
                usage(argv);
                return EXIT_FAILURE;
        }
    }

    //testfunc();

    if (resetter_init(&ctx) != 0) {
        return EXIT_FAILURE;
    }

    signal(SIGINT, on_signal_trapped); // Trap ^C

    if (filter == NULL) {
        // Default filter: All TCP traffic
        filter = "tcp";
    }

    if (resetter_start(&ctx, filter) != 0) {
        resetter_cleanup(&ctx);
        return EXIT_FAILURE;
    }

    resetter_cleanup(&ctx);
    return EXIT_SUCCESS;
}
