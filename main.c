#include "core.h"
#include "arp_mitm.h"
#include "listener.h"
#include "resetter.h"

static thread_node *thread_list = NULL;

extern void testfunc();

static void on_signal_trapped(int);

static void append_thread(thread_node *);

static void wait_for_threads(thread_node *);

static void cleanup();

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
    append_thread(&resetter_thread_node);
    if (start_resetter_thread(&resetter_thread_node, filter) != 0) {
        fprintf(stderr, "failed to start resetter thread\n");
        cleanup();
        return EXIT_FAILURE;
    }

    // Start arpmitm thread
    if (arp_mitm) {
        thread_node arp_mitm_thread_node;
        append_thread(&arp_mitm_thread_node);
        if (start_arp_mitm_thread(&arp_mitm_thread_node) != 0) {
            fprintf(stderr, "failed to start arp mitm thread\n");
            cleanup();
            return EXIT_FAILURE;
        }
    }

    wait_for_threads(thread_list);
    cleanup();
    return EXIT_SUCCESS;
}

static void append_thread(thread_node *thread) {
    thread->next = NULL;

    if (thread_list == NULL) {
        thread_list = thread;
    } else {
        thread_node *list = thread_list;
        thread_node *tail;

        do {
            tail = list;
        } while (list->next != NULL);

        tail->next = thread;
    }
}

static void on_signal_trapped(int signum) {
    if (signum == SIGINT) {
        cleanup();
    }
}

static void cleanup() {
    thread_node *thread;

    while ((thread = thread_list) != NULL) {
        resetter_context_t *ctx = &thread->ctx;

        if (is_listener_started(ctx)) {
            listener_stop(ctx);
        }

        core_cleanup(ctx);
        pthread_cancel(thread->thread_id);

        thread_list = thread->next;
    }
}

static void wait_for_threads(thread_node *list) {
    thread_node *thread;

    while ((thread = list) != NULL) {
        pthread_join(thread->thread_id, NULL);
        list = thread->next;
    }
}
