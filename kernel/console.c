/**
 *  thunix/kernel/console.c
 * 
 *  The implemention of console write and something about console.
 *
 *  Aleaxander (C) 2007-2008
 *         
 *             Aleaxander@gmail.com
 *
 */

#include <console.h>
#include <string.h>
#include <asm/io.h>
#include <asm/system.h>
/*#include <keyboard.h>*/

static unsigned long video_num_columns;
static unsigned long video_size_row;
static unsigned long video_size_all;
static unsigned long video_num_lines;
static unsigned long video_mem_start;
static unsigned long video_mem_end;
static unsigned long origin;
static unsigned long scr_end;
static unsigned long pos;
static unsigned long x,y;
static unsigned long top,bottom;
//static unsigned char attr = 0x07;
//static unsigned char space = 0x20;
static unsigned short video_port_reg;
static unsigned short video_port_val;
static unsigned short video_erase_char;

static void sysbeep(void);

extern int printk(char *fmt, ...);

static inline void gotoxy(unsigned int new_x, unsigned int new_y)
{
        if (new_x > video_num_columns || new_y >= video_num_lines)
                return ;
        x = new_x;
        y = new_y;
        pos = origin + y*video_size_row + (x << 1);
        /* pos = origin + (y<<8) + (y<<6) +(x<<1); */
}

static inline void set_origin(void)
{
        cli();
        outb_p(12,video_port_reg);
        outb_p(0xff&((origin-video_mem_start)>>9),video_port_val);
        outb_p(13,video_port_reg);
        outb_p(0xff&((origin-video_mem_start)>>1),video_port_val);
        sti();
}

void cls(void)
{
        int i = 0;
        unsigned char c=' ';

        for (; i < video_size_all/2; i++ ) {
                con_write((char *)&c, 1);
        }

	/* goto the begining of the screen after the cls operation */
	gotoxy(0,0);
        
}



static void scrup(void)
{
        if (!top && bottom == video_num_lines) {
                origin += video_size_row;
                pos += video_size_row;
                scr_end += video_size_row;
                if (scr_end > video_mem_end) {
			memcpy((void *)video_mem_start, origin, (video_num_lines - 1) * video_size_row);
                        scr_end -= origin-video_mem_start;
                        pos -= origin-video_mem_start;
                        origin = video_mem_start;
		}

		/* erase the list line */
		memset_word((void *)(scr_end - video_size_row), video_erase_char, video_num_columns);
                set_origin();
                
        } else {
                __asm__("cld\n\t"
                        "rep\n\t"
                        "movsl\n\t"
                        "movl video_num_columns,%%ecx\n\t"
                        "rep\n\t"
                        "stosw"
                        ::"a" (video_erase_char),
                        "c" ((bottom-top-1)*video_num_columns>>1),
                         "D" (origin+video_size_row*top),
                        "S" (origin+video_size_row*(top+1)));
        }
}

static void scrdown(void)
{
        __asm__("std\n\t"
                "rep\n\t"
                "movsl\n\t"
                "addl $2,%%edi\n\t"
                "movl video_num_columns,%%ecx\n\t"
                "rep\n\t"
                "stosw"
                ::"a" (video_erase_char),
                "c" ((bottom-top-1)*video_num_columns>>1),
                "D" (origin+video_size_row*bottom-4),
                "S" (origin+video_size_row*(bottom-1)-4)
                );
        
}

static void lf(void)
{
        if (y + 1 < bottom) {
                y ++;
                pos = pos + video_size_row;
                return;
        }
        scrup();
}

static void ri(void)
{
        if (y > top) {
                y --;
                pos -= video_size_row;
                return;
        }
        scrdown();
}

static void cr(void)
{
        pos -= x << 1;
        x = 0;
}

static void del(void)
{
        if (x) {
                pos -= 2;
                x --;
                *(unsigned short *) pos = video_erase_char;
        }
}

static void insert_char(void)
{
        int i = x;
        unsigned short tmp, old = video_erase_char;
        unsigned short * p = (unsigned short *)pos;

        while (i++ < video_num_columns) {
                tmp = *p;
                *p  = old;
                old = tmp;
                p ++;
        }
}

static void insert_line(void)
{
        int oldtop, oldbottom;
        oldtop = top;
        oldbottom = bottom;
        top = y;
        bottom = video_num_lines;
        scrdown();
        top = oldtop;
        bottom = oldbottom;
}

static void delete_char(void)
{
        int i;
        unsigned short *p = (unsigned short *) pos;
        
        if (x >= video_num_columns)
                return;
        i = x;
  
        while (++i < video_num_columns) {
                *p = *(p+1);
                p++;
        }
        *p = video_erase_char;
}

static void delete_line(void)
{
        int oldtop, oldbottom;
        
        oldtop = top;
        oldbottom = bottom;
        top = y;
        bottom = video_num_lines;
        scrup();
        top = oldtop;
        bottom = oldbottom;
}
 

static int saved_x = 0;
static int saved_y = 0;

static void save_cur(void)
{
        saved_x = x;
        saved_y = y;
}

static void restore_cur(void)
{
        gotoxy(saved_x, saved_y);
}

void get_cursor(int *new_x, int *new_y)
{
        *new_x = x;
        *new_y = y;
}

void set_cursor(void)
{
        cli();
        outb_p(14, video_port_reg);
        outb_p(0xff & ((pos - video_mem_start)>>9), video_port_val);
        outb_p(15, video_port_reg);
        outb_p(0xff & ((pos - video_mem_start)>>1), video_port_val);
        sti();
}

void con_init(void)
{
        /* We have seen os goes there ! */
        extern void keyboard_interrupt(void);
        
        video_num_columns = ORIG_VIDEO_COLS;
        video_size_row = video_num_columns * 2;
        video_num_lines = ORIG_VIDEO_LINES;
        video_size_all = video_size_row * video_num_lines;
        video_erase_char = 0x0720;
        
        video_mem_start = 0xb8000;
        video_mem_end   = 0xba000;
        video_port_reg = 0x3d4;
        video_port_val = 0x3d5;

        origin = video_mem_start;
        scr_end = video_mem_start + video_num_lines * video_size_row;
        top = 0;
        bottom = video_num_lines;
        
        gotoxy(ORIG_X, ORIG_Y);
        set_cursor();
        cls();
        gotoxy(ORIG_X, ORIG_Y);
        set_cursor();
        

        /*
         * Something else need do here
         * say, keyboard interrupt and so on
         */

}

void con_write(char *buf, int nr)
{
        unsigned char c;
        while (nr-- > 0) {
                c = *buf++;
                switch (c) {
                case 10: case 11: case 12:
                        cr();
                        lf();
                        set_cursor();
                        break;
                case 13:
                        cr();
                        lf();
                        set_cursor();
                        break;

                case 127:
                        del();
                        break;

                case 8: /* '\b' */
                        if (x){
                                x--;
                                pos-=2;
                                *(unsigned short*)pos = video_erase_char;
                        }
                        break;

                case 9:/* TAB */
                        c = 8 - (x&7);
                        x += c;
                        pos += c << 1;
                        if (x > video_num_columns) {
                                x -=video_num_columns;
                                pos -=video_size_row;
                                lf();
                        }
                        break;
                case 7:
                        sysbeep();
                        break;

                /* special key handled here */
                case 0xE0:       /* HOME  */
                        x = 0;
                        gotoxy (x, y);
                        break;
                
                case 0xE1:       /* END   */
                        x = video_num_columns - 1;
                        gotoxy (x, y);
                        break;
                
                case 0xE2:       /* UP    */
                        if (y == 0) {
                                scrdown();
                                y = 0;
                        } else
                                y --;

                        gotoxy (x, y);
                        break;

                case 0xE3:       /* DN    */
                        if ( y == video_num_lines) {
                                scrup();
                                y = video_num_lines;
                        } else 
                                y ++;
                        gotoxy (x, y);
                        break;
                        
                case 0xE4:       /* LeFt  */
                        if (x == 0) {
                                x = video_num_columns;
                                y --;
                        } else 
                                x --;
                        gotoxy (x, y);
                        break;

                case 0xE5:       /* RighT */
                        if (x == video_num_columns) {
                                x = 0;
                                y ++;
                        } else
                                x +=1;            
                        gotoxy (x, y);
                        break;
                        
                case 0xE6:       /* PGUP  */
                        break;
                case 0xE7:       /* PGDN  */       
                        break;
                case 0xE8:       /* INS   */
                        break;
                case 0xE9:       /* DEL   */
                        break;
                default:
                        if (x >= video_num_columns) {
                                x -= video_num_columns;
                                pos -= video_size_row;
                                lf();
                        }
                        
                        *(unsigned char *)pos ++ = c;
                        *(unsigned char *)pos ++ = 0x02;
                        x++;
                        break;
                }
        }
        set_cursor();
}



static void sysbeep(void)
{
        /* not yet */ 
}






/* 
 * Following functions shoud be in lib dir but not created yet, 
 * so be here now. 
 */
int puts(char *s)
{
        return printk("%s",s);
}
