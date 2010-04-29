#ifndef _DDEBUG_H_
#define _DDEBUG_H_

#if defined(DDEBUG) && (DDEBUG)

#define dd(...) fprintf(stderr, "[MODULE_DEBUG]:");\
				fprintf(stderr, __VA_ARGS__); \
				fprintf(stderr, " at %s line %d.\n", __FILE__, __LINE__)
# else

#include <stdarg.h>
#include <stdio.h>
#include <stdarg.h>

static void dd(const char* fmt, ...) {}

#endif
#endif
