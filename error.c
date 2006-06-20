
#include <stdio.h>
#include <stdarg.h>

#include "error.h"

void fperror(const char *format, ...)
{
	char buffer[256];

	va_list argptr;

	va_start(argptr, format);
	vsprintf(buffer, format, argptr);
	va_end(argptr);

	perror(buffer);
}
