#ifndef _LOCAL_CONFIG_H
#define _LOCAL_CONFIG_H

#include "DcDefine.h"


class LocalConfig
{

public:
	int LoadXml(const char* xml);

public:
	NODE_INFO m_NodeInfo;	
};

typedef singleton<LocalConfig> LOCALCONFIG;

#endif
