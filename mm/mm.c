#include <mm.h>


long start_mem = 0x800000;
long end_mem   = 0x2000000;

unsigned long *pg_dir = (unsigned long *)PG_DIR;
unsigned long *pg_table = (unsigned long *)PG_TABLE;

static unsigned char mem_map[PG_PAGES] = {0,};
static unsigned long HIGH_MEMORY=0;

#define invalidate() __asm__("movl %%eax, %%cr3"::"a"(PG_DIR))


static inline volatile void oom(void)
{
	panic("out of memory");

}


void setup_page(void)
{

        unsigned long address = 0;
        unsigned long table = PG_TABLE;
        int i;

        for (i=0; i<PG_PAGES; i++) {
                pg_table[i] = address | 7;
                address += PG_SIZE;
        }
        
        for (i=0; i<8; i++) {
                pg_dir[i] = table | 7;
                table += PG_SIZE;
        }

        for (; i<1024;i++)
                pg_dir[i] = 6;

        __asm__("movl $0x800000,%%eax\n"
                "movl %%eax, %%cr3\n"
                "movl %%cr0, %%eax\n"
                "orl  $0x80000000, %%eax\n"
                "movl %%eax, %%cr0\n"
                ::"a"(PG_DIR));
        
}
	  

unsigned long get_free_page(void)
{
  int i;
  
  for (i=PG_PAGES; i>0; i--)
          if (mem_map[i] == 0)
                  break;
  
  if (i) {
          mem_map[i]++;
          return i<<12;
  }
  oom();
  return 0;
}

void free_page(unsigned long addr)
{
        if (addr >= HIGH_MEMORY)
                panic("Trying to free nonexistent page");
  
        addr >>= 12;
        if (mem_map[addr]--)
                return;
  
        mem_map[addr] = 0;
        panic("Trying to free free page");
}



int free_page_tables(unsigned long from,unsigned long size)
{
        unsigned long *pg_table;
        unsigned long * dir, nr;
        
        if (from & 0x3fffff)
                panic("free_page_tables called with wrong alignment");
        if (!from)
                panic("Trying to free up swapper memory space");

        size = (size + 0x3fffff) >> 22;
        dir = (unsigned long *) ((from>>20) & 0xffc); /* _pg_dir = 0 */

        for ( ; size-->0 ; dir++) {
                if (!(1 & *dir))
                        continue;
                pg_table = (unsigned long *) (0xfffff000 & *dir);
                for (nr=0 ; nr<1024 ; nr++) {
                        if (1 & *pg_table)
                                free_page(0xfffff000 & *pg_table);
                        *pg_table = 0;
                        pg_table++;
                }

                free_page(0xfffff000 & *dir);
                *dir = 0;
        }
        
        invalidate();
        return 0;
}


int copy_page_tables(unsigned long from,unsigned long to,long size)
{
        unsigned long * from_page_table;
        unsigned long * to_page_table;
        unsigned long this_page;
        unsigned long * from_dir, * to_dir;
        unsigned long nr, ndir;
  
        if ((from&0x3fffff) || (to&0x3fffff))
                panic("copy_page_tables called with wrong alignment");
        from_dir = (unsigned long *) ((from>>20) & 0xffc); /* _pg_dir = 0 */
        to_dir = (unsigned long *) ((to>>20) & 0xffc);
        ndir = ((unsigned) (size+0x3fffff)) >> 22;

        for( ; ndir-->0 ; from_dir++,to_dir++) {
                if (1 & *to_dir)
                        panic("copy_page_tables: already exist");
                if (!(1 & *from_dir))
                        continue;
                from_page_table = (unsigned long *) (0xfffff000 & *from_dir);

                if (!(to_page_table = (unsigned long *) get_free_page()))
                        return -1;	/* Out of memory, see freeing */
                *to_dir = ((unsigned long) to_page_table) | 7;
                nr = (from==0)?0xA0:1024;

                for ( ; nr-- > 0 ; from_page_table++,to_page_table++) {
                        this_page = *from_page_table;
                        if (!(1 & this_page))
                                continue;
                        this_page &= ~2;
                        *to_page_table = this_page;
                        if (this_page > LOW_MEM) {
                                *from_page_table = this_page;
                                this_page >>= 12;
                                mem_map[this_page]++;
                        }
                }
        }

        invalidate();
        return 0;
}


void un_wp_page(unsigned long * table_entry)
{
        unsigned long old_page,new_page;
  
        old_page = 0xfffff000 & *table_entry;
        if (old_page >= LOW_MEM && mem_map[MAP_NR(old_page)]==1) {
                *table_entry |= 2;
                invalidate();
                return;
        }
        if (!(new_page=get_free_page()))
                oom();
        if (old_page >= LOW_MEM)
                mem_map[MAP_NR(old_page)]--;
        *table_entry = new_page | 7;
        invalidate();
        copy_page(old_page,new_page);
}	


void do_wp_page(unsigned long error_code,unsigned long address)
{
        un_wp_page((unsigned long *)
                   (((address>>10) & 0xffc) + (0xfffff000 &
                                               *((unsigned long *) ((address>>20) &0xffc)))));
  
}




unsigned long put_page(unsigned long page,unsigned long address)
{
        unsigned long tmp, *page_table, *page_dir;
        
        /* NOTE !!! This uses the fact that _pg_dir=0 */

        if (page < LOW_MEM || page > HIGH_MEMORY) {
                printk("\nTrying to put page %p at %p\n",page,address);
                panic("\nput_page:...wrong...");
        }
        if (mem_map[page>>12] != 1) {
                printk("\nmem_map disagrees with %p at %p\n",page,address);
                panic("put_page:...wrong...");
        }
        page_dir = (unsigned long *) ((address>>20) & 0xffc);
        if ((*page_dir)&1)
                page_table = (unsigned long *) (0xfffff000 & *page_dir);
        else {
                if (!(tmp=get_free_page()))
                        return 0;
                *page_dir = tmp|7;
                page_table = (unsigned long *) tmp;
        }
        page_table[(address>>12) & 0x3ff] = page | 7;
  
        return page;
}

void setp(unsigned long address)
{
        unsigned long *pg_dir, *pg_table;
        pg_dir = (unsigned long *)((address>>20)&0xffc);
        *pg_dir |= 7;
        pg_table = (unsigned long *)(*pg_dir &0xfffff000);
        *pg_table |= 7;
}


void get_empty_page(unsigned long address)
{
	unsigned long tmp;
        
	if (!(tmp=get_free_page()) || !put_page(tmp,address)) {
		free_page(tmp);		/* 0 is ok - ignored */
		oom();
	}
}



#define _fs() ({                                                \
                        register unsigned short __res;          \
                        __asm__("mov %%fs,%%ax":"=a" (__res):); \
                        __res;})

#define _ds() ({                                                \
                        register unsigned short __res;          \
                        __asm__("mov %%ds,%%ax":"=a" (__res):); \
                        __res;})

#define _es() ({                                                \
                        register unsigned short __res;          \
                        __asm__("mov %%es,%%ax":"=a" (__res):); \
                        __res;})

void do_no_page(unsigned long err_code, unsigned long cr2, unsigned long esp_ptr)
{
  
        unsigned long *esp = (unsigned long*)esp_ptr;
        
        printk("\ncr2:\t%08x\nerr:\t%08x\n",cr2,err_code);

        printk("EIP:\t%04x:%p\nEFLAGS:\t%p\nESP:\t%04x:%p\n",
               esp[1],esp[0],esp[2],esp[4],esp[3]);

        printk("fs: %04x\n",_fs());
        printk("ds: %04x\n",_ds());
        printk("es: %04x\n",_es());
}
/*
  void do_no_page(unsigned long address, long fs, long es, long ds, unsigned long edx, unsigned long ecx, unsigned long error_code, unsigned long eip, unsigned long cs, unsigned long eflags/*, unsigned long esp, unsigned long ss/)
{
  int blcok[8];
  int bk;
  static int i=0;
  unsigned long page;
  unsigned long tmp = 0;
  /*printk("\nPage Not Found:The wrong address is %p\n",address);
  //__asm__ ("movl %%eip, %%eax":"=a"(eip):);
  eip = read_eip();
  printk("The EIP is: %p\n", eip);/
 
  printk("\nTHE PAGE FAULT INFORMATION\n\n");
  printk("THE FAULT ADDRESS:\t%p\n",address);
  printk("EIP:\t%p:%p\n",cs,eip);
  //printk("ESP:\t%p:%p\n",ss,esp);
  printk("EFLAGS:\t%p\n",eflags);
  printk("ERROR_CODE:%p\n",error_code);
  printk("ds:\t%p\nes:\t%p\nfs:\t%p\n",ds,es,fs);

  while(1);
  
  /*address &= 0xfffff000;/
			  get_empty_page(address);
			  /*setp(address);/


  if (address==tmp)
    i++;
  if (i>0)
    panic("\ndo_no_page");
  tmp = address;
  /* tmp = address;
  bk = tmp/BLOCK_SIZE;
  for (i = 0; i<8; bk++, i++)
    block[i] = bmap(current->executable, block);
  bread_page(page, block);
  i = tmp + /*...As process part not implemented, I can do here so far...*/
   
  /* for(;;)
     ;/
  
}
*/


void mem_init(void)
{
        int i;
  
        HIGH_MEMORY = end_mem;
        for (i=0 ; i<PG_PAGES ; i++)
                mem_map[i] = USED;
        i = MAP_NR(start_mem);
        end_mem -= start_mem;
        end_mem >>= 12;
        while (end_mem-->0)
                mem_map[i++]=0;
        setup_page();
}

void calc_mem(void)
{
        int i,j,k=0,free=0;
        long * pg_tbl;
  
        for(i=0 ; i<PG_PAGES ; i++)
                if (!mem_map[i]) free++;
        printk("\n%d pages free (of %d)\n\r",free,PG_PAGES);
        for(i=0; i<1024 ; i++) {
                if (1&pg_dir[i]) {
                        pg_tbl=(long *) (0xfffff000 & pg_dir[i]);
                        for(j=k=0 ; j<1024 ; j++)
                                if (pg_tbl[j]&1)
                                        k++;
                        printk("Pg_dir[%d] uses %d pages\n",i,k);
                }
        }
        
}


void mem_debug(void)
{
        extern int find_first_zero(void *);
        extern int testb(void *, unsigned int);
        int i = 0;
        printk("\nThe address of mem_init and calc_mem are:%p\t%p\n",mem_init,calc_mem);
        for (; i<8;i++) {
                printk("pg_dir[%d]  = %p\t",i, pg_dir[i]);
                printk("*[] = %p\n",*(unsigned long *)(pg_dir[i]&0xfffff000));
        }

        unsigned long *pg_table1 = (unsigned long *)(pg_dir[1]&0xfffff000);
        printk("pg_table1 = %p\n", pg_table1);
        for (i = 0; i < 3; i++)
                printk("*table[1][%d] = %p\n",i, /**(unsigned long*)*/(pg_table1[i]));
        
        /*fs*/
        printk("Address of ffz= %p\n",find_first_zero);
        printk("Address of tsb= %p\n",testb);
        printk("the tsb result= %p\n",(*((unsigned char *)0x401000)));
        printk("the tsb result= %d\n",(*((unsigned char*)0x401000) & (1<<7))!=0);
  /*
    printk("is this OK...???\n");
    memset(0x400000,'f',12);
    printk("go there>?");
  */
}

void page_test(void)
{
        int i = 0;
        unsigned char *p = (unsigned char *)0x00900006;
        for (;i<10;i++)
                *p++ = 'A'+i;
        *p = '\0';
        
        printk("\n%c",*(p-10));
        printk("\n%s",(p-10));
}
