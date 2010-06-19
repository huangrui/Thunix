#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_DEBUG printk

#define MAX_NAME_LEN 	255

#define BLOCK_SIZE(fs) 	(1 << (fs)->block_shift)
#define SECTOR_SIZE(fs) (1 << (fs)->sector_shift)

/* namei: path lookup flags */
#define LOOKUP_PARENT  	0x1
#define LOOKUP_CREATE	0x2

#define TFS_FILE	0x1
#define TFS_DIR		0x2
#define IS_DIR(dir)	((dir)->i_mode == TFS_DIR)

struct fs {
	char fs_name[8];	
	void *sb;	/* The fs-specific information */
	int sector_shift;
	int block_shift;
	
	uint32_t offset; /* Disk offset as sectors */
	
	char *pwd_str;
	struct inode *pwd;
	struct inode *root;
};

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


struct file {
	struct inode * inode;
	struct fs    * fs;
	uint32_t       offset;

	struct file_operations *f_op;
};

/* 
 * The inode structure of VFS layer
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

	int 	     i_blksize;

	struct fs *  i_fs;
	
	/* Operations */
	struct inode_operations * i_op;
	struct file_operations  * i_fop;
};

struct inode_operations {
	struct inode * (*iget) (const char *, struct inode *);
	struct inode * (*iget_root)(struct fs *fs);
	struct inode * (*iget_by_inr)(int); 
	int (*iwrite) (struct inode *);

	int (*mkdir)(struct inode *, const char *);
	int (*rmdir)(struct inode *, const char *);
	int (*unlink)(struct inode *, const char *);
	struct inode *(*mknod)(struct inode *, const char *, int);
};

struct dirent;

struct file_operations {
	int (*read) (struct file *, void *, int);
	int (*write) (struct file *, void *, int);
	void (*close) (struct file *);
	struct dirent * (*readdir)(struct file *);
};

extern struct fs * current_root_fs;

static inline struct fs * root_fs(void)
{
	return current_root_fs;
}

extern uint8_t fds[];
extern struct file *files[];

/*
 * Functions
 */

/* inode.c */
struct inode *new_inode(int, int);
void free_inode(struct inode *);
struct inode * iget(const char *, struct inode *);

/* file.c */
int sys_open(const char *, uint32_t);
int sys_read(int, void *, int);
int sys_write(int, void *, int);
int sys_close(int);
int sys_lseek(int, int, int);

/* namei.c */
struct inode *namei(const char *, uint32_t);
struct inode *namei_parent(const char *);
int sys_mkdir(const char *);
int sys_rmdir(const char *);
int sys_unlink(const char *);

/* fslib.c */
const char *get_base_name(const char *);

/* super.c */
struct fs *fs_init(void);


#endif /* fs.h */
