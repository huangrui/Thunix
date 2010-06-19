#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <thunix.h>
#include <err.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>
#include <dirent.h>

#define current_time 0

/*
 * release all the stuff related to the inode
 */
int tfs_release_inode(struct tfs_sb_info *sbi, struct inode *inode)
{
	int inr = inode->i_ino;
	int index = 0;
	int block;
	int res = 0;

	if (!inode) {
		printk("ERROR: trying to free a NULL inode!\n");
		return -1;
	}

	TFS_DEBUG("trying to release inode: %d\n", inr);

	res = tfs_free_inode(sbi, inr);
	if (res < 0) {
		TFS_DEBUG("ERROR: trying to free inode %d failed!\n", inr);
		return res;
	}

	while ((block = tfs_bmap(inode, index++))) {
		res = tfs_free_block(sbi, block);
		if (res < 0)
			break;
	}

	free_inode(inode);
	return res;
}

	
struct inode * tfs_new_inode(struct inode *dir, int mode, int data_size)
{
	struct inode *inode;
	int inr;

	inode = new_inode(mode, data_size);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	/* 
	 * allocate it start from TFS_ROOT_INODE, so only the first one
	 * will get it:)
	 */ 
	inr = tfs_alloc_inode(TFS_SBI(dir->i_fs), TFS_ROOT_INODE);
	if (inr < 0) {
		free(inode);
		return ERR_PTR(inr);
	}

	inode->i_ino = inr;

	/*
	 * FIXME: new_inode() doesn't init the i_blksize filed. And in
	 * here, we assign this filed with the dir's i_blksize; this is
	 * out design. But in iget_...() function, then are assigned from
	 * fs->block_size.
	 *
	 * We should make the assigment be constant.
	 */
	inode->i_blksize = dir->i_blksize;

	inode->i_op  = &tfs_inode_ops;
	inode->i_fop = &tfs_file_ops;

	return inode;
}


static void tfs_fill_inode(struct inode *inode, struct tfs_inode *tinode)
{
	//inode->i_mode = get_mode(tinode->i_mode);
	inode->i_mode   = tinode->i_mode;
	inode->i_size   = tinode->i_size;
	inode->i_atime  = tinode->i_atime;
	inode->i_ctime  = tinode->i_ctime;
	inode->i_mtime  = tinode->i_mtime;
	inode->i_dtime  = tinode->i_dtime;
	inode->i_flags  = tinode->i_flags;

	memcpy(inode->i_data, tinode->i_block, TFS_N_BLOCKS * sizeof(uint32_t *));
}

static int tfs_read_inode(struct tfs_sb_info *sbi, struct tfs_inode *tinode, int inr)
{
	uint32_t inode_block;
	struct cache_struct *cs;
	
	inode_block = sbi->s_inode_table + (inr - 1) / TFS_INODES_PER_BLOCK(sbi);
	cs = get_cache_block(sbi, inode_block);
	if (IS_ERR(cs))
		return -EIO;

	memcpy(tinode, cs->data + ((inr - 1) % TFS_INODES_PER_BLOCK(sbi)) * sizeof(*tinode), sizeof(*tinode));

	return 0;
}
	
struct inode * tfs_iget_by_inr(struct fs *fs, int inr)
{
	struct inode *inode;
	struct tfs_inode tinode;
	int err;

	err = tfs_read_inode(TFS_SBI(fs), &tinode, inr);
	if (err)
		return ERR_PTR(err);

	inode = new_inode(0, TFS_N_BLOCKS * sizeof(uint32_t *));
	if (inode) {
		tfs_fill_inode(inode, &tinode);
		inode->i_blksize = BLOCK_SIZE(fs);
		inode->i_ino = inr;
	} else {
		return ERR_PTR(-ENOMEM);
	}

	inode->i_op  = &tfs_inode_ops;
	inode->i_fop = &tfs_file_ops;

	return inode;
}
	
struct inode *tfs_iget_root(struct fs *fs)
{
	return tfs_iget_by_inr(fs, TFS_ROOT_INODE);
}

struct inode *tfs_iget(const char *dname, struct inode *dir)
{
	struct tfs_dir_entry *de;
	struct cache_struct *cs;

	cs = tfs_find_entry(dir, dname, &de);
	if (IS_ERR_OR_NULL(cs))
		return cs ? ERR_CAST(cs) : ERR_PTR(-ENOENT);
	
	return tfs_iget_by_inr(dir->i_fs, de->d_inode);
}

static void tfs_write_inode(struct tfs_inode *tinode, struct inode *inode)
{
	tinode->i_mode   = inode->i_mode;
	tinode->i_size   = inode->i_size;
	tinode->i_atime  = inode->i_atime;
	tinode->i_ctime  = inode->i_ctime;
	tinode->i_mtime  = inode->i_mtime;
	tinode->i_dtime  = inode->i_dtime;
	tinode->i_flags  = inode->i_flags;

	memcpy(tinode->i_block, inode->i_data, TFS_N_BLOCKS * sizeof(uint32_t *));
	tinode->i_reserved[0] = 0;

}

int tfs_iwrite(struct inode *inode)
{
	struct cache_struct *cs;
	struct tfs_inode *tinode;
	struct tfs_sb_info *sbi = TFS_SBI(inode->i_fs);
	uint32_t inode_block;
	int err = 0;

	inode_block = sbi->s_inode_table + (inode->i_ino - 1) / TFS_INODES_PER_BLOCK(sbi);
	cs = get_cache_block(sbi, inode_block);
	if (!cs)
		return -EIO;
	tinode = (struct tfs_inode *)cs->data + ((inode->i_ino - 1) % TFS_INODES_PER_BLOCK(sbi));
	tfs_write_inode(tinode, inode);
	err = tfs_bwrite(sbi, inode_block, cs->data);
	
	return err;
}

uint32_t tfs_bmap(struct inode *inode, int index)
{
	if (index >= TFS_N_BLOCKS)
		return 0;

	return inode->i_data[index];
}


struct inode * tfs_mknod(struct inode *dir, const char *dname, int mode)
{
	struct inode *inode;
	struct tfs_dir_entry *de;
	struct cache_struct *cs;
	struct tfs_sb_info *sbi = TFS_SBI(dir->i_fs);
	int dirty = 0;
	int err;

	TFS_DEBUG("mknod: dir->inode: %d, name: %s\n", dir->i_ino, dname);

	cs = tfs_find_entry(dir, dname, &de);
	if (!IS_ERR_OR_NULL(cs))
		return ERR_PTR(-EEXIST);

	inode = tfs_new_inode(dir, mode, TFS_N_BLOCKS * sizeof(uint32_t *));
	if (IS_ERR(inode))
		return ERR_CAST(inode);
	inode->i_mtime = inode->i_atime = current_time;
	if (tfs_iwrite(inode)) {
		err = -EIO;
		goto error;
	}
	err = tfs_add_entry(dir, dname, inode->i_ino, &dirty);
	if (err) {
		TFS_DEBUG("trying to add a new entry: %s faild!\n", dname);
		goto error;
	}
	if (dirty) {
		err = -EIO;
		if (tfs_iwrite(dir))
			goto error;
	}

	return inode;
error:
	free_inode(inode);
	return ERR_PTR(err);
}

struct inode_operations tfs_inode_ops = {
	.iget 		= tfs_iget,
	.iget_root	= tfs_iget_root,
	.iwrite		= tfs_iwrite,
	.mkdir		= tfs_mkdir,
	.rmdir		= tfs_rmdir,
	.unlink		= tfs_unlink,
	.mknod		= tfs_mknod,
};
