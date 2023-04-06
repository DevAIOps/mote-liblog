#include <time.h>
#include <glob.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"

#if LOG_USE_THREAD
#include <pthread.h>
#endif

#include "debug.h"
#include "helper.h"
#include "rotater.h"

int rotater_init(struct rotater *r, int mode)
{
	r->mode = mode;
	if (mode == LOGR_M_PROC) {
		r->lock.proc.fd = -1;
		r->lock.proc.file = LOG_LOCK_FILE;
#if LOG_USE_THREAD
	} else if (mode == LOGR_M_MUTEX) {
		return pthread_mutex_init(&r->lock.thread, NULL);
#endif
	}
	return 0;
}

static int rotater_trylock_process(struct rotater *r)
{
	int fd = r->lock.proc.fd;

	if (r->lock.proc.fd < 0) {
		fd = open(r->lock.proc.file, O_CLOEXEC | O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (fd < 0) {
			_error("open lock file '%s' failed, %d:%s.", r->lock.proc.file,
					errno, strerror(errno));
			return -1;
		}
		r->lock.proc.fd  = fd;
	}

	struct flock fl = {
		.l_type   = F_WRLCK,
		.l_start  = 0,
		.l_whence = SEEK_SET,
		.l_len    = 0
	};
	if (fcntl(fd, F_SETLK, &fl)) {
		/* lock by other process, EAGAIN(linux) EACCES(AIX) */
		if (errno == EAGAIN || errno == EACCES)
			_warn("fcntl lock fail, as file is lock by other process");
		else
			_error("lock fd(%d) failed, %d:%s.", fd, errno, strerror(errno));
		return -1;
	}

	return 0;
}

#if LOG_USE_THREAD
static int rotater_trylock_thread(struct rotater *r)
{
	int rc;

	rc = pthread_mutex_trylock(&r->lock.thread);
	if (rc == EBUSY) {
		_warn("pthread_mutex_trylock fail, locked by other threads");
		return -1;
	} else if (rc != 0) {
		_error("pthread_mutex_trylock fail, rc %d", rc);
		return -1;
	}
	return 0;
}
#endif

static int rotater_trylock(struct rotater *r)
{
	if (r->mode == LOGR_M_PROC)
		return rotater_trylock_process(r);
#if LOG_USE_THREAD
	else if (r->mode == LOGR_M_MUTEX)
		return rotater_trylock_thread(r);
#endif
	return 0;
}

static int rotater_unlock(struct rotater *r)
{
	struct flock fl = {
		.l_type = F_UNLCK,
		.l_start = 0,
		.l_whence = SEEK_SET,
		.l_len = 0
	};

	if (r->mode == LOGR_M_PROC && r->lock.proc.fd >= 0) {
		if (fcntl(r->lock.proc.fd, F_SETLK, &fl)) {
			_error("unlock fd(%d) failed, %d:%s.", r->lock.proc.fd,
				errno, strerror(errno));
			return -1;
		}
#if LOG_USE_THREAD
	} else if (r->mode == LOGR_M_MUTEX) {
		if (pthread_mutex_unlock(&r->lock.thread)) {
			_error("pthread_mutex_unlock failed, %d:%s.",
				errno, strerror(errno));
			return -1;
		}
#endif
	}
	return 0;
}

void rotater_destory(struct rotater *r)
{
	if (r->mode == LOGR_M_PROC) {
		if (r->lock.proc.fd < 0)
			return;
		close(r->lock.proc.fd);
		r->lock.proc.fd = -1;
#if 0
	} else if (r->mode == LOGR_M_MUTEX) {
		pthread_mutextattr_destroy(&r->lock.thread);
#endif
	}
}

static void rotater_lsrm(const char *file)
{
	int rc;
	int nwrite;
	size_t pathc;
	char **pathv;
	glob_t globbuf;
	struct stat statbuf;
	char pattern[LOG_PATH_MAX];

	nwrite = snprintf(pattern, sizeof(pattern), "%s.[0-9]*-[0-9]*", file);
	if (nwrite < 0 || nwrite >= (int)sizeof(pattern)) {
		_error("snprintf() return %d, overflow or errno %d.", nwrite, errno);
		return;
	}

	/*
	 * scan file which likes the following, and should sorted.
	 *   file.log.20090506-221224
	 *   file.log.20090506-221225
	 *   file.log.20090506-221226
	 *   file.log.20090506-221227
	 */
	rc = glob(pattern, GLOB_ERR | GLOB_MARK, NULL, &globbuf);
	if (rc == GLOB_NOMATCH) {
		_error("no match for glob(%s)", pattern);
		globfree(&globbuf);
		return;
	} else if (rc) {
		_error("glob(%s) error, rc %d errno %d", pattern, rc, errno);
		return;
	}
	/* TODO: ensure the files sorted by time */

	pathv = globbuf.gl_pathv;
	pathc = globbuf.gl_pathc;

	for (; pathc-- > LOG_FILE_NUMS_MAX; pathv++) {
		rc = lstat(*pathv, &statbuf);
		if (rc < 0) {
			_error("lstat(%s) error, rc %d errono %", *pathv, rc, errno);
			continue;
		}

		if (S_ISREG(statbuf.st_mode)) {
			rc = remove(*pathv);
			if (rc < 0) {
				_error("remove(%s) error, rc %d errono %", *pathv, rc, errno);
				continue;
			}
		}
	}

	globfree(&globbuf);
}

int rotater_rotate(struct rotater *r, const char *file, long file_size)
{
	int rc = 0;
	char new_path[LOG_PATH_MAX];

	/* 19901203-123535, 1990-12-03 12:35:35 */
	time_t tt;
	char timestr[16];
	struct tm timenow;

	if (rotater_trylock(r)) {
		_warn("rotater trylock fail, maybe lock by other process or threads.");
		return 0;
	}

	/* recheck it, not large enough, may rotate by others */
	if (file_is_larger(file, file_size) == 0)
		return rotater_unlock(r);

	time(&tt);
	localtime_r(&tt, &timenow);
	rc = strftime(timestr, sizeof(timestr), "%Y%m%d-%H%M%S", &timenow);
	assert(rc < (int)sizeof(timestr) && rc != 0);

	/* do the base_path mv  */
	memset(new_path, 0, sizeof(new_path));
	rc = snprintf(new_path, sizeof(new_path), "%s.%s", file, timestr);
	if (rc < 0 || rc >= (int)sizeof(new_path)) {
		_error("new log file, nwirte %d, overflow or errno %d.", rc, errno);
		return -1;
	}

	if (rename(file, new_path)) {
		_error("rename(%s)->(%s) failed, %d:%s.", file, new_path,
				errno, strerror(errno));
		return -1;
	}
	chmod(new_path, S_IRUSR | S_IRGRP | S_IROTH); /* try best */

	/* begin list and move files */
	rotater_lsrm(file);
	return rotater_unlock(r);
}

