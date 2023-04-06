#ifndef LIBLOG_INTERNAL_H_
#define LIBLOG_INTERNAL_H_

#define  likely(x)    __builtin_expect(!!(x), 1)
#define  unlikely(x)  __builtin_expect(!!(x), 0)

#endif
