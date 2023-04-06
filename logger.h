#ifndef LIBLOG_LOGGER_H_
#define LIBLOG_LOGGER_H_

#include "config.h"

#if LOG_USE_THREAD
#include <pthread.h>
#endif

#include "rotater.h"

struct logger {
	int flag;
	int level;
	int inited;

	struct {
		int fd;
		char *name;
	} file;
	union {
#if LOG_USE_THREAD
		pthread_key_t key;
#endif
		struct abuff *buff;
	} data;
	struct {
		int fsync;
	} stat;
	struct rotater rotater;

	struct abuff *(*buffer)(struct logger *);
	void (*write)(struct logger *, struct abuff *);
};

void logger_reset(struct logger *log);
int logger_init(struct logger *log, const char *file, int level, int flag);

void logger_it(FARGS struct logger *log, int level, const char *fmt, ...);
void logger_hex_it(FARGS struct logger *log, int level, void *data, int size, const char *fmt, ...);

int logger_level_dec(struct logger *log);
int logger_level_inc(struct logger *log);
int logger_set_level(struct logger *log, int level);
const char *logger_get_mode(struct logger *log);

#endif
