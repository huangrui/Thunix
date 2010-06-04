#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "tfs.h"
#include "cache.h"
#include "dirent.h"

#define current_time 0


void free_inode(struct inode *inode)
{
	if(inode->i_data)
		free(inode->i_data);
	free(inode);
}

struct inode *new_inode(int mode)
{
	struct inode *inode;

	inode = malloc(sizeof(*inode));
	if (!inode) {
		printk("malloc for new inode failed!\n");
		return NULL;
	}
	memset(inode, 0, sizeof(*inode));
	inode->i_mode = mode;

	inode->i_atime = current_time;
	inode->i_ctime = current_time;
	inode->i_mtime = current_time;

	inode->i_data = malloc(TFS_N_BLOCKS * sizeof(uint32_t *));
	memset(inode->i_data, 0, TFS_N_BLOCKS * sizeof(uint32_t *));
	if (!inode->i_data) {
		printk("malloc for inode data failed!\n");
		free(inode);
		return NULL;
	}


	return inode;
}


/*
 * release all the stuff related to the inode
 */
int tfs_release_inode(struct tfs_sb_info *sbi, struct inode *inode)
{
	int inr = inode->i_ino;
	int index = 0;
	int block;

	if (!inode) {
		printk("ERROR: trying to free a NULL inode!\n");
		return -1;
	}

	TFS_DEBUG("trying to release inode: %d\n", inr);

	if (tfs_free_inode(sbi, inr) == -1) {
		printk("ERROR: trying to free inode %d failed!\n", inr);
		return -1;
	}

	while ((block = tfs_bmap(inode, index++)))
		tfs_free_block(sbi, block);

	free_inode(inode);
}

	
struct inode * tfs_new_inode(struct tfs_sb_info *sbi, int mode)
{
	struct inode *inode;
	int inr;

	inode = new_inode(mode);
	if (!inode) 
		return NULL;
	/* 
	 * allocate it start from TFS_ROOT_INODE, so only the first one
	 * will get it:)
	 */ 
	inr = tfs_alloc_inode(sbi, TFS_ROOT_INODE);
	if (inr == -1) {
		free(inode);
		return NULL;
	}

	inode->i_ino = inr;

	return inode;
}

struct inode * tfs_root_init(struct tfs_sb_info *sbi)
{
	struct inode *inode;

	inode = tfs_new_inode(sbi, TFS_DIR); 
	if (inode->i_ino != TFS_ROOT_INODE) {
		TFS_DEBUG("root init error!\n");
		free_inode(inode);
		inode = NULL;
	}

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
	if (!cs)
		return -1;

	memcpy(tinode, cs->data + ((inr - 1) % TFS_INODES_PER_BLOCK(sbi)) * sizeof(*tinode), sizeof(*tinode));

	return 0;
}
	
struct inode * tfs_iget_by_inr(struct tfs_sb_info *sbi, int inr)
{
	struct inode *inode;
	struct tfs_inode tinode;

	if (tfs_read_inode(sbi, &tinode, inr) == -1) {
		printk("ERROR: read disk inode error!\n");
		return NULL;
	}

	inode = new_inode(0);
	if (inode) {
		tfs_fill_inode(inode, &tinode);
		inode->i_ino = inr;
	}

	return inode;
}
	
struct inode *tfs_iget_root(struct tfs_sb_info *sbi)
{
	return tfs_iget_by_inr(sbi, TFS_ROOT_INODE);
}

struct inode *tfs_iget(struct tfs_sb_info * sbi, char *dname, struct inode *dir)
{
	struct tfs_dir_entry *de;

	if (!tfs_find_entry(sbi, dname, dir, &de))
		return NULL;
	
	return tfs_iget_by_inr(sbi, de->d_inode);
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

int tfs_iwrite(struct tfs_sb_info *sbi, struct inode *inode)
{
	struct cache_struct *cs;
	struct tfs_inode *tinode;
	uint32_t inode_block;
	int res = 0;

	inode_block = sbi->s_inode_table + (inode->i_ino - 1) / TFS_INODES_PER_BLOCK(sbi);
	cs = get_cache_block(sbi, inode_block);
	if (!cs)
		return -1;
	tinode = (struct tfs_inode *)cs->data + ((inode->i_ino - 1) % TFS_INODES_PER_BLOCK(sbi));
	tfs_write_inode(tinode, inode);
	tfs_bwrite(sbi, inode_block, cs->data);
	
	return -1;
}

uint32_t tfs_bmap(struct inode *inode, int index)
{
	if (index >= TFS_N_BLOCKS) {
		printk("File too big!\n");
		return 0;
	}

	return inode->i_data[index];
}
		


struct inode * tfs_namei(struct tfs_sb_info *sbi, const char *name, uint32_t flag)
{
	struct inode *inode;
	struct inode *parent;
	char part[TFS_NAME_LEN + 1];
	char *p;

	if (*name == '/') {
		inode = tfs_iget_root(sbi);
		while (*name == '/')
			name++;
	} else {
		inode = this_dir->dd_dir->inode;

	}
	parent = inode;

	while (*name) {
		p = part;
		while (*name && *name != '/') {
			if (p >= part + TFS_NAME_LEN) {
				printk("ERROR: file name to long!\n");
				return NULL;
			}
			*p++ = *name++;
		}
		*p = '\0';
		while (*name && *name == '/')
			name++;
		if (!*name && (flag & LOOKUP_PARENT))
			return parent;
		inode = tfs_iget(sbi, part, parent);
		if (!inode)
			break;

		if (parent != this_dir->dd_dir->inode)
			free_inode(parent);
		parent = inode;
		if (!*name)
			break;
	}

	if (!inode && flag & LOOKUP_CREATE) {
		inode = __mknod(sbi, parent, part, TFS_FILE);
		tfs_iwrite(sbi, inode);
	}

	return inode;
}

static const char *__strrchr(const char *s, int c)
{
	const char *end = s + strlen(s) - 1;
	while (*end != c && end >= s)
		end--;
	if (end < s)
		return NULL;
	return end;
}

const char *get_base_name(const char *path_org)
{
	char *p;
	char *path = strdup(path_org);

	p = strrchr(path, '/');
	if (!p) {
		free(path);
		return path_org;
	}
	/* the /linux/hello/ case */
	if (*(p + 1) == 0) {
		*p = 0;
		p--;
		while (*p != '/' && p >= path) {
			*p = 0;
			p--;
		}
		if (p < path)
			return NULL;
	}

	return p + 1;
}

struct inode * __mknod(struct tfs_sb_info *sbi, struct inode *dir, const char *filename, int mode)
{
	struct inode *inode;
	struct tfs_dir_entry *de;
	struct cache_struct *cs;
	int dirty = 0;

	if (cs = tfs_find_entry(sbi, filename, dir, &de)) {
		printk("ERROR: %s exist!\n", filename);
		return NULL;
	}

	inode = tfs_new_inode(sbi, mode);
	if (!inode)
		return NULL;
	inode->i_mtime = inode->i_atime = current_time;
	tfs_iwrite(sbi, inode);

	if (tfs_add_entry(sbi, dir, filename, inode->i_ino, &dirty) == -1) {
		TFS_DEBUG("trying to add a new entry: %s faild!\n", filename);
		free_inode(dir);
		free_inode(inode);
		return NULL;
	}
	if (dirty)
		tfs_iwrite(sbi, dir);

	return inode;
}

struct inode * tfs_mknod(struct tfs_sb_info *sbi, const char *filename, int mode, struct inode **parent_dir)
{
	struct inode *dir;
	struct inode *inode;
	const char *base_name = get_base_name(filename);

	dir = tfs_namei(sbi, filename, LOOKUP_PARENT);
	if (!dir) {
		printk("ERROR: path not exist!\n");
		return NULL;
	}
	
	inode = __mknod(sbi, dir, base_name, mode);
	if (parent_dir) {
		*parent_dir = dir;
	} else {
		if (this_dir->dd_dir->inode != dir)
			free_inode(dir);
	}

	return inode;
}
