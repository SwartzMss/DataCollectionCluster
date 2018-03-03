#include "DcServer.h"
#include <stdio.h>

typedef singleton<HttpServer> g_HttpServer;

int main(int argc,char**argv)
{
	g_HttpServer::Instance()->StartServer(5001);
	getchar();
	return	0;
}
