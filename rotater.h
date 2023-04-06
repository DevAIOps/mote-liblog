#ifndef LIBLOG_ROTATER_H_
#define LIBLOG_ROTATER_H_

#include "config.h"

#include <stdint.h>

#define LOGR_M_INVAID 0
#define LOGR_M_PROC   1
#define LOGR_M_MUTEX  2

struct rotater_process {
	int fd;
	const char *file;
};

struct rotater {
	int mode;
	union {
#if LOG_USE_THREAD
		pthread_mutex_t thread;
#endif
		struct rotater_process proc;
	} lock;
};


void rotater_destory(struct rotater *r);
int rotater_init(struct rotater *r, int mode);

int rotater_rotate(struct rotater *r, const char *file, long file_size);


#endif

