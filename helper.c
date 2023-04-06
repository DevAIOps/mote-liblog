#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "debug.h"

int file_is_larger(const char *file, long size)
{
	struct stat info;

	if (stat(file, &info)) {
		_error("stat file '%s' failed, %d:%s.", file,
				errno, strerror(errno));
		return 0;
	}

	if (info.st_size > size) {
		_info("file size %d is larger than %d.", info.st_size, size);
		return 1;
	}

	return 0;
}
