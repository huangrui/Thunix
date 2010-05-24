/* 
 * thunix/fs/inode.c
 *
 * it contains the most important function of ext2 fs here
 * so lets look it carefully
 */

#include <thunix.h>
#include <fs_ext2.h>
#include <string.h>


static int _bmap(struct m_inode * inode,int block,int create)
{
        int i;
        void *block_buffer;


	if (block<0)
		panic("_bmap: block<0");
        /* we temporey just handle double indirect at most */
	if (block >= 12 + 256 + 256*256)
		panic("_bmap: block>big"); 

        /* Direct */
	if (block < 12) {
		if (create && !inode->inode.i_block[block])
			if (inode->inode.i_block[block]=ext2_alloc_block(34)) {
				inode->i_ctime=CURRENT_TIME;
				inode->i_dirt=1;
			}
		return inode->inode.i_block[block];
	}

        /* Indirect */
	block -= 12;
	if (block < 256) {
		if (create && !inode->inode.i_block[12])
			if ((inode->inode.i_block[12]=ext2_alloc_block(inode->inode.i_block[12]))) {
				inode->i_dirt=1;
				inode->i_ctime=CURRENT_TIME;
			}
		if (!inode->inode.i_block[12])
			return 0;
		if (!(block_buffer = bread(inode->inode.i_block[12])))
			return 0;
		i = ((unsigned int *) block_buffer)[block];
		if (create && !i)
			if ((i=ext2_alloc_block(inode->inode.i_block[12]))) 
				((unsigned int *) block_buffer)[block]=i;

		/* brelse(bh); */
		return i;
	}

        /* Double Indirect */
	block -= 256;
	if (create && !inode->inode.i_block[13])
		if ((inode->inode.i_block[13]=ext2_alloc_block(inode->inode.i_block[13]))) {
			inode->i_dirt=1;
			inode->i_ctime=CURRENT_TIME;
		}
	if (!inode->inode.i_block[13])
		return 0;
	if (!(block_buffer=bread(inode->inode.i_block[13])))
		return 0;
	i = ((unsigned int *)block_buffer)[block>>8];
	if (create && !i)
		if ((i=ext2_alloc_block(inode->inode.i_block[13]))) 
			((unsigned int *) block_buffer)[block>>8]=i;
        
        /*brelse(bh);*/
	if (!i)
		return 0;
	if (!(block_buffer=bread(i)))
		return 0;
	i = ((unsigned int *)block_buffer)[block&255];
	if (create && !i)
		if ((i=ext2_alloc_block(inode->inode.i_block[13]))) 
			((unsigned int *) block_buffer)[block&511]=i;

	/*brelse(bh);*/
	return i;
}

int bmap(struct m_inode * inode,int block)
{
	return _bmap(inode,block,0);
}

int create_block(struct m_inode * inode, int block)
{
	return _bmap(inode,block,1);
}


struct ext2_inode * ext2_get_inode (int inr)
{
        unsigned int group_num;
        unsigned int group_offset;
        unsigned int inode_block;
        unsigned int offset;
        struct ext2_group_desc * desc;
        void * block_buffer;

        EXT2_DEBUG();
        
        group_num = ext2_get_group_num (inr, INODE);
        group_offset = ext2_get_group_offset (inr, INODE);

        desc = ext2_get_group_desc (group_num);
        inode_block = group_num * EXT2_BLOCKS_PER_GROUP
                + desc->bg_inode_table
                + group_offset / EXT2_INODES_PER_BLOCK;

        block_buffer = (struct ext2_inode*)bread(inode_block);
        offset = (group_offset & (EXT2_INODES_PER_BLOCK - 1));
                
        return block_buffer + offset;
}


static void ext2_read_inode (struct m_inode * inode)
{
        struct ext2_inode *raw_inode = ext2_get_inode (inode->i_num);

        memcpy (&inode->inode, raw_inode, sizeof(struct ext2_inode));
}


static void ext2_write_inode (struct m_inode * inode)
{
        struct ext2_inode *raw_inode = ext2_get_inode (inode->i_num);
        
        memcpy (raw_inode, &inode->inode, sizeof(struct ext2_inode));
}

#ifdef RAM_EXT2_FS
struct m_inode * ext2_iget(int inr)
{
        struct m_inode *inode;
        void *inode_table;
        
        inode_table = ram_start + EXT2_BLOCK_SIZE * 10;
        
        inode = (struct m_inode*) ((struct ext2_inode *)inode_table + (inr-1));
        inode->i_num = inr;

        return (struct m_inode*)inode;
}
#else
struct m_inode * ext2_iget (int inr)
{
        struct m_inode inode;

        /* Yeah, i found 
           struct m_inode *inode;
           ....
           is really a bad and unsafe operation.
           'cause the compiler just alloc 4byte 
           space (just for a pointer to inode).
           
           so the follwing instruction :
           memset(inode, 0, sizeof (struct m_inode));
           will cover others data, then break into some
           unknow conditions .

           i think the safe way is :
           struct m_inode *inode;
           inode = malloc(*inode);

           yeah, we need our own memeory allocation function, 
           but not implemented yet ,
           so we can do it silly:
           struct m_inode;
           memset(&m_inode, sizeof (m_inode));

           So we need to implement our malloc() somehow, but for now,
           it's ok since we just use RAM.
        */


        EXT2_DEBUG();
        memset (&inode, 0, sizeof (struct m_inode));
        inode->i_num = inr;
        ext2_read_inode (&inode);

        return inode;
}
#endif

void ext2_iput(struct m_inode * inode)
{

        EXT2_DEBUG();
        if (!inode)
                return;
 
        inode->i_count --;
        if (!inode->i_count)
                panic("iput: trying to free free inode");
  
  
        if (!inode->i_nlinks) {
                ext2_free_inode(inode);
                return;
        }

        if (inode->i_dirt) {
                ext2_write_inode(inode);
                inode->i_count--;
        }
        return;
}
