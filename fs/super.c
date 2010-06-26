#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <err.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>

struct fs * current_root_fs;

struct fs_type fs_types [] = {
	{tfs_mount, tfs_get_blk_shift},
	{NULL, NULL}
};

struct fs *fs_init(void)
{
	struct fs *fs = malloc(sizeof(*fs));
	struct fs_type *fs_type;

	printk("fs system initialiaztion...");

	fs_type = fs_types;
	while (fs_type->mount) {
		fs->sb = fs_type->mount();
		if (fs->sb)
			break;
		fs_type++;
	}
	if (!fs_type->mount)
		return NULL;
	fs->block_shift = fs_type->get_blk_shift(fs->sb);
	fs->sector_shift = 9;
	current_root_fs = fs;

	cache_init(fs->sb);

	fs->root = tfs_iget_root(fs);
	fs->pwd = fs->root;

	strcpy(fs->cwd, "/");
	
	/*
	 * ZERO the fd table, since we currently do not support process, we just have
	 * one set of fd table. So, init it here.
	 */
	memset(fds, 0, 32);

	printk("\t\t%s\n", "[OK]");
	return fs;
}

