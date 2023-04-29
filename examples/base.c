#include <stdlib.h>

#include "../log.h"

int main()
{	
	log_init("stdout", LOG_TRACE, LOG_F_STDOUT);
	log_info("Hello World.");
	log_init("test.log", LOG_TRACE, LOG_F_PROC);
	log_info("Hello World.");
	return EXIT_SUCCESS;
}
