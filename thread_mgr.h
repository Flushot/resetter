#ifndef __THREAD_MGR_H__
#define __THREAD_MGR_H__

#include "core.h"

void thmgr_append_thread(thread_node *);

void thmgr_wait_for_threads();

void thmgr_cleanup();

#endif
