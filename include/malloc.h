#ifndef FSTK_MALLOC_H
#define FSTK_MALLOC_H

#include <stdio.h>

/* The memory managemant structure */
struct mem_struct {
        struct mem_struct *prev;
        int size;
        int free;
};

void malloc_init(void);
void *malloc(int);
void free(void *);
void check_mem(void);

static inline void malloc_error(char *obj)
{
        printk("malloc error: can't allocate memory for %s\n", obj);
        check_mem();
}


#endif /* malloc.h */
