#include "NodeServer.h"
#include "dp.h"

NodeServer::NodeServer(void) :
m_hThread(NULL),
m_hThreadPool(NULL),
m_hRegisterThread(NULL),
m_bstop(SWARTZ_FALSE),
m_bregister_ok(SWARTZ_FALSE)
{
}

NodeServer::~NodeServer(void)
{

}

bool RequestHandler::ClusterWork(const std::string& msg) 
{
	DP::Instance()->send_message(msg);
	DC_INFO("ClusterWork %s",msg.c_str());
	return true;
}


int NodeServer::StartServer( NODE_INFO &info)
{
	m_nodeInfo = info;
	
	if (SWARTZ_OK != swartz_sem_create(&m_sem, 0))
    {
        DC_ERROR("swartz_sem_create error");
        return SWARTZ_ERR;
    }
	
	DP::Instance()->init(info.mqIP,info.mqPort,info.mqQueue);
	
	if (SWARTZ_OK != swartz_thread_create(&m_hThread, (void*)S_StartService, this, 0, 0))
	{
		StopServer();
		return SWARTZ_ERR;
	}
	if (SWARTZ_OK != swartz_thread_create(&m_hRegisterThread, (void*)S_SendHeartBeat, this, 0, 0))
	{
		StopServer();
		return SWARTZ_ERR;
	}
	//初始化线程池 100个线程
	if (SWARTZ_OK != swartz_threadpool_create(&m_hThreadPool, 300, 30000, S_WorkService, 0))
	{
		DC_ERROR("swartz_threadpool_create error");
		return SWARTZ_ERR;
	}
	return SWARTZ_OK;
}

void NodeServer::StopServer()
{

	m_bstop = SWARTZ_TRUE;
	//销毁线程池
	if (NULL != m_hThreadPool)
	{
		swartz_threadpool_destroy(m_hThreadPool);
		m_hThreadPool = NULL;
	}
	
	if (m_sem != NULL) 
    {
        swartz_sem_post_1(m_sem);
        swartz_sem_destroy(m_sem);
        m_sem = NULL;
    }

	//等待线程退出
	if (NULL != m_hThread)
	{
		swartz_thread_wait(m_hThread);
		m_hThread = NULL;
	}
	//等待心跳线程退出
	if (NULL != m_hRegisterThread)
	{
		swartz_thread_wait(m_hRegisterThread);
		m_hRegisterThread = NULL;
	}
	
	DP::Instance()->uninit();
}

void NodeServer::S_StartService(void* arg)
{
	NodeServer* server = (NodeServer*)arg;
	server->StartService();
}

void NodeServer::S_SendHeartBeat(void* arg)
{
	NodeServer* server = (NodeServer*)arg;
	server->SendHeartBeat();
}

swartz_bool NodeServer::RegisterClient(void)
{
	boost::shared_ptr<TSocket> socket(new TSocket(m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort));
	boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

	RegistClient client(protocol);
	RegistResult::type resType = RegistResult::UNDEFINE_ERR;
	try 
	{
		transport->open();
		ClientInfo info;
		info.Ip = m_nodeInfo.NodeIp;
		info.Port = m_nodeInfo.NodePort;
		resType = client.registClient(info);
		transport->close();
	}
	catch(...)
	{
		
	}
	return (resType == RegistResult::SUCCESS)?SWARTZ_TRUE:SWARTZ_FALSE;
}

swartz_bool NodeServer::HeartBeatWork(void)
{
	boost::shared_ptr<TSocket> socket(new TSocket(m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort));
	boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

	RegistClient client(protocol);
	bool bsuccess = false;
	try 
	{
		transport->open();

		HeartBeatInfo hbinfo ;
		hbinfo.Ip = m_nodeInfo.NodeIp;
		hbinfo.Port = m_nodeInfo.NodePort;
		bsuccess = client.heartbeat(hbinfo);
		transport->close();
	}
	catch(...)
	{
		
	}
	return (bsuccess == true)?SWARTZ_TRUE:SWARTZ_FALSE;

}

void NodeServer::StartService()
{
	boost::shared_ptr<RequestHandler> handler(new RequestHandler());
	boost::shared_ptr<TProcessor> processor(new RequestProcessor(handler));
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
	boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(15);
	boost::shared_ptr<PosixThreadFactory> threadFactory = boost::shared_ptr<PosixThreadFactory >(new PosixThreadFactory());
	threadManager->threadFactory(threadFactory);
	threadManager->start();
	TNonblockingServer server(processor, protocolFactory, m_nodeInfo.NodePort, threadManager);
	DC_INFO("start NodeServer success!");
	server.serve();
}

void NodeServer::SendHeartBeat()
{
	 for (;;)
    {
        if (SWARTZ_FALSE == m_bregister_ok)
		{
			if(RegisterClient()== SWARTZ_FALSE)
			{
				DC_ERROR("send register %s:%d failed", m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort);
			}
			else
			{
				m_bregister_ok = SWARTZ_TRUE;
				DC_INFO("send register %s:%d success", m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort);
			}
		}  
		  
		if (m_bregister_ok)
		{
			if(HeartBeatWork()== SWARTZ_FALSE)
			{
				m_bregister_ok = SWARTZ_FALSE;
				DC_ERROR("send heartbeat %s:%d failed", m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort);
			}
			else
			{
				DC_INFO("send heartbeat %s:%d success", m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort);
			}
		}
		swartz_sem_wait(m_sem, 30*1000);
        if (m_bstop)
        {
            break;
        }
    }
}

void* NodeServer::S_WorkService(void* arg)
{
	return NULL;
}

void NodeServer::WorkService(void* arg)
{
	
}



