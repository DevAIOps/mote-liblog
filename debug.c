#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>

#include <sys/syscall.h>

#include "config.h"

static int debug_get_level(const char *level)
{
	if (level == NULL)
		return LOG_DEBUG;
	if (strcasecmp(level, "error") == 0)
		return LOG_ERR;
	else if (strcasecmp(level, "info") == 0)
		return LOG_INFO;
	else if (strcasecmp(level, "debug") == 0)
		return LOG_DEBUG;
	return LOG_DEBUG;
}

void log_inner(int severity, const char *fmt, ...)
{
	va_list args;
	char time_str[64];
	FILE *fp = NULL;
	time_t tt;
	struct tm local_time;
	char *log_profile, *log_level = "info";
	int level = LOG_TRACE;
	static char *_log[] = { "", "", "", "ERROR", "WARN", "", "INFO", "DEBUG", ""};

        assert(severity < LOG_LEVEL_NUM);

	log_profile = getenv("LOG_PROFILE");
	if (log_profile == NULL)
		return;

	log_level = getenv("LOG_PROFILE_LEVEL");
	if (log_level)
		level = debug_get_level(log_level);
        if (severity > level)
                return;

	fp = fopen(log_profile, "a");
	if (fp == NULL)
		return;

	time(&tt);
	localtime_r(&tt, &local_time);
	strftime(time_str, sizeof(time_str), "%m-%d %T", &local_time);
	fprintf(fp, "%s %s (%d) ", time_str, _log[severity],
				(pid_t)syscall(SYS_gettid));

	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fprintf(fp, "\n");

	fclose(fp);

	return;
}

