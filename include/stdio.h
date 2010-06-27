#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>


#ifndef NULL
#define NULL ((void *) 0)
#endif

extern int vsprintf (char *, const char *, va_list);
#ifdef __USER
extern int printf(const char *, ...);
extern int sprintf (char *, const char *, ...);
#else
extern int printk(const char *, ...);
extern int sprintk (char *, const char *, ...);
#endif


/* It's maybe not a good idea stay here */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif /* stdio.h */
