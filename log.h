#ifndef LIBLOG_LOG_H_
#define LIBLOG_LOG_H_

#include "config.h"
#include "logger.h"

extern struct logger logger;

void log_destroy(void);

int log_get_level(const char *level);
const char *log_get_name(const int level);

static inline int log_init(const char *file, int level, int flag)
{
	return logger_init(&logger, file, level, flag);
}

static inline const char *log_get_mode(void)
{
	return logger_get_mode(&logger);
}

static inline int log_level_inc(void)
{
	return logger_level_inc(&logger);
}

static inline int log_level_dec(void)
{
	return logger_level_dec(&logger);
}

static inline int log_set_level(int level)
{
	return logger_set_level(&logger, level);
}

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	#define logh_audit(buff, len, ...)   do { logger_hex_it(FPARS &logger, LOG_AUDIT, buff, len, __VA_ARGS__); } while(0)
	#define logh_panic(buff, len, ...)   do { logger_hex_it(FPARS &logger, LOG_PANIC, buff, len, __VA_ARGS__); log_leave(); _exit(LOG_LEAVE_CODE); } while(0)
	#define logh_fatal(buff, len, ...)   do { if (logger.level >= LOG_FATAL)   logger_hex_it(FPARS &logger, LOG_FATAL,   buff, len, __VA_ARGS__); } while(0)
	#define logh_error(buff, len, ...)   do { if (logger.level >= LOG_ERR)     logger_hex_it(FPARS &logger, LOG_ERR,     buff, len, __VA_ARGS__); } while(0)
	#define logh_warning(buff, len, ...) do { if (logger.level >= LOG_WARNING) logger_hex_it(FPARS &logger, LOG_WARNING, buff, len, __VA_ARGS__); } while(0)
	#define logh_notice(buff, len, ...)  do { if (logger.level >= LOG_NOTICE)  logger_hex_it(FPARS &logger, LOG_NOTICE,  buff, len, __VA_ARGS__); } while(0)
	#define logh_info(buff, len, ...)    do { if (logger.level >= LOG_INFO)    logger_hex_it(FPARS &logger, LOG_INFO,    buff, len, __VA_ARGS__); } while(0)
	#define logh_debug(buff, len, ...)   do { if (logger.level >= LOG_DEBUG)   logger_hex_it(FPARS &logger, LOG_DEBUG,   buff, len, __VA_ARGS__); } while(0)
	#define logh_trace(buff, len, ...)   do { if (logger.level >= LOG_TRACE)   logger_hex_it(FPARS &logger, LOG_TRACE,   buff, len, __VA_ARGS__); } while(0)

	#define log_audit(...)         do { logger_it(FPARS &logger, LOG_AUDIT, __VA_ARGS__); } while(0)
	#define log_panic(...)         do { logger_it(FPARS &logger, LOG_PANIC, __VA_ARGS__); log_leave(); _exit(LOG_LEAVE_CODE); } while(0)
	#define log_fatal(...)         do { if (logger.level >= LOG_FATAL)   logger_it(FPARS &logger, LOG_FATAL,   __VA_ARGS__); } while(0)
	#define log_error(...)         do { if (logger.level >= LOG_ERR)     logger_it(FPARS &logger, LOG_ERR,     __VA_ARGS__); } while(0)
	#define log_warning(...)       do { if (logger.level >= LOG_WARNING) logger_it(FPARS &logger, LOG_WARNING, __VA_ARGS__); } while(0)
	#define log_notice(...)        do { if (logger.level >= LOG_NOTICE)  logger_it(FPARS &logger, LOG_NOTICE,  __VA_ARGS__); } while(0)
	#define log_info(...)          do { if (logger.level >= LOG_INFO)    logger_it(FPARS &logger, LOG_INFO,    __VA_ARGS__); } while(0)
	#define log_debug(...)         do { if (logger.level >= LOG_DEBUG)   logger_it(FPARS &logger, LOG_DEBUG,   __VA_ARGS__); } while(0)
	#define log_trace(...)         do { if (logger.level >= LOG_TRACE)   logger_it(FPARS &logger, LOG_TRACE,   __VA_ARGS__); } while(0)
#elif defined __GNUC__
	#define logh_audit(buff, len, ...)   do { logger_hex_it(FPARS &logger, LOG_AUDIT, buff, len, fmt, ## args); } while(0)
	#define logh_panic(buff, len, ...)   do { logger_hex_it(FPARS &logger, LOG_PANIC, buff, len, fmt, ## args); log_leave(); _exit(LOG_LEAVE_CODE); } while(0)
	#define logh_fatal(buff, len, ...)   do { if (logger.level >= LOG_FATAL)   logger_hex_it(FPARS &logger, LOG_FATAL,   buff, len, fmt, ## args); } while(0)
	#define logh_error(buff, len, ...)   do { if (logger.level >= LOG_ERR)     logger_hex_it(FPARS &logger, LOG_ERR,     buff, len, fmt, ## args); } while(0)
	#define logh_warning(buff, len, ...) do { if (logger.level >= LOG_WARNING) logger_hex_it(FPARS &logger, LOG_WARNING, buff, len, fmt, ## args); } while(0)
	#define logh_notice(buff, len, ...)  do { if (logger.level >= LOG_NOTICE)  logger_hex_it(FPARS &logger, LOG_NOTICE,  buff, len, fmt, ## args); } while(0)
	#define logh_info(buff, len, ...)    do { if (logger.level >= LOG_INFO)    logger_hex_it(FPARS &logger, LOG_INFO,    buff, len, fmt, ## args); } while(0)
	#define logh_debug(buff, len, ...)   do { if (logger.level >= LOG_DEBUG)   logger_hex_it(FPARS &logger, LOG_DEBUG,   buff, len, fmt, ## args); } while(0)
	#define logh_trace(buff, len, ...)   do { if (logger.level >= LOG_TRACE)   logger_hex_it(FPARS &logger, LOG_TRACE,   buff, len, fmt, ## args); } while(0)

	#define log_audit(...)         do { logger_it(FPARS &logger, LOG_AUDIT, fmt, ## args); log_leave(); } while(0)
	#define log_panic(...)         do { logger_it(FPARS &logger, LOG_PANIC, fmt, ## args); log_leave(); _exit(LOG_LEAVE_CODE); } while(0)
	#define log_fatal(...)         do { if (logger.level >= LOG_FATAL)   logger_it(FPARS &logger, LOG_FATAL,   fmt, ## args); } while(0)
	#define log_error(...)         do { if (logger.level >= LOG_ERR)     logger_it(FPARS &logger, LOG_ERR,     fmt, ## args); } while(0)
	#define log_warning(...)       do { if (logger.level >= LOG_WARNING) logger_it(FPARS &logger, LOG_WARNING, fmt, ## args); } while(0)
	#define log_notice(...)        do { if (logger.level >= LOG_NOTICE)  logger_it(FPARS &logger, LOG_NOTICE,  fmt, ## args); } while(0)
	#define log_info(...)          do { if (logger.level >= LOG_INFO)    logger_it(FPARS &logger, LOG_INFO,    fmt, ## args); } while(0)
	#define log_debug(...)         do { if (logger.level >= LOG_DEBUG)   logger_it(FPARS &logger, LOG_DEBUG,   fmt, ## args); } while(0)
	#define log_trace(...)         do { if (logger.level >= LOG_TRACE)   logger_it(FPARS &logger, LOG_TRACE,   fmt, ## args); } while(0)
#endif

#endif
