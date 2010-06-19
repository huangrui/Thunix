#include <stdio.h>
#include <malloc.h>
#include <bitopts.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>
#include <err.h>

/*
 * Cache the inode bitmap block
 *
 * NOTE: this may be failed!
 */
static inline struct cache_struct * tfs_read_inode_bitmap(struct tfs_sb_info *sbi)
{
	struct cache_struct *cs = get_cache_block(sbi, sbi->s_inode_bitmap);

	return cs;
}

/*
 * free an inode, return -1 if failed, or return 9
 */
int tfs_free_inode(struct tfs_sb_info *sbi, int inr)
{
	struct cache_struct *cs = tfs_read_inode_bitmap(sbi);
	void *bitmap = cs ? cs->data : NULL;
	int res = 0;
	
	if (!bitmap)
		return -EIO;

	/* inode number count from 1 */
	if (clear_bit(bitmap, inr - 1) == 0)
		printk("ERROR: trying to free an unallocated inode!\n");
	else
		res = tfs_bwrite(sbi, sbi->s_inode_bitmap, bitmap);

	return res;
}

int tfs_alloc_inode(struct tfs_sb_info *sbi, int inr)
{
	struct cache_struct *cs;
	void *bitmap;
	int err = 0;

	if (inr < 1) {
		printk("ERROR: trying to alloc a negtive inode!\n");
		return -EINVAL;
	}
	TFS_DEBUG("allocating inr: %d\n", inr);
	inr--;

	cs = tfs_read_inode_bitmap(sbi);
	bitmap = cs ? cs->data : NULL;
	if (!bitmap)
		return -EIO;
	/* try the target first */
	if (test_bit(bitmap, inr) == 1) {
		inr = find_first_zero(bitmap, bitmap + sbi->s_block_size);
	}
	if (inr >= 0) {
		set_bit(bitmap, inr);
		err = tfs_bwrite(sbi, sbi->s_inode_bitmap, bitmap);
		if (err) {
			clear_bit(bitmap, inr);
			return err;
		}
	} else {
		return -ENOSPC;
	}

	TFS_DEBUG("and retured: %d\n", inr + 1);

	return inr + 1;
}
