#include <stdio.h>
#include <stdarg.h>

#include "log.h"

void log_msg(enum log_level_t level, char *format, ...) {
	va_list argp;
	
	va_start(argp, format);
	
	vfprintf(stderr, format, argp);
	fprintf(stderr, "\n");
	
	va_end(argp);
}