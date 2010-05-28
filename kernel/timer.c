#include <asm/io.h>
#include <asm/system.h>
#include <timer.h>
#include <types.h>
#include <thunix.h>

volatile unsigned long timer_ticks = 0;
volatile unsigned long count_down = 0;


//extern current_DOR;

#define TIME_REQUESTS 64

struct timer_list {
        unsigned long timer_ticks;
        void (*fn) ();
        struct timer_list *next;
} timer_list[TIME_REQUESTS], * next_timer = NULL;



void add_timer( unsigned long timer_ticks, void (*fn)(void))
{
        struct timer_list *p;
        
        if (!fn)
                return ;
        cli();
        if (timer_ticks <= 0)
                (fn)();
        else {
                for (p = timer_list; p < timer_list + TIME_REQUESTS ; p ++)
                        if (!p->fn)
                                break;
                if (p >= timer_list + TIME_REQUESTS)
                        panic("No more time requests free");
                p->fn = fn;
                p->timer_ticks = timer_ticks;
                next_timer = p;
                while (p->next && p->timer_ticks > p->next->timer_ticks) {
                        p->timer_ticks -= p->next->timer_ticks;
                        fn = p->fn;
                        p->fn = p->next->fn;
                        p->next->fn = fn;
                        
                        timer_ticks = p->timer_ticks;
                        p->timer_ticks = p->next->timer_ticks;
                        p->next->timer_ticks = timer_ticks;
                        p = p->next;
                }
        }
        sti();
}


/* 
 *   Yeah, we are implenmenting our SLEEP function,
 * 
 *   Hope it works well cause i just know the folloing
 *  ways to implenment it by now. And you konw, it's 
 *  very simple!
 *
 */
void sleep(unsigned long sleep_value)
{
        unsigned long now_ticks = timer_ticks;
        do{
                ;
        }while ( timer_ticks < now_ticks + sleep_value);
        
}


void do_timer(void)
{

        /*
         * Yeah, we also implenment a COUNT_DOWN here.
         * it's sounds like we have lots things well.
         *
         */
        if (count_down)
                count_down --;


        if (next_timer) {
                next_timer->timer_ticks --;
                while (next_timer && next_timer->timer_ticks <=0) {
                        void  (*fn) (void);
                        
                        fn = next_timer->fn;
                        next_timer->fn = NULL;
                        next_timer = next_timer->next;
                        (fn)();
                }
        }

        //if (current_DOR & 0xf0)
        //      do_floppy_timer();
}

void timer_init(int hz)
{
	unsigned int divisor = 1193180/hz;
	outb(0x36, 0x43);
	outb(divisor&0xff, 0x40);
	outb(divisor>>8, 0x40);
	set_trap_gate(0x20,timer_interrupt);
	outb(inb(0x21)&0xfe, 0x21);
}
