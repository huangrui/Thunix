/**
 *  thunix/lib/printf.c
 *
 *  the printf function for user space programs
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>

int printf(const char *fmt, ...)
{
	char buf[1024];
        va_list args;
        int i;
  
        va_start(args,fmt);
        i = vsprintf(buf, fmt, args);
        va_end(args);

        conwrite(buf);
  
        return i;
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args,fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	return i;
}
