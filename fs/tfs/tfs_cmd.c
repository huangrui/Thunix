#include <stdio.h>
#include <string.h>
#include <tfs.h>
#include <file.h>
#include <dirent.h>
#include <err.h>


static struct file * tfs_file_open(struct tfs_sb_info *sbi, char *filename, uint32_t flags)
{
	struct file *file;

	file = tfs_open(sbi, filename, flags);
	if (IS_ERR(file))
		return ERR_CAST(file);	

	return file;
}

void cd(struct tfs_sb_info *sbi, char *dst_dir)
{
	DIR *old = this_dir;

	if (*dst_dir) {
		this_dir = tfs_opendir(sbi, dst_dir);	
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
	
	TFS_DEBUG("CDed in '%s' with inode '%d'\n", dst_dir, this_dir->dd_dir->inode->i_ino);
}


void cat(struct tfs_sb_info *sbi, char *filename)
{
	struct file *file = tfs_file_open(sbi, filename, 0);
	char buf[1024];
	int bytes_read;

	if (IS_ERR(file)) {
		printk("open file %s error:%d\n", filename, PTR_ERR(file));
		return;
	}
	while ((bytes_read = tfs_read(file, buf, sizeof(buf))) > 0)
		printk("%s", buf);
	printk("\n");
}

void ls(struct tfs_sb_info *sbi, char *filename)
{
	DIR *dir = tfs_opendir(sbi, filename);
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

	while ((de = tfs_readdir(dir))) {
		printk("%6d\t %s\n", de->d_ino, de->d_name);
		free(de);
	}

	tfs_closedir(dir);
}

void mkdir(struct tfs_sb_info *sbi, char *filename)
{
	int err = tfs_mkdir(sbi, filename);
	if (err)
		printk("mkdir error: %d\n", err);
}

void rmdir(struct tfs_sb_info *sbi, char *filename)
{
	int err = tfs_rmdir(sbi, filename);
	if (err)
		printk("rmdir error: %d\n", err);
}

void rm(struct tfs_sb_info *sbi, char *filename)
{
	int err = tfs_unlink(sbi, filename);
	if (err)
		printk("rm file error: %d\n", err);
}

void touch (struct tfs_sb_info *sbi, char *filename)
{
	struct file *file = tfs_file_open(sbi, filename, LOOKUP_CREATE);
	if (IS_ERR(file)) printk("touch file error: %d\n", PTR_ERR(file));
}


void cp(struct tfs_sb_info *sbi, char *from, char *to)
{
	char buf[1024];
	int count;
	struct file *from_file;
	struct file *to_file;


	from_file = tfs_file_open(sbi, from, 0);
	if (IS_ERR(from_file)) {
		printk("open file %s error:%d\n", from, PTR_ERR(from_file));
		return;
	}
	to_file = tfs_file_open(sbi, to, LOOKUP_CREATE);
	if (IS_ERR(to_file)) {
		printk("open file %s error: %d\n", to, PTR_ERR(to_file));
		tfs_close(from_file);
		return;
	}
	
	while ((count = tfs_read(from_file, buf, sizeof(buf))) > 0) {
		count = tfs_write(to_file, buf, count);
		if (count < 0) 
			printk("write error!\n");
		else
			printk("  == %d bytes written!\n", count);
	}

	tfs_close(from_file);
	tfs_close(to_file);
}

