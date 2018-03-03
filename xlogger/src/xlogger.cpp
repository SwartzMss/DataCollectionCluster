#include "xlogger.h"

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

#include <string>
using namespace  std;

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


static int g_log_init = 0;

#define bzero(p, size)     (void)memset((p), 0, (size))

#if _MSC_VER
#define snprintf _snprintf
#endif



#define SAFE_DELETE_ARRAY(v_para)\
do \
{\
	if (NULL != v_para) {\
	delete[] v_para;\
	v_para = NULL;\
					}\
} while (0)


static std::string ensure_log_complete(IN const char* format, IN va_list args)
{
	if (NULL == format)
	{
		return "";
	}

	int iNum = 0;
	unsigned int uiSize = 1024;
	string strLog("");

	char *pcBuff = new(std::nothrow) char[uiSize];
	if (NULL == pcBuff)
	{
		return strLog;
	}

	while (true)
	{
		bzero(pcBuff, uiSize);

		iNum = vsnprintf(pcBuff, uiSize, format, args);
		if ((iNum > -1) && (iNum < (int)uiSize))
		{
			strLog = pcBuff;
			SAFE_DELETE_ARRAY(pcBuff);

			return strLog;
		}

		//如果字符串值比默认分配大，则分配更大空间
		uiSize = (iNum > -1) ? (int)(iNum + 1) : (uiSize * 2);
		SAFE_DELETE_ARRAY(pcBuff);

		pcBuff = new(std::nothrow) char[uiSize];
		if (NULL == pcBuff)
		{
			return strLog;
		}
	}

	SAFE_DELETE_ARRAY(pcBuff);

	return strLog;
}

void log_init()
{
	if (g_log_init == 1)
	{
		return;
	}

	log4cxx::PropertyConfigurator::configure("./log4cxx.properties");
	g_log_init = 1;

}

LoggerPtr get_logger_ptr(IN const char* user)
{
	return Logger::getLogger(user);
}

void log4cxx_package(IN const LOG_LEVEL level, IN const char* user, IN const char* file, IN const char* function, IN const int line, IN const char* format, ...)
{
	log_init();

	if (level > LOG_FATAL_ || level < LOG_TRACE_)
	{
		return;
	}

	if (NULL == file || NULL == function || NULL == format)
	{
		return;
	}

	char acTmp[30] = { 0 };
	snprintf(acTmp, sizeof(acTmp) - 1, "%d", line);

	va_list args;
	std::string strLog;
	strLog = "[" + std::string(file) + ":" + std::string(acTmp) + " " + std::string(function) + "] ";

	va_start(args, format);
	strLog += ensure_log_complete(format, args);
	va_end(args);

	switch (level)
	{
	case LOG_TRACE_:
		LOG4CXX_TRACE(get_logger_ptr(user), strLog);
		break;
	case LOG_DEBUG_:
		LOG4CXX_DEBUG(get_logger_ptr(user), strLog);
		break;
	case LOG_INFO_:
		LOG4CXX_INFO(get_logger_ptr(user), strLog);
		break;
	case LOG_WARN_:
		LOG4CXX_WARN(get_logger_ptr(user), strLog);
		break;
	case LOG_ERROR_:
		LOG4CXX_ERROR(get_logger_ptr(user), strLog);
		break;
	case LOG_FATAL_:
		LOG4CXX_FATAL(get_logger_ptr(user), strLog);
		break;
	default:
		break;
	}

	return;
}


