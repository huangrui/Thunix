/**
 *  thunix/kernel/printk.c
 *
 *  Just one printf-like function that's printk
 *
 */
#include <stdarg.h>
#include <stddef.h>
#include <console.h>


extern int vsprintf (char *buf, const char *fmt, va_list args);

int printk(const char *fmt, ...)
{
	char buf[1024];
        va_list args;
        int i;
  
        va_start(args,fmt);
        i = vsprintf(buf, fmt, args);
        va_end(args);

        con_write(buf, i);
  
/*
        __asm__("push %%fs\n\t"
                "push %%ds\n\t"
                "pop  %%fs\n\t"
                "push %0  \n\t"
                "push $buf\n\t"
                "call con_write\n\t"
                "addl $4, %%esp\n\t"
                "pop %0\n\t"
                "pop %%fs"
                ::"r"(i));
*/
        return i;
}

int sprintk(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args,fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	return i;
}	
