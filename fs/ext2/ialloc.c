/* 
 * thunix/fs/ialloc.c
 *
 * it contains the inode allocation and deallocation routines
 */

#include <thunix.h>
#include <fs_ext2.h>



/* get the group's inode bitmap  of the given group num */
void * ext2_read_inode_bitmap(unsigned int block_group)
{
        void *inode_bitmap;

        inode_bitmap =(void *)( EXT2_BITMAP_BUFFER
                                + ((block_group << 1) + 1) * EXT2_BLOCK_SIZE);

        return inode_bitmap;
}


void ext2_free_inode(int inr)
{
        unsigned int inode_group;
        unsigned int bit;
        struct ext2_group_desc * desc;
        struct ext2_sb_info * sbi = EXT2_SBI();
        void *bitmap;

        
        inode_group = ext2_get_group_num (inr, INODE);
        bit = ext2_get_group_offset (inr, INODE);

        if ( !ext2_clear_bit(bitmap, bit) )
                ext2_error ("bit %d is alread cleared",bit);

        desc = ext2_get_group_desc (inode_group);
        desc->bg_free_inodes_count ++;
        sbi->s_free_inodes_count ++;
}



int ext2_grab_inode (char *bitmap, unsigned int goal)
{
        if ( !ext2_test_bit(bitmap, goal) )
                goto got_inode;
        /* else looking forward */
        goal = find_first_zero (bitmap, goal + 1, EXT2_INODES_PER_GROUP);
 got_inode:
        return goal;
}

/*
 * find a free inode in the inode table
 */
int ext2_alloc_inode (unsigned int goal, int mode)
{
        unsigned int inode;
        unsigned int inode_group;
        unsigned int bit;
        struct ext2_group_desc *desc;
        struct ext2_sb_info * sbi = EXT2_SBI();
        void *bitmap;
        
        EXT2_DEBUG(printk("\n"));

        inode_group = ext2_get_group_num (goal, INODE);
        bit = ext2_get_group_offset (goal, INODE);

        bitmap = ext2_read_inode_bitmap (inode_group);
        inode = ext2_grab_inode (bitmap, bit);

        if ( !inode )
                ext2_error ("no free inodes any more");
        
        desc = ext2_get_group_desc (inode_group);
        desc->bg_free_inodes_count --;
        if (S_ISDIR(mode)) {
                desc->bg_used_dirs_count ++;
                sbi->s_dirs_count ++;
        }

        sbi->s_free_inodes_count --;
        ext2_set_bit (bitmap, inode);

        return inode;
}

/*
 * make a new inode for mknod
 */
struct m_inode *ext2_new_inode(struct m_inode *dir, int mode)
{
        struct m_inode *inode;
        int inr;

        /* find a near inode */
        inr = ext2_alloc_inode (dir->i_num, mode);

        EXT2_DEBUG(printk("the allocted inode inr : %d\n",inr));
        
        inode = ext2_iget(inr);

        inode->i_count = 1;
        inode->i_nlinks = 1;
        inode->i_atime = CURRENT_TIME;
        inode->i_ctime = CURRENT_TIME;
        inode->i_mode = mode;

        return inode;
}
