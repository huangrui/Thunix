#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>


#ifndef NULL
#define NULL ((void *) 0)
#endif

extern int vsprintf (char *, const char *, va_list);
extern int sprintk (char *, const char *, ...);
extern int printk(const char *, ...);

#endif /* stdio.h */
