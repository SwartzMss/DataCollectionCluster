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

class RegistHandler : virtual public RegistIf {
public:

	RegistHandler()
	{
		
	}

	RegistResult::type registClient(const ClientInfo& clientInfo) 
	{
		DC_INFO("Clinet register Ip = %s, port =%d ",clientInfo.Ip.c_str(),clientInfo.Port);
		return RegistResult::SUCCESS;
	}

	bool heartbeat(const HeartBeatInfo& heartBeatInfo) 
	{
		DC_INFO("Clinet heartbeat Ip = %s, port =%d ",heartBeatInfo.Ip.c_str(),heartBeatInfo.Port);
		return true;
	}
};



class DcServer
{
public:
	DcServer(void);
	~DcServer(void);
public:
	int StartServer();
	void StopServer();
};

typedef singleton<DcServer> DCServer;
#endif
