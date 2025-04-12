#include <pthread.h>

#include "thread_mgr.h"
#include "listener.h"

static thread_node* thread_list = NULL;

void thmgr_append_thread(thread_node* thread) {
    thread->next = NULL;

    if (thread_list == NULL) {
        thread_list = thread;
    }
    else {
        thread_node* list = thread_list;
        thread_node* tail;

        do {
            tail = list;
        }
        while (list->next != NULL);

        tail->next = thread;
    }
}

void thmgr_wait_for_threads() {
    thread_node* list = thread_list;
    thread_node* thread;

    while ((thread = list) != NULL) {
        pthread_join(thread->thread_id, NULL);
        list = thread->next;
    }
}

void thmgr_cleanup() {
    thread_node* thread;

    while ((thread = thread_list) != NULL) {
        resetter_context* ctx = &thread->ctx;

        if (ctx->cleanup) {
            (*ctx->cleanup)(ctx);
        }

        pthread_cancel(thread->thread_id);

        thread_list = thread->next;
    }
}
