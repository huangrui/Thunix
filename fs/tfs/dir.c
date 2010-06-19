#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <err.h>
#include <fs.h>
#include <tfs.h>
#include <cache.h>
#include <dirent.h>

/*
 * NOTE! unlike strncmp, ext2_match_entry returns 1 for success, 0 for failure.
 *
 * len <= EXT2_NAME_LEN and de != NULL are guaranteed by caller.
 */
static inline int tfs_match_entry (const char * const name,
                                   struct tfs_dir_entry * de)
{
        if (!de->d_inode)
                return 0;
	if (strlen(name) != strlen(de->d_name) && strlen(name) != TFS_NAME_LEN)
		return 0;
        return !strncmp(name, de->d_name, strlen(name));
}

/*
 * Returns:
 * 	NULL, entry not found
 * 	ERROR, errors happend.
 * 	cs,   entry found
 */
struct cache_struct * tfs_find_entry(struct inode *inode, const char *dname, struct tfs_dir_entry **res)
{
        uint32_t block;
        int index = 0;
        struct tfs_dir_entry *de;
	struct cache_struct *cs;
	struct tfs_sb_info *sbi = TFS_SBI(inode->i_fs);
                
        block = inode->i_data[index++];
	if (!block)
                return NULL;
        cs = get_cache_block(sbi, block);
	if (!cs)
		return ERR_PTR(-EIO);
        de = (struct tfs_dir_entry *)cs->data;
        
        while(1) {
		if ((char *)de >= (char *)cs->data + sbi->s_block_size) {
			if ((block = inode->i_data[index++]) < sbi->s_data_area)
                                return NULL;
                        cs = get_cache_block(sbi, block);
			if (!cs)
				return ERR_PTR(-EIO);
                        de = (struct tfs_dir_entry *)cs->data;
		}
		if (de->d_inode == 0) {
			de++;
			continue;
		}
                if (tfs_match_entry(dname, de)) {
			*res = de;
                        return cs;
		}

                de++;
        }
        
        return NULL;
}

int tfs_add_entry(struct inode *dir, const char *name, int inr, int * dirty)
{
	uint32_t block;
	int index = 0;
	struct cache_struct *cs;
	struct tfs_dir_entry *de;
	struct tfs_sb_info *sbi = TFS_SBI(dir->i_fs);

	if (strlen(name) > TFS_NAME_LEN)
		return -ENAMETOOLONG;

	if (!(block = dir->i_data[index++]))
		goto alloc_new_block;
	cs = get_cache_block(sbi, block);
	if (!cs)
		return -EIO;
	de = (struct tfs_dir_entry *)cs->data;
	while (1) {
		if ((void *)de >= cs->data + sbi->s_block_size) {
			if (!(block = dir->i_data[index++]))
				break;
			cs = get_cache_block(sbi, block);
			if (!cs)
				return -EIO;
			de = (struct tfs_dir_entry *)cs->data;
		}
		if (!de->d_inode)
			break;
		de++;
	}

	*dirty = 0;

alloc_new_block:
	/* allocate a new block to hold the new entry */
	if (!block) {
		block = tfs_alloc_block(sbi, sbi->s_data_area);
		if (block < 0)
			return -ENOSPC;
		if (index > TFS_N_BLOCKS)
			return -EFBIG;

		dir->i_data[index - 1] = block;
		cs = get_cache_block(sbi, block);
		if (!cs)
			return -EIO;
		de = (struct tfs_dir_entry *)cs->data;
		memset(cs->data, 0, sbi->s_block_size);
	}

	/* Add a new entry at last */
	dir->i_size += sizeof(struct tfs_dir_entry);
	/* tell the caller to update this inode */
	*dirty = 1;

	memset(de, 0, sizeof(*de));
	de->d_inode = inr;
	memcpy(de->d_name, name, strlen(name));
	
	/* write the entry back to disk */
	if (tfs_bwrite(sbi, block, cs->data))
		return -EIO;

	return 0;
}


int tfs_mkdir(struct inode *parent_dir, const char *dname)
{
	struct inode *dir;
	int dirty;
	int err = 0;

	dir = tfs_mknod(parent_dir, dname, TFS_DIR);
	if (IS_ERR(dir)) {
		free_inode(parent_dir);
		return PTR_ERR(dir);
	}

	err = tfs_add_entry(dir, ".", dir->i_ino, &dirty);
	if (err) 
		goto out;

	err = tfs_add_entry(dir, "..", parent_dir->i_ino, &dirty);
	if (err < 0)
		goto out;

	if (dirty)
		err = tfs_iwrite(dir);
out:
	free_inode(dir);
	free_inode(parent_dir);

	return err;
}

/*
 * Check if the dir is empty or not.
 */
static int is_empty_dir(struct inode *dir)
{
	if (dir->i_size > 2 * sizeof(struct tfs_dir_entry))
		return 0;
	else if (dir->i_size < 2 * sizeof(struct tfs_dir_entry))
		return -1;
	else
		return 1;
}


int tfs_rmdir(struct inode *dir, const char *dname) 
{
	struct inode *inode;
	struct cache_struct *cs;
	struct tfs_dir_entry *de;
	struct tfs_sb_info *sbi = TFS_SBI(dir->i_fs);
	int err;


	cs = tfs_find_entry(dir, dname, &de);
	if (IS_ERR_OR_NULL(cs)) {
		err = cs ? PTR_ERR(cs) : -ENOENT;
		goto out;
	}

	inode = tfs_iget_by_inr(dir->i_fs, de->d_inode);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}

	if (inode->i_mode != TFS_DIR) {
		err = -ENOTDIR;
		goto error;
	}

	err = is_empty_dir(inode);
	if (err == 0) {
		err = -ENOTEMPTY;
		goto error;
	} else if (err == -1) {
		printk("%s: path correupted: the size is less than two direntry!\n", dname);
		err = -EINVAL;
		goto error;
	}
	
	dir->i_size -= sizeof(struct tfs_dir_entry);
	err = tfs_iwrite(dir);
	if (err)
		goto error;
	de->d_inode = 0;
	err = tfs_bwrite(sbi, cs->block, cs->data);
	if (err)
		goto error;
	err = tfs_release_inode(sbi, inode);
	goto out;

error:
	free_inode(inode);
out:
	return err;
}


int tfs_unlink(struct inode *dir, const char *dname)
{
	struct inode *inode;
	struct cache_struct *cs;
	struct tfs_dir_entry *de;
	struct tfs_sb_info *sbi = TFS_SBI(dir->i_fs);
	int err;

	cs = tfs_find_entry(dir, dname, &de);
	if (IS_ERR_OR_NULL(cs)) {
		err = cs ? PTR_ERR(cs) : -ENOENT;
		goto out;
	}

	inode = tfs_iget_by_inr(dir->i_fs, de->d_inode);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}

	if (inode->i_mode != TFS_FILE) {
		printk("%s: not a file!\n", dname);
		goto error;
	}

	dir->i_size -= sizeof(struct tfs_dir_entry);
	err = tfs_iwrite(dir);
	if (err)
		goto error;
	de->d_inode = 0;
	err = tfs_bwrite(sbi, cs->block, cs->data);
	if (err)
		goto error;
	err = tfs_release_inode(sbi, inode);
	goto out;

error:
	free_inode(inode);
out:
	return err;
}


/* read one directry entry at a time */
struct dirent * tfs_readdir(struct file *file)
{
        struct dirent *dirent;
        struct tfs_dir_entry *de;
        struct cache_struct *cs;
	struct inode *inode = file->inode;
	struct tfs_sb_info *sbi  = TFS_SBI(file->fs);
        int index = file->offset >> sbi->s_block_shift;
        uint32_t block;

        if (!(block = tfs_bmap(inode, index)))
                return NULL;        
        cs = get_cache_block(sbi, block);
        de = (struct tfs_dir_entry *)(cs->data + (file->offset & (sbi->s_block_size- 1)));

        if (!(dirent = malloc(sizeof(*dirent)))) {
                printk("malloc dirent structure in tfs_readdir error!\n");
                return NULL;
        }
	memset(dirent, 0, sizeof(*dirent));
        dirent->d_ino = de->d_inode;
        dirent->d_off = file->offset;
        dirent->d_reclen = sizeof(struct tfs_dir_entry);
        dirent->d_type = 0;
        memcpy(dirent->d_name, de->d_name, TFS_NAME_LEN);

	file->offset += sizeof(struct tfs_dir_entry);

	/* Skip the invalid one */
	if (de->d_inode == 0) {
		free(dirent);
		return tfs_readdir(file);
	}

	
        return dirent;
}

void tfs_closedir(DIR *dir)
{
	if (dir) {
		free_inode(dir->dd_dir->inode);
		free(dir->dd_dir);
		free(dir);
	}
}
