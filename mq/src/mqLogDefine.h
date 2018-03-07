#ifndef  _DCLOGDEFINE_H
#define  _DCLOGDEFINE_H

#include "xlogger.h"


#define MQ_STR "mq"

#define MQ_DEBUG(args...)  log4cxx_package(LOG_DEBUG_,MQ_STR, __FILE__, __FUNCTION__,__LINE__, ##args)
#define MQ_INFO(args...)   log4cxx_package(LOG_INFO_, MQ_STR, __FILE__, __FUNCTION__,__LINE__, ##args)
#define MQ_WARN(args...)   log4cxx_package(LOG_WARN_, MQ_STR, __FILE__, __FUNCTION__,__LINE__,##args)
#define MQ_ERROR(args...)  log4cxx_package(LOG_ERROR_,MQ_STR, __FILE__, __FUNCTION__,__LINE__, ##args)

#endif
