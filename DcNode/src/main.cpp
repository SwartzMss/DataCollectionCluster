#include "localconfig.h"
#include "NodeServer.h"

int main(int argc,char**argv)
{
	if(SWARTZ_OK != LOCALCONFIG::Instance()->LoadXml(NODE_XML))
	{
		DC_ERROR("please check the %s ",NODE_XML);
		return SWARTZ_ERR;
	}
	DC_INFO("load  the %s success ",NODE_XML);
	NODEServer::Instance()->StartServer(LOCALCONFIG::Instance()->m_NodeInfo);
	getchar();
	return 0;
}
