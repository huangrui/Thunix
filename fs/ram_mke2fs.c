/**
 * thunix/fs/ramfs/ram_mke2fs.c
 *
 * make an ext2 ram fs
 * yeah, it's far-fetched, all the things done here
 * are by ourself, all the things here are static.
 *
 * but, it just a TEST for ext2_file system, 
 * and that's enough.
 *
 * Copyright (c) 2008 - 2009 by Aleaxander
 *                 
 *           Aleaxander@gmail.com
 *
 */

#include <fs_ext2.h>
#include <thunix.h>

extern unsigned long ram_start;
extern unsigned long ram_end;


void ram_ext2_init_super_block()
{
        struct ext2_super_block *sb;

        sb = (struct ext2_super_block *) (ram_start + EXT2_BLOCK_SIZE);


        /* just init. some nessesary filed */
        sb->s_inodes_count = 184;
        sb->s_blocks_count = 1440;
        sb->s_r_blocks_count = 72;
        sb->s_free_blocks_count = 1393;
        sb->s_free_inodes_count = 173;
        sb->s_first_data_block = 1;
        sb->s_log_block_size = 0;
        sb->s_log_frag_size = 0;
        sb->s_blocks_per_group = 8192;
        sb->s_frags_per_group = 8192;
        sb->s_inodes_per_group = 184;
        
        sb->s_magic = 0x0ef53;
}


void ram_ext2_init_group_desc()
{
        struct ext2_group_desc *desc;

        desc = (struct ext2_group_desc *)(ram_start + EXT2_BLOCK_SIZE * 2);

        desc->bg_block_bitmap = 8;
        desc->bg_inode_bitmap = 9;
        desc->bg_inode_table = 10;
        desc->bg_free_blocks_count = 1393;
        desc->bg_free_inodes_count = 173;
        desc->bg_used_dirs_count = 2;

        /* we just make it to be so, but in fact ,
         * i think this are be done by ext_mkdir, 
         * ext2_alloc_block or something of that 
         * sort.
         *
         * we just do it for test.
         */
}


void ram_ext2_init_bitmap()
{/*
   struct ext2_group_desc *desc;
   desc = ext2_get_group(0);
 */
        unsigned long *block_bitmap;
        unsigned char *inode_bitmap;

        block_bitmap = (unsigned long *)(ram_start + EXT2_BLOCK_SIZE * 8);
        inode_bitmap = (unsigned char* )(ram_start + EXT2_BLOCK_SIZE * 9);

        *block_bitmap = 0xffffffff;
        *(++block_bitmap) = 0x3fff;

        *inode_bitmap = 0xff;
        *(++inode_bitmap) = 0xf0;
}


void ram_ext2_init_root_inode()
{
        struct ext2_inode *root_inode;
        unsigned long inode_table;
        
        inode_table = ram_start + EXT2_BLOCK_SIZE * 10;
        
        root_inode = (struct ext2_inode *)inode_table + (ROOT_INODE - 1) ;
        _memset(root_inode, 0, sizeof (struct ext2_inode));

        root_inode->i_size = 0x400;  /* 1K */
        root_inode->i_links_count = 3;
        root_inode->i_blocks = 2;
        root_inode->i_block[0] = 33;

        
        
}


void ram_ext2fs_init()
{
        EXT2_DEBUG();   

        ram_ext2_init_super_block();
        ram_ext2_init_group_desc();
        ram_ext2_init_bitmap();
        ram_ext2_init_root_inode();

       
}
  
struct m_inode * ram_ext2_get_root_inode()
{
        struct ext2_inode *root_inode;
        unsigned long inode_table;
        
        inode_table = ram_start + EXT2_BLOCK_SIZE * 10;
        
        root_inode = (struct ext2_inode *)inode_table + (ROOT_INODE - 1) ;

        return (struct m_inode*)root_inode;
}        

void ram_mke2fs()
{      
        int block;
        void * block_buffer;

        struct m_inode *root_inode;
        struct ext2_dir_entry root = {2,12,1,2,"."};
        struct ext2_dir_entry root_dot_dot = {2,12,2,2,".."};
        struct ext2_dir_entry lost = {11,20,10,2,"lost+found"}; 
             
        EXT2_DEBUG();
        
        root_inode = ram_ext2_get_root_inode ();

        block = root_inode->inode.i_block[0];
        block_buffer = (void *)(ram_start + EXT2_BLOCK_SIZE * block);
        
        _memcpy(block_buffer, &root, EXT2_DIR_REC_LEN(1));
        block_buffer += EXT2_DIR_REC_LEN(1);

        _memcpy(block_buffer, &root_dot_dot, EXT2_DIR_REC_LEN(1));
        block_buffer += EXT2_DIR_REC_LEN(2);

        _memcpy(block_buffer, &lost, EXT2_DIR_REC_LEN(10));

}
