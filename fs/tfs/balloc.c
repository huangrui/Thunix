#include <stdio.h>
#include <malloc.h>
#include <bitopts.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>
#include <err.h>
#include <kernel.h>

/* 
 * cache the block bitmap data
 */
static struct cache_struct * tfs_read_block_bitmap(struct tfs_sb_info *sbi)
{
	struct fs * fs = container_of((void *)sbi, struct fs, sb);
	struct cache_struct *cs = get_cache_block(fs, sbi->s_block_bitmap);
	
	return cs;
}

/*
 * Free a block, return 0 if successed.
 */
int tfs_free_block(struct tfs_sb_info *sbi, uint32_t block)
{
	struct cache_struct *cs = tfs_read_block_bitmap(sbi);
	void * bitmap = cs ? cs->data : NULL;
	int res = 0;

	if (!bitmap) 
		return -EIO;
	if (clear_bit(bitmap, block) == 0)
		printk("ERROR: trying to free an free block!\n");
	else
		res = tfs_bwrite(sbi, sbi->s_block_bitmap, bitmap);

	return res;
}

/*
 * Return the allocated 'block' number if successed.
 */
int tfs_alloc_block(struct tfs_sb_info *sbi, uint32_t block)
{
	struct cache_struct *cs = tfs_read_block_bitmap(sbi);
	void * bitmap = cs ? cs->data : NULL;
	int res = 0;
	
	if (!bitmap)
		return -EIO;

	/* try the target first */
	if (test_bit(bitmap, block) != 0) 
		block = find_first_zero(bitmap, bitmap + sbi->s_block_size);
	if (block != -1) {
		set_bit(bitmap, block);
		res = tfs_bwrite(sbi, sbi->s_block_bitmap, bitmap);
		if (res < 0)
			return res;
	} else {
		return -ENOSPC;
	}

	return block;
}
