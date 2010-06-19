/*
 * A simple LRU cache implemention. Even the fstk is a user-space program, 
 * but we really do need a cache algorithm to make fstk more effective.
 *
 * This file is from Syslinux_project/core/cache.c. This file can
 * be redistributed under the terms of the GNU Public License.
 */

#include <stdio.h>
#include <string.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>


/* The cache data, we just make it be 32K, and it's enough for us */
#define CACHE_SIZE	(32 << 10)

static char cache_data[CACHE_SIZE];

#define MAX_CACHE_ENTRIES 64     /* assume we have a block size as 512 */
static struct cache_struct cache_head;
static struct cache_struct cache[MAX_CACHE_ENTRIES];
static int cache_entries = 0;

/*
 * Initialize the cache data structres
 */
void cache_init(struct tfs_sb_info *sbi)
{
        struct cache_struct *prev, *cur;
        char *data = cache_data;
        int i;
        
        cache_entries = CACHE_SIZE >> sbi->s_block_shift;
        if (cache_entries > MAX_CACHE_ENTRIES)
                cache_entries = MAX_CACHE_ENTRIES;
        
        cache_head.prev = &cache[cache_entries-1];
        cache_head.prev->next = &cache_head;
        prev = &cache_head;
        
        for (i = 0; i < cache_entries; i++) {
                cur = &cache[i];
                cur->block   = -1;
                cur->prev    = prev;
                prev->next   = cur;
                cur->data    = data;
                data += sbi->s_block_size;
                prev = cur++;
        }
}


/*
 * Check for a particular BLOCK in the block cache, 
 * and if it is already there, just do nothing and return;
 * otherwise load it and updata the relative cache
 * structre with data pointer.
 */
struct cache_struct* get_cache_block(struct tfs_sb_info *sbi, uint32_t block)
{
        struct cache_struct *head = &cache_head;
        struct cache_struct *last = head->prev;
        /* let's find it from the end, 'cause the endest is the freshest */
        struct cache_struct *cs = head->prev;
        int i;
         
        if (block < 0) {
                printk("ERR: we got a NEGTIVE block number that's not we want!\n");
                return NULL;
        }

    
        /* it's aleardy the freshest, so nothing we need do , just return it */
        if (cs->block == block)
                goto out;
        
        for (i = 0; i < cache_entries; i ++) {
                if (cs->block == block)
                        break;
                else
                        cs = cs->prev;
        }
    
        /* missed, so we need to load it */
        if (i == cache_entries) {        
                /* store it at the head of real cache */
                cs = head->next;        
                if (tfs_bread(sbi, block, cs->data))
			return NULL;
                cs->block = block;
	}
    
        /* remove cs from current position in list */
        cs->prev->next = cs->next;
        cs->next->prev = cs->prev;    
    
        /* add to just before head node */
        last->next = cs;
        cs->prev = last;
        head->prev = cs;
        cs->next = head;    
out:
        return cs;
}    


/**
 * Just print the sector, and according the LRU algorithm, 
 * Left most value is the most least secotr, and Right most 
 * value is the most Recent sector. I see it's a Left Right Used
 * (LRU) algorithm; Just kidding:)
 */
void print_cache(void)
{
        int i = 0;
        struct cache_struct *cs = &cache_head;
        for (; i < cache_entries; i++) {
                cs = cs->next;
                printk("%d(%p) \n", cs->block, cs->data);
        }

        printk("\n");
}
