/*
 * thunix/fs/dir.c
 *
 * implement something relations to dir, like
 * mkdir, rmdir or cd some of sort ...
 *
 * Copyright (C) 2008 - 2009 Aleaxander
 * 
 *           Aleaxander@gmail.com
 */

#include <string.h>
#include <thunix.h>
#include <fs_ext2.h>


void ext2_make_empty_dir(struct m_inode *dir, struct m_inode *parent)
{
        int block;

        block = create_block(dir, 0);
        struct ext2_dir_entry dot, ddot;

        dot.inode = dir->i_num;
        dot.name_len = 1;
        dot.rec_len = EXT2_DIR_REC_LEN(1);
        strcpy(dot.name, ".");

        ddot.inode = parent->i_num;
        ddot.name_len = 2;
        ddot.rec_len = EXT2_DIR_REC_LEN(2);
        strcpy(ddot.name, "..");

        ext2_add_entry(dir, &dot);
        ext2_add_entry(dir, &ddot);
}

int  ext2_dir_empty(struct m_inode *dir)
{
        void *dir_data;
        struct ext2_dir_entry *de;
        unsigned int block;
        unsigned int dir_block = 1;
        int i = 0;
        
        EXT2_DEBUG();
        
        if ( !(block = dir->inode.i_block[0]) )
                return 2;

        dir_data = bread(block);
        de = (struct ext2_dir_entry *)dir_data;
        while (1) {
                if ( (char *)de >= (char *)(EXT2_BLOCK_SIZE + dir_data) ) {
                        block = bmap (dir, dir_block);
                        dir_data = bread (block);
                        de = (struct ext2_dir_entry *)dir_data;
                        dir_block ++;
                }
                
                if (strcmp(de->name, ".") || strcmp(de->inode, "..") )
                        return 0;
                i ++;
                if ( i > 2)
                        return 1;
                
                de = ext2_next_entry(de);
        }
}

void mkdir(char *pathname)
{
        struct m_inode *dir, *inode;
        int mode;
        char *dir_name, *file_name;
        struct ext2_dir_entry entry;

        char temp_dir[8] = "/";


        /**
         * for now, we just one levele dir like /xx but not /xx/yy.
         * and for now, it's enough
         */

        dir_name = pathname;
        file_name = strchr(pathname,'/');
        if (! file_name) {
                printk("error: please input the absolute pathname\n");
                return ;
        }
                
        *file_name = '\0';
        file_name ++;
        
        EXT2_DEBUG(printk("dir:%s file: %s\n",dir_name, file_name));

        dir = ext2_namei(temp_dir);

        EXT2_DEBUG(printk("dir's inr: %d\n", dir->i_num));

        mode = mode | S_IFDIR;
        inode  = ext2_new_inode(dir, mode);
        
        
        entry.inode = inode->i_num;
        entry.name_len = strlen(file_name);
        entry.rec_len = EXT2_DIR_REC_LEN(entry.name_len);
        strcpy(entry.name, file_name);

        EXT2_DEBUG(printk("the new entry's name is %s\n",entry.name));

        ext2_add_entry(dir, &entry);

        

        ext2_make_empty_dir(inode, dir);
}
       
void rmdir(struct m_inode *dir)
{
        if ( !ext2_dir_empty(dir) ) {
                printk("dir not empty\n");
                return;
        }
        
        ext2_iput(dir);
}
                
