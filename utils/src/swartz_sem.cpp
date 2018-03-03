#include "swartz_types.h"
#include "swartz_sem.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <semaphore.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#endif

typedef struct swartz_sem_t
{
#if defined(_WIN32) || defined(_WIN64)
	HANDLE sem;
#else
	sem_t sem;
#endif
}swartz_sem_t;

int swartz_sem_create(swartz_sem_t** handle, int count)
{
	if (NULL == handle)
	{
		return SWARTZ_ERR;
	}

	*handle = (swartz_sem_t*)malloc(sizeof(swartz_sem_t));
	if (NULL == *handle)
	{
		return SWARTZ_ERR;
	}

#if defined(_WIN32) || defined(_WIN64)
	(*handle)->sem = CreateSemaphore(NULL, count, 65535, NULL);

	if (NULL == (*handle)->sem)
	{
		free(*handle);
		*handle = NULL;

		return SWARTZ_ERR;
	}

	return SWARTZ_OK;
#else
	int ret = 0;

	ret = sem_init(&((*handle)->sem), 0, count);

	if (ret != 0)
	{
		free(*handle);
		*handle = NULL;
	}

	return (ret == 0) ? SWARTZ_OK : SWARTZ_ERR;
#endif
}

int swartz_sem_post(swartz_sem_t* handle, int count)
{
#if defined(_WIN32) || defined(_WIN64)
	LONG old_count = 0;

	if ((!handle) || (handle->sem == NULL))
	{
		return SWARTZ_ERR;
	}

	if (ReleaseSemaphore(handle->sem, (LONG)count, &old_count))
	{
		return SWARTZ_OK;
	}

	return SWARTZ_ERR;
#else
	return (sem_post(&(handle->sem)) == 0) ? SWARTZ_OK : SWARTZ_ERR;
#endif
}

int swartz_sem_post_1(swartz_sem_t* handle)
{
	return swartz_sem_post(handle, 1);
}

int swartz_sem_wait(swartz_sem_t* handle, int timeout)
{
#if defined(_WIN32) || defined(_WIN64)
	DWORD timed = static_cast<DWORD>(timeout);

	if ((!handle) || (handle->sem == NULL))
	{
		return SWARTZ_ERR;
	}
		
	if ((timeout <= -1) || (timeout > INFINITE))
	{
		timed = INFINITE;
	}

	if (WAIT_OBJECT_0 == WaitForSingleObject(handle->sem, timed))
	{
		return SWARTZ_OK;
	}

	return SWARTZ_ERR;
#else
	if ( !handle )
	{
		return SWARTZ_ERR;
	}

	if (timeout == -1)
	{
		return (sem_wait(&(handle->sem)) == 0) ? SWARTZ_OK : SWARTZ_ERR;
	}

	if(timeout == 0)
	{
		int ret = 0;

		while(((ret = sem_trywait(&(handle->sem))) != 0) && (EINTR == errno));

		return (ret == 0) ? SWARTZ_OK : SWARTZ_ERR;
	}


	timeval tv;
	timespec ts;
	signed long long now;

	memset(&tv, 0, sizeof(tv));
	memset(&ts, 0, sizeof(ts));

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec + timeout/1000;

	now = tv.tv_usec;
	now *= 1000;
	now += (timeout%1000)*1000000;
	if (now > 999999999)
	{
		ts.tv_sec++;
	}
	ts.tv_nsec = now%(1000000000);

	int ret = 0;
	while( (ret = sem_timedwait(&(handle->sem), &ts)) != 0 && EINTR == errno);
	return ret;
#endif
}

int swartz_sem_destroy(swartz_sem_t* handle)
{
#if defined(_WIN32) || defined(_WIN64)
	int ret = SWARTZ_ERR;

	if ((!handle) || (handle->sem == NULL))
	{
		return SWARTZ_ERR;
	}

	ret = (CloseHandle(handle->sem)) ? SWARTZ_OK : SWARTZ_ERR;

	free(handle);

	return ret;

#else
	int ret = 0;
	if (!handle)
	{
		return SWARTZ_ERR;
	}

	ret = sem_destroy(&(handle->sem));

	free(handle);
	handle = NULL;

	return (ret == 0) ? SWARTZ_OK : SWARTZ_ERR;;
#endif
}


