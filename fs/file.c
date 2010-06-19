#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <fs.h>
#include <err.h>

#define MAX_FD  32
uint8_t fds[MAX_FD] = {0,0,0};
struct file *files[MAX_FD] = {NULL, };

static int get_unused_fd(void)
{
	int i;

	for (i = 0; i < MAX_FD; i++) {
		if (fds[i] == 0)
			break;
	}

	if (i < MAX_FD)
		fds[i] = 1;

	return i;
}

int sys_open(const char *pathname, uint32_t flags)
{
	struct inode *inode = namei(pathname, flags);
	struct file *file;
	int fd;

	if (IS_ERR_OR_NULL(inode))
		return inode ? PTR_ERR(inode) : -ENOENT;

	FS_DEBUG("%s: namei returned successfully!\n", __FUNCTION__);
	
	file = malloc(sizeof(*file));
	if (!file)
		return -ENOMEM;
	file->inode  = inode;
	file->fs     = inode->i_fs;
	file->offset = 0;
	file->f_op   = inode->i_fop;

	fd = get_unused_fd();
	if (fd >= MAX_FD) {
		printk("You opened two much files!\n");
		free(file);
		return -EINVAL;
	}
	files[fd] = file;
	
	return fd;
}

static inline struct file *fget(int fd)
{
	return files[fd];
}

int sys_read(int fd, void *buf, int size)
{
	struct file *file;
	struct inode *inode;
	int blocks;
	int bytes_need;
	int bytes_read;

	if (fd < 0 || fd > MAX_FD)
		return -EBADF;
	
	file = fget(fd);
	if (!file)
		return -EBADF;

	inode      = file->inode;
	bytes_need = MIN(size, inode->i_size - file->offset);
	blocks 	   = (bytes_need + inode->i_blksize - 1) >> file->fs->block_shift;

	bytes_read = file->f_op->read(file, buf, blocks);
	bytes_read = MIN(bytes_need, bytes_read);
	if (bytes_read > 0) {
		memmove(buf, buf + (file->offset & (inode->i_blksize - 1)), bytes_read);
		file->offset += bytes_read;
	}

	return bytes_read;
}

int sys_write(int fd, void *buf, int size)
{
	struct file *file;
	int blocks;
	int bytes_written;

	if (fd < 0 || fd > MAX_FD)
		return -EBADF;
	file = fget(fd);
	if (!file)
		return -EBADF;

	blocks = (size + file->inode->i_blksize - 1) >> file->fs->block_shift;

	bytes_written = file->f_op->write(file, buf, blocks);
	bytes_written = MIN(size, bytes_written);
	if (bytes_written > 0)
		file->offset += bytes_written;
	
	return bytes_written;
}

int sys_close(int fd)
{
	struct file *file;
	
	if (fd < 0 || fd > MAX_FD)
		return -EBADF;

	file = fget(fd);
	if (!file)
		return -EBADF;

	free_inode(file->inode);
	fds[fd] = 0;
	files[fd] = NULL;
	free(file);
	return 0;
}
	
int sys_lseek(int fd, int off, int mode)
{
	struct file *file;
	
	if (fd < 0 || fd > MAX_FD)
		return -EBADF;

	file = fget(fd);
	if (!file)
		return -EBADF;

	if (mode == SEEK_CUR)
		file->offset += off;
	else if (mode == SEEK_END)
		file->offset = file->inode->i_size + off;
	else if (mode == SEEK_SET)
		file->offset = off;
	else
		return -EINVAL;

	return file->offset;
}
