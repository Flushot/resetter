#include <stdio.h>
#include <unistd.h> // getopt

#include "resetter.h"

static resetter_context_t ctx;

extern void testfunc();

static void on_signal_trapped(int signum) {
    if (signum == SIGINT) {
        resetter_stop(&ctx);
    }
}

void usage(char **argv) {
    printf("usage: %s [ -f filter ]\n", argv[0]);
}

int main(int argc, char **argv) {
    int flag;
    char *filter = NULL;

    while ((flag = getopt(argc, argv, "hf:")) != -1) {
        switch (flag) {
            case 'f':
                // Filter expr
                filter = optarg;
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

    // Testing
    testfunc();

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
