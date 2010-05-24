/** 
 * thunix/fs/namei.c
 *
 * contains the path name parse and some important routines
 *
 * Copyright (C) Aleaxander 2008-2009
 *
 *               Aleaxander@gmail.com
 *
 */
#include <thunix.h>
#include <fs_ext2.h>
#include <string.h>


/*
 * NOTE! unlike strncmp, ext2_match_entry returns 1 for success, 0 for failure.
 *
 * len <= EXT2_NAME_LEN and de != NULL are guaranteed by caller.
 */
static inline int ext2_match_entry (int len, const char * const name,
					struct ext2_dir_entry * de)
{
        EXT2_DEBUG();
	if (len != de->name_len)
		return 0;
	if (!de->inode)
		return 0;
	return !strncmp(name, de->name, len);
}


/*
 * p is at least 6 bytes before the end of page
 */
inline struct ext2_dir_entry *ext2_next_entry(struct ext2_dir_entry *p)
{
        //EXT2_DEBUG();
	return (struct ext2_dir_entry *)((char*)p + p->rec_len);
}


struct ext2_dir_entry * ext2_find_entry(struct m_inode *dir, char *name, int namelen)
{
        void *dir_data;
        struct ext2_dir_entry *de;
        unsigned int block;
        unsigned int dir_block = 1;
        
        EXT2_DEBUG();

        if ( !(block = dir->inode.i_block[0]) )
                return NULL;

        dir_data = bread(block);
        de = (struct ext2_dir_entry *)dir_data;
        while (1) {
                if ( (char *)de >= (char *)(EXT2_BLOCK_SIZE + dir_data) ) {
                        block = bmap (dir, dir_block);
                        dir_data = bread (block);
                        de = (struct ext2_dir_entry *)dir_data;
                        dir_block ++;
                }
                
                if (de->name == NULL || de->inode == 0)
                        return NULL;

                EXT2_DEBUG(printk("name %s ,inode number %d\n",de->name, de->inode));

                if ( ext2_match_entry (namelen, name, de) )
                        return de;

                de = ext2_next_entry(de);
        }
}


void ext2_add_entry (struct m_inode *dir_inode, struct ext2_dir_entry *dir)
{
        void *dir_data;
        struct ext2_dir_entry *de;
        unsigned int block;
        unsigned int dir_block = 1;

        
        if ( !(block = dir_inode->inode.i_block[0]) )
                return;
        EXT2_DEBUG(printk("inr %d's first block nr %d\n",dir_inode->i_num,block));

        dir_data = bread(block);
        de = (struct ext2_dir_entry *)dir_data;
        while (1) {
                if ( (char *)de >= (char *)(EXT2_BLOCK_SIZE + dir_data) ) {
                        
                        EXT2_DEBUG(printk("de addr %08x dir_data's end %08x\n", \
                                          (char *)de, dir_data+EXT2_BLOCK_SIZE));
                        
                        block = bmap (dir, dir_block);
                        dir_data = bread (block);
                        de = (struct ext2_dir_entry *)dir_data;
                        dir_block ++;
                }
                
                

                if (de->name == NULL || de->inode == 0)
                        break;

                if ( !strcmp(de->name, dir->name) ) {
                        printk("%s: already exist\n", dir->name);
                        return;
                }
                        
                EXT2_DEBUG(printk("name %s ,inode number %d\n",de->name, de->inode));

                de = ext2_next_entry(de);
        }

        de->inode = dir->inode;
        de->name_len = strlen(dir->name);
        de->rec_len = EXT2_DIR_REC_LEN(de->name_len);
        strcpy(de->name, dir->name);

        
        EXT2_DEBUG(printk("the new added entry's name is %s\n",de->name));
          /*
            EXT2_DEBUG(printk("inr %d's one block buffer:\n", dir_inode->i_num));
            dump_from_addr(dir_data,60); 
          */

}
    

struct m_inode * ext2_namei(char *pathname)
{
        char c, thisname[20], *p;
        struct m_inode *working_inode, *inode;
        unsigned short block;
        struct ext2_dir_entry *de;

        EXT2_DEBUG(printk("%s\n",pathname));
        
        if ((c=*pathname)!='/') {
                printk("error: we don't support relatetive searching now, sorry!\n");
                return NULL ;
        }

        pathname++;
        working_inode = ext2_iget(ROOT_INODE);
        
        while (*pathname != '\0') {
                p = pathname;
                while (*pathname!='/'&&*pathname)
                        pathname++;
                memcpy (thisname, p, pathname - p);
                thisname[pathname-p] = '\0';
                
                EXT2_DEBUG(printk("thisname %s\n",thisname));

                
                if (de = ext2_find_entry(working_inode, thisname, strlen(thisname))) {
                        inode = ext2_iget(de->inode);
                        ext2_iput(working_inode);
                        working_inode = inode;
                }else
                        return NULL;
        }
        
        EXT2_DEBUG(printk("inr:%d \n",working_inode->i_num));

        return working_inode;
}   


 
