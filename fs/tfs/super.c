#include <stdio.h>
#include <malloc.h>
#include <fs.h>
#include <tfs.h>
#include <fd.h>


struct tfs_sb_info * tfs_mount(void)
{
	struct tfs_sb_info *sbi;
	struct tfs_super_block sb;
	
	/* Read the sector */
	floppy_read(TFS_SB_SECTOR, &sb);
	if (sb.s_magic != TFS_MAGIC) {
		printk("Trying to mount not tfs file system!\n");
		return NULL;
	}

	sbi = malloc(sizeof(*sbi));
	if (!sbi) {
		printk("Malloc from tfs sbi object failed!\n");
		return NULL;
	}

	sbi->s_block_shift       = sb.s_block_shift;
	sbi->s_block_size        = 1 << sbi->s_block_shift;
	sbi->s_blocks_count      = sb.s_blocks_count;
	sbi->s_inodes_count      = sb.s_inodes_count;
	sbi->s_free_blocks_count = sb.s_free_blocks_count;
	sbi->s_free_inodes_count = sb.s_free_inodes_count;


	sbi->s_inode_bitmap = sb.s_inode_bitmap;
	sbi->s_block_bitmap = sb.s_block_bitmap;
	sbi->s_inode_table  = sb.s_inode_table;
	sbi->s_data_area    = sb.s_data_area;

	sbi->s_inode_bitmap_count = sb.s_block_bitmap - sb.s_inode_bitmap;
	sbi->s_block_bitmap_count = sb.s_inode_table - sb.s_block_bitmap;
	sbi->s_inode_table_count  = sb.s_data_area - sb.s_inode_table; 


	sbi->s_offset = sb.s_offset;
	
	sbi->s_inodes_per_block = sbi->s_block_size / sizeof(struct tfs_inode);

	return sbi;
}
