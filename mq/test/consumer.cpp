#include "libactivemq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <time.h>

 void xMqMsgCb(const char* pMsg, unsigned uMsgLen,void* pUser)
{
	printf("msg = %s",pMsg);
}

int main()
{
	int iRet = CommMgr_Init();
	if (0 != iRet)
    {
        printf("CommMgr_Init fail. ret[%d].", iRet);
        return -1;
    }
	printf("CommMgr_Init success.");
	
	std::stringstream uri;
    uri << "mq/" << "10.20.139.19" << ":" << 61616
        << "/" << "queue/" << "test";///< 格式形如mq/127.0.0.1:61618/queue/acs.event.queue
		
	iRet =CommMgr_AddConsumer(uri.str(),xMqMsgCb,NULL);
	if (0 != iRet)
    {
        printf("CommMgr_AddConsumer fail. ret[%d].", iRet);
        return -1;
    }
	
	sleep(3);
	return 0;
}