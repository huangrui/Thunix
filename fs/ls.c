#include <fs_ext2.h>
#include <thunix.h>

char dir[8] = "/usr";

void ls(char *pathname,char *arg)
{
        void *dir_data;
        int entries, i = 0;
        struct ext2_dir_entry *de;
        struct m_inode *dir;
        unsigned short block;
        int dir_block = 1;
        
        EXT2_DEBUG();
        
        dir = ext2_namei(pathname);
        if ( !dir) {
                printk("error: dir or file %s not found\n",pathname);
                return;
        }
        EXT2_DEBUG(printk("pathname %s\n",pathname));
        block = dir->inode.i_block[0];          
        dir_data = (void *)bread(block);
        de = (struct ext2_dir_entry *)dir_data;
        if (strcmp(arg,"-l") == 0)
                printk("\nfile name\tinode number\n");

        while (de->name && de->inode) {
                if ( (char *)de >= (char *)(EXT2_BLOCK_SIZE + dir_data) ) {
                        block = bmap (dir, dir_block);
                        dir_data = bread (block);
                        de = (struct ext2_dir_entry *)dir_data;
                        dir_block ++;
                }
                
                
                if (_strcmp(arg, "-l") == 0) {
                        printk("%-16s%d\n",de->name,de->inode);
                        goto next;
                }
                
                printk("%s\n",de->name);
  
        next:
                
                de = ext2_next_entry(de);
        }
       
        /* 
           EXT2_DEBUG(printk("inr %d's one block buffer:\n", dir->i_num));
           dump_from_addr(dir_data,60); 
        */
}

