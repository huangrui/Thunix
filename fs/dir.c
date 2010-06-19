#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fs.h>
#include <dirent.h>
#include <err.h>

/* for relative path searching */
DIR *this_dir;

DIR *opendir(const char *path)
{
	struct inode *inode;
	DIR *dir;
	
	inode = namei(path, 0);
	if (IS_ERR_OR_NULL(inode))
		return inode ? ERR_CAST(inode) : ERR_PTR(-ENOENT);

	dir = malloc(sizeof(*dir));
	if (!dir)
		return ERR_PTR(-ENOMEM);
	dir->dd_dir = malloc(sizeof(struct file));
	if (!dir->dd_dir) {
		free(dir);
		return ERR_PTR(-ENOMEM);
	}
	dir->dd_dir->inode  = inode;
	dir->dd_dir->fs     = inode->i_fs;
	dir->dd_dir->offset = 0;
	dir->dd_dir->f_op   = inode->i_fop;
	
	return dir;
}
