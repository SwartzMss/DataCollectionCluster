#ifndef _DC_SERVER_H
#define _DC_SERVER_H

#include "DcDefine.h"
#include "HttpServer.h"

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


class RegistHandler : virtual public RegistIf 
{
public:
	RegistHandler(){}

	RegistResult::type registClient(const ClientInfo& clientInfo); 
	
	bool heartbeat(const HeartBeatInfo& heartBeatInfo); 

};


class DcServer
{
public:
	DcServer(void);
	~DcServer(void);
public:
	int StartServer();
	void StopServer();
	
	void CollectWorkProc(http_task_t* taskinfo);
	RegistResult::type RegisterWork(const ClientInfo& clientInfo);
	void HeartBeatWork(const HeartBeatInfo& heartBeatInfo);
	
private:
	static void S_StartService(void* arg);
	void StartService();
	
private:
	std::list<dcnode_t> m_node_list;
	swartz_thread_t* m_hThread;				
	swartz_sem_t* m_sem; 
	swartz_bool m_bstop;	

	CMutex m_Manmutex; // 多线程处理任务数的时候需要加锁

};

typedef singleton<DcServer> DCServer;
#endif
