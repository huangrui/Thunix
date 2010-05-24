#ifndef _STRING_H
#define _STRING_H    1

#include <types.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

static int strcmp(const char *s1, const char *s2)
{
        while ((*s1 == *s2) && *s1 != '\0') {
                s1++;
                s2++;
        }
  
        return (*s1 - *s2);
}

static int strncmp(const char *s1, const char *s2, int n)
{
        while ((--n) && (*s1 == *s2) && *s1 != '\0') {
                s1++;
                s2++;
        }
  
        return (*s1 - *s2);
}


static char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}


static char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}


static size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc; ++sc)
		/* nothing */;
	return sc - s;
}


static char *strchr (const char *s, int c)
{
        while (*s) {
                if (*s == c)
                        return (char *) s;
                s++;
        }
        
        return (void *)0;
}

static void *memset(void *s, int c, size_t count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}

static void *memset_word(void *s, unsigned short word, size_t count)
{
        unsigned short *xs = (unsigned short *)s;
  
        while (count--)
                *xs++ = word;
        return s;
}

static void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;
        
	while (count--)
		*tmp++ = *s++;
	return dest;
}


static void *memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;
        
	if (dest <= src) {
		tmp = dest;
		s = src;
		while (count--)
			*tmp++ = *s++;
	} else {
		tmp = dest;
		tmp += count;
		s = src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}


static int memcmp(const void *cs, const void *ct, size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;

	for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}


#endif /* _STRING_H */
