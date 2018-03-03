#ifndef _DC_SERVER_H
#define _DC_SERVER_H

#include "swartz_singleton.h"
#include "swartz_thread.h"
#include "swartz_threadpool.h"
#include "swartz_types.h"

#include <string>
#include "writer.h"
#include "reader.h"
#include "document.h" 
#include "stringbuffer.h"

#include "evhttp.h"

#include <event2/bufferevent.h>
#include <event2/bufferevent_compat.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer_compat.h>
#include <event2/http_struct.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>


using namespace rapidjson;
using namespace std;

#define READ_TIMEOUT   5  //超时时间为5S
#define WRITE_TIMEOUT  5



typedef struct http_task_t
{
	struct evhttp_request* request;
	void* usrdata;
}http_task_t;

class HttpServer
{
public:
	HttpServer(void);
	~HttpServer(void);
	static void HttpCallback(struct evhttp_request* request, void* arg);
	static void S_StartService(void* arg);
	void StartService();

	//回应报错消息格式
	void SendReply(http_task_t* task, string strMsg, int MsgType = 1);

	static  void* CALLBACK S_WorkService(void* arg);
	void WorkService(void* arg);

public:
	int StartServer(int port);
	void StopServer();
public:
	swartz_thread_pool_t* m_hThreadPool;		     //线程池的句柄

private:
	int            m_nport;					//端口号
	swartz_thread_t* m_hThread;				//发送心跳的单独线程
	struct evhttp* m_evhttp;
	struct event_base *m_pBase;
};

#endif
