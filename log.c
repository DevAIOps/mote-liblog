#include <stdlib.h>
#include <strings.h>

#include "logger.h"

extern struct logger logger;

void log_destroy(void)
{
	logger_reset(&logger);
}

int log_get_level(const char *level)
{
	if (level == NULL)
		return -1;
	else if (strcasecmp(level, "emerg") == 0)
		return LOG_EMERG;
	else if (strcasecmp(level, "alert") == 0)
		return LOG_ALERT;
	else if (strcasecmp(level, "error") == 0)
		return LOG_ERR;
	else if (strcasecmp(level, "warning") == 0)
		return LOG_WARNING;
	else if (strcasecmp(level, "info") == 0)
		return LOG_INFO;
	else if (strcasecmp(level, "debug") == 0)
		return LOG_DEBUG;
	else if (strcasecmp(level, "trace") == 0)
		return LOG_TRACE;

	return -1;
}

const char *log_get_name(const int level)
{
	static const char *lvlname[] = {
		"emerg", "alert", "critical", "error",
		"warning", "notice", "info", "debug", "trace"
	};

	if (level < LOG_LEVEL_MIN || level > LOG_LEVEL_MAX)
		return "invalid";

	return lvlname[level];
}
