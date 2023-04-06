#ifndef LIBLOG_WRITER_H_
#define LIBLOG_WRITER_H_

#include "logger.h"

void writer_stdout(struct logger *log, struct abuff *buff);
void writer_file_once(struct logger *log, struct abuff *buff);
void writer_opened_file(struct logger *log, struct abuff *buff);

#endif

