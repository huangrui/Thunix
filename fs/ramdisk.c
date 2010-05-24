#include <fs_ext2.h>
#include <thunix.h>
#include <string.h>
#include <rd.h>

unsigned long ram_start;
static unsigned long ram_end;


void inline rd_read(void *mem, void *ram, int block)
{
        memcpy(mem, ram, RD_BLOCK_SIZE);
}

void inline rd_write(void *ram, void *mem, int block)
{
        memcpy(ram, mem, RD_BLOCK_SIZE);
}

void inline rd_read_block(void *mem, void *ram)
{
        rd_read(mem, ram, 1);
}

void inline rd_write_block(void *ram, void *mem)
{
        rd_write(ram, mem, 1);
}

#ifdef RAM_EXT2_FS
void * bread (unsigned int block)
{
        unsigned long ram;

        ram = ram_start + block * EXT2_BLOCK_SIZE;
        
        return (void*)ram;
}

void bwrite(unsigned int block, char *buf)
{
	unsigned long ram;
	ram = ram_start + block * EXT2_BLOCK_SIZE;

	memcpy(ram, buf, EXT2_BLOCK_SIZE);
}
#else
void * bread (unsigned int block)
{
        static void block_buffer[1024];
        void *ram;
        
        ram = ram_start + block * RD_BLOCK_SIZE;
        rd_read_block(block_buffer,ram);
        
        return block_buffer;
}

void bwrite(unsigned int block, char *buf)
{
	unsigned long ram;
	ram = ram_start + block * RD_BLOCK_SIZE;

	rd_write_block((void *)ram, buf);
}

#endif /* bread */


/*
 * we make it to be a 1.44M ram floppy disk 
 */
void rd_init()
{
        EXT2_DEBUG();
        ram_start = 0x400000;
        ram_end   = ram_start + 1440 * 1024;
}
