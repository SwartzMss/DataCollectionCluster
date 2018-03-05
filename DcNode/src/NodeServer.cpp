#include "NodeServer.h"


NodeServer::NodeServer(void) :
m_hThread(NULL),
m_hThreadPool(NULL)
{
}

NodeServer::~NodeServer(void)
{

}



int NodeServer::StartServer( NODE_INFO &info)
{
	m_nodeInfo = info;
	if (SWARTZ_OK != swartz_thread_create(&m_hThread, (void*)S_StartService, this, 0, 0))
	{
		StopServer();
		return SWARTZ_ERR;
	}
	//初始化线程池 100个线程
	if (SWARTZ_OK != swartz_threadpool_create(&m_hThreadPool, 300, 30000, S_WorkService, 0))
	{
		DC_ERROR("hpr2_threadpool_create error");
		return SWARTZ_ERR;
	}
	DC_INFO("start NodeServer success!");
	return SWARTZ_OK;
}

void NodeServer::StopServer()
{
	//销毁线程池
	if (NULL != m_hThreadPool)
	{
		swartz_threadpool_destroy(m_hThreadPool);
		m_hThreadPool = NULL;
	}

	//等待线程退出
	if (NULL != m_hThread)
	{
		swartz_thread_wait(m_hThread);
		m_hThread = NULL;
	}
}

void NodeServer::S_StartService(void* arg)
{
	NodeServer* server = (NodeServer*)arg;
	server->StartService();
}

void NodeServer::StartService()
{
	boost::shared_ptr<TSocket> socket(new TSocket(m_nodeInfo.ClusterIp.c_str(), m_nodeInfo.ClusterPort));
	boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

	RegistClient client(protocol);

	transport->open();
	ClientInfo info;
	info.Ip = m_nodeInfo.NodeIp;
	info.Port = m_nodeInfo.NodePort;
	RegistResult::type resType= client.registClient(info);
	printf("resType = %d\n", resType);
	transport->close();
}

void* NodeServer::S_WorkService(void* arg)
{
	return NULL;
}

void NodeServer::WorkService(void* arg)
{
	
}



