#include "core.h"
#include "arp_mitm.h"
#include "listener.h"
#include "resetter.h"
#include "thread_mgr.h"

extern void testfunc();

static void on_signal_trapped(int);

void usage(char **argv) {
    printf("resetter (c)2019 Chris Lyon\n\n"
           "usage: %s [ -f bpf_filter ]\n",
           argv[0]);
}

int main(int argc, char **argv) {
    int flag;
    char *filter = NULL;
    int arp_mitm = 0;

    while ((flag = getopt(argc, argv, "haf:")) != -1) {
        switch (flag) {
            case 'f':
                // Filter expr
                filter = optarg;
                break;

            case 'a':
                // Enable ARP MITM attack
                arp_mitm = 1;
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

    signal(SIGINT, on_signal_trapped); // Trap ^C

    // Start resetter thread
    thread_node resetter_thread_node;
    thmgr_append_thread(&resetter_thread_node);
    if (start_resetter_thread(&resetter_thread_node, filter) != 0) {
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
