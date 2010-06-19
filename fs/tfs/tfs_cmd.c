#include <stdio.h>
#include <string.h>
#include <fs.h>
#include <tfs.h>
#include <dirent.h>
#include <err.h>

void cd(char *dst_dir)
{
	DIR *old = this_dir;

	if (*dst_dir) {
		this_dir = opendir(dst_dir);
		if (IS_ERR(this_dir)) {
			/*
			 * FIXME: add error code handler here
			 */
			printk("cd: %s: no such directory\n", dst_dir);
			this_dir = old;
			return;
		}

		if (this_dir->dd_dir->inode->i_mode != TFS_DIR) {
			printk("cd: %s: is not a directory\n", dst_dir);
			tfs_closedir(this_dir);
			this_dir = old;
		} else {
			tfs_closedir(old);
		}
	}
	
	root_fs()->pwd = this_dir->dd_dir->inode;
	TFS_DEBUG("CDed in '%s' with inode '%d'\n", dst_dir, this_dir->dd_dir->inode->i_ino);
}


void cat(char *filename)
{
	int fd = sys_open(filename, 0);
	char buf[1024];
	int bytes_read;

	if (fd < 0) {
		printk("open file %s error:%d\n", filename, fd);
		return;
	}

	while ((bytes_read = sys_read(fd, buf, sizeof(buf))) > 0)
		printk("%s", buf);
	printk("\n");
}

void ls(char *filename)
{
	DIR *dir = opendir(filename);
	struct dirent *de;
	
	if (IS_ERR(dir)) {
		/*
		 * FIXME: handle error code here
		 */
		printk("open dir %s failed(%d)!\n", filename, PTR_ERR(dir));
		return;
	}

	if (dir->dd_dir->inode->i_mode == TFS_FILE) {
		printk("%d\t %s\n", dir->dd_dir->inode->i_ino, filename);
		tfs_closedir(dir);
		return;
	}

	while ((de = tfs_readdir(dir->dd_dir))) {
		printk("%6d\t %s\n", de->d_ino, de->d_name);
		free(de);
	}

	tfs_closedir(dir);
}

void mkdir(char *filename)
{
	int err = sys_mkdir(filename);
	if (err)
		printk("mkdir error: %d\n", err);
}

void rmdir(char *filename)
{
	int err = sys_rmdir(filename);
	if (err)
		printk("rmdir error: %d\n", err);
}

void rm(char *filename)
{
	int err = sys_unlink(filename);
	if (err)
		printk("rm file error: %d\n", err);
}

void touch (char *filename)
{
	int fd = sys_open(filename, LOOKUP_CREATE);
	if (fd < 0)
		printk("touch file error: %d\n", fd);
}


void cp(char *from, char *to)
{
	char buf[1024];
	int count;
	int from_fd, to_fd;


	from_fd = sys_open(from, 0);
	if (from_fd < 0) {
		printk("open file %s error:%d\n", from, from_fd);
		return;
	}
	to_fd = sys_open(to, LOOKUP_CREATE);
	if (to_fd < 0) {
		printk("open file %s error: %d\n", to, to_fd);
		sys_close(from_fd);
		return;
	}

	TFS_DEBUG("from_fd: %d, to_fd: %d\n", from_fd, to_fd);
	
	while ((count = sys_read(from_fd, buf, sizeof(buf))) > 0) {
		TFS_DEBUG("  == %d bytes readn!\n", count);
		count = sys_write(to_fd, buf, count);
		if (count < 0) 
			TFS_DEBUG("write error!\n");
		else
			TFS_DEBUG("  == %d bytes written!\n", count);
	}

	sys_close(from_fd);
	sys_close(to_fd);
}

