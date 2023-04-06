#ifndef LIBLOG_DEBUG_H_
#define LIBLOG_DEBUG_H_

#include "config.h"

void log_inner(int severity, const char *fmt, ...);

#define _debug(...)    do { log_inner(LOG_DEBUG,   __VA_ARGS__); } while(0)
#define _warn(...)     do { log_inner(LOG_WARNING, __VA_ARGS__); } while(0)
#define _info(...)     do { log_inner(LOG_INFO, __VA_ARGS__);    } while(0)
#define _error(...)    do { log_inner(LOG_ERR,     __VA_ARGS__); } while(0)

#endif
