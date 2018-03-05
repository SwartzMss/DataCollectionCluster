#include "DcServer.h"


DcServer::DcServer(void) 
{
}

DcServer::~DcServer(void)
{

}


int DcServer::StartServer()
{
	HTTPServer::Instance()->StartServer(5001);
	
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

void DcServer::StopServer()
{
	
}


