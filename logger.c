#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#include <sys/time.h>
#include <sys/syscall.h>

#include "debug.h"
#include "config.h"
#include "logger.h"
#include "writer.h"
#include "rotater.h"

#include "libase/base.h"

struct logger logger;
static int exit_register;

const char *logger_get_mode(struct logger *log)
{
	int mode = log->flag & LOG_F_MASK;
	switch (mode) {
	case LOG_F_PROC:
		return "process";
	case LOG_F_THREAD:
		return "thread";
	case LOG_F_MULTI:
		return "multi-process";
	case LOG_F_STDOUT:
		return "stdout";
	case LOG_F_STDERR:
		return "stderr";
	default:
		return "invalid";
	}
}

int logger_set_level(struct logger *log, int level)
{
	if (level > LOG_LEVEL_MAX)
		log->level = LOG_LEVEL_MAX;
	else if (level < LOG_LEVEL_MIN)
		log->level = LOG_LEVEL_MIN;
	else
		log->level = level;
	return log->level;
}

/* more logs */
int logger_level_dec(struct logger *log)
{
	if (log->level < LOG_LEVEL_MAX)
		log->level++;
	return log->level;
}

int logger_level_inc(struct logger *log)
{
	if (log->level > LOG_LEVEL_MIN)
		log->level--;
	return log->level;
}

static struct abuff *logger_default_buffer(struct logger *log)
{
	return log->data.buff;
}

#if LOG_USE_THREAD
static struct abuff *logger_thread_buffer(struct logger *log)
{
	int rc;
	struct abuff *buff;

	buff = (struct abuff *)pthread_getspecific(log->data.key);
        if (unlikely(buff == NULL)) {
		buff = abuff_create(LOG_BUFFER_MIN, LOG_BUFFER_MAX);
		if (unlikely(buff == NULL)) {
			_error("create buffer failed, out of memory.");
			return NULL;
		}
                rc = pthread_setspecific(log->data.key, buff);
                if (unlikely(rc)) {
			_error("set specific fail, rc %d.", rc);
			abuff_destroy(buff);
                        return NULL;
                }
        }
	return buff;
}
#endif

// if failed, ignore the prefix.
static int logger_do_prefix(FARGS char *buff, int len, int level)
{
	int rc, offset;
	struct timeval tv;
        struct tm timenow;

	static char _log[LOG_LEVEL_NUM] = { 'A', 'P', 'F', 'E', 'W', 'N', 'I', 'D', 'T' };

	if (unlikely(gettimeofday(&tv,NULL) < 0)) {
		_error("gettimeofday failed, %d:%s.", errno, strerror(errno));
		return 0;
	}

        localtime_r(&tv.tv_sec, &timenow);
	offset = strftime(buff, len, "%m-%d %T.", &timenow);
	if (unlikely(offset < 0)) {
		_error("format time failed, %d:%s.", errno, strerror(errno));
		return 0;
	}

        rc = snprintf(buff + offset, len - offset, "%03d %c (%d) " FFMT,
			(int)(tv.tv_usec / 1000), _log[level],
			(pid_t)syscall(SYS_gettid) FPARS__);
	if (unlikely(rc < 0)) {
		_error("format content failed, %d:%s.", errno, strerror(errno));
		return 0;
	}
	return offset + rc;
}

static void logger_do_write(FARGS struct logger *log, int level, const char *fmt, va_list ap)
{
	int rc;
	va_list args;
	struct abuff *buff;

        assert(fmt);
        assert(level <= LOG_LEVEL_MAX && level >= LOG_LEVEL_MIN);

	buff = log->buffer(log);
	abuff_restart(buff);
	rc = logger_do_prefix(FPARS_ abuff_tail(buff), abuff_tailroom(buff), level);
	assert(rc >= 0); // message has dumped.
	abuff_tail_step_fast(buff, rc);

        va_copy(args, ap);
	rc = abuff_vprintf(buff, fmt, args);
	if (unlikely(rc < 0)) {
		_error("buffer printf failed, rc %d %d:%s.", rc, errno, strerror(errno));
		return;
	}
        va_end(ap);

	abuff_must_push(buff, '\n');

	log->write(log, buff);
}

void logger_it(FARGS struct logger *log, int level, const char *fmt, ...)
{
	va_list ap;
        va_start(ap, fmt);
	logger_do_write(FPARS_ log, level, fmt, ap);
        va_end(ap);
}

/*
 * 1100: 6572 7428 7365 7665 7269 7479 203c 204c  ert(severity < L
 * NOTE: failed only when write() error.
 */
static int logger_prepare_hex(struct abuff *buff, char *data, int bytes)
{
	char hex[65], ch;
	int i, j, idx = 0, off, aff, lines;
        static uint8_t bcd2ascii[16] = {
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F'
        };

#define HEX_BEGIN() do {                                \
	hex[0] = '\t';                                  \
	off = 1; aff = off + 47;                        \
	hex[off++] = bcd2ascii[(idx >> 12) & 0x0F];     \
	hex[off++] = bcd2ascii[(idx >>  8) & 0x0F];     \
	hex[off++] = bcd2ascii[(idx >>  4) & 0x0F];     \
	hex[off++] = bcd2ascii[(idx      ) & 0x0F];     \
	hex[off++] = ':';                               \
	hex[off++] = ' ';                               \
} while(0)

#define HEX_ONE_BYTE() do {                             \
	ch = data[idx++];                               \
	hex[off++] = bcd2ascii[(ch >> 4) & 0xf];        \
	hex[off++] = bcd2ascii[ch & 0xf];               \
	hex[aff++] = (ch < ' ' || ch > '~') ? '.' : ch; \
	if (j & 0x01)                                   \
		hex[off++] = ' ';                       \
} while(0)

/* add for debug: hex[aff++] = '\n'; */
#define HEX_END() do {                                  \
	hex[off++] = ' ';                               \
	hex[aff++] = '\n';                              \
} while(0)

	lines = bytes >> 4;
	for (i = 0; i < lines; i++) {
		HEX_BEGIN();
		for (j = 0; j < 16; j++)
			HEX_ONE_BYTE();
		HEX_END();
		abuff_put_data(buff, hex, sizeof(hex));
	}
	lines = bytes & 0x0f;
	if (lines) {
		HEX_BEGIN();
		for (j = 0; j < lines; j++)
			HEX_ONE_BYTE();
		for (; j < 16; j++) {
			hex[off++] = ' ';
			hex[off++] = ' ';
			hex[aff++] = ' ';
			if (j & 0x01)
				hex[off++] = ' ';
		}
		HEX_END();
		abuff_put_data(buff, hex, sizeof(hex));
	}

	return 0;
}

void logger_hex_it(FARGS struct logger *log, int level, void *data, int size, const char *fmt, ...)
{
	va_list ap;
	struct abuff *buff;

        va_start(ap, fmt);
	logger_do_write(FPARS_ log, level, fmt, ap);
        va_end(ap);

	buff = log->buffer(log);
	abuff_restart(buff);
	logger_prepare_hex(buff, data, size);
	log->write(log, buff);
}

static void logger_set_default(struct logger *log)
{
	log->flag = 0;
	log->inited = 0;
	log->stat.fsync = 0;
	log->level = LOG_LEVEL_MAX;
}

void logger_reset(struct logger *log)
{
	struct abuff *buff;
	int mode = log->flag & LOG_F_MASK;

	if (mode > LOG_F_STDERR && log->file.fd > 0) {
                fsync(log->file.fd);
                close(log->file.fd);
	}
	log->file.fd = -1;

	if (log->file.name != NULL)
		free(log->file.name);
	log->file.name = NULL;

	if (mode != LOG_F_THREAD && mode != LOG_F_INVALID) {
		buff = log->data.buff;
		abuff_destroy(buff);
		log->data.buff = NULL;
#if LOG_USE_THREAD
	} else {
		buff = (struct abuff *)pthread_getspecific(log->data.key);
		abuff_destroy(buff);
		pthread_key_delete(log->data.key);
		log->data.key = -1;
#endif
	}
	logger_set_default(log);
}

static void logger_terminate(void)
{
	logger_reset(&logger);
}

static int logger_prepare_hook(struct logger *log, int mode)
{
	log->buffer = logger_default_buffer;
	log->write = writer_opened_file;
	if (mode == LOG_F_STDOUT) {
		log->write = writer_stdout;
		log->file.fd = STDOUT_FILENO;
	} else if (mode == LOG_F_STDERR) {
		log->write = writer_stdout;
		log->file.fd = STDERR_FILENO;
	} else if (mode == LOG_F_MULTI) {
		log->write = writer_file_once;
	}

	if (mode != LOG_F_THREAD) {
		if (log->data.buff != NULL)
			return 0;
		log->data.buff = abuff_create(LOG_BUFFER_MIN, LOG_BUFFER_MAX);
		if (unlikely(log->data.buff == NULL)) {
			_error("create buffer failed, out of memory.");
			return -1;
		}
#if LOG_USE_THREAD
	} else {
		log->buffer = logger_thread_buffer;
		if (pthread_getspecific(log->data.key) != NULL)
			return 0;
		if (pthread_key_create(&(log->data.key), (void(*)(void *))abuff_destroy) < 0) {
			_error("create pthread key failed, %d:%s.", errno, strerror(errno));
			return -1;
		}
#endif
	}

	return 0;
}

static int logger_do_init(struct logger *log, const char *file, int flag)
{
	int rotater = LOGR_M_INVAID;
	int mode = flag & LOG_F_MASK;

	if (log->inited) {
		_info("logger has initilized, ignore it.");
		return 0;
	}

	if (mode == LOG_F_INVALID) {
		mode = LOG_F_STDOUT;
		flag |= mode;
	}
	log->flag = flag;
	if (logger_prepare_hook(log, mode) < 0)
		return -1;

	/* no need to open files */
	if (mode == LOG_F_STDOUT || mode == LOG_F_STDERR)
		return 0;

	if (file == NULL || strlen(file) >= LOG_PATH_MAX) {
		_error("invalid argument, file %p, %s.", file,
				file == NULL ? "null" : file);
		return -EINVAL;
	}

#if LOG_USE_THREAD == 0
	if (mode == LOG_F_THREAD) {
		_error("unsupport thread mode.");
		return -EPERM;
	}
#endif

	if (mode == LOG_F_MULTI)
		rotater = LOGR_M_PROC;
	else if (mode == LOG_F_THREAD)
		rotater = LOGR_M_MUTEX;
	if (rotater_init(&log->rotater, rotater) < 0)
		return -1;

	log->file.name = strdup(file);
	if (log->file.name == NULL)
		return -ENOMEM;

	if (mode != LOG_F_MULTI) {
		assert(mode == LOG_F_PROC || mode == LOG_F_THREAD);
		log->file.fd = open(file, LOG_FILE_FLAG, LOG_FILE_MODE);
		if (log->file.fd < 0) {
			_error("open log file '%s' failed, %d:%s.", file,
					errno, strerror(errno));
			return -1;
		}
	}

        /* Register logger_destory to be called at program termination */
	log->inited = 1;

       	/* try to rotate when start. */
	return rotater_rotate(&log->rotater, log->file.name, LOG_FILE_SIZE_MAX);
}

void logger_leave(struct logger *log)
{
	int fd = log->file.fd;
	int mode = log->flag & LOG_F_MASK;

	if (mode == LOG_F_MULTI) {
		fd = open(log->file.name, LOG_FILE_FLAG, LOG_FILE_MODE);
		if (unlikely(fd < 0)) {
			_error("open(%s) failed, %d:%s.", log->file.name,
					errno, strerror(errno));
			return;
		}
	}

	write(fd, LOG_LEAVE_STR, sizeof(LOG_LEAVE_STR) - 1);
	fsync(fd);

	if (mode == LOG_F_MULTI)
		close(fd);
}

int logger_init(struct logger *log, const char *file, int level, int flag)
{
	int rc;

	if (exit_register == 0) {
		/* it's a global cleanup method. */
		atexit(logger_terminate);
		exit_register = 1;
	}

	if (log == NULL) {
		_error("invalid argument, log=NULL.");
		return -EINVAL;
	}

	rc = logger_do_init(log, file, flag);
	if (rc < 0)
		return rc;

	return logger_set_level(&logger, level);
}
