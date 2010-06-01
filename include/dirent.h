#ifndef DIRENT_H
#define DIRENT_H

#include <stdint.h>

#include "file.h"

struct dirent {
        uint32_t d_ino;
        uint32_t d_off;
        uint16_t d_reclen;
        uint16_t d_type;
        char d_name[256];
};


typedef struct {
        struct file *dd_dir;
} DIR;


DIR * tfs_opendir(struct tfs_sb_info *, const char *);
struct dirent * tfs_readdir(DIR *);
void tfs_closedir(DIR *);

extern DIR *this_dir;

#endif /* dirent.h */
