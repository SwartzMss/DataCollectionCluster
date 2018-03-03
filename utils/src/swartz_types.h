#ifndef __SWARTZ_TYPES_H__
#define __SWARTZ_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWARTZ_OK    (0)
#define SWARTZ_ERR   (-1)
#define SWARTZ_TRUE  (1)
#define SWARTZ_FALSE (0)
#define SWARTZ_INFINITE (-1)
#define SWARTZ_FOREVER SWARTZ_INFINITE
#define SWARTZ_TIMEDOUT (0)


#ifndef CALLBACK
#define CALLBACK __stdcall
#endif


/************************************************************************/
/** integer */
	typedef signed char		swartz_int8;
	typedef unsigned char	swartz_uint8;
	typedef signed short	swartz_int16;
	typedef unsigned short	swartz_uint16;
	typedef signed int		swartz_int32;
	typedef unsigned int	swartz_uint32;
#if defined(_MSC_VER)
#include <stddef.h>
	typedef signed __int64	swartz_int64;
	typedef unsigned __int64 swartz_uint64;
	typedef intptr_t         swartz_intptr;
	typedef intptr_t         swartz_uintptr;
#elif defined(__GNUC__) || defined(__SYMBIAN32__)
#include <ctype.h>
#include <stdint.h>
#if defined(__LP64__)
	typedef signed long swartz_int64;
	typedef unsigned long swartz_uint64;
#else
typedef signed long long swartz_int64;
	typedef unsigned long long swartz_uint64;
#endif	//defined(__LP64__)
    #include <ctype.h>
	typedef uintptr_t         swartz_uintptr;
	typedef intptr_t          swartz_intptr;
    #define CALLBACK
#endif/** #if defined(_MSC_VER) */
/** end integer */
/************************************************************************/

typedef bool			swartz_bool;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



