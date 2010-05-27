#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

extern int vsprintf (char *, const char *, va_list);
extern int sprintk (char *, const char *, va_list);
extern int printk(const char *, ...);

#endif /* stdio.h */
