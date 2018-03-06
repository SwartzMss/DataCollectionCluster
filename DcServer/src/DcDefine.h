#ifndef _DC_DEFINE_H
#define _DC_DEFINE_H

#include "DcLogDefine.h"

#include "swartz_singleton.h"
#include "swartz_init.h"
#include "swartz_thread.h"
#include "swartz_threadpool.h"
#include "swartz_types.h"
#include "swartz_sem.h"
#include "swartz_mutexEx.h"

#include <unistd.h>
#include <time.h>  
#include <sys/types.h>

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

#include <list>
#include <iostream>
#include <string>
#include <sstream>

using namespace rapidjson;

#define DC_SERVICE   "/DcCluster/api/v1.0/Service"

typedef enum
{     
	DC_NO_ERR  = 0,
	DC_URL_ERR = 1,
	DC_BODY_ERR = 2,
	DC_NO_NODE = 3,
}DC_HTTP_REPLY;

typedef struct http_task_t
{
	 struct evhttp_request* request;
	 void* usrdata;
}http_task_t;

typedef struct dcnode_t
{
	std::string ip;
	int port ;
	long lHeartBeatTime;
}dcnode_t;

static long DCGetTickCount()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#endif
