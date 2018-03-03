#include "swartz_mutex.h"
#include "swartz_thread.h"
#include "swartz_sem.h"
#include "swartz_atomic.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#endif

#if !defined(_WIN32_WCE)
#include <sys/timeb.h>
#endif

/**
* Function:	gettimeofday
* Desc:
* Input:
* Output:	current time in seconds and microseconds of the day
* Return:	void
* */
void gettimeofday(struct timeval* tms)
{
#if !defined(_WIN32_WCE)
#if defined(_WIN32) || defined(_WIN64)
	struct _timeb tb;
#if (_MSC_VER >= 1500)
	_ftime_s(&tb);
#else
	_ftime(&tb);
#endif
#elif defined __linux__
	struct timeb tb;
	ftime(&tb);
#endif

	tms->tv_sec = (long)tb.time;
	tms->tv_usec = tb.millitm * 1000;
#else
	//2011-07-13
	SYSTEMTIME st = {0};
	GetSystemTime(&st);
	FILETIME ft = {0};
	SystemTimeToFileTime(&st, &ft);

	__int64 tmp = (__int64(ft.dwHighDateTime) << 32) + ft.dwLowDateTime;

	tms->tv_sec = (long)((__int64)(tmp - 116444736000000000) / 10000000);
	tms->tv_usec = st.wMilliseconds * 1000;
#endif
}


#include "swartz_thrmgr.h"

typedef struct work_item_tag {
	struct work_item_tag *next;
	void *data;
	struct timeval time_queued;
} work_item_t;

typedef struct work_queue_tag {
	work_item_t *head;
	work_item_t *tail;
	int item_count;
} work_queue_t;

typedef enum {
	POOL_VALID,
	POOL_EXIT
} pool_state_t;

typedef struct threadpool_tag {
	int thr_stacksize;

	pool_state_t state;
	int thr_max;
	int thr_alive;
	int thr_idle;
	int idle_timeout;

	void (CALLBACK *handler)(void *);

	struct swartz_sem_t* pool_sem;	//2011-12-14 增加信号量用于通知最后一个线程退出
	struct swartz_sem_t* pool_sig_sem;	//2011-12-15 改条件变量通知为信号量通知
	struct swartz_mutex_t* pool_mutex;	//2011-12-15 锁仅为保护任务队列操作
	
	work_queue_t *queue;
} threadpool_t;


//////////////////////////////////////////////////////////////////////////

static work_queue_t *work_queue_new(void)
{
	work_queue_t *work_q = (work_queue_t *)malloc(sizeof(work_queue_t));
	if (NULL == work_q) 
	{
		return NULL;
	}
	
	memset(work_q, 0, sizeof(*work_q));
	return work_q;
}

static swartz_bool work_queue_add(work_queue_t *work_q, void *data)
{
	if (NULL == work_q) 
	{
		return SWARTZ_FALSE;
	}
	
	work_item_t *work_item = (work_item_t *)malloc(sizeof(work_item_t));
	if (NULL == work_item) 
	{
		return SWARTZ_FALSE;
	}
	
	work_item->next = NULL;
	work_item->data = data;
	gettimeofday(&(work_item->time_queued));
	
	if (work_q->head == NULL) 
	{
		work_q->head = work_q->tail = work_item;
		work_q->item_count = 1;
	} 
	else 
	{
		work_q->tail->next = work_item;
		work_q->tail = work_item;
		work_q->item_count += 1;
	}
	return SWARTZ_TRUE;
}

static swartz_bool work_queue_pop(work_queue_t *work_q, void** pparam)
{
	if (NULL == work_q || NULL == work_q->head) 
	{
		return SWARTZ_FALSE;
	}

	work_item_t *work_item = work_q->head;
	//2011-04-13 将用户数据改为传出参数，不再作为返回值，避免判断混淆
	*pparam = work_item->data;
	work_q->head = work_item->next;
	//2011-12-12 补充减计数操作
	work_q->item_count -= 1;
	if (work_q->head == NULL) 
	{
		work_q->tail = NULL;
	}
	free(work_item);

	return SWARTZ_TRUE;
}

static void work_queue_free(work_queue_t *work_q)
{
	//2011-12-12 增加释放队列函数，用于内存的释放操作
	if (NULL == work_q)
	{
		return;
	}

	void* tmp = NULL;
	while (work_q->item_count != 0)
	{
		work_queue_pop(work_q, &tmp);
	}

	free(work_q);
}

//////////////////////////////////////////////////////////////////////////

void thrmgr_destroy(void *p)
{
	threadpool_t *threadpool = (threadpool_t *)(p);

	if (threadpool == NULL || (threadpool->state != POOL_VALID)) 
	{
		return;
	}
  	
	threadpool->state = POOL_EXIT;

	int existed = 0;
	//2012-01-05 增加此标记用于判断是否需要等待最后的信号，否则无任务时会出现堵死的问题
	swartz_bool bNeedWait = SWARTZ_FALSE;
	//这里要锁定，避免destroy的时候在dispatch，多了一个线程少发一个信号造成堵塞
	swartz_mutex_lock(threadpool->pool_mutex);
	//2011-12-15 销毁时取当前存活线程数，发送对应数量的信号通知各线程退出
	if ((existed = threadpool->thr_alive) > 0) 
	{
		bNeedWait = SWARTZ_TRUE;
		while (existed--)
		{
			swartz_sem_post_1(threadpool->pool_sig_sem);
		}
	}
	swartz_mutex_unlock(threadpool->pool_mutex);

	//2011-12-15 单在此等待最后一个线程退出
	if (bNeedWait)
	{
		swartz_sem_wait(threadpool->pool_sem, SWARTZ_INFINITE);
	}

	swartz_sem_destroy(threadpool->pool_sem);
	swartz_sem_destroy(threadpool->pool_sig_sem);
	swartz_mutex_destroy(threadpool->pool_mutex);

	work_queue_free(threadpool->queue);
	free(threadpool);
	return;
}

void *thrmgr_new(int max_threads, int idle_timeout, void (CALLBACK *handler)(void *), int stack_size)
{
	if (stack_size < 0 || max_threads <= 0 || handler == NULL)
	{
		return NULL;
	}
	
	threadpool_t *threadpool = (threadpool_t *)malloc(sizeof(threadpool_t));
	if (threadpool == NULL) 
	{
		return NULL;
	}

	threadpool->queue = work_queue_new();
	if (threadpool->queue == NULL) 
	{
		free(threadpool);
		return NULL;
	}	

	swartz_mutex_create(&(threadpool->pool_mutex), SWARTZ_MUTEX_ATTR_FAST);
	swartz_sem_create(&(threadpool->pool_sem), 0);
	swartz_sem_create(&(threadpool->pool_sig_sem), 0);

	threadpool->thr_max = max_threads;
	threadpool->thr_alive = 0;
	threadpool->thr_idle = 0;
	threadpool->idle_timeout = idle_timeout;
	threadpool->handler = handler;
	
	//置0为默认1MB
	if (stack_size == 0)
	{
		threadpool->thr_stacksize = 1024 * 1024;
	}
	//非默认线程栈大小需要修改线程属性
#if defined(_WIN32) || defined(_WIN64)
	else if (stack_size < 64 * 1024) 
	{
		threadpool->thr_stacksize = 64 * 1024;
	}
#elif defined __linux__
	else if (stack_size < 256 * 1024) 
	{
		threadpool->thr_stacksize = 256 * 1024;
	}
#endif
	else
	{
		threadpool->thr_stacksize = stack_size;
	}

	threadpool->state = POOL_VALID;

	return threadpool;
}

static void * CALLBACK thrmgr_worker(void *arg)
{
	threadpool_t *threadpool = (threadpool_t *)arg;
	void *job_data = NULL;
	int retval = SWARTZ_OK;
	swartz_bool get_data = SWARTZ_FALSE;
	//2011-12-14 加这个标记是要将对计数的操作和判断都保护为串行，避免出现并行判断导致错误
	swartz_bool bSingal = SWARTZ_FALSE;
	
	while (threadpool->state != POOL_EXIT)
	{
		swartz_atomic_inc(&(threadpool->thr_idle));
			
		retval = swartz_sem_wait(threadpool->pool_sig_sem, threadpool->idle_timeout);

		//2013-4-8 空闲线程计数减1应该在信号量等待结束之后立即减去，后续无论工作还是退出都表示此线程不空闲
		//此前在下面的if判断之后才减计数，会造成线程空闲退出时没有减空闲计数造成dispatch在判断是否需要新建线程时永远判断不需要创建
		//使得线程池一阵繁忙之后出现空闲多数线程退出后，线程池无法创建新的线程，只依赖仅存的一个线程工作
		swartz_atomic_dec(&(threadpool->thr_idle));
		
		//只剩下一个线程的时候不直接退出
		//2011-12-14 最后一个线程退出时改为信号量通知，因此正常情况下应保持至少一个线程运作
		//否则会导致释放信号发生，造成销毁时出错
        //2013-06-13 取消保留最后一个线程，会出现最后两个线程同时判断break退出触发最后通知信号量的问题导致destroy时误收信号量
		if (retval == SWARTZ_ERR || threadpool->state == POOL_EXIT)
        {
            break;
        }
		
		//2011-04-13 不能使用用户数据作为是否取到数据的判断依据，因为有用户数据传任何值都有可能
		//因此，必须另设标记来判断
		swartz_mutex_lock(threadpool->pool_mutex);
		get_data = work_queue_pop(threadpool->queue, &job_data);
		swartz_mutex_unlock(threadpool->pool_mutex);
		
		if (get_data) 
		{
			threadpool->handler(job_data);
			get_data = SWARTZ_FALSE;
		}
	}

	swartz_mutex_lock(threadpool->pool_mutex);
	-- threadpool->thr_alive;
	//2011-12-15 减计数，判断为0并置标记一定要在互斥保护中进行
    //2013-06-13 最后一个线程在线程池销毁时触发信号量
	if (threadpool->thr_alive == 0 && threadpool->state == POOL_EXIT)
	{
		bSingal = SWARTZ_TRUE;
	}
	swartz_mutex_unlock(threadpool->pool_mutex);

	//2011-12-15 只在最后一个线程退出时发出此信号
	if (bSingal) 
	{
		swartz_sem_post(threadpool->pool_sem, 1);
	}
	
	return NULL;
}

swartz_bool thrmgr_dispatch(void* p, void *user_data)
{
	threadpool_t *threadpool = (threadpool_t *)(p);
	if (threadpool == NULL) 
	{
		return SWARTZ_FALSE;
	}

	/* Lock the threadpool */
	swartz_mutex_lock(threadpool->pool_mutex);

	if (threadpool->state != POOL_VALID) 
	{
		swartz_mutex_unlock(threadpool->pool_mutex);
		return SWARTZ_FALSE;
	}

	if (!work_queue_add(threadpool->queue, user_data)) 
	{
		swartz_mutex_unlock(threadpool->pool_mutex);
		return SWARTZ_FALSE;
	}

	if ((threadpool->thr_idle == 0) && (threadpool->thr_alive < threadpool->thr_max)) 
	{
		//int swartz_thread_detached_create(void*fn, void* arg, int stack_size, int priority);
		if (SWARTZ_OK != swartz_thread_detached_create((void*)thrmgr_worker, (void*)threadpool, threadpool->thr_stacksize, 0))
		//if (!HPR_ThreadDetached_Create((void* (CALLBACK*)(void*))thrmgr_worker, threadpool, (unsigned)threadpool->thr_stacksize)) 
		{
			//printf("!pthread_create failed\n");
			//2011-12-15 第一个线程就创建失败就返回失败，其他时候至少还有一个线程可用
			if (threadpool->thr_alive == 0)
			{
				swartz_mutex_unlock(threadpool->pool_mutex);
				return SWARTZ_FALSE;
			}
		}
		else 
		{
			//多一个线程要增加一个存活计数
			++ threadpool->thr_alive;
		}
	}
	swartz_mutex_unlock(threadpool->pool_mutex);

	//2011-12-15 每有一个任务来即给出一个信号
	swartz_sem_post(threadpool->pool_sig_sem, 1);
	return SWARTZ_TRUE;
}

