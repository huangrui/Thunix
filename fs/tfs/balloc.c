#include <stdio.h>
#include <malloc.h>
#include <bitopts.h>
#include <tfs.h>
#include <cache.h>
#include <errno.h>

/* 
 * cache the block bitmap data
 */
static struct cache_struct * tfs_read_block_bitmap(struct tfs_sb_info *sbi)
{
	struct cache_struct *cs;

	cs = get_cache_block(sbi, sbi->s_block_bitmap);
	
	return cs;
}

/*
 * Free a block, return 0 if successed.
 */
int tfs_free_block(struct tfs_sb_info *sbi, uint32_t block)
{
	struct cache_struct *cs = tfs_read_block_bitmap(sbi);
	void * bitmap = cs ? cs->data : NULL;

	if (!bitmap) 
		return -EIO;
	if (clear_bit(bitmap, block) == 0)
		printk("ERROR: trying to free an free block!\n");
	else
		tfs_bwrite(sbi, sbi->s_block_bitmap, bitmap);

	return 0;
}

/*
 * Return the allocated 'block' number if successed.
 */
int tfs_alloc_block(struct tfs_sb_info *sbi, uint32_t block)
{
	struct cache_struct *cs = tfs_read_block_bitmap(sbi);
	void * bitmap = cs ? cs->data : NULL;
	
	if (!bitmap)
		return -EIO;

	/* try the target first */
	if (test_bit(bitmap, block) != 0) 
		block = find_first_zero(bitmap, bitmap + sbi->s_block_size);
	if (block != -1) {
		set_bit(bitmap, block);
		tfs_bwrite(sbi, sbi->s_block_bitmap, bitmap);
	} else {
		return -ENOSPC;
	}

	return block;
}
