#include "swartz_threadpool.h"
#include "swartz_thrmgr.h"
#include <stdlib.h>

typedef struct swartz_thread_pool_t
{
	void* handle;
}swartz_thread_pool_t;

int swartz_threadpool_create(swartz_thread_pool_t** handle, int thread_num, int idle_time, void*(CALLBACK *fn)(void*), int stack_size)
{
	swartz_thread_pool_t* tmp = (swartz_thread_pool_t*)malloc(sizeof(swartz_thread_pool_t));
	if (NULL != tmp)
	{
		tmp->handle = thrmgr_new(thread_num, idle_time, (void(CALLBACK *)(void*))fn, stack_size);
		if (NULL != tmp->handle)
		{
			*handle = tmp;
			return SWARTZ_OK;
		}
		
		free(tmp);
		tmp = NULL;
	}
	return SWARTZ_ERR;
}

int swartz_threadpool_destroy(swartz_thread_pool_t* handle)
{
	if (!handle || !handle->handle)
	{
		return SWARTZ_TRUE;
	}

	thrmgr_destroy(handle->handle);

	return SWARTZ_OK;
}

int swartz_threadpool_work(swartz_thread_pool_t* handle, void* param)
{
	if (!handle || !param || !handle->handle)
	{
		return SWARTZ_ERR;
	}

	return thrmgr_dispatch(handle->handle, param) ? SWARTZ_OK : SWARTZ_ERR;
}

