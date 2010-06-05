#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "tfs.h"
#include "cache.h"
#include "dirent.h"

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
        return !strncmp(name, de->d_name, strlen(name));
}

struct cache_struct * tfs_find_entry(struct tfs_sb_info *sbi,
				     const char *dname,
				     struct inode *inode,
				     struct tfs_dir_entry **res)
{
        uint32_t block;
        int index = 0;
        struct tfs_dir_entry *de;
	struct cache_struct *cs;
                
        block = inode->i_data[index++];
	if (!block)
                return NULL;
        cs = get_cache_block(sbi, block);
        de = (struct tfs_dir_entry *)cs->data;
        
        while(1) {
		if ((char *)de >= (char *)cs->data + sbi->s_block_size) {
			if ((block = inode->i_data[index++]) < sbi->s_data_area)
                                return NULL;
                        cs = get_cache_block(sbi, block);
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

int tfs_add_entry(struct tfs_sb_info *sbi, struct inode *dir, const char *name, int inr, int * dirty)
{
	uint32_t block;
	int index = 0;
	struct cache_struct *cs;
	struct tfs_dir_entry *de;

	if (strlen(name) > TFS_NAME_LEN) {
		printk("ERROR: file name too long!\n");
		return -1;
	}

	if (!(block = dir->i_data[index++]))
		goto alloc_new_block;
	cs = get_cache_block(sbi, block);
	de = (struct tfs_dir_entry *)cs->data;
	while (1) {
		if ((void *)de >= cs->data + sbi->s_block_size) {
			if (!(block = dir->i_data[index++]))
				break;
			cs = get_cache_block(sbi, block);
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
		if (block == -1) {
			printk("ERROR: allocate new block failed, out of space!\n");
			return -1;
		}
		if (index > TFS_N_BLOCKS) {
			printk("file too big!\n");
			return -1;
		}
		dir->i_data[index - 1] = block;
		cs = get_cache_block(sbi, block);
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
	tfs_bwrite(sbi, block, cs->data);

	return 0;
}


int tfs_mkdir(struct tfs_sb_info *sbi, const char *path)
{
	struct inode *dir;
	struct inode *parent_dir;
	int dirty;
	int res = 0;

	dir = tfs_mknod(sbi, path, TFS_DIR, &parent_dir);
	if (!dir) {
		TFS_DEBUG("mknod for path failed!\n");
		return -1;
	}

	res = tfs_add_entry(sbi, dir, ".", dir->i_ino, &dirty);
	if (res == -1) {
		TFS_DEBUG("trying to add '.' under %s failed!\n", path);
		goto out;
	}

	res = tfs_add_entry(sbi, dir, "..", parent_dir->i_ino, &dirty);
	if (res == -1) {
		TFS_DEBUG("trying to add .. under %s failed!\n", path);
		goto out;	
	}

	if (dirty)
		tfs_iwrite(sbi, dir);
out:
	free_inode(dir);
	if (this_dir->dd_dir->inode != parent_dir)
		free_inode(parent_dir);

	return res;

}

/*
 * Check if the dir is empty or not.
 */
static int is_empty_dir(struct tfs_sb_info *sbi, struct inode *dir)
{
	if (dir->i_size > 2 * sizeof(struct tfs_dir_entry))
		return 0;
	else if (dir->i_size < 2 * sizeof(struct tfs_dir_entry))
		return -1;
	else
		return 1;
}


int tfs_rmdir(struct tfs_sb_info *sbi, const char *path) 
{
	int res;
	struct inode *dir;
	struct inode *inode;
	struct cache_struct *cs;
	struct tfs_dir_entry *de;
	const char * base_name = get_base_name(path);

	if (!base_name) {
		printk("%s: invalid path name!\n", path);
		return -1;
	}

	dir = tfs_namei(sbi, path, LOOKUP_PARENT);
	if (!dir) {
		printk("ERROR: path not exist!\n");
		return -1;
	}

	cs = tfs_find_entry(sbi, base_name, dir, &de);
	if (!cs) {
		printk("%s: path not exist!\n", path);
		return -1;
	}

	inode = tfs_iget_by_inr(sbi, de->d_inode);
	if (!inode) {
		printk("%s: path not exist!\n", path);
		return -1;
	}
	if (inode->i_mode != TFS_DIR) {
		printk("%s: not a directory!\n", path);
		return -1;
	}

	res = is_empty_dir(sbi, inode);
	if (res == 0) {
		printk("%s: path not empty!\n", path);
		return -1;
	} else if (res == -1) {
		printk("%s: path correupted: the size is less than two direntry!\n", path);
		return -1;
	}
	
	dir->i_size -= sizeof(struct tfs_dir_entry);
	tfs_iwrite(sbi, dir);
	de->d_inode = 0;
	tfs_bwrite(sbi, cs->block, cs->data);
	tfs_release_inode(sbi, inode);

	return 0;
}


int tfs_unlink(struct tfs_sb_info *sbi, const char *path)
{
	int res;
	struct inode *dir;
	struct inode *inode;
	struct cache_struct *cs;
	struct tfs_dir_entry *de;
	const char * base_name = get_base_name(path);

	if (!base_name) {
		printk("%s: invalid path name!\n", path);
		return -1;
	}

	dir = tfs_namei(sbi, path, LOOKUP_PARENT);
	if (!dir) {
		printk("ERROR: path not exist!\n");
		return -1;
	}

	cs = tfs_find_entry(sbi, base_name, dir, &de);
	if (!cs) {
		printk("%s: path not exist!\n", path);
		return -1;
	}

	inode = tfs_iget_by_inr(sbi, de->d_inode);
	if (!inode) {
		printk("%s: path not exist!\n", path);
		return -1;
	}
	if (inode->i_mode != TFS_FILE) {
		printk("%s: not a file!\n", path);
		return -1;
	}

	dir->i_size -= sizeof(struct tfs_dir_entry);
	tfs_iwrite(sbi, dir);
	de->d_inode = 0;
	tfs_bwrite(sbi, cs->block, cs->data);
	tfs_release_inode(sbi, inode);
	
	return 0;
}


/* for relative path searching */
DIR *this_dir;

DIR *tfs_opendir(struct tfs_sb_info *sbi, const char *path)
{
	DIR *dir = malloc(sizeof(*dir));
	
	if (!dir) {
		printk("malloc for DIR structure error!\n");
		return NULL;
	}

	dir->dd_dir = tfs_open(sbi, path, 0);
	if (!dir->dd_dir) {
		free(dir);
		return NULL;
	}

	return dir;
}

/* read one directry entry at a time */
struct dirent * tfs_readdir(DIR *dir)
{
        struct dirent *dirent;
        struct tfs_dir_entry *de;
        struct cache_struct *cs;
	struct file *file   = dir->dd_dir;
	struct inode *inode = file->inode;
	struct tfs_sb_info *sbi  = file->sbi;
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
		return tfs_readdir(dir);
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
