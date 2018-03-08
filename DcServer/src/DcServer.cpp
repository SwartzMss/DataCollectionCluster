#include "DcServer.h"



RegistResult::type RegistHandler::registClient(const ClientInfo& clientInfo) 
{
	DC_INFO("Clinet register Ip = %s, port =%d ",clientInfo.Ip.c_str(),clientInfo.Port);
	
	return DCServer::Instance()->RegisterWork(clientInfo);
}

bool RegistHandler::heartbeat(const HeartBeatInfo& heartBeatInfo) 
{
	DC_INFO("Clinet heartbeat Ip = %s, port =%d ",heartBeatInfo.Ip.c_str(),heartBeatInfo.Port);
	
	return DCServer::Instance()->HeartBeatWork(heartBeatInfo);//必须按照先注册后心跳的顺序,因为注册的时候可以进行一些初始化动作
}


RegistResult::type DcServer::RegisterWork(const ClientInfo& clientInfo)
{
	CGuard<CMutex> g(m_Manmutex);
	swartz_bool nNodeExist = SWARTZ_FALSE;
	for (std::vector<dcnode_t>::iterator itor = m_node_vec.begin();itor != m_node_vec.end();itor++)
	{
		if(itor->ip == clientInfo.Ip && itor->port == clientInfo.Port )
		{
			itor->lHeartBeatTime = DCGetTickCount();
			nNodeExist = SWARTZ_TRUE;
			break;
		}
	}
	if(!nNodeExist)
	{
		dcnode_t node ;
		node.ip = clientInfo.Ip;
		node.port = clientInfo.Port;
		node.lHeartBeatTime = DCGetTickCount();
		DCServer::Instance()->m_node_vec.push_back(node);
	}
	/*这里不区分是崩溃重启之后注册还是第一次注册了,应用层可以在这里做对节点的初始化操作*/
	return RegistResult::SUCCESS;
}

bool DcServer::HeartBeatWork(const HeartBeatInfo& heartBeatInfo)
{
	bool bret = false;
	CGuard<CMutex> g(m_Manmutex);
	for (std::vector<dcnode_t>::iterator itor = m_node_vec.begin();itor != m_node_vec.end();itor++)
	{
		if( itor->ip == heartBeatInfo.Ip && itor->port == heartBeatInfo.Port )
		{
			itor->lHeartBeatTime = DCGetTickCount();
			bret = true;
			break;
		}
	}
	return bret;
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

			for (std::vector<dcnode_t>::iterator itor = m_node_vec.begin();itor != m_node_vec.end();)
			{
				if(lCheckTime - itor->lHeartBeatTime > 60*1000 )
				{
					itor = DCServer::Instance()->m_node_vec.erase(itor);
					continue;
				}
				else
				{
					DC_INFO("current Clinet node Ip = %s, port =%d ",itor->ip.c_str(),itor->port);
					++itor;
				}
			}	
			DC_INFO("current Clinet node size = %d ",m_node_vec.size());
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

bool DcServer::ClusterWorkProc(const std::string& ip , const int& port ,const std::string& msg) 
{	
	boost::shared_ptr<TSocket> socket(new TSocket(ip.c_str(), port));
	boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

	RequestClient client(protocol);
	bool bsuccess = true;
	try 
	{
		transport->open();
		bsuccess = client.ClusterWork(msg);
		transport->close();
	}
	catch(...)
	{
		bsuccess = false;
	}
	return bsuccess;
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
	if(szIndexCode == NULL)
	{
		HTTPServer::Instance()->SendReply(taskinfo, "Body Data Error",DC_BODY_ERR);
	}
	
	static int pickNum = 0;
	dcnode_t node ;
	int node_size = 0 ;
	{
		//平均调度
		CGuard<CMutex> g(m_Manmutex);
		node_size = m_node_vec.size();
		if(node_size > 0)
		{
			pickNum = pickNum%node_size;
			node = m_node_vec[pickNum++];
		}
	}

	if( node_size > 0) 
	{
		if( ClusterWorkProc(node.ip,node.port,szIndexCode))
		{

			HTTPServer::Instance()->SendReply(taskinfo, "200 OK",DC_NO_ERR);
		}
		else
		{
			{
				CGuard<CMutex> g(m_Manmutex);
				for (std::vector<dcnode_t>::iterator itor = m_node_vec.begin();itor != m_node_vec.end();itor++)
				{
					if( itor->ip == node.ip&& itor->port == node.port )
					{
						m_node_vec.erase(itor);
						break;
					}
				}
			}
			HTTPServer::Instance()->SendReply(taskinfo, "DCNODE ERROR",DC_NODE_ERR);
		}
	}
	else
	{
		HTTPServer::Instance()->SendReply(taskinfo, "NO AVAILABLE Node",DC_NO_NODE);
	}

	delete(req_buf);
}



