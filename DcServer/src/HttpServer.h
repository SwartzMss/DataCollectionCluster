#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#include "DcDefine.h"

using namespace std;

class HttpServer
{
public:
	HttpServer(void);
	~HttpServer(void);
	static void HttpCallback(struct evhttp_request* request, void* arg);
	static void S_StartService(void* arg);
	void StartService();

	//回应报错消息格式
	void SendReply(http_task_t* task, string strMsg, DC_HTTP_REPLY  MsgType = DC_NO_ERR);

	static  void* CALLBACK S_WorkService(void* arg);
	void WorkService(void* arg);

public:
	int StartServer(int port);
	void StopServer();


private:
	int            m_nport;					//端口号
	swartz_thread_t* m_hThread;				//发送心跳的单独线程
	struct evhttp* m_evhttp;
	struct event_base *m_pBase;
	swartz_thread_pool_t* m_hThreadPool;		     //线程池的句柄
};

typedef singleton<HttpServer> HTTPServer;

#endif
