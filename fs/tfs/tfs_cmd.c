#include <stdio.h>
#include <string.h>
#include <tfs.h>
#include <file.h>
#include <dirent.h>


static struct file * tfs_file_open(struct tfs_sb_info *sbi, char *filename, uint32_t flags)
{
	struct file *file;

	file = tfs_open(sbi, filename, flags);
	if (!file) {
		printk("tfs_open: open file %s error!\n", filename);
		return NULL;
	}

	return file;
}

void cd(struct tfs_sb_info *sbi, char *dst_dir)
{
	DIR *old = this_dir;

	if (*dst_dir) {
		this_dir = tfs_opendir(sbi, dst_dir);	
		if (!this_dir) {
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

	if (!file)
		return;
	while ((bytes_read = tfs_read(file, buf, sizeof(buf))) > 0)
		printk("%s", buf);
	printk("\n");
}

void ls(struct tfs_sb_info *sbi, char *filename)
{
	DIR *dir = tfs_opendir(sbi, filename);
	struct dirent *de;
	
	if (!dir) {
		printk("open dir %s failed!\n", filename);
		return;
	}

	if (dir->dd_dir->inode->i_mode == TFS_FILE) {
		printk("%d\t %s\n", dir->dd_dir->inode->i_ino, filename);
		tfs_closedir(dir);
		return;
	}

	while (de = tfs_readdir(dir)) {
		printk("%6d\t %s\n", de->d_ino, de->d_name);
		free(de);
	}

	tfs_closedir(dir);
}

void mkdir(struct tfs_sb_info *sbi, char *filename)
{
	tfs_mkdir(sbi, filename);
}

void rmdir(struct tfs_sb_info *sbi, char *filename)
{
	tfs_rmdir(sbi, filename);
}

void rm(struct tfs_sb_info *sbi, char *filename)
{
	tfs_unlink(sbi, filename);
}

void touch (struct tfs_sb_info *sbi, char *filename)
{
	struct file *file = tfs_file_open(sbi, filename, LOOKUP_CREATE);
}


void cp(struct tfs_sb_info *sbi, char *from, char *to)
{
	char buf[1024];
	int count;
	struct file *from_file;
	struct file *to_file;


	from_file = tfs_file_open(sbi, from, 0);
	if (!from_file)
		return;
	to_file = tfs_file_open(sbi, to, LOOKUP_CREATE);
	if (!to_file) {
		tfs_close(from_file);
		return;
	}
	
	while ((count = tfs_read(from_file, buf, sizeof(buf))) > 0) {
		count = tfs_write(to_file, buf, count);
		if (count == -1) 
			printk("write error!\n");
		else
			printk("  == %d bytes written!\n", count);
	}

	tfs_close(from_file);
	tfs_close(to_file);
}

