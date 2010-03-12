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

extern unsigned long count_down;

#define FALSE 0
#define TRUE  1


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

char tmp_floppy_area[1024];


/* Store the return vaule of get_result, we need it to do some check */
//static int res;

static int done = FALSE;
static int motoron = FALSE;
static int changed = FALSE;
static unsigned char sr0;
static unsigned char fdc_track = 255;

static int head;
static int track;
static int sector;

/*
 *   send a byte to FD_DATA register 
 * @param: byte is the byte that needed to send to the FD_DATA
 * @return: none
 */
void send_byte (unsigned char byte)
{
        volatile int msr;
        int counter;

        DEBUG(printk("send_byte() called ...\n"));

        for (counter = 0; counter < 1000; counter++) {
                sleep(1); /* delay 10s */
                msr = inb_p(FD_STATUS) & (STATUS_READY | STATUS_DIR);
                if (msr == STATUS_READY) {
                        outb(byte,FD_DATA);
                        return ;
                }
        }
        printk("Unable to send byte to FDC\n");
}
/*
 * get *ONE* byte of results from FD_DATA register then return what 
 * it get, or retrun -1 if faile.
 */
int get_byte()
{
        volatile int msr;
        int counter;
        
        DEBUG(printk("get_byte() called ...\n"));

        for (counter = 0; counter < 1000; counter ++) {
                sleep(1); /* delay 10ms */
                msr = inb_p(FD_STATUS) & (STATUS_DIR|STATUS_READY|STATUS_BUSY);
                if (msr == (STATUS_DIR|STATUS_READY|STATUS_BUSY))
                        return inb_p(FD_DATA);
        }
        printk("get_byte: get status times out!\n");
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
int get_result ( void )
{
        int i = 0, counter, msr;
        
        DEBUG(printk("get_result() called ...\n"));

        for (counter = 0; counter < 1000; counter ++) {
                sleep(1); /* delay 10ms */
                msr = inb_p(FD_STATUS) & (STATUS_DIR|STATUS_READY|STATUS_BUSY);
                if (msr == STATUS_READY)
                        return i;
                if (msr == (STATUS_DIR|STATUS_READY|STATUS_BUSY)) {
                        if ( i >= MAX_REPLIES)
                                break;
                        reply_buffer[i] = inb_p(FD_DATA);
                        i ++;
                }

        }
        printk("get_result:get status times out!\n");
        
        return -1;
}


/*
 * This waits for FDC command to complete 
 *
 * @param: sensei
 * @return: if successfull then returns TRUE, or FALSE
 *
 */
int wait_fdc( int sensei)
{
        int time_out;
        count_down = 1000; /* set count_down init. value to 2 second */
        
        DEBUG(printk("wait_fdc() called ...\n"));


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
         */
        ST0 = get_byte();
        ST1 = get_byte();
        ST2 = get_byte();
        ST3 = get_byte();
              

        if (sensei) {
                /* send a "sense interrupt status" command */
                send_byte(FD_SENSEI);
                sr0 = get_byte();
                fdc_track = get_byte();
        }
        
        done = FALSE;
        if (time_out == 0)
                return FALSE;
        else
                return TRUE;
}
    
int times = 0;                
/* the FLOPPY_INTERRUPT handler */
void floppy_interrupt(void)
{
        DEBUG(printk("floppy_interrupt() called ...\n"));
        times ++;
        printk("floppy interrupt %d times!\n",times);
        /* signal operation finished */
        done = TRUE;
        outb(0x20,0x20);  /* EOI */
}
        

/**
 * Converts liner block address to head/track/sector
 *
 * @param: block is the liner block we wanna convert.
 * @param: *head, save the head number to head
 * @param: *track, save the track number to track
 * @param: *sector, save the sector number to sector
 *
 * we return all the info. by the POINTER args
 *
 */
void block_to_hts(int block, int *head, int *track, int *sector)
{
        DEBUG(printk("block_to_hts() called ...\n"));
        *sector = block % floppy.sector;
        
        block /= floppy.sector;
        
        *head = block % floppy.head;
        *track = block / floppy.head;
}


/* test whether the motor is on or not */
static inline int is_motor_on()
{
        DEBUG(printk("is_motor_on() called ...\n"));
        return motoron;
}



/* Turns the motor on if not */
void motor_on(void)
{
        DEBUG(printk("motor_on() called ...\n"));
        if ( !is_motor_on()) {
                outb_p(0x1c,FD_DOR);
                sleep(100);  /* delay 1 second for motor on */
                motoron = TRUE;
        }
}


/* Truns the motor off if on */
void motor_off (void)
{
        DEBUG(printk("motor_off() called ...\n"));
        if (is_motor_on() ) {
                count_down = 200;  /* start motor kill countdown: about 2s */
                while(count_down)
                        ;
                outb_p(0x0c,FD_DOR);
                motoron = FALSE;
        }
}


/* recalibrate the drive */
void recalibrate(void)
{

        DEBUG(printk("recalibrate() called ...\n"));

        /*turn the motor on first */
        motor_on();
        
        /* send actual command bytes */
        send_byte(FD_RECALIBRATE);
        send_byte(0);

        /* wait until seek finished */
        wait_fdc(TRUE);

        /*turn the motor off */
        motor_off();
}



/* seek to track */
int seek(int track)
{
        DEBUG(printk("seek() called ...\n"));
        
        if (fdc_track == track) 
                return TRUE;   /* already there*/

        /* send actual command bytes */
        send_byte(FD_SEEK);
        send_byte(0);
        send_byte(track);
        
        /* wait until seek finished */
        if ( !wait_fdc(TRUE) )
                return FALSE;  /* time out */
        
        /* now let head settle for 100ms */
        sleep(10);

        if ( ((sr0 & 0xF8) != 0x20) || (fdc_track != track))
                return FALSE;
        else {
                DEBUG(printk("seek ok ...\n"));
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
void reset( )
{
        DEBUG(printk("reset() called ...\n"));

        /* stop the motor and disable IRQ/DMA */
        outb_p(0,FD_DOR);

        /* program data rate (500K/s) */
        outb_p(0,FD_DCR);
        
        /* re-enable interrupts */
        outb_p(0x0c,FD_DOR);

        /* resetting triggered an interrupt - handle it */
        done = TRUE;
        wait_fdc(TRUE);

        /* specify drive timings (got these off the BIOS) */
        send_byte(FD_SPECIFY);
        send_byte(0xdf);      /* SRT = 3ms, HUT = 240ms */
        send_byte(0x06);      /* HLT = 16ms, ND = 0     */

        /* clear "disk change" status */
        seek(1);
        recalibrate();
}



/*
 * here we will setup the DMA, then we can use it to transfer data
 * more efficiently.
 *
 * BUT NOW i have something that can't be sure of it. So i write it 
 * here for FIXING and also for you, the reader.
 *
 * THIS IS i am not sure whether the address of buffer need be below
 * at 1M memory. FIX ME: thank you!
 *
 */
static void setup_DMA(unsigned long addr, int count, int command)
{
        DEBUG(printk("setup_DMA() called ...\n"));

        count = count * 512 - 1;
        
        cli();                          /* we need a safe env. */
        
	immoutb_p(4|2,0x0a);            /* mask DMA 2 */

        immoutb_p(0,0x0c);               /* clear flip flop */
        
        immoutb_p((command == FD_READ)?DMA_READ:DMA_WRITE,0x0b);

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
 */
int floppy_rw(int block, char *blockbuff, int sectors, int command)
{
        

        DEBUG(printk("floppy_rw() called ...\n"));
                

        block_to_hts(block,&head,&track,&sector);

        /* turn it on if not */
        motor_on();

        if ( inb_p(FD_DIR) & 0x80) {
                changed = TRUE;
                seek(1);        /* clear "disk change" status */
                recalibrate();
                motor_off();
                printk("floppy_rw: Disk change detected. You are going to DIE:)\n");
                
                pause();  /* just put it in DIE */
        }

        /* move head to the right track */
        if (!seek(track)) {
                motor_off();
                printk("floppy_rw: Error seeking to track\n");
                return FALSE;
        }
        
        send_byte(FD_SPECIFY);
        send_byte(floppy.spec1);	/* hut etc */
        send_byte(6);			/* Head load time =6ms, DMA */
        
        
        /* program data rate (500k/s) */
        outb_p(0,FD_DCR);
                
        setup_DMA((unsigned long)blockbuff, sectors, command);

        send_byte(command);
        send_byte(head<<2 | 0);
        send_byte(track);
        send_byte(head);
        send_byte(2);           /* sector size = 125 * 2^(2) */
        send_byte(floppy.sector);
        send_byte(floppy.gap);
        send_byte(0xFF);        /* sector size(only two valid vaules, 0xff when n!=0*/


        if ( !wait_fdc(TRUE) ) {
                printk("Time out, DIE!\n");
                return 0;
                /*
                printk("Time out, trying operation again after reset() \n");
                reset();
                return floppy_rw(block, blockbuff, command, sectors);
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
                printk("Congratulations!\nfloppy read_write OK!\n");
                 return 1;
        }
}


void floppy_read(int block, char* blockbuff, int sectors)
{
        floppy_rw(block, blockbuff, sectors, FD_READ);
}

void floppy_write(int block, char* blockbuff, int sectors)
{
        floppy_rw(block, blockbuff, sectors, FD_WRITE);
}

/*
 * OK, finally we got our last thing to do.
 * You are right, that's it, initialing the 
 * floppy. 
 * 
 * As you know, init. is always easy,
 * just set the interrupt handler and mask 
 * off the bit of corresponding interrupt.
 *
 */
void floppy_init(void)
{
        set_trap_gate(0x26,&floppy_interrupt);
        outb(inb_p(0x21)&~0x40,0x21);
}
