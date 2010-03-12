/*
 * thunix/fs/ext2.c
 *
 * contains some useful routines of ext2fs
 */

#include <stdarg.h>
//#include <string.h>
#include <fs_ext2.h>
#include <thunix.h>


extern int vsprintf (char *buf, const char *fmt, va_list args);


char * _strcpy (char *dest, const char *src)
{
  char *p = dest;

  while ((*p++ = *src++) != '\0')
    ;

  return dest;
}


int _strcmp(const char *s1, const char *s2)
{
  while ((*s1 == *s2) && *s1 != '\0') {
    s1++;
    s2++;
  }
  
  return (*s1 - *s2);
}

char *_strchr (const char *s, int c)
{
        while (*s) {
                if (*s == c)
                        return (char *) s;
                s++;
        }
        
        return (void *)0;
}

void * _memcpy(void *dest, const void *src, int count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
}

void * copy_block(void *dest, const void *src)
{
        _memcpy(dest, src, EXT2_BLOCK_SIZE);
}



int _strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc; ++sc)
		/* nothing */;
	return sc - s;
}

void * _memset(void *s, int c, int count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}
int is_print(char c)
{
        return ((c >= '!') && (c <= '~') ); 
}

void dump_from_addr(char * addr, int size)
{
        for(; size; size --) {
                if ( is_print(*addr) )
                        printk("%c",*addr);
                else
                        printk("%02x",*addr);
                addr ++;
        }
}



unsigned int ext2_get_group_num(unsigned int num, int type)
{
	unsigned int group_num;

	if (type == BLOCK) {
		group_num = num >> EXT2_BLOCKS_PER_GROUP_BITS;
		return group_num;
	} else if (type == INODE) {
                group_num = (num - 1 ) / EXT2_INODES_PER_GROUP;
		return group_num;
	} else {
		return 0;
	}
}
		

unsigned int ext2_get_group_offset(unsigned int num, int type)
{
	unsigned int group_offset;

	if (type == BLOCK) {
		group_offset = num & (1 << EXT2_BLOCKS_PER_GROUP_BITS - 1);
		return group_offset;
	} else if (type == INODE) {
                group_offset = (num - 1) & ( EXT2_INODES_PER_GROUP - 1);
		return group_offset;
	} else {
		return 0;
	}
}


void ext2_error(char *fmt, ...)
{
        va_list args;
        int i;
        char buf[1024];

        
        va_start(args,fmt);
        i = vsprintf(buf, fmt, args);
        va_end(args);
        
        printk("%s\n",buf);
        panic("EXT2 ERROR ...");
}


void ext2_fs_init()
{
        void *sb;
        void *desc;
        void *bitmap;

        EXT2_DEBUG();

        sb = bread (SUPER_BLOCK);
        copy_block( (void *)EXT2_BUFFER, sb);
       
        desc = bread(GROUP_DESC);
        copy_block( (void *)EXT2_GROUP_DESC_BUFFER, desc);
        
        bitmap = bread(BLOCK_BITMAP);
        copy_block( (void *)EXT2_BITMAP_BUFFER, bitmap);
        
        bitmap = bread(INODE_BITMAP);
        copy_block( (void *)EXT2_BITMAP_BUFFER + EXT2_BLOCK_SIZE, bitmap);
        
}      
               
