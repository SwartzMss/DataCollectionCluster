#include "swartz_atomic.h"



#if (defined(_WIN32) || defined(_WIN64))
#include <Windows.h>


int swartz_atomic_inc(volatile int *mem)
{
	return InterlockedIncrement((volatile LONG*)mem) - 1;
}

int swartz_atomic_dec(volatile int *mem)
{
	return InterlockedDecrement((volatile LONG*)mem);
}


#else
void HPR_AtomicDec_(volatile int* pVal)
{
	unsigned char prev;
	asm volatile ("lock; decl %0; setnz %1"
		: "=m" (*pVal), "=qm" (prev)
		: "m" (*pVal)
		: "memory");
}

int HPR_AtomicAdd_(volatile int* pDst, int nVal)
{
	asm volatile ("lock; xaddl %0,%1"
		: "=r" (nVal), "=m" (*pDst)
		: "0" (nVal), "m" (*pDst)
		: "memory", "cc");
	return nVal;

}
int swartz_atomic_inc(volatile int *mem)
{
	return  HPR_AtomicAdd_(mem, 1);
}
int swartz_atomic_dec(volatile int *mem)
{       
	unsigned char prev;
	asm volatile ("lock; decl %0; setnz %1"
		: "=m" (*mem), "=qm" (prev)
		: "m" (*mem)
		: "memory");
}

#endif

