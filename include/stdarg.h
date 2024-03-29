#ifndef _STDARG_H
#define _STDARG_H    1

typedef char * va_list;

/*
 * Amount of space required in an argument list for an arg of type TYPE
 */
#define __va_rounded_size(TYPE) \
	(((sizeof(TYPE) + sizeof(int)-1) / sizeof(int)) * sizeof(int))

#define va_start(AP,LASTARG)	\
	(AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))

void va_end (va_list);
#define va_end(AP)

#define va_arg(AP,TYPE)	\
	(AP += __va_rounded_size(TYPE),	\
	 *((TYPE *)(AP - __va_rounded_size(TYPE))))

#endif /* stdarg.h */
