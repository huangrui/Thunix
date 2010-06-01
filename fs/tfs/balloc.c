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

#if 0  /* the deubg part */
#include <stdlib.h>
int main(int argc, char *argv[])
{
	int i;
	int block;
	struct tfs_sb_info *sbi;
	char *fs = argv[1];
	char *command = argv[2];
	char **blocks = argv + 3;
	int count = argc - 3;

	if (argc < 4) {
		printk("Usage: balloc tfs.img command blocks...\n");
		printk("       alloc, to alloc blocks\n");
		printk("       feee,  to free blocks\n");
		return 1;
	}

	sbi = tfs_mount_by_fsname(fs);
	if (!sbi) {
		printk("tfs mount failed!\n");
		return 1;
	}

	if (strcmp(command, "alloc") == 0) {
		for (i = 0; i <  count; i++) {
			block = atoi(blocks[i]);
			printk("trying to alloc block %u\n", block);
			block = tfs_alloc_block(sbi, block);
			printk("block number: %u allocated\n", block);
		}
	} else if (strcmp(command, "free") == 0) {
		for (i = 0; i < count; i++) {
			block = atoi(blocks[i]);
			printk("trying to free block %u\n", block);
			block = tfs_free_block(sbi, block);
			printk("block number: %u freed\n", block);
		}
	} else {
		printk("Unknown command!\n");
	}

	return 0;
}
#endif
