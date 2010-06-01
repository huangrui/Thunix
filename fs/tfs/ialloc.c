#include <stdio.h>
#include <malloc.h>
#include <bitopts.h>
#include <tfs.h>


static void * tfs_read_inode_bitmap(struct tfs_sb_info *sbi)
{
	char *buf = malloc(sbi->s_block_size);

	tfs_bread(sbi, sbi->s_inode_bitmap, buf);

	return buf;
}

/*
 * free an inode, return -1 if failed, or return 9
 */
int tfs_free_inode(struct tfs_sb_info *sbi, int inr)
{
	char *bitmap = tfs_read_inode_bitmap(sbi);

	/* inode number count from 1 */
	if (clear_bit(bitmap, inr - 1) == 0) {
		printk("ERROR: trying to free an unallocated inode!\n");
		free(bitmap);
		return -1;
	}

	tfs_bwrite(sbi, sbi->s_inode_bitmap, bitmap);
	free(bitmap);
	return 0;
}

int tfs_alloc_inode(struct tfs_sb_info *sbi, int inr)
{
	char *bitmap;

	if (inr < 0) {
		printk("ERROR: trying to alloc a negtive inode!\n");
		return -1;
	}

	bitmap = tfs_read_inode_bitmap(sbi);
	/* try the target first */
	if (test_bit(bitmap, inr - 1) != 0)
		inr = find_first_zero(bitmap, bitmap + sbi->s_block_size) + 1;
	if (inr != -1) {
		set_bit(bitmap, inr - 1);
		tfs_bwrite(sbi, sbi->s_inode_bitmap, bitmap);
	}

	free(bitmap);
	return inr;
}

#if 0  /* the deubg part */
#include <stdlib.h>
int main(int argc, char *argv[])
{
	int i;
	int inr;
	struct tfs_sb_info *sbi;
	char *fs = argv[1];
	char *command = argv[2];
	char **inodes = argv + 3;
	int count = argc - 3;

	if (argc < 4) {
		printk("Usage: ialloc tfs.img command inodes...\n");
		printk("       alloc, to alloc inodes\n");
		printk("       feee,  to free inodes\n");
		return 1;
	}

	sbi = tfs_mount_by_fsname(fs);
	if (!sbi) {
		printk("tfs mount failed!\n");
		return 1;
	}

	if (strcmp(command, "alloc") == 0) {
		for (i = 0; i <  count; i++) {
			inr = atoi(inodes[i]);
			printk("trying to alloc inode %u\n", inr);
			inr = tfs_alloc_inode(sbi, inr);
			printk("inode number: %u allocated\n", inr);
		}
	} else if (strcmp(command, "free") == 0) {
		for (i = 0; i < count; i++) {
			inr = atoi(inodes[i]);
			printk("trying to free inode %u\n", inr);
			inr = tfs_free_inode(sbi, inr);
			printk("inode number: %d freed\n", inr);
		}
	} else {
		printk("Unknown command!\n");
	}

	return 0;
}
#endif
