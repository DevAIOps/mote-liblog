#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "../log.h"
#include "testing.h"

DEF_TEST(simple) {
	EXPECT_EQ_INT(LOG_TRACE, log_init(NULL, LOG_TRACE, 0));
	EXPECT_EQ_STR("stdout", log_get_mode());

	log_destroy();
	return 0;
}

int main(void)
{
	RUN_TEST(simple);
	END_TEST;
	return 0;
}

