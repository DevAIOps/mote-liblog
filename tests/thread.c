#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "../log.h"
#include "testing.h"

int simple(int type)
{
	uint64_t val = 0x0505050505050505;
	EXPECT_EQ_INT(LOG_TRACE, log_init("basic.log", LOG_TRACE, type));

	log_audit("audit");
	log_fatal("fatal");
	log_error("error");
	log_warning("warning");
	log_notice("notice");
	log_info("info");
	log_debug("debug");
	log_trace("trace");
	logh_info((void*)&val, sizeof(val), "binary value dump:");

	return 0;
}

DEF_TEST(stdout_simple) {
	EXPECT_EQ_INT(0, simple(LOG_F_STDOUT));
	log_destroy();
	return 0;
}

DEF_TEST(stderr_simple) {
	EXPECT_EQ_INT(0, simple(LOG_F_STDERR));
	log_destroy();
	return 0;
}

DEF_TEST(process_simple) {
	EXPECT_EQ_INT(0, simple(LOG_F_PROC));
	log_destroy();
	return 0;
}

DEF_TEST(multi_process_simple) {
	EXPECT_EQ_INT(0, simple(LOG_F_MULTI));
	log_destroy();
	return 0;
}

DEF_TEST(thread_simple) {
	EXPECT_EQ_INT(0, simple(LOG_F_THREAD));
	log_destroy();
	return 0;
}

int main(void)
{
	RUN_TEST(stdout_simple);
	RUN_TEST(stderr_simple);
	RUN_TEST(process_simple);
	RUN_TEST(multi_process_simple);
	RUN_TEST(thread_simple);

	END_TEST;
	return 0;
}

