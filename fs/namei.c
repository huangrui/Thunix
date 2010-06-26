#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <thunix.h>
#include <fs.h>
#include <err.h>

/*
 * Looking for an inode with the given path
 *
 * returns NULL for -ENOENT, ERROR for errors happend
 * and inode for finding responding file or directory.
 */
struct inode * namei(const char *name, uint32_t flag)
{
	struct inode *inode;
	struct inode *parent;
	char part[256];
	char *p;

	FS_DEBUG("trying to open path: %s\n", name);
	if (*name == '/') {
		inode = root_fs()->root;
		if (IS_ERR(inode))
			panic("namei: Read root inode error!\n");
		while (*name == '/')
			name++;
	} else {
		inode = root_fs()->pwd;
		FS_DEBUG("pwd->inode: %d\n", inode->i_ino);
	}
	parent = inode;

	while (*name) {
		p = part;
		while (*name && *name != '/') {
			if (p >= part + MAX_NAME_LEN)
				return ERR_PTR(-ENAMETOOLONG);
			*p++ = *name++;
		}
		*p = '\0';
		while (*name && *name == '/')
			name++;
		if (!*name && (flag & LOOKUP_PARENT))
			return parent;
		FS_DEBUG("Looking for part: %s, parent->inode: %d\n", part, parent->i_ino);
		inode = iget(part, parent);
		if (IS_ERR_OR_NULL(inode))
			break;

		free_inode(parent);
		parent = inode;
		if (!*name)
			break;
	}

	if (PTR_ERR(inode) == -ENOENT && flag & LOOKUP_CREATE) {
		inode = parent->i_op->mknod(parent, part, TFS_FILE);
		free_inode(parent);
		if (IS_ERR(inode))
			return ERR_CAST(inode);
		if (inode->i_op->iwrite(inode)) {
			free_inode(inode); 
			return ERR_PTR(-EIO);
		}
	}

	return inode;
}

/*
 * Lookup the parent inode 
 *
 * NOTE: the parent should  be directory!
 */
struct inode *namei_parent(const char *pathname)
{
	struct inode *dir = namei(pathname, LOOKUP_PARENT);
	
	if (!IS_ERR_OR_NULL(dir))
		return dir ? ERR_CAST(dir) : ERR_PTR(-ENOENT);
	
	if (!IS_DIR(dir)) {
		free_inode(dir);
		return ERR_PTR(-ENOTDIR);
	}

	return dir;
}
	

int sys_mkdir(const char *pathname)
{
	int err;
	struct inode *dir = namei_parent(pathname);

	if (IS_ERR(dir))
		return PTR_ERR(dir);

	err = -ENOSYS;
	if (dir->i_op && dir->i_op->mkdir)
		err = dir->i_op->mkdir(dir, get_base_name(pathname));

	free_inode(dir);
	return err;
}

int sys_rmdir(const char *pathname)
{
	struct inode *dir = namei_parent(pathname);
	int err;

	if (IS_ERR(dir))
		return PTR_ERR(dir);

	err = -ENOSYS;
	if (dir->i_op && dir->i_op->rmdir)
		err = dir->i_op->rmdir(dir, get_base_name(pathname));

	free_inode(dir);
	return err;
}

int sys_chdir(const char *pathname)
{
	struct inode * inode;
	char *cwd;

	while (*pathname && *pathname == '.' && *(pathname + 1) == '/')
			pathname += 2;
	/* If we are changing to current dir, then just do nothing */
	if (*pathname == '.' && !*(pathname + 1))
		return 0;

	inode = namei(pathname, 0);
	if (IS_ERR_OR_NULL(inode))
		return inode ? PTR_ERR(inode) : -ENOENT;
	if (!IS_DIR(inode)) {
		free_inode(inode);
		return -ENOTDIR;
	}

	free_inode(root_fs()->pwd);
	root_fs()->pwd = inode;

	cwd = root_fs()->cwd;
	if (*pathname == '/') {
		strcpy(cwd, pathname);
	} else if (strcmp(pathname, "..") == 0) {
		char *p = strrchr(cwd, '/');
		
		/* At least we have a '/' char */
		if (!p) 
			return -EINVAL;

		if (p == cwd)
			*(p + 1) = '\0';
		else
			*p = '\0';
	} else {
		strcat(cwd, pathname);
	}

	return 0;
}

/*
 * The sys call to get the current directory
 */
int sys_getcwd(char *buf, int size)
{
	int len = strlen(root_fs()->cwd);

	if (size <= 0)
		return -EINVAL;
	if (len > size)
		return -ERANGE;
	strcpy(buf, root_fs()->cwd);
	return len;
}

int sys_unlink(const char *pathname)
{
	struct inode *dir = namei_parent(pathname);
	int err;

	if (IS_ERR(dir))
		return PTR_ERR(dir);

	err = -ENOSYS;
	if (dir->i_op && dir->i_op->unlink)
		err = dir->i_op->unlink(dir, get_base_name(pathname));
	
	free_inode(dir);
	return err;
}
