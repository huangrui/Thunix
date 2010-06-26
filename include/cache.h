#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

/* The cache structure */
struct cache_struct {
        uint32_t block;
        struct cache_struct *prev;
        struct cache_struct *next;
        void  *data;
};


/* functions */
void cache_init(struct fs*);
struct cache_struct *get_cache_block(struct fs *, uint32_t);
void print_cache(void);


#endif /* cache.h */
