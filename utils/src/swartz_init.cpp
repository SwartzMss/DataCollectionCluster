#include "swartz_types.h"
#include "swartz_init.h"

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib ") 
#else
#include <signal.h>
#endif


static int s_init_count = 0;

int swartz_init()
{
	s_init_count++;
	if (s_init_count > 0)
	{
#if defined(_WIN32) || defined(_WIN64)
		WSADATA wsaData;
		if (0 != WSAStartup(MAKEWORD(2,0), &wsaData))
		{
			s_init_count--;
			return SWARTZ_ERR;
		}
#else
		signal(SIGPIPE, SIG_IGN);
		return SWARTZ_OK;
#endif
	}

	return SWARTZ_OK;

}

void swartz_fini()
{
	s_init_count--;
	if (s_init_count == 0)
	{
#if defined(_WIN32) || defined(_WIN64)
		WSACleanup();
#endif
	}
}




