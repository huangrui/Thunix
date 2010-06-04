/**
 *  thuix/kernel/fd.c
 *
 *  The floppy driver but now seems can't work correctly
 *
 *  Aleaxander (C) 2007-2008
 *
 *             Aleaxander@gmail.com
 *
 */ 

#include <fd.h>
#include <timer.h>
#include <thunix.h>
#include <asm/io.h>
#include <asm/system.h>
#include <stdio.h>
#include <string.h>
#include <hexdump.h>

extern unsigned long count_down;

#define FALSE 0
#define TRUE  1


#define LOG //printk


#define immoutb_p(val,port)                                             \
        __asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (val)),"i" (port))


/* these are globals used by get_result() */
#define MAX_REPLIES 7
static unsigned char reply_buffer[MAX_REPLIES];
#define ST0 (reply_buffer[0])
#define ST1 (reply_buffer[1])
#define ST2 (reply_buffer[2])
#define ST3 (reply_buffer[3])


static struct floppy_struct {
	unsigned int size, sector, head, track, stretch;
	unsigned char gap,rate,spec1;
} floppy =  {
	 2880,18,2,80,0,
         0x1b,0x00,0xCF 	/* 1.44MB diskette */
};


/* Store the return vaule of get_result, we need it to do some check */
//static int res;

static int done = FALSE;
static int motoron = FALSE;
static int changed = FALSE;
static unsigned char sr0;
static unsigned char fdc_track = 255;

static int track;
static int sector;

/*
 *   send a byte to FD_DATA register 
 * @param: byte is the byte that needed to send to the FD_DATA
 * @return: none
 */
static void send_byte(unsigned char byte)
{
        volatile int msr;
        int counter;

        //LOG("send_byte() called ...\n");

        for (counter = 0; counter < 1000; counter++) {
                sleep(1); /* delay 10s */
                msr = inb_p(FD_STATUS) & (STATUS_READY | STATUS_DIR);
                if (msr == STATUS_READY) {
                        outb(byte,FD_DATA);
                        return ;
                }
        }
        LOG("Unable to send byte to FDC\n");
}
/*
 * get *ONE* byte of results from FD_DATA register then return what 
 * it get, or retrun -1 if faile.
 */
static int get_byte()
{
        volatile int msr;
        int counter;
        
        //LOG("get_byte() called ...\n");

        for (counter = 0; counter < 1000; counter ++) {
                sleep(1); /* delay 10ms */
                msr = inb_p(FD_STATUS) & (STATUS_DIR|STATUS_READY|STATUS_BUSY);
                if (msr == (STATUS_DIR|STATUS_READY|STATUS_BUSY))
                        return inb_p(FD_DATA);
        }
        LOG("get_byte: get status times out!\n");
        return -1;
}

/*
 * get *ALL* the results from the FD_DATA register then store 
 * it in the global fileds reply_buffer. that's the only 
 * diffrence between get_byte() and get_result().
 *
 * @param: none
 * @return: the number of reply chars
 *
 */
static int get_result(void)
{
        int i = 0, counter, msr;
        
        //LOG("get_result() called ...\n");

        for (counter = 0; counter < 1000; counter ++) {
                sleep(1); /* delay 10ms */
                msr = inb_p(FD_STATUS) & (STATUS_DIR|STATUS_READY|STATUS_BUSY);
		//LOG("msr %d: %x\n", i, msr);
                if (msr == STATUS_READY)
                        return i;
                if (msr == (STATUS_DIR|STATUS_READY|STATUS_BUSY)) {
                        if ( i >= MAX_REPLIES)
                                break;
                        reply_buffer[i++] = inb_p(FD_DATA);
                }
        }
        LOG("get_result:get status times out!\n");
        
        return -1;
}


/*
 * This waits for FDC command to complete 
 *
 * @param: sensei
 * @return: if successfull then returns TRUE, or FALSE
 *
 */
static int wait_fdc(int sensei)
{
        int time_out;
        count_down = 30; /* set count_down init. value to 300 ms*/

	/* 
 	 * As I was developing thunix on bochs, that means we don't
	 * need a hardware delay, so I make the count_down to 10 ms
	 */
        count_down = 1; 

        /* wait for FLOPPY_INTERRUPT hander to signal command finished */
        while (!done && count_down)
                ;
        time_out = count_down;
#if 0       
        do{
                printk("time_out:%d\n",time_out);
                printk("done:    %d\n",done);
        }while(0);
        res = get_result(); /* get the result of the command */
#endif

        /* 
         * we use get_byte() but NOT get_result() here, because i don't
         * know where the error happened. or maybe get_byte() is better
         * than get_result(), it just come from the test
         *
        ST0 = get_byte();
        ST1 = get_byte();
        ST2 = get_byte();
        ST3 = get_byte();
         */
	
	memset(reply_buffer, 0, sizeof(reply_buffer));
	get_result();
#if 0
	hexdump(reply_buffer, sizeof(reply_buffer));
#endif

        if (sensei) {
                /* send a "sense interrupt status" command */
                send_byte(FD_SENSEI);
                sr0 = get_byte();
                fdc_track = get_byte();
        }
        
	LOG("time left: %d\t done: %d\n", time_out, done);
        done = FALSE;
        if (time_out == 0)
                return FALSE;
        else
                return TRUE;
}
       

/**
 * Converts liner sector address to head/track/sector
 *
 * @param: sector is the liner sector we wanna convert.
 * @param: *head, save the head number to head
 * @param: *track, save the track number to track
 * @param: *sector, save the sector number to sector
 *
 * we return all the info. by the POINTER args
 *
 */
static void lba_to_chs(int line_sector, int *head, int *track, int *sector)
{
        //LOG("sector_to_hts() called ...\n");
        *sector = line_sector % floppy.sector;
	*sector += 1;
        
        line_sector /= floppy.sector;
        
        *head = line_sector % floppy.head;
        *track = line_sector / floppy.head;
}


/* test whether the motor is on or not */
static inline int is_motor_on()
{
        //LOG("is_motor_on() called ...\n");
        return motoron;
}



/* Turns the motor on if not */
static void motor_on(void)
{
        //LOG("motor_on() called ...\n");
        if ( !is_motor_on()) {
		//LOG("Switch on motor...\n");
                outb_p(0x1c,FD_DOR);
                sleep(30);  /* delay 300 milliseconds for motor on */
                motoron = TRUE;
        }
}


/* Truns the motor off if on */
static void motor_off (void)
{
        //LOG("motor_off() called ...\n");
        if (is_motor_on() ) {
                count_down = 30;  /* start motor kill countdown: about 300 ms */
                while(count_down)
                        ;
                outb_p(0x0c,FD_DOR);
                motoron = FALSE;
        }
}


/* recalibrate the drive */
static void recalibrate(void)
{

        //LOG("recalibrate() called ...\n");

        /*turn the motor on first */
        motor_on();
        
        /* send actual command bytes */
        send_byte(FD_RECALIBRATE);
        send_byte(0);

        /* wait until seek finished */
        wait_fdc(TRUE);
}



/* seek to track */
static int seek(int track, int head)
{
        //LOG("seek() called ...\n");
        
	/*
	if (track == 0) {
		LOG("RECALIBRATE...\n");
                recalibrate();
		return TRUE;
	}
	*/

        if (fdc_track == track) 
                return TRUE;   /* already there*/

        /* send actual command bytes */
        send_byte(FD_SEEK);
        send_byte(head << 2);
        send_byte(track);
        
        /* wait until seek finished */
        if ( !wait_fdc(TRUE) )
                ;//return FALSE;  /* time out */
        
	//LOG("ST0: %x\t ST1: %x\n", sr0, fdc_track);
        if ( ((sr0 & 0xF8) != 0x20) || (fdc_track != track)) {
		LOG("Seek track#: %d failed\n", track);
                return FALSE;
        } else {
                LOG("Seek track#: %d OK ...\n", track);
                return TRUE;
        }
}




/*
 *    reset the floppy.
 *
 *    The first thing that the driver needs to do is reset the controller.This 
 * will put it in a known state. To reset the primary floppy controller,(in C)
 *
 * 1.write 0x00 to the DIGITAL_OUTPUT_REG of the desired controller
 * 2.write 0x0C to the DIGITAL_OUTPUT_REG of the desired controller
 * 3.wait for an interrupt from the controller
 * 4.check interrupt status (this is function 0x08 of controllers)
 * 5.write 0x00 to the CONFIG_CONTROL_REG
 * 6.configure the drive desired on the controller (function 0x03 of controller)
 * 7.calibrate the drive (function 0x07 of controller)
 *
 */
static void reset( )
{
        //LOG("reset() called ...\n");

        /* stop the motor and disable IRQ/DMA */
        outb_p(0x0c,FD_DOR);

        /* program data rate (500K/s) */
        outb_p(0,FD_DCR);
        
        /* re-enable interrupts */
        outb_p(0x1c,FD_DOR);

        /* resetting triggered an interrupt - handle it */
        done = TRUE;
        wait_fdc(TRUE);

        /* specify drive timings (got these off the BIOS) */
        send_byte(FD_SPECIFY);
        send_byte(0xdf);      /* SRT = 3ms, HUT = 240ms */
        send_byte(0x06);      /* HLT = 16ms, ND = 0     */

        recalibrate();
}



/*
 * here we will setup the DMA, then we can use it to transfer data
 * more efficiently. For now, we just make it transfer one sector's 
 * data once.
 *
 */
static void setup_DMA(unsigned long addr, int command)
{
	int cmd = (command == FD_READ) ? DMA_READ : DMA_WRITE;
        int count =  512 - 1;
        
        cli();                          /* we need a safe env. */
        
	immoutb_p(4|2,0x0a);            /* mask DMA 2 */

        immoutb_p(0x0,0x0c);               /* clear flip flop */
        
        immoutb_p(cmd,0x0b);

	immoutb_p(addr,4);              /* 8 low bits of addr */
	
        addr >>= 8;
	immoutb_p(addr,4);              /* bits 8-15 of addr */

	addr >>= 8;
	immoutb_p(addr,0x81);           /* bits 16-19 of addr */

	immoutb_p(count & 0xff,5);      /* low 8 bits of count-1 (1024-1=0x3ff) */

	immoutb_p(count >> 8,5);        /* high 8 bits of count-1 */

	immoutb_p(0|2,10);              /* activate DMA 2 */
	sti();
}


/*
 * And now, it's time to implenent the read or write function, that's
 * all the floppy driver mean!
 * 
 * Read/Write one sector once.
 */
static int floppy_rw(int sector, char *buf, int command)
{
	int head;
	char *dma_buffer = buf;
	static char tmp_dma_buffer[512];

	//LOG("TMP dma buffer: %p\n", tmp_dma_buffer);

        lba_to_chs(sector, &head, &track, &sector);
	LOG("head: %d \ttrack: %d \tsector: %d\n", head, track, sector);

        /* turn it on if not */
        motor_on();

        if (inb_p(FD_DIR) & 0x80) {
                changed = TRUE;
                seek(1, head);        /* clear "disk change" status */
                recalibrate();
                motor_off();
                printk("floppy_rw: Disk change detected. You are going to DIE:)\n");
                
                pause();  /* just put it in DIE */
        }

        /* move head to the right track */
        if (!seek(track, head)) {
                motor_off();
                printk("floppy_rw: Error seeking to track#%d\n", track);
                return FALSE;
        }
                
	if ((unsigned long)buf >= 0xff000) {
		dma_buffer = tmp_dma_buffer;
		if (command == FD_WRITE)
			memcpy(dma_buffer, buf, 512);
	}

        setup_DMA((unsigned long)dma_buffer, command);

        send_byte(command);
        send_byte(head<<2 | 0);
        send_byte(track);
        send_byte(head);
	send_byte(sector);
        send_byte(2);           /* sector size = 125 * 2^(2) */
        send_byte(floppy.sector);
        send_byte(0);
        send_byte(0xFF);        /* sector size(only two valid vaules, 0xff when n!=0*/

        if (!wait_fdc(FALSE)) {
                //LOG("wait fdc failed!\n");
                //return 0;
                /*
                printk("Time out, trying operation again after reset() \n");
                reset();
                return floppy_rw(sector, buf, command);
                */
        }

        motor_off();

        if (/*res != 7 || */(ST0 & 0xf8) || (ST1 & 0xbf) || (ST2 & 0x73) ) {
                if (ST1 & 0x02) 
                        printk("Drive is write protected!\n");
                else
                        printk("floppy_rw: bad interrupt!\n");

                return 0;
        } else {
		LOG("floppy_rw: OK\n");
		if ((unsigned long)buf >= 0xff000 && command == FD_READ)
			memcpy(buf, dma_buffer, 512);
		return 1;
        }
}


/* Read ONE sector */
void floppy_read(int sector, void * buf)
{
        floppy_rw(sector, buf, FD_READ);
}

/* Write ONE sector */
void floppy_write(int sector, void * buf)
{
        floppy_rw(sector, buf, FD_WRITE);
}

/*
 * The two following function handles multi-sectors reading
 * and writing.
 */
void floppy_reads(int sector, void *buf, unsigned int sectors)
{
	while (sectors--) {
		floppy_rw(sector++, buf, FD_READ);
		buf += 512;
	}
}

void floppy_writes(int sector, void *buf, unsigned int sectors)
{
	while (sectors--) {
		floppy_rw(sector++, buf, FD_WRITE);
		buf += 512;
	}
}
    
static int times = 0;                
/* 
 * The FLOPPY_INTERRUPT handler 
 *
 * FIXME: the current do_floppy seems wouldn't be called after every
 * interrupt. I have no idea what's wrong with it.
 */ 
void do_floppy(void)
{
        //LOG("floppy_interrupt() called ...\n");
        times ++;
        LOG("floppy interrupt %d times!\n",times);
        /* signal operation finished */
        done = TRUE;
        outb(0x20,0x20);  /* EOI */
}
 
/*
 * OK, finally we got our last thing to do. You are right, that's it,
 * initialing the floppy. As you know, initialization is always easy,
 * just set the interrupt handler and mask off the bit of corresponding 
 * interrupt.
 */
void floppy_init(void)
{
        set_trap_gate(0x26, floppy_interrupt);
        outb(inb_p(0x21)&~0x40,0x21);
}

/* debug fd read */
void Debug_rd(void)
{
	char buf[512];

	LOG("BUF addr: %p\n", buf);

        floppy_read(70, buf);
	hexdump(buf, 128);
}

/* debug fd write */
void Debug(void)
{
        char str[512 * 10] = "hello word! This is just a floppy writing test";
	char *buf = (char *)0x800000;
	memcpy(buf, str, sizeof(str));

	LOG("STRING addr: %p\n", str);

        floppy_write(0, str);
	floppy_writes(0, buf, 10);
	LOG("interrupts happens %d times\n", times);
}
