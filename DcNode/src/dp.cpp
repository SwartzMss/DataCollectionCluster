#include "dp.h"

int dp::init(const std::string& ip,const long port,const std::string& quename )
{
	int iRet = CommMgr_Init();
	if (0 != iRet)
    {
        DC_ERROR("CommMgr_Init fail. ret[%d].", iRet);
        return SWARTZ_ERR;
    }	
	DC_INFO("CommMgr_Init success");
	
	std::stringstream uri;
    uri << "mq/" << ip << ":" << port
        << "/" << "queue/" << quename;
		
	m_uri = uri.str();	
	iRet = CommMgr_AddProducer(uri.str());
    if (0 != iRet)
    {
        DC_ERROR("Add producer fail.rui[%s] ret[%d].", uri.str().c_str(), iRet);
        return SWARTZ_ERR;
    }	
	return SWARTZ_OK;
}

int dp::send_message(const std::string& msg)
{
	int iRet = CommMgr_SendMsg(m_uri, msg.c_str());
    if (0 != iRet)
    {
        DC_ERROR("CommMgr_SendMsg fail.rui[%s],ret[%d].", m_uri.c_str(), iRet);
        return SWARTZ_ERR;
    }
	return SWARTZ_OK;
}

void dp::uninit()
{
	CommMgr_Fini();
}
