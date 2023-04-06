#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/stat.h>

#include "debug.h"
#include "config.h"
#include "helper.h"
#include "writer.h"

#include "libase/base.h"

void writer_stdout(struct logger *log, struct abuff *buff)
{
        write(log->file.fd, abuff_string(buff), abuff_length(buff));
}

void writer_opened_file(struct logger *log, struct abuff *buff)
{
	assert(log->file.name != NULL);
	if (unlikely(log->file.fd < 0)) {
		log->file.fd = open(log->file.name, LOG_FILE_FLAG, LOG_FILE_MODE);
		_warn("logfile < 0, try to re-open(%s), %d:%s.", log->file.name,
				errno, strerror(errno));
		if (unlikely(log->file.fd < 0))
			return;
	}

	if (unlikely(write(log->file.fd, abuff_string(buff), abuff_length(buff)) < 0)) {
		_error("write to '%s' failed, %d:%s.", log->file.name,
				errno, strerror(errno));
		close(log->file.fd);
		log->file.fd = -1;
	}

	if (++log->stat.fsync > LOG_FSYNC_PERIOD) {
		log->stat.fsync = 0;
		if (file_is_larger(log->file.name, LOG_FILE_SIZE_MAX) == 0)
			return;
		if (log->file.fd > 0) {
			fsync(log->file.fd);
			close(log->file.fd);
		}
		log->file.fd = -1;
		rotater_rotate(&log->rotater, log->file.name, LOG_FILE_SIZE_MAX);
	}
}

void writer_file_once(struct logger *log, struct abuff *buff)
{
	int fd;

	assert(log->file.fd < 0);
	fd = open(log->file.name, LOG_FILE_FLAG, LOG_FILE_MODE);
	if (unlikely(fd < 0)) { /* this should not happen, checked when log_init() */
		_error("open(%s) failed, %d:%s.", log->file.name,
				errno, strerror(errno));
		return;
	}

	if (unlikely(write(fd, abuff_string(buff), abuff_length(buff)) < 0)) {
		_error("write(%s) failed, %d:%s.", log->file.name,
				errno, strerror(errno));
		close(fd);
		return;
	}
	close(fd);

	if (++log->stat.fsync > LOG_FSYNC_PERIOD) {
		log->stat.fsync = 0;
		if (file_is_larger(log->file.name, LOG_FILE_SIZE_MAX) == 0)
			return;
		rotater_rotate(&log->rotater, log->file.name, LOG_FILE_SIZE_MAX);
	}
}
