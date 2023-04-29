#ifndef LIBLOG_CONFIG_H_
#define LIBLOG_CONFIG_H_

#include <syslog.h>

/* default setting */
#ifndef LOG_PATH_MAX
#define LOG_PATH_MAX            1024
#endif

#ifndef LOG_LOCK_FILE
#define LOG_LOCK_FILE           "/var/run/log.lock"
#endif

#ifndef LOG_FILE_SIZE_MAX
#define LOG_FILE_SIZE_MAX       1024 * 1024 * 50
#endif

#ifndef LOG_FILE_NUMS_MAX
#define LOG_FILE_NUMS_MAX       4
#endif

#ifndef LOG_USE_FILENO
#define LOG_USE_FILENO          0
#endif

#ifndef LOG_USE_THREAD
#define LOG_USE_THREAD          0
#endif

#ifndef LOG_BUFFER_MIN
#define LOG_BUFFER_MIN          1024
#endif

#ifndef LOG_BUFFER_MAX
#define LOG_BUFFER_MAX          16 * 1024
#endif

#ifndef LOG_LEAVE_STR
#define LOG_LEAVE_STR           ">>>>> PANIC <<<<<\n"
#endif

#ifndef LOG_LEAVE_CODE
#define LOG_LEAVE_CODE          123
#endif

#ifndef __FILENAME__
#define __FILENAME__            __FILE__
#endif

#ifndef LOG_FILE_MODE
#define LOG_FILE_MODE           (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)
#endif

#ifndef LOG_FILE_FLAG
#define LOG_FILE_FLAG           (O_CLOEXEC | O_WRONLY | O_APPEND | O_CREAT)
#endif

#ifndef LOG_BAK_FILE_MODE
#define LOG_BAK_FILE_MODE       (S_IRUSR | S_IRGRP | S_IROTH)
#endif

#ifndef LOG_FSYNC_PERIOD
#define LOG_FSYNC_PERIOD        20
#endif

/* log mode */
#define LOG_F_INVALID           0x00
#define LOG_F_STDOUT            0x01
#define LOG_F_STDERR            0x02
#define LOG_F_PROC              0x03
#define LOG_F_MULTI             0x04
#define LOG_F_THREAD            0x05
#define LOG_F_MASK              0xff

/* log level setting */
#ifndef LOG_EMERG
        /* NOTE: Keep the following same with <syslog.h> file. */
        #define LOG_EMERG       0          /* system is unusable */
        #define LOG_ALERT       1          /* (used)action must be taken immediately, such as no memory */
        #define LOG_CRIT        2          /* critical conditions */
        #define LOG_ERR         3          /* (used)error conditions */
        #define LOG_WARNING     4          /* (used)warning conditions */
        #define LOG_NOTICE      5          /* normal but significant condition */
        #define LOG_INFO        6          /* (used)informational */
        #define LOG_DEBUG       7          /* (used)debug-level messages */
#endif
#define LOG_TRACE               8          /* (used)more details for debug-level */
#define LOG_AUDIT               LOG_EMERG
#define LOG_PANIC               LOG_ALERT
#define LOG_FATAL               LOG_CRIT
#define LOG_LEVEL_NUM           9
#define LOG_LEVEL_MAX           8
#define LOG_LEVEL_MIN           0
#define LOG_DEF_LEVEL           LOG_INFO

/* file line number setting */
#if LOG_USE_FILENO
	#define FFMT        "[%s:%d] "
	#define FARGS       const char *file, const int line,
	#define FPARS       __FILENAME__, __LINE__,
	#define FPARS_      file, line,
	#define FPARS__     ,file, line
#else
	#define FFMT
	#define FARGS
	#define FPARS
	#define FPARS_
	#define FPARS__
#endif

#endif
