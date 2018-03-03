#ifndef __SWARTZ_ATOMIC_H__
#define __SWARTZ_ATOMIC_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int swartz_atomic_inc(volatile int *mem);

int swartz_atomic_dec(volatile int *mem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

