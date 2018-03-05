#ifndef  _DCLOGDEFINE_H
#define  _DCLOGDEFINE_H

#include "xlogger.h"


#define DCNODE "dcnode"

#define DC_DEBUG(args...)  log4cxx_package(LOG_DEBUG_,DCNODE, __FILE__, __FUNCTION__,__LINE__, ##args)
#define DC_INFO(args...)   log4cxx_package(LOG_INFO_, DCNODE, __FILE__, __FUNCTION__,__LINE__, ##args)
#define DC_WARN(args...)   log4cxx_package(LOG_WARN_, DCNODE, __FILE__, __FUNCTION__,__LINE__,##args)
#define DC_ERROR(args...)  log4cxx_package(LOG_ERROR_,DCNODE, __FILE__, __FUNCTION__,__LINE__, ##args)

#endif
