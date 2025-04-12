#ifndef __THREAD_MGR_H__
#define __THREAD_MGR_H__

#include "context.h"

/**
 * Linked list for tracking threads
 */
typedef struct thread_node {
    /**
     *
     */
    pthread_t thread_id;

    /**
     *
     */
    resetter_context ctx;

    /**
     *
     */
    struct thread_node* next;
} thread_node;

/**
 *
 * @param thread
 */
void thmgr_append_thread(thread_node* thread);

/**
 *
 */
void thmgr_wait_for_threads();

/**
 *
 */
void thmgr_cleanup();

#endif
