#ifndef _DP_H
#define _DP_H

#include "DcDefine.h"

#include "libactivemq.h"

class dp
{
public:
	dp(void){}
	~dp(void){}
public:	
	int init(const std::string& ip,const long port,const std::string& quename );
	void uninit();
	int send_message(const std::string& msg);
private:
	std::string m_uri;
};

typedef singleton<dp> DP;

#endif
