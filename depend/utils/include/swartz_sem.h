#ifndef __SWARTZ_SEM_H__
#define __SWARTZ_SEM_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct swartz_sem_t swartz_sem_t;

int swartz_sem_create(swartz_sem_t** handle, int count);

int swartz_sem_post(swartz_sem_t* handle, int count);

int swartz_sem_post_1(swartz_sem_t* handle);

int swartz_sem_wait(swartz_sem_t* handle, int timeout);

int swartz_sem_destroy(swartz_sem_t* handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

