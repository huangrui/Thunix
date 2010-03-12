#ifndef __FS_EXT2_H
#define __FS_EXT2_H


#include <types.h>
#include <rd.h>

/*
 * here is the DISK DATA STRUCTURE of EXT2:
 *
 *       |<-------------------Block  Group 0------------------------>|
+-------+-------+-------------+------------+---------------+--------+---...---+-----------------+
| Boot  | super |   Group     | Data block | inode | inode |  Data  |         |	     block	|
| Block | Block | Descriptors |  bitmap    | bitmap| Table | blocks |         |	    group n	|
+-------+-------+-------------+------------+-------+-------+--------+---...---+-----------------+
1 block  1 block   n blocks      1 block    1 block n blcoks n blocks

Ok,that's a nice structure.here are some more detials:
1. I'm goin to use a 1.44M floppy.
2. So, the block size should be 1024B, that's two sector's size,means 1block = 2sector
3. As the EXT2 does, we leave the boot block along, doing nothing to her.It just reversed and takes
   two sectors size.
4. We have ONE super block, ONE group descriptor which means it only takes ONE block size.
5. We have set the block size to be 1024B,so one block group can hold 1024 * 8 = 8096 blocks, and 
   that's quite enough to our 1.44M floppy which totally has 1440 blocks.
6. We use 23 blocks to store the inode disk structure, so there are 23*8=184 inodes per block group.

 *
 */


/*
 * super block structure:
 * include/linux/ext2_fs.h 
 */
struct ext2_super_block 
{
	__u32 s_inodes_count;	/* Inodes count */
	__u32 s_blocks_count;	/* Blocks count */
	__u32 s_r_blocks_count;	/* Reserved blocks count */
	__u32 s_free_blocks_count;	/* Free blocks count */
	__u32 s_free_inodes_count;	/* Free inodes count */
	__u32 s_first_data_block;	/* First Data Block */
	__u32 s_log_block_size;	/* Block size */
	__s32 s_log_frag_size;	/* Fragment size */
	__u32 s_blocks_per_group;	/* # Blocks per group */
	__u32 s_frags_per_group;	/* # Fragments per group */
	__u32 s_inodes_per_group;	/* # Inodes per group */
	__u32 s_mtime;		/* Mount time */
	__u32 s_wtime;		/* Write time */
	__u16 s_mnt_count;		/* Mount count */
	__s16 s_max_mnt_count;	/* Maximal mount count */
	__u16 s_magic;		/* Magic signature */
	__u16 s_state;		/* File system state */
	__u16 s_errors;		/* Behaviour when detecting errors */
	__u16 s_pad;
	__u32 s_lastcheck;		/* time of last check */
	__u32 s_checkinterval;	/* max. time between checks */
	__u32 s_creator_os;		/* OS */
	__u32 s_rev_level;		/* Revision level */
	__u16 s_def_resuid;		/* Default uid for reserved blocks */
	__u16 s_def_resgid;		/* Default gid for reserved blocks */
	__u32 s_reserved[235];	/* Padding to the end of the block */
};

/*
 * and typically, there are only serveral fileds of interest.
 * s_inodes_count   	     records the total inode number the filesystem have, here is 184
 * s_blocks_count   	     records the total block number the filesystem have, here is 1440.
 * s_r_blocks count 	     records the reserved blocks, usually be 5% of the filestem size. 
 *   	       		     here is  72.
 * s_free_blocks_count	     records the free number of free blocks, here is 1393
 * s_free_inodes_count	     records the free number of free inodes, here is 173.
 * s_first_data_block	     records the start block number of block group one, here is 1.
 * s_log_block_size	     block size = 1 << (10 + s_log_blocks_size), so here is ZERO.
 * s_log_frag_size	     similar to s_log_block_size, we don's use it.
 * s_blocks_per_group	     records the block number per group, here is 8092.
 * S_inodes_per_group	     records the block number per group, here is 184.
 * s_magic		     a macro that indicate it IS an EXT2 filesystem, it's 0XEE53
 */  
  

/*  
 *  ext2 group desc structure:
 */
struct ext2_group_desc
{
	__u32 bg_block_bitmap;	/* Blocks bitmap block */
	__u32 bg_inode_bitmap;	/* Inodes bitmap block */
	__u32 bg_inode_table;	/* Inodes table block */
	__u16 bg_free_blocks_count;	/* Free blocks count */
	__u16 bg_free_inodes_count;	/* Free inodes count */
	__u16 bg_used_dirs_count;	/* Directories count */
	__u16 bg_pad;
	__u32 bg_reserved[3];
};

/*
 * As you can see, it's simply, just recode the state of the group, such as in which block the 
 * blocks/inodes bitmap stores, in which block the indoes table stores, and how many free 
 * blocks/inodes the group have now. And the bg_used_dirs_count keep the record of directories 
 * count in this group to keep the blance of each group.
 */

#define EXT2_N_BLOCKS 15

/*
 * ext2 inode structure:
 */
struct ext2_inode
{
	__u16 i_mode;		/* File mode */
	__u16 i_uid;		/* Owner Uid */
	__u32 i_size;		/* 4: Size in bytes */
	__u32 i_atime;		/* Access time */
	__u32 i_ctime;		/* 12: Creation time */
	__u32 i_mtime;		/* Modification time */
	__u32 i_dtime;		/* 20: Deletion Time */
	__u16 i_gid;		/* Group Id */
	__u16 i_links_count;	/* 24: Links count */
	__u32 i_blocks;		/* Blocks count */
	__u32 i_flags;		/* 32: File flags */
	union
	{
		struct
		{
			__u32 l_i_reserved1;
		}
			linux1;
		struct
		{
			__u32 h_i_translator;
		}
			hurd1;
		struct
		{
			__u32 m_i_reserved1;
		}
			masix1;
	}
		osd1;			/* OS dependent 1 */
	__u32 i_block[EXT2_N_BLOCKS];	/* 40: Pointers to blocks */
	__u32 i_version;		/* File version (for NFS) */
	__u32 i_file_acl;		/* File ACL */
	__u32 i_dir_acl;		/* Directory ACL */
	__u32 i_faddr;		/* Fragment address */
	union
	{
		struct
		{
			__u8 l_i_frag;	/* Fragment number */
			__u8 l_i_fsize;	/* Fragment size */
			__u16 i_pad1;
			__u32 l_i_reserved2[2];
		}
			linux2;
		struct
		{
			__u8 h_i_frag;	/* Fragment number */
			__u8 h_i_fsize;	/* Fragment size */
			__u16 h_i_mode_high;
			__u16 h_i_uid_high;
			__u16 h_i_gid_high;
			__u32 h_i_author;
		}
			hurd2;
		struct
		{
			__u8 m_i_frag;	/* Fragment number */
			__u8 m_i_fsize;	/* Fragment size */
			__u16 m_pad1;
			__u32 m_i_reserved2[2];
		}
			masix2;
	}
		osd2;			/* OS dependent 2 */
};
/*
 * Yeah, that looks really quite large, but just only several fileds we should note it.
 *     i_size	 recods the file size in bytes.
 *     i_block	 recods the data block not the filesyste block the file has taken up.      
 *     i_blocks[] that's really IMPORTANT.it stors the block address where the file stores.
 */




struct ext2_sb_info {
        unsigned long s_frag_size;      /* Size of a fragment in bytes */
        unsigned long s_frags_per_block;/* Number of fragments per block */
        unsigned long s_inodes_per_block;/* Number of inodes per block */
        unsigned long s_frags_per_group;/* Number of fragments in a group */
        unsigned long s_blocks_per_group;/* Number of blocks in a group */
        unsigned long s_inodes_per_group;/* Number of inodes in a group */
        unsigned long s_itb_per_group;  /* Number of inode table blocks per group */
        unsigned long s_gdb_count;      /* Number of group descriptor blocks */
        unsigned long s_desc_per_block; /* Number of group descriptors per block */
        unsigned long s_groups_count;   /* Number of groups in the fs */
        //struct buffer_head * s_sbh;     /* Buffer containing the super block */
        //struct ext2_super_block * s_es; /* Pointer to the super block in the buffer */
        //struct buffer_head ** s_group_desc;

        /*
          unsigned long  s_mount_opt;
          uid_t s_resuid;
          gid_t s_resgid;
          unsigned short s_mount_state;
          unsigned short s_pad;
          int s_addr_per_block_bits;
          int s_desc_per_block_bits;
          int s_inode_size;
          int s_first_ino;
          spinlock_t s_next_gen_lock;
          u32 s_next_generation;
          unsigned long s_dir_count;
          u8 *s_debts;
          struct percpu_counter s_freeblocks_counter;
          struct percpu_counter s_freeinodes_counter;
          struct percpu_counter s_dirs_counter;
          struct blockgroup_lock s_blockgroup_lock;
        */
        unsigned int s_free_blocks_count;
        unsigned int s_free_inodes_count;
        unsigned int s_dirs_count;
};



/*
 * second extended file system inode data in memory
 */
struct m_inode {
        struct ext2_inode inode;
        /*addiation part in memory */
        unsigned int i_mode;
        unsigned int i_num;
        unsigned int i_count;
        unsigned int i_nlinks;
        unsigned long i_atime;
        unsigned long i_ctime;
        unsigned char i_lock;
        unsigned char i_dirt;
        unsigned char i_mount;
        unsigned char i_update;
};


#define S_IFREG 1
#define S_IFDIR 2

#define S_ISDIR(m) ( (m & S_IFDIR) == S_IFDIR )
#define S_ISREG(m) ( (m & S_IFREG) == S_IFREG )



#define EXT2_BUFFER            (0x800000)        
#define EXT2_GROUP_DESC_BUFFER (0x800400)
#define EXT2_BITMAP_BUFFER     (0x802000)

#define EXT2_SBI() ((struct ext2_sb_info *)EXT2_BUFFER)
 
#define EXT2_BLOCK_SIZE          (1024)
#define EXT2_INODE_SIZE          (sizeof(struct ext2_inode))


#define EXT2_BLOCKS_PER_GROUP    (8096)
#define EXT2_INODES_PER_GROUP    (184) 
#define EXT2_INODES_PER_BLOCK    (8)

#define EXT2_BLOCKS_PER_GROUP_BITS (13)
#define EXT2_INODES_PER_GROUP_BITS (0)

#define SUPER_BLOCK 1
#define GROUP_DESC  2
#define BLOCK_BITMAP 8
#define INODE_BITMAP 9

#define ROOT_INODE  2

#define BLOCK  1
#define INODE  2


#define EXT2_NAME_LEN 255
struct ext2_dir_entry {
	unsigned int	inode;			/* Inode number */
	unsigned short	rec_len;		/* Directory entry length */
	unsigned char	name_len;		/* Name length */
	unsigned char	file_type;
	char	name[EXT2_NAME_LEN];	        /* File name */
};

#define EXT2_DIR_PAD	 4
#define EXT2_DIR_ROUND 	(EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len)	(((name_len) + 8 + EXT2_DIR_ROUND) & \
					 ~EXT2_DIR_ROUND)

//extern struct ext2_dir_entry * ext2_next_entry(struct ext2_dir_entry *);




/* namei.c */
extern void ext2_add_entry(struct m_inode *, struct ext2_dir_entry *);


/* inode.c */
extern void ext2_iput(struct m_inode *);

/* dir.c */
extern void mkdir(char *);

#endif /* fs.ext2.h */
