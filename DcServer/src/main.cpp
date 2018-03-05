#include <stdio.h>
#include "DcDefine.h"
#include "DcServer.h"


int main(int argc,char**argv)
{
	DCServer::Instance()->StartServer();
	getchar();
	return	0;
}
