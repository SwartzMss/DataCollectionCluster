#include "DcServer.h"



RegistResult::type RegistHandler::registClient(const ClientInfo& clientInfo) 
{
	DC_INFO("Clinet register Ip = %s, port =%d ",clientInfo.Ip.c_str(),clientInfo.Port);
	
	return DCServer::Instance()->RegisterWork(clientInfo);
}

bool RegistHandler::heartbeat(const HeartBeatInfo& heartBeatInfo) 
{
	DC_INFO("Clinet heartbeat Ip = %s, port =%d ",heartBeatInfo.Ip.c_str(),heartBeatInfo.Port);
	
	DCServer::Instance()->HeartBeatWork(heartBeatInfo);
	return true;
}


RegistResult::type DcServer::RegisterWork(const ClientInfo& clientInfo)
{
	CGuard<CMutex> g(m_Manmutex);
	swartz_bool nNodeExist = SWARTZ_FALSE;
	for (std::list<dcnode_t>::iterator itor = m_node_list.begin();itor != m_node_list.end();itor++)
	{
		if(itor->ip == clientInfo.Ip && itor->port == clientInfo.Port )
		{
			itor->lHeartBeatTime = DCGetTickCount();
			nNodeExist = SWARTZ_TRUE;
		}
	}
	if(!nNodeExist)
	{
		dcnode_t node ;
		node.ip = clientInfo.Ip;
		node.port = clientInfo.Port;
		node.lHeartBeatTime = DCGetTickCount();
		DCServer::Instance()->m_node_list.push_back(node);
	}
	return RegistResult::SUCCESS;
}

void DcServer::HeartBeatWork(const HeartBeatInfo& heartBeatInfo)
{
	CGuard<CMutex> g(m_Manmutex);
	
	for (std::list<dcnode_t>::iterator itor = m_node_list.begin();itor != m_node_list.end();itor++)
	{
		if( itor->ip == heartBeatInfo.Ip && itor->port == heartBeatInfo.Port )
		{
			itor->lHeartBeatTime = DCGetTickCount();
			break;
		}
	}
}

DcServer::DcServer(void):
 m_hThread(NULL)
,m_sem(NULL)
,m_bstop(SWARTZ_FALSE)
{
}

DcServer::~DcServer(void)
{

}


int DcServer::StartServer()
{
    if (SWARTZ_OK != swartz_sem_create(&m_sem, 0))
    {
        DC_ERROR("swartz_sem_create error");
        return SWARTZ_ERR;
    }
	
	HTTPServer::Instance()->StartServer(5001);

	if (SWARTZ_OK != swartz_thread_create(&m_hThread, (void*)S_StartService, this, 0, 0))
	{
		StopServer();
		return SWARTZ_ERR;
	}
	boost::shared_ptr<RegistHandler> handler(new RegistHandler());
	boost::shared_ptr<TProcessor> processor(new RegistProcessor(handler));
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(15);
	boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory >(new PosixThreadFactory());
	threadManager->threadFactory(threadFactory);
	threadManager->start();
	TNonblockingServer server(processor, protocolFactory, 5002, threadManager);
	DC_INFO("start DcServer success!");
	server.serve();
	
	return SWARTZ_OK;
}


void DcServer::S_StartService(void* arg)
{
	DcServer* server = (DcServer*)arg;
	server->StartService();
}

void DcServer::StartService()
{
	 for (;;)
    {
		{
			long lCheckTime = DCGetTickCount();
			CGuard<CMutex> g(m_Manmutex);

			for (std::list<dcnode_t>::iterator itor = m_node_list.begin();itor != m_node_list.end();)
			{
				if(lCheckTime - itor->lHeartBeatTime > 60 )
				{
					itor = DCServer::Instance()->m_node_list.erase(itor);
					continue;
				}
				else
				{
					DC_INFO("current Clinet node Ip = %s, port =%d ",itor->ip.c_str(),itor->port);
					++itor;
				}
			}	
			DC_INFO("current Clinet node size = %d ",m_node_list.size());
		}
        swartz_sem_wait(m_sem, 45*1000);
        if (m_bstop)
        {
            break;
        }
    }
}

void DcServer::StopServer()
{
	DC_INFO("stop DcServer");
	m_bstop = SWARTZ_TRUE;
	
	if (m_sem != NULL) 
    {
        swartz_sem_post_1(m_sem);
        swartz_sem_destroy(m_sem);
        m_sem = NULL;
    }
	
	 if (NULL != m_hThread)
    {
        swartz_thread_wait(m_hThread);
        m_hThread = NULL;
    }
}

/*
{
	"indexCode":"131000000514",
}
*/
void DcServer::CollectWorkProc(http_task_t* taskinfo)
{
	struct evbuffer *req_evb = evhttp_request_get_input_buffer(taskinfo->request);;

	int req_len = evbuffer_get_length(req_evb);
	char * req_buf = new char[req_len +1]();
	memset(req_buf ,0 ,req_len);
	int ret = evbuffer_remove(req_evb, req_buf, req_len);
	if(ret != req_len)
	{
		DC_ERROR("Recv  Body Data failed!");
		HTTPServer::Instance()->SendReply(taskinfo, "Body Data Recv Error",DC_BODY_ERR);
		delete(req_buf);
		return;
	}
	
	const char* uri = (char*)evhttp_request_get_uri(taskinfo->request);
	const char* host = evhttp_request_get_host(taskinfo->request);
	DC_INFO("accept request host:%s, url:%s ,body:%s", host,uri,req_buf);
	
	Document document;  
	if (document.Parse(req_buf).HasParseError())
	{
		DC_ERROR("parse URL  Body err,info = %s",req_buf);
		HTTPServer::Instance()->SendReply(taskinfo, "Body Data Error",DC_BODY_ERR);
		delete(req_buf);
		return ;
	}

	const char* szIndexCode =  document["indexCode"].GetString();
	HTTPServer::Instance()->SendReply(taskinfo, "200 OK",DC_BODY_ERR);
	delete(req_buf);
}


