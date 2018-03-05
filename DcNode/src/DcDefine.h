#ifndef __DC_DEFINE_H
#define __DC_DEFINE_H

#include "tinyxml2.h"
using namespace tinyxml2;

#include "DcLogDefine.h"

#include "swartz_singleton.h"
#include "swartz_init.h"
#include "swartz_thread.h"
#include "swartz_threadpool.h"
#include "swartz_types.h"


#include <unistd.h>

#include <iostream>
#include <string>
#include <map>
#include <list>

#define NODE_XML "dcnode.xml"

typedef struct NODE_INFO_S
{
	std::string NodeIp;
	int         NodePort;
	std::string ClusterIp;
	int         ClusterPort;

	NODE_INFO_S()
	{
		NodeIp = "127.0.0.1";
		NodePort = 5002;
		ClusterIp = "127.0.0.1";
		ClusterPort = 6001;
	}
	NODE_INFO_S& operator=(NODE_INFO_S& src)
	{
		NodeIp = src.NodeIp;
		NodePort = src.NodePort;
		ClusterIp = src.ClusterIp;
		ClusterPort = src.ClusterPort;
		return (*this);
	} 
}NODE_INFO;

#endif
