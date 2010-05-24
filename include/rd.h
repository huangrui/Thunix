#ifndef _RD_H
#define _RD_H


#define RD_BLOCK_SHIFT  10
#define RD_BLOCK_SIZE 	( 1 << RD_BLOCK_SHIFT )


extern unsigned long ram_start;

extern void * bread(unsigned int);
extern void bwrite(unsigned int, char *);


#endif /* rd.h */
