#ifndef FILE_H
#define FILE_H

#include "tfs.h"

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

struct file {
        struct tfs_sb_info *sbi;     /* which fs we belong to */
        struct inode *inode;  /* the file-specific information */
        uint32_t offset;      /* for next read */
};

struct file *tfs_open(struct tfs_sb_info *, const char *);
int tfs_read(struct file *, void *, uint32_t);
int tfs_write(struct file *, void *, uint32_t);
int tfs_lseek(struct file *, int, int);
void tfs_close(struct file *);

#endif /* file.h */
