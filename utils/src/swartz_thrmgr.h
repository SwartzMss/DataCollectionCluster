#ifndef __SWARTZ_THRMGR_H__
#define __SWARTZ_THRMGR_H__

#include "swartz_types.h"

void* thrmgr_new(int max_threads, int idle_timeout, void (CALLBACK *handler)(void *), int stack_size);
void thrmgr_destroy(void* threadpool);

swartz_bool thrmgr_dispatch(void*  threadpool, void *user_data);

#endif

