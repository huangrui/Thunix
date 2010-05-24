/*
 *  thunix/fs/balloc.c
 *  it contains the blocks allocation and deallocation routines
 */

#include <fs_ext2.h>

#define in_range(b, first, len)	((b) >= (first) && (b) <= (first) + (len) - 1)

struct ext2_group_desc * ext2_get_group_desc(unsigned int block_group)
{
	struct ext2_group_desc * desc;
        /*	struct ext2_sb_info *sbi = EXT2_SBI();

	
	if (block_group >= sbi->s_groups_count) {
		ext2_error ("ext2_get_group_desc",
			    "block_group >= groups_count - "
			    "block_group = %d, groups_count = %lu",
			    block_group, sbi->s_groups_count);

		return NULL;
	}
        */
		
	desc = (struct ext2_group_desc *) EXT2_GROUP_DESC_BUFFER;

	return desc + block_group;
}



/*
 * Read the bitmap for a given block_group, reading into the specified 
 * slot in the superblock's bitmap cache.
 *
 */
void * ext2_read_block_bitmap(unsigned int block_group)
{
	struct ext2_group_desc * desc;
	void * bitmap = NULL;
	
	desc = ext2_get_group_desc (block_group);
	if (!desc)
		goto error_out;
	bitmap = (void *)(EXT2_BITMAP_BUFFER + (block_group << 1) * EXT2_BLOCK_SIZE);
	if (!bitmap)
		ext2_error ("read_block_bitmap",
			    "Cannot read block bitmap - "
			    "block_group = %d, block_bitmap = %u",
			    block_group, desc->bg_block_bitmap);
error_out:
	return bitmap;
}


/* free the goal block, clean it */
void ext2_free_block(unsigned int block)
{
	unsigned int block_group;
	unsigned int bit;
	struct ext2_group_desc *desc;
        struct ext2_sb_info * sbi = EXT2_SBI();
	void * bitmap;

	block_group = ext2_get_group_num (block, BLOCK);
	bit = ext2_get_group_offset (block, BLOCK);

	desc = ext2_get_group_desc (block_group);
	bitmap = ext2_read_block_bitmap (block_group);

	
	if ( !ext2_clear_bit(bitmap, bit) ) 
		ext2_error("bit %d (%d) alread cleard", bit, block_group);

	desc->bg_free_blocks_count ++;
	sbi->s_free_blocks_count ++;
}

/*
 * get the specified goal block
 * if alread alloced find it by looking forward 
 */
int ext2_grab_block( char *bitmap, unsigned int goal)
{
        if ( !ext2_test_bit(bitmap, goal) )
                goto got_block;

        /* else looking forward */
        goal = find_first_zero (bitmap, goal + 1, EXT2_BLOCKS_PER_GROUP);
 got_block:
        return goal;
}

/* alloc a new block */
int ext2_alloc_block ( unsigned int goal)
{
        unsigned int block;
        unsigned int block_group;
        unsigned int bit;
        struct ext2_group_desc *desc;
        struct ext2_sb_info * sbi = EXT2_SBI();
        void *bitmap;

        block_group = ext2_get_group_num (goal, BLOCK);
        bit = ext2_get_group_offset (goal, BLOCK);
        
        bitmap = ext2_read_block_bitmap (block_group);
        block = ext2_grab_block (bitmap, bit);

        if ( !block)
                ext2_error ("no free blocks any more");

        desc = ext2_get_group_desc (block_group);
        desc->bg_free_blocks_count --;
        sbi->s_free_blocks_count --;
        ext2_set_bit (bitmap, block);

        return block;
}
                
