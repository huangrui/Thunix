#include <stdio.h>
#include <string.h>
#include <err.h>
#include <malloc.h>
#include <fs.h>

#define current_time  0

void free_inode(struct inode *inode)
{
	/* Can not free current working directory */
	if (inode == inode->i_fs->pwd)
		return;
	if(inode->i_data)
		free(inode->i_data);
	free(inode);
}

struct inode *new_inode(int mode, int data_size)
{
	struct inode *inode;

	inode = malloc(sizeof(*inode));
	if (!inode)
		return NULL;
	memset(inode, 0, sizeof(*inode));

	inode->i_data = malloc(data_size);
	if (!inode->i_data) {
		free(inode);
		return NULL;
	}
	memset(inode->i_data, 0, data_size);

	inode->i_mode = mode;
	
	inode->i_atime = current_time;
	inode->i_ctime = current_time;
	inode->i_mtime = current_time;

	inode->i_fs    = root_fs();

	return inode;
}

/* 
 * get the inode for dname under given dir
 *
 * Return NULL if not found, or error if errors happened,
 * or return inode if found.
 *
 * The current implementation has no cache, it would be 
 * better to add one.
 */
struct inode * iget(const char *dname, struct inode *dir)
{
	struct inode * inode = ERR_PTR(-ENOSYS);

	if (dir->i_op && dir->i_op->iget)
		inode = dir->i_op->iget(dname, dir);

	return inode;
}


int sys_mknod(const char *pathname, int mode)
{
	struct inode *dir = namei_parent(pathname);
	struct inode *inode;
	int err;

	if (IS_ERR(dir))
		return PTR_ERR(dir);

	err = -ENOSYS;
	if (dir->i_op && dir->i_op->mknod) {
		inode = dir->i_op->mknod(dir, get_base_name(pathname), mode);
		if (IS_ERR(inode)) 
			err = PTR_ERR(inode);
		else
			free_inode(inode);
	}

	free_inode(dir);
	return err;
}
