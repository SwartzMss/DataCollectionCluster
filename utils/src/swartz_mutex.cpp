#include "swartz_types.h"
#include "swartz_mutex.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#include <stdlib.h>
#endif

typedef struct swartz_mutex_t
{
#if defined(_WIN32) || defined(_WIN64)
	CRITICAL_SECTION mutex;
#else
	pthread_mutex_t mutex;
#endif
}swartz_mutex_t;

int swartz_mutex_create(swartz_mutex_t** handle, int flag)
{
	if (NULL == handle)
	{
		return SWARTZ_ERR;
	}

	*handle = (swartz_mutex_t*)malloc(sizeof(swartz_mutex_t));
	if (NULL == *handle)
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	(void)(flag);
	__try
	{
		InitializeCriticalSection(&(*handle)->mutex);
	}
	__except(GetExceptionCode() == STATUS_NO_MEMORY)
	{
		free(*handle);
		*handle = NULL;

		return SWARTZ_ERR;
	}

	return SWARTZ_OK;
#else
	int ret = 0;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);

	pthread_mutexattr_settype(&attr, flag);

	ret = pthread_mutex_init(&((*handle)->mutex), &attr); 

	pthread_mutexattr_destroy(&attr);
	
	if (ret != 0)
	{
		free(*handle);
		*handle = NULL;
	}

	return (ret == 0) ? SWARTZ_OK : SWARTZ_ERR;
#endif
}

int swartz_mutex_lock(swartz_mutex_t* handle)
{
	if (!handle)
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	EnterCriticalSection(&(handle->mutex));
	return SWARTZ_OK;
#else
	return pthread_mutex_lock(&(handle->mutex)) == 0 ? SWARTZ_OK : SWARTZ_ERR;
#endif
}

int swartz_mutex_trylock(swartz_mutex_t* handle)
{
	if ( (!handle))
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	TryEnterCriticalSection(&handle->mutex);
	return SWARTZ_OK;
#else
	return pthread_mutex_trylock(&(handle->mutex)) == 0 ? SWARTZ_OK : SWARTZ_ERR;
#endif
}

int swartz_mutex_unlock(swartz_mutex_t* handle)
{
	if ( (!handle))
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	LeaveCriticalSection(&handle->mutex);
	return SWARTZ_OK;
#else

	return pthread_mutex_unlock(&(handle->mutex)) == 0 ? SWARTZ_OK : SWARTZ_ERR;

#endif
}

int swartz_mutex_destroy(swartz_mutex_t* handle)
{
	if (!handle)
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	DeleteCriticalSection(&(handle->mutex));
#else
	 pthread_mutex_destroy(&(handle->mutex));
#endif

	 free(handle);
	 return SWARTZ_OK;
}

