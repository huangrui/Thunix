/*#include <string.h>*/
#include <console.h>
#include <asm/system.h>
#include <asm/io.h>
#include <keyboard.h>
#include <timer.h>
#include <time.h>
#include <thunix.h>
/*#include <sched.h>*/
#include <rd.h>
#include <fs_ext2.h>
#include <fd.h>

extern void trap_init(void);
extern void con_init(void);
extern void keyboard_init(void);
extern void timer_init(int);
extern long kernel_mktime(struct tm*);
extern int  printk(char *fmt, ...);
extern void timer_interrupt(void);
extern void floppy_interrupt(void);
extern void rd_init(void);
extern void ram_ext2fs_init();
extern void ext2_fs_init();


#define CMOS_READ(addr) ({                      \
                        outb_p(addr, 0x70);     \
                        inb_p(0x71);            \
                })

#define BCD_TO_BIN(val) ((val) = ((val)&0xF) + ((val)>>4)*10)




unsigned long  get_current_time(struct tm * time)
{
        unsigned long current_time;
        do {
                time->tm_sec  = CMOS_READ(0);
                time->tm_min  = CMOS_READ(2);
                time->tm_hour = CMOS_READ(4);
                time->tm_mday = CMOS_READ(7);
                time->tm_mon  = CMOS_READ(8);
                time->tm_year = CMOS_READ(9);
        }while(time->tm_sec != CMOS_READ(0));
        
        

        BCD_TO_BIN(time->tm_sec);
        BCD_TO_BIN(time->tm_min);
        BCD_TO_BIN(time->tm_hour);
        BCD_TO_BIN(time->tm_mday);
        BCD_TO_BIN(time->tm_mon);
        BCD_TO_BIN(time->tm_year);

        /* desrease for mktime to get the right seconds*/
        time->tm_mon--;
        current_time = kernel_mktime(time);

        /* restore it */
        time->tm_mon++;

        return current_time;
}





/* 
 *  Detecting Floppy Drives
 */
void detect_floppy_drive()
{
        /*   First Getting the data from cmos */
        int high, low;
        unsigned char c;
        outb_p (0x10, 0x70);
        c = inb_p (0x71);
        
        /* Decodding it...  */
        high = c >> 4;
        low  = c &0xF;
        
        /*
         *    Now, there are 5 official types of floppy drives that the most
         * CMOSes can detect:
         * Type of drive:		Number the CMOS gives it:
         * 360kb 5.25in		1
         * 1.2mb 5.25in		2
         * 720kb 3.5in		3
         * 1.44mb 3.5in		4
         * 2.88mb 3.5in		5
         * No drive		0
         */
        char *drive_type[6] = { 
                "no floppy drive", 
                "360kb 5.25in floppy drive", 
                "1.2mb 5.25in floppy drive", 
                "720kb 3.5in", "1.44mb 3.5in", 
                "2.88mb 3.5in"
        };
        printk("Floppy drive A is an:\n");
        printk(drive_type[high]);
        printk("\nFloppy drive B is an:\n");
        printk(drive_type[low]);
        printk("\n");
}



extern unsigned long long * idt;
extern unsigned long timer_ticks;
extern unsigned long count_down;
extern char tmp_floppy_area[1024];

extern void keyboard_interrupt(void);
extern void do_timer(void);
extern unsigned long read_eip();
extern void ram_mke2fs();
extern struct m_inode *ext2_namei(char *);

extern void shell_init();



char *trap_msg = "Setting Interrupt Handling ...";
char *con_msg  = "Console initialization ...";
char *kb_msg   = "Keyboard initialization ...";
char *time_msg = "Timer clock initialization ...";
char *flp_msg  = "Floppy initialization ...";
char *rd_msg   = "Ram disk initialization ...";
char *ram_ext2_msg = "Ram ext2 fs initialization ...";
char *ext2_msg = "Ext2 filesystem initialization ...";
char *ram_mke2fs_msg = "Making Ram ext2 filesystem ...";


void init(void)
{
        char ok[] = "[OK]";
        unsigned long startup_time;
        struct tm time;
        
        cli();
	
	malloc_init();
        
        trap_init();
        con_init();        

        printk("%s", kb_msg);
        keyboard_init();
        printk("\t\t%s\n", ok);        

        printk("%s", time_msg);
        timer_init(100);
        printk("\t\t%s\n", ok);
        
        printk("Getting Current time ....");
        startup_time = get_current_time(&time);
        printk("\t\t%s\n", ok);

        printk("%s", flp_msg);
        floppy_init();
        printk("\t\t%s\n", ok);

        printk("%s", rd_msg);
        rd_init();
        printk("\t\t%s\n", ok);

        printk("%s", ram_ext2_msg);
        ram_ext2fs_init();
        printk("\t\t%s\n", ok);

        printk("%s", ext2_msg);
        ext2_fs_init();        
        printk("\t%s\n", ok);

        printk("%s",ram_mke2fs_msg);
        ram_mke2fs();
        printk("\t\t%s\n", ok);
        
        /* Hope it quite safe now */
        sti();
        



#if 0   /* debug like, read curret eip or divide error */
        memcpy((char *)0, "ERROR",4);
        printk("%s\n",(char *)0);
        printk("eip: %08x\n",read_eip());
        a = a/b;
        printk("eip: %08x\n",read_eip());
        printk("eip: %0xx\n",get_eip());
#endif   
        

            
#if 0            
        for (;;) {
                __asm__ ("movb	%%al,	0xb8000+160*24"::"a"(wheel[i]));
                if (i == sizeof wheel)
                        i = 0;
                else
                        ++i;
        }
#endif
      
          
#if 0   /* idt addr test */
        printk("num\taddr\t\tnum\taddr\n");
        for (i = 0; i < 0x32;) {
                printk("%x\t%08x\t",i,idt[i]&0xffff);
                i++;
                printk("%x\t%08x\n",i,idt[i]&0xffff);
                i++;
        }
#endif
     
        
#if 0   /* timer ticks test */
        while (1) {
                if (timer_ticks % 16 == 0)
                        printk("timer ticks: %08x\n",timer_ticks);
        }
#endif
        
#if 0   /* count down  And sleep test */
        count_down = 10;
        while (count_down) {
                printk("counter rest : %08x\n", count_down);
                sleep(9);
        }


        printk("timer_ticks:%d\n",timer_ticks);
        sleep(500); /* sleep 5s */
        printk("timer_ticks:%d\n",timer_ticks);
        pause();
#endif



	/* 
         * the floppy driver is not ready , and i think....
	 */
        
#if 0   /* 
         * floppy driver test 
         *
         * it seems work now! what a happy day - the last day of 2008
         * And laterly the first day of 2009.
         * but it always output something noisy...
         *
         *
         * Bochs is exiting with the following message:
         * [FDD ] read/write command: sector size 33554432 not supported
         *
         * and i got no idea. Please FIX ME.
         *
         */
{
	char *buf = (char *)0x10000;

        //detect_floppy_drive();
        printk("floppy_interrupt addr:%08x\n",floppy_interrupt);
        floppy_read(0, buf, 1);
        printk("returned but not sure read_ok\n");
	hexdump(buf, 64);
}
#endif


#if 0  /* EXT2_FS test */
        /* debug */
        extern char dir[8];
        mkdir("/usr");
        ls("/","-l");
        ls(dir, "-l");
#endif   
        
       
        printk("\n");
        printk("\t**************************\n");
        printk("\t*                        *\n");
        printk("\t*   Welcome to Thunix    *\n");
        printk("\t*                        *\n");
        printk("\t**************************\n");
        printk("\tType 'help' for more information\n");
        printk("\n");

       
        shell_init();

        pause();    
}
