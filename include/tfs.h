#ifndef TFS_H
#define TFS_H

#include <stdint.h>

/* in whice sector the tfs stored in the boot image */
#define TFS_FS_SECTOR   (832)
#define TFS_SB_SECTOR 	(TFS_FS_SECTOR + 1)

#define TFS_ROOT_INODE 	1
#define TFS_FILE	0x1
#define TFS_DIR		0x2

/* namei: path lookup flags */
#define LOOKUP_PARENT  	0x1
#define LOOKUP_CREATE	0x2

/* just support I_FILE and I_DIR only currently */
enum tfs_inode_mode { I_FILE, I_DIR, I_UNKNOWN };

static inline int get_mode(int mode)
{
	if (mode & TFS_FILE)
		return I_FILE;
	else if (mode & TFS_DIR)
		return I_DIR;
	else
		return I_UNKNOWN;
}

/* It's really enought for a test file system */
#define TFS_N_BLOCKS 8

struct tfs_inode {
	uint16_t i_mode;		/* File mode */
	uint16_t i_uid;			/* Owner UID */
	uint32_t i_size;		/* File size */
	uint32_t i_atime;		/* Access time */ 
	uint32_t i_ctime;		/* Create time */
	uint32_t i_mtime;		/* modify time */
	uint32_t i_dtime;		/* delete time */
	uint32_t i_block[TFS_N_BLOCKS]; /* block address of file's count */
	uint32_t i_flags;               /* flags */
	uint32_t i_reserved[1];
};

/* 
 * The inode structure in memory, including the detail file information 
 */
struct inode {
        int          i_mode;   /* FILE or DIR */
        uint32_t     i_size;
        uint32_t     i_ino;    /* Inode number */
        uint32_t     i_atime;  /* Access time */
        uint32_t     i_mtime;  /* Modify time */
        uint32_t     i_ctime;  /* Create time */
        uint32_t     i_dtime;  /* Delete time */
        uint32_t *   i_data;   /* The block address array where the file stores */
        uint32_t     i_flags;
};

/* The max lenght of each file name */
#define TFS_NAME_LEN 28
struct tfs_dir_entry {
	uint32_t d_inode;		/* inode number */
	char     d_name[TFS_NAME_LEN];  /* entry name */
};

#define TFS_MAGIC  0x4c534654           /* TFSL */

/* TFS super block */
struct tfs_super_block {
	uint32_t s_inodes_count;	/* Max file count */
	uint32_t s_blocks_count;
	uint32_t s_free_blocks_count;
	uint32_t s_free_inodes_count;
	uint32_t s_magic;		/* TFS's magic signature */
	uint32_t s_block_shift;         /* Block size */
	
	uint32_t s_inode_bitmap;
	uint32_t s_block_bitmap;
	uint32_t s_inode_table;
	uint32_t s_data_area;		/* where the data starts */


	uint32_t s_offset;              /* In which sector the fs stored */
	uint32_t s_reserved[117];
};

/* TFS super block  info */
struct tfs_sb_info {
	uint32_t s_inodes_count;	/* Max file count */
	uint32_t s_blocks_count;
	uint32_t s_free_blocks_count;
	uint32_t s_free_inodes_count;
	uint32_t s_block_shift;         /* Block size in bits */
	uint32_t s_block_size;          /* Block size in bytes */
	
	uint32_t s_inode_bitmap;
	uint32_t s_inode_bitmap_count;
	uint32_t s_block_bitmap;
	uint32_t s_block_bitmap_count;
	uint32_t s_inode_table;
	uint32_t s_inode_table_count;
	uint32_t s_data_area;		/* where the data starts */


	uint32_t s_offset;              /* In which sector the fs stored */

	int 	 s_inodes_per_block;
};

extern struct tfs_sb_info *tfs_sbi;


#define TFS_INODES_PER_BLOCK(sbi) (sbi->s_inodes_per_block)

#define TFS_DEBUG printk

#define roundup(x, y) ((x) / (y) + (((x) % (y)) ? 1 : 0))

/* tfs_diskio.c */
extern void tfs_bread(struct tfs_sb_info *, uint32_t , void *);
extern void tfs_bwrite(struct tfs_sb_info *, uint32_t, void *);

/* super.c */
struct tfs_sb_info * tfs_mount(void);

/* ialloc.c */
int tfs_free_inode(struct tfs_sb_info *, int);
int tfs_alloc_inode(struct tfs_sb_info *, int);

/* balloc.c */
int tfs_alloc_block(struct tfs_sb_info *, uint32_t);
int tfs_free_block(struct tfs_sb_info *, uint32_t);


/* dir.c */
struct cache_struct *tfs_find_entry(struct tfs_sb_info *, const char *, struct inode *, struct tfs_dir_entry **);
int tfs_add_entry(struct tfs_sb_info *, struct inode *, const char *, int , int *);
int tfs_mkdir(struct tfs_sb_info *, const char *);
int tfs_rmdir(struct tfs_sb_info *, const char *);

/* inode.c */
struct inode *new_inode(int);
void free_inode(struct inode *);
struct inode *tfs_root_init(struct tfs_sb_info *);
struct inode *tfs_iget_root(struct tfs_sb_info *);
struct inode *tfs_iget(struct tfs_sb_info *, char *, struct inode *);
struct inode *tfs_namei(struct tfs_sb_info *, const char *, uint32_t);
uint32_t tfs_bmap(struct inode *, int);
int tfs_iwrite(struct tfs_sb_info *, struct inode *);
struct inode *__mknod(struct tfs_sb_info *, struct inode *, const char *, int);
struct inode *tfs_mknod(struct tfs_sb_info *, const char *, int, struct inode **);
const char *get_base_name(const char *);

#endif /* tfs.h */
