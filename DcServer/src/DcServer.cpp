#include "DcServer.h"
#include "DcLogDefine.h"

#include <string>

HttpServer::HttpServer(void) :
m_hThread(NULL),
m_evhttp(NULL),
m_pBase(NULL),
m_hThreadPool(NULL),
m_nport(5001)
{
}

HttpServer::~HttpServer(void)
{

}

void HttpServer::HttpCallback(struct evhttp_request* request, void* arg)
{
	HttpServer* server = (HttpServer*)arg;
	http_task_t* task = new http_task_t;
	task->usrdata = arg;
	task->request = request;
	if (SWARTZ_OK != swartz_threadpool_work(server->m_hThreadPool, task))
	{
		DC_ERROR("hpr2_threadpool_work error");
	}
}

int HttpServer::StartServer(int port)
{
	m_nport = port;
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
	DC_INFO("start DcServer success!");
	return SWARTZ_OK;
}

void HttpServer::StopServer()
{
	if (m_evhttp != NULL)
	{
		evhttp_free(m_evhttp);
		m_evhttp = NULL;
	}

	if (m_pBase != NULL)
	{
		event_base_free(m_pBase);
		m_pBase = NULL;
	}
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

void HttpServer::S_StartService(void* arg)
{
	HttpServer* server = (HttpServer*)arg;
	server->StartService();
}

void HttpServer::StartService()
{
	int nret = -1;
	nret = evthread_use_pthreads();
	if (nret != SWARTZ_OK)
	{
		StopServer();
	}

	m_pBase = event_base_new();

	if (m_pBase == NULL)
	{
		StopServer();
	}
	m_evhttp = evhttp_new(m_pBase);

	if (m_evhttp == NULL)
	{
		StopServer();
	}
	nret = evhttp_bind_socket(m_evhttp, "0.0.0.0", m_nport);
	if (nret != SWARTZ_OK)
	{
		StopServer();
	}

	evhttp_set_gencb(m_evhttp, HttpCallback, this);

	nret = event_base_dispatch(m_pBase);

	if (nret != SWARTZ_OK)
	{
		StopServer();
	}
}

void* HttpServer::S_WorkService(void* arg)
{
	http_task_t* task = (http_task_t*)arg;
	HttpServer* server = (HttpServer*)task->usrdata;
	server->WorkService(arg);
	return NULL;
}

void HttpServer::WorkService(void* arg)
{
	http_task_t* task = (http_task_t*)arg;
	const char* uri = (char*)evhttp_request_get_uri(task->request);
	static int num = 0;
	printf("accept request url:%s ,num= %d\n", uri,num++);
	DC_INFO("accept request url:%s ,num= %d", uri,num++);

	//增加http的头信息 ,默认json格式 .utf8编码格式
	evhttp_add_header(evhttp_request_get_output_headers(task->request), "Content-Type", "application/json; charset=UTF-8");

	SendReply(task, "Just Support The POST Request");

#if 0
	//目前只支持post请求
	if (EVHTTP_REQ_POST != evhttp_request_get_command(task->request))
	{
		SendReply(task, "Just Support The POST Request", DAG_URL_ERR);
		return;
	}
#endif
}

void HttpServer::SendReply(http_task_t* task, string strMsg, int MsgType /*= 1*/)
{
	struct evbuffer* evbuf = evbuffer_new();
	if (!evbuf)
	{
		DC_ERROR("create evbuffer failed!");
		return;
	}

	StringBuffer s;
	Writer<StringBuffer> writer(s);

	writer.StartObject();
	writer.Key("code");
	writer.String(std::to_string(MsgType).c_str());
	writer.Key("msg");
	writer.String(strMsg.c_str());
	writer.EndObject();

	std::string strResult = s.GetString();

	evbuffer_add_printf(evbuf, strResult.c_str());
	evhttp_send_reply(task->request, HTTP_OK, "OK", evbuf);
	evbuffer_free(evbuf);
	delete(task);
	task = NULL;
}

