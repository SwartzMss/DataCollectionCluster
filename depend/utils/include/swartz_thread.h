#ifndef __SWARTZ_THREAD_H__
#define __SWARTZ_THREAD_H__

#include "swartz_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct swartz_thread_t swartz_thread_t;

int swartz_thread_create(swartz_thread_t** handle, void*fn, void* arg, int stack_size, int priority);

int swartz_thread_wait(swartz_thread_t* handle);

int swartz_thread_detached_create(void*fn, void* arg, int stack_size, int priority);

swartz_uint64 swartz_thread_get_selfid();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

