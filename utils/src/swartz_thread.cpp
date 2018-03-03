#include "swartz_thread.h"
#if defined(_WIN32) || defined(_WIN64)
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#include <stdlib.h>
#endif

#define SWARTZ_ERR (-1)
#define SWARTZ_OK (0)

#define SWARTZ_THREAD_DEFAULT_STACKSIZE 1048576 //1MB

typedef struct swartz_thread_t
{
#if defined(_WIN32) || defined(_WIN64)
	HANDLE handle;
#else
	pthread_t handle;
#endif
	int priority;
	void* fn;
	void* arg;
}swartz_thread_t;

int swartz_thread_create(swartz_thread_t** handle, void*fn, void* arg, int stack_size, int priority)
{
	(void)(priority);

	if (!handle || !fn)
	{
		return SWARTZ_ERR;
	}

	*handle = (swartz_thread_t*)malloc(sizeof(swartz_thread_t));
	if (*handle == NULL)
	{
		return SWARTZ_ERR;
	}

	if (stack_size == 0)
	{
		stack_size = SWARTZ_THREAD_DEFAULT_STACKSIZE;
	}

#if defined(_WIN32) || defined(_WIN64)
	(*handle)->handle = (HANDLE)_beginthreadex(NULL, stack_size, (unsigned (__stdcall*)(void*))fn, arg, 0, NULL);
	if ((*handle)->handle == NULL)
	{
		free(*handle);
		*handle = NULL;
		return SWARTZ_ERR;
	}

	(*handle)->arg = arg;
	(*handle)->fn = fn;
	(*handle)->priority = priority;

	return SWARTZ_OK;
#else
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr);
	if (ret)
	{
		free(*handle);
		*handle = NULL;
		return SWARTZ_ERR;
	}

	if (stack_size != 0)
	{
		ret = pthread_attr_setstacksize(&attr, stack_size);
		if (ret != 0)
		{
			free(*handle);
			*handle = NULL;
			pthread_attr_destroy(&attr);
			return SWARTZ_ERR;
		}
	}

	ret = pthread_create(&(*handle)->handle, &attr, (void *(*)(void*))fn, arg);
	if (ret)
	{
		free(*handle);
		*handle = NULL;
		pthread_attr_destroy(&attr);
		return SWARTZ_ERR;
	}
	else
	{
		(*handle)->arg = arg;
		(*handle)->fn = fn;
		(*handle)->priority = priority;

		pthread_attr_destroy(&attr);
		return SWARTZ_OK;
	}
#endif
}

int swartz_thread_wait(swartz_thread_t* handle)
{
	if (!handle)
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	if (!handle->handle)
	{
		return SWARTZ_ERR;
	}

	if (WAIT_OBJECT_0 == WaitForSingleObject(handle->handle, INFINITE))
	{
		CloseHandle(handle->handle);
		handle->handle = NULL;

		free(handle);
		handle = NULL;

		return SWARTZ_OK;
	}

	return SWARTZ_ERR;
#else
	int ret = 0;

	ret = pthread_join(handle->handle, NULL); 
	free(handle);
	handle = NULL;

	return (ret == 0) ? SWARTZ_OK : SWARTZ_ERR;
#endif
}

int swartz_thread_detached_create(void*fn, void* arg, int stack_size, int priority)
{
	(void)(priority);

	if (!fn)
	{
		return SWARTZ_ERR;
	}

	if (stack_size == 0)
	{
		stack_size = SWARTZ_THREAD_DEFAULT_STACKSIZE;
	}

#if defined(_WIN32) || defined(_WIN64)
	HANDLE handle = (HANDLE)_beginthreadex(NULL, stack_size, (unsigned (__stdcall*)(void*))fn, arg, 0, NULL);
	if (handle == NULL)
	{
		return SWARTZ_ERR;
	}

	CloseHandle(handle);

	return SWARTZ_OK;
#else
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr);
	if (ret)
	{
		return SWARTZ_ERR;
	}

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret)
	{
		pthread_attr_destroy(&attr);
		return SWARTZ_ERR;
	}

	if (stack_size != 0)
	{
		ret = pthread_attr_setstacksize(&attr, stack_size);
		if (ret != 0)
		{
			pthread_attr_destroy(&attr);
			return SWARTZ_ERR;
		}
	}

	pthread_t id;
	ret = pthread_create(&id, &attr, (void *(*)(void*))fn, arg);
	if (ret)
	{
		pthread_attr_destroy(&attr);
		return SWARTZ_ERR;
	}
	else
	{
		pthread_attr_destroy(&attr);
		return SWARTZ_OK;
	}
#endif
}

swartz_uint64 swartz_thread_get_selfid()
{
#if defined(_WIN32) || defined(_WIN64)
	return (swartz_uint64)GetCurrentThreadId();
#else
	return (swartz_uint64)pthread_self();
#endif
}


