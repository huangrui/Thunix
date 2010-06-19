#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <err.h>
#include <fs.h>
#include <tfs.h>

struct fs * current_root_fs;

struct fs *fs_init(void)
{
	struct fs *fs = malloc(sizeof(*fs));

	fs->sb = tfs_mount();
	fs->block_shift = 10;
	fs->sector_shift = 9;
	current_root_fs = fs;

	return fs;
}

