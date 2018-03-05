#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#include "DcDefine.h"

using namespace std;

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/transport/TBufferTransports.h>

#include "Regist.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::DcCluster;

class NodeServer
{
public:
	NodeServer(void);
	~NodeServer(void);
	static void S_StartService(void* arg);
	void StartService();


	static  void* CALLBACK S_WorkService(void* arg);
	void WorkService(void* arg);

public:
	int StartServer( NODE_INFO& info);
	void StopServer();
	

private:
	NODE_INFO m_nodeInfo;
	swartz_thread_t* m_hThread;	
	swartz_thread_pool_t* m_hThreadPool;		     //线程池的句柄	
};

typedef singleton<NodeServer> NODEServer;

#endif
