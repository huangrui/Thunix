#ifndef _MM_H
#define _MM_H


#define PG_SIZE   (0x1000)
#define PG_DIR    (0x800000)
#define PG_TABLE  (PG_DIR + PG_SIZE)

#define LOW_MEM   (0x100000)
#define PG_MEMORY (32*1024*1024)
#define PG_PAGES  (PG_MEMORY>>12)
#define MAIN_MEM  (0x800000)

#define USED         (100)
#define MAP_NR(addr) (((addr)>>12))

#define TABLE_SIZE   (0x400000)

#define copy_page(from, to)			\
  __asm__("cld;rep;movsl\n\t"			\
	  ::"S"(from),"D"(to),"c"(1024))


#endif   /* mm.h */
