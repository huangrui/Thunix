/*
 * The malloc stuff taken from fstk project
 *
 * ------------ from fstk ---------------
 * The malloc lib for fstk. 
 * 
 * Copyright (C) 2009, 2010 Liu Aleaxander -- All rights reserved.
 * This file may be redistributed under the terms of the GNU Public License.
 */
 

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

/* First, assume we just need 1M memory */
#define MEM_SIZE 0x100000

/*
 * Allocat memory from mem adrress 8M
 *
 * I will make this much more stable and robust when I'm going to
 * implement the whole memory managerment thing
 */
#define memory   0x800000

//static char memory[MEM_SIZE];

/* Next free memory address */
static struct mem_struct *next_start = (struct mem_struct *)memory;
static uint32_t mem_end = (uint32_t)(memory + MEM_SIZE);


static inline struct mem_struct *get_next(struct mem_struct *mm)
{
        uint32_t next = (uint32_t)mm + mm->size;
        
        if (next >= mem_end)
                return NULL;
        else
                return (struct mem_struct *)next;
}

/*
 * Here are the _merge_ functions, that merges a adjacent memory region, 
 * from front, or from back, or even merges both. It returns the headest 
 * region mem_struct.
 *
 */

static struct mem_struct *  merge_front(struct mem_struct *mm, 
                                        struct mem_struct *prev)
{
        struct mem_struct *next = get_next(mm);
        
        prev->size += mm->size;
        if (next)
                next->prev = prev;
        return prev;
}

static struct mem_struct *  merge_back(struct mem_struct *mm, 
                                       struct mem_struct *next)
{
        mm->free = 1;    /* Mark it free first */
        mm->size += next->size;

        next = get_next(next);
        if (next)
                next->prev = mm;
        return mm;
}

static struct mem_struct *  merge_both(struct mem_struct *mm, 
                                       struct mem_struct *prev, 
                                       struct mem_struct *next)
{
        prev->size += mm->size + next->size;
        
        next = get_next(next);
        if (next)
                next->prev = prev;
        return prev;
}

static inline struct mem_struct *  try_merge_front(struct mem_struct *mm)
{
        mm->free = 1;
        if (mm->prev->free)
                mm = merge_front(mm, mm->prev);
        return mm;
}

static inline struct mem_struct *  try_merge_back(struct mem_struct *mm)
{
        struct mem_struct *next = get_next(mm);
        
        mm->free = 1;
        if (next->free)
                merge_back(mm, next);
        return mm;
}

/* 
 * Here's the main function, malloc, which allocates a memory rigon
 * of size _size_. Returns NULL if failed, or the address newly allocated.
 * 
 */
void *malloc(int size)
{
        struct mem_struct *next = next_start;
        struct mem_struct *good = next, *prev;
        int size_needed = (size + sizeof(struct mem_struct) + 3) & ~3;
        
        while(next) {
                if (next->free && next->size >= size_needed) {
                        good = next;
                        break;
                }
                next = get_next(next);
        }
        if (good->size < size_needed) {
                printk("Out of memory, maybe we need append it\n");
                return NULL;
        } else if (good->size == size_needed) {
                /* 
                 * We just found a right memory that with the exact 
                 * size we want. So we just Mark it _not_free_ here,
                 * and move on the _next_start_ pointer, even though
                 * the next may not be a right next start.
                 */
                good->free = 0;
                next_start = get_next(good);
                goto out;
        } else
                size = good->size;  /* save the total size */
        
        /* 
         * Note: allocate a new memory region will not change 
         * it's prev memory, so we don't need change it here.
         */
        good->free = 0;       /* Mark it not free any more */
        good->size = size_needed;
        
        next = get_next(good);
        if (next) {
                next->size = size - size_needed;
                /* check if it can contain 1 byte allocation at least */
                if (next->size <= (int)sizeof(struct mem_struct)) {
                        good->size = size;     /* restore the original size */
                        next_start = get_next(good);
                        goto out;
                }
                        
                next->prev = good;
                next->free = 1;                
                next_start = next; /* Update next_start */

                prev = next;
                next = get_next(next);
                if (next)
                        next->prev = prev;
        } else
                next_start = (struct mem_struct *)memory;        
out:
        return (void *)((uint32_t)good + sizeof(struct mem_struct));
}

void free(void *ptr)
{
        struct mem_struct *mm = ptr - sizeof(*mm);
        struct mem_struct *prev = mm->prev;
        struct mem_struct *next = get_next(mm);

        if (!prev)
                mm = try_merge_back(mm);
        else if (!next)
                mm = try_merge_front(mm);        
        else if (prev->free && !next->free)
                merge_front(mm, prev);
        else if (!prev->free && next->free)
                merge_back(mm, next);
        else if (prev->free && next->free)
                merge_both(mm, prev, next);
        else
                mm->free = 1;

        if (mm < next_start)
                next_start = mm;
}

/* 
 * The debug function
 */
void check_mem(void)
{
        struct mem_struct *next = (struct mem_struct *)memory;
        
        printk("____________\n");
        while (next) {
                printk("%-6d  %s\n", next->size, next->free ? "Free" : "Notf");
                next = get_next(next);
        }
        printk("\n");
}

void Debug_mm(void)
{
	char *buf;

#include <hexdump.h>
	buf = malloc(1024);
	printk("malloc returned: %p\n", buf);
	floppy_reads(0, buf, 2);
	hexdump(buf + 512, 128);
}
           

void malloc_init(void)
{
        
        struct mem_struct *first = (struct mem_struct *)memory;
               
        first->prev = NULL;
        first->size = MEM_SIZE;
        first->free = 1;
        
        next_start = first;
}
