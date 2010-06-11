#include <stdio.h>
#include <stdint.h>
#include <tfs.h>
#include <fd.h>

int tfs_bread(struct tfs_sb_info *sbi, uint32_t block, void *buf)
{
	uint32_t sector = (block << (sbi->s_block_shift - 9)) + sbi->s_offset + TFS_FS_SECTOR;

	return floppy_reads(sector, buf, 1 << (sbi->s_block_shift - 9));
}

int tfs_bwrite(struct tfs_sb_info *sbi, uint32_t block, void *buf)
{
	uint32_t sector = (block << (sbi->s_block_shift - 9)) + sbi->s_offset + TFS_FS_SECTOR;

	return floppy_writes(sector, buf, 1 << (sbi->s_block_shift - 9));
}
