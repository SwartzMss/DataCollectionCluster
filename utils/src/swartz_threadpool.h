#ifndef __swartz_THREADPOOL_H__   
#define __swartz_THREADPOOL_H__  
  
#include "swartz_types.h"  
#ifdef __cplusplus  
extern "C" {  
#endif /* __cplusplus */  
  
typedef struct swartz_thread_pool_t swartz_thread_pool_t;  
  
  
int swartz_threadpool_create(swartz_thread_pool_t** handle, int thread_num, int idle_time, void*(CALLBACK *fn)(void*), int stack_size);  
  
  
int swartz_threadpool_destroy(swartz_thread_pool_t* handle);  
  
  
int swartz_threadpool_work(swartz_thread_pool_t* handle, void* param);  
  
#ifdef __cplusplus  
}  
#endif /* __cplusplus */  
  
#endif  
