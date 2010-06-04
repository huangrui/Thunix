#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "tfs.h"
#include "cache.h"
#include "file.h"


struct file *tfs_open(struct tfs_sb_info *sbi, const char *filename, uint32_t flag)
{
	struct inode *inode;
	struct file *file;

	inode = tfs_namei(sbi, filename, flag);
	if (!inode) {
		printk("ERROR: open file: %s falied!\n", filename);
		return NULL;
	}	

	file = malloc(sizeof(*file));
	if (!file) {
		printk("malloc file structure error!\n");
		free_inode(inode);
		return NULL;
	}

	file->sbi    = sbi;
	file->inode  = inode;
	file->offset = 0;

	return file;
}

uint32_t fstk_lseek(struct file *file, uint32_t off, int mode)
{
        if (mode == SEEK_CUR)
                file->offset += off;
        else if (mode == SEEK_END)
                file->offset = file->inode->i_size + off;
        else if (mode == SEEK_SET)
                file->offset = off;
        else
                file->offset = -1;
        return file->offset;
}

int tfs_read(struct file *file, void *buf, uint32_t count)
{
	struct tfs_sb_info *sbi = file->sbi;
	int blocks = roundup(count, sbi->s_block_size);
	int index  = file->offset >> sbi->s_block_shift;
	int block  = file->inode->i_data[index++];
	int bufoff = file->offset & (sbi->s_block_size - 1);
	int bytes_read = 0;

	if (!blocks)
		return -1;
	if (!block)
		return -1;
	if (file->offset >= file->inode->i_size)
		return -1;
	tfs_bread(sbi, block, buf);
	bytes_read = sbi->s_block_size - bufoff;
	memcpy(buf, buf + bufoff, bytes_read);
	buf          += bytes_read;
	file->offset += bytes_read;
	blocks--;

	while (blocks--) {
		block = file->inode->i_data[index++];
		if (!block)
			break;
		tfs_bread(sbi, block, buf);
		bytes_read   += sbi->s_block_size;
		file->offset += sbi->s_block_size;
		buf          += sbi->s_block_size;
	}

	return bytes_read;
}


#define min(a, b) ((a) < (b) ? a : b)
int tfs_write(struct file *file, void *buf, uint32_t count)
{
	struct tfs_sb_info *sbi = file->sbi;
	struct cache_struct *cs;
	int blocks = roundup(count, sbi->s_block_size);
	int index  = file->offset >> sbi->s_block_shift;
	int block;
	int bufoff = file->offset & (sbi->s_block_size - 1);
	int bytes_written = 0;


	if (!blocks)
		return -1;

	block  = tfs_bmap(file->inode, index++);
	if (!block) {
		if (index - 1 < TFS_N_BLOCKS) {
			block = tfs_alloc_block(sbi, sbi->s_data_area);
			if (block == -1) {
				printk("allocating block for new file faile! OUT OF SPACE!\n");
				return -1;
			}
			file->inode->i_data[index - 1] = block;
		} else  {
			/* file too big */
			return -1;
		}
	}
	cs = get_cache_block(sbi, block);
	bytes_written = min(sbi->s_block_size, count) - bufoff;
	memcpy(cs->data + bufoff, buf, bytes_written);
	buf	     += bytes_written;
	file->offset += bytes_written;
	count        -= bytes_written;
	file->inode->i_size += bytes_written;
	/* write back to disk */
	tfs_bwrite(sbi, block, cs->data);
	blocks--;

	while (blocks--) {
		int bytes_need;
		block = tfs_bmap(file->inode, index++);
		if (!block) {
			if (index - 1 < TFS_N_BLOCKS) {
				block = tfs_alloc_block(sbi, sbi->s_data_area);
				if (block == -1) {
					printk("allocating block for new file faile: out of space!\n");
					goto err;
				}
				file->inode->i_data[index - 1] = block;
			} else { 
				/* fle too big */
				goto err;
			}
		}
		bytes_need = min(sbi->s_block_size, count);
		cs = get_cache_block(sbi, block);
		memcpy(cs->data, buf, bytes_need);
		bytes_written += bytes_need;
		file->offset  += bytes_need;
		buf           += bytes_need;
		file->inode->i_size += bytes_need;
		tfs_bwrite(sbi, block, cs->data);
	}

done:
	tfs_iwrite(sbi, file->inode);
	
	return bytes_written;

err:
	bytes_written = -1;
	goto done;
}

void tfs_close(struct file *file)
{
	if (file) {
		free_inode(file->inode);
		free(file);
	}
}

