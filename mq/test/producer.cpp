#include "libactivemq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

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
		
	iRet = CommMgr_AddProducer(uri.str());
    if (0 != iRet)
    {
        printf("Add producer fail.rui[%s] ret[%d].", uri.str().c_str(), iRet);
        return -1;
    }	
	
    iRet = CommMgr_SendMsg(uri.str(), "123456");
    if (0 != iRet)
    {
        printf("CommMgr_SendMsg fail.rui[%s],ret[%d].", uri.str().c_str(), iRet);
        return -1;
    }
	return 0;
}