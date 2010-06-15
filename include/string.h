#ifndef _STRING_H
#define _STRING_H    1

#include <types.h>
#include <malloc.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

extern int strcmp(const char *, const char *);
extern int strncmp(const char *, const char *, int);
extern char *strcpy(char *, const char *);
extern char *strcat(char *, const char *);
extern size_t strlen(const char *);
extern char *strchr (const char *, int);
extern char *strrchr (char *, int);
extern char *strdup(const char *);
extern void *memset(void *, int , size_t);
extern void *memset_word(void *, unsigned short, size_t);
extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *, const void *, size_t);
extern int memcmp(const void *, const void *, size_t);


#endif /* _STRING_H */
