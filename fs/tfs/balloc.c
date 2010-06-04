#include <stdio.h>
#include <malloc.h>
#include <bitopts.h>
#include <tfs.h>

static void * tfs_read_block_bitmap(struct tfs_sb_info *sbi)
{
	char *buf = malloc(sbi->s_block_size);	

	tfs_bread(sbi, sbi->s_block_bitmap, buf);
	
	return buf;
}

/*
 * Free a block, return -1 if failed, or return 0
 */
int tfs_free_block(struct tfs_sb_info *sbi, uint32_t block)
{
	char *bitmap = tfs_read_block_bitmap(sbi);

	if (clear_bit(bitmap, block) == 0) {
		printk("ERROR: trying to free an free block!\n");
		free(bitmap);
		return -1;
	}

	tfs_bwrite(sbi, sbi->s_block_bitmap, bitmap);
	free(bitmap);
	return block;
}

int tfs_alloc_block(struct tfs_sb_info *sbi, uint32_t block)
{
	char *bitmap = tfs_read_block_bitmap(sbi);
	
	/* try the target first */
	if (test_bit(bitmap, block) != 0) 
		block = find_first_zero(bitmap, bitmap + sbi->s_block_size);
	if (block != -1) {
		set_bit(bitmap, block);
		tfs_bwrite(sbi, sbi->s_block_bitmap, bitmap);
	}

	free(bitmap);
	return block;
}
