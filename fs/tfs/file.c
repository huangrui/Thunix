#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <err.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>
#include <dirent.h>

int tfs_read(struct file *file, void *buf, int blocks)
{
	struct tfs_sb_info *sbi = TFS_SBI(file->fs);
	struct cache_struct *cs;
	uint32_t block;
	int index  = file->offset >> sbi->s_block_shift;
	int bytes_read = 0;

	TFS_DEBUG("rbuf: %p, blocks: %d, offset: %d\n", buf, blocks, file->offset);

	if (!blocks)
		return 0;
	if (file->offset >= file->inode->i_size)
		return -1;

	while (blocks--) {
		block = tfs_bmap(file->inode, index++);
		if (!block)
			break;
		cs = get_cache_block(sbi, block);
		if (!cs)
			return -EIO;
		memcpy(buf, cs->data, sbi->s_block_size);
		bytes_read   += sbi->s_block_size;
		buf          += sbi->s_block_size;
	}

	return bytes_read;
}


#define min(a, b) ((a) < (b) ? a : b)
int tfs_write(struct file *file, void *buf, int blocks)
{
	struct tfs_sb_info *sbi = TFS_SBI(file->fs);
	struct cache_struct *cs;
	int index  = file->offset >> sbi->s_block_shift;
	int block;
	int bufoff = file->offset & (sbi->s_block_size - 1);
	int bytes_written = 0;

	TFS_DEBUG("wbuf: %p, blocks: %d, offset: %d\n", buf, blocks, file->offset);

	if (!blocks)
		return 0;

	block  = tfs_bmap(file->inode, index++);
	if (!block) {
		if (index - 1 < TFS_N_BLOCKS) {
			block = tfs_alloc_block(sbi, sbi->s_data_area);
			if (block < 0)
				return -ENOSPC;
			file->inode->i_data[index - 1] = block;
		} else  {
			/* file too big */
			return -EFBIG;
		}
	}
	cs = get_cache_block(sbi, block);
	if (!cs)
		return -EIO;
	bytes_written = sbi->s_block_size - bufoff;
	memmove(cs->data + bufoff, buf, bytes_written);
	buf	     += bytes_written;
	file->inode->i_size += MAX(0, file->offset + bytes_written - file->inode->i_size);
	/* write back to disk */
	if (tfs_bwrite(sbi, block, cs->data))
		return -EIO;
		
	blocks--;
	while (blocks--) {
		int bytes_need;
		block = tfs_bmap(file->inode, index++);
		if (!block) {
			if (index - 1 < TFS_N_BLOCKS) {
				block = tfs_alloc_block(sbi, sbi->s_data_area);
				if (block < 0)
					return -ENOSPC;
				file->inode->i_data[index - 1] = block;
			} else { 
				/* fle too big */
				return -EFBIG;
			}
		}
		bytes_need = sbi->s_block_size;
		cs = get_cache_block(sbi, block);
		if (!cs)
			return -EIO;
		memcpy(cs->data, buf, bytes_need);
		bytes_written += bytes_need;
		buf           += bytes_need;
		file->inode->i_size += MAX(0, file->offset + bytes_written - file->inode->i_size);
		if (tfs_bwrite(sbi, block, cs->data))
			return -EIO;
	}

	if (tfs_iwrite(file->inode))
		return -EIO;
	
	return bytes_written;
}

void tfs_close(struct file *file)
{
	if (file) {
		free_inode(file->inode);
		free(file);
	}
}

struct file_operations tfs_file_ops = {
	.read 		= tfs_read,
	.write 		= tfs_write,
	.close 		= tfs_close,
	.readdir 	= tfs_readdir
};
