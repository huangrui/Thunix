/*   OK!
 *   this is the program that i don't wanna write most.
 * there are even some mistakes, such as when CAPS_LOCK
 * is on, i just put it in the SHIFT case, so mistakes 
 * happens, saying i press the 1 he will put the '!', 
 * yeah, that's realy and simply equal to the SHIFT case.
 *   
 *   what it can't do also is it can't handle when the CAPS
 * Lock and SHIFT key press both present. And we only can ues
 * the PAD key by CAPS_LOCK or by SHIFT key. Really ugly. but
 * i don't wanna do any work on it.
 *
 *   Maybe i will correct it in furture when i'm free and 
 * with the passion that swearing to correct it or even re-
 * write it.
 *  
 *   What an ugly work ! 
 *
 *                  Aleaxander, 2007-2008
 *                       Aleaxander@gmail.com
 */

/*
 *   OK again!
 *   Altough it makes mistakes, but i am happy that it can handle
 * the regular things like move the cursor left,right..., it also
 * can move the start of line (HOME) or the end of a line(END).
 * and what the most important is you can move it anywhere you like.
 *   God bless you. :)
 */

#include <asm/io.h>
#include <asm/system.h>
#include <keyboard.h>
#include <console.h>
#include <string.h>

static unsigned char scancode;
//static unsigned char brkflag;
static unsigned char mode = 0;
static unsigned char e0   = 2;  /* num lock is opened by default */
static unsigned char key;
static unsigned char leds;
//static unsigned char shift;

static void kb_wait(void);
extern void con_write(char *, int);

#define   NO   0x0

static unsigned char normal_map[256] =
{
        NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
        '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
        'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
        'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
        'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
        '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
        'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
        NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
        NO,   NO,   NO,   NO,   NO,   NO,   NO,   HOME,  // 0x40
        UP,   PGUP, '-',  LF,   '5',  RT,   '+',  END,
        DN,   PGDN, INS,  DEL,  NO,   NO,   NO,   NO,   // 0x50
        [0x97] KEY_HOME,
        [0x9C] '\n',      // KP_Enter
        [0xB5] '/',       // KP_Div
        [0xC8] KEY_UP,
        [0xC9] KEY_PGUP,
        [0xCB] KEY_LF,
        [0xCD] KEY_RT,
        [0xCF] KEY_END,
        [0xD0] KEY_DN,
        [0xD1] KEY_PGDN,
        [0xD2] KEY_INS,
        [0xD3] KEY_DEL
};

static unsigned char shift_map[256] = {
        NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
        '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
        'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
        'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
        'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
        '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
        'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
        NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
        NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
        '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
        '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
        [0x97] KEY_HOME,
        [0x9C] '\n',      // KP_Enter
        [0xB5] '/',       // KP_Div
        [0xC8] KEY_UP,
        [0xC9] KEY_PGUP,
        [0xCB] KEY_LF,
        [0xCD] KEY_RT,
        [0xCF] KEY_END,
        [0xD0] KEY_DN,
        [0xD1] KEY_PGDN,
        [0xD2] KEY_INS,
        [0xD3] KEY_DEL
};

#define C(x) (x - '@')

static unsigned char ctl_map[256] = {
        NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
        NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
        C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
        C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
        C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
        NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
        C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
        [0x97] KEY_HOME,
        [0xB5] C('/'),    // KP_Div
        [0xC8] KEY_UP,
        [0xC9] KEY_PGUP,
        [0xCB] KEY_LF,
        [0xCD] KEY_RT,
        [0xCF] KEY_END,
        [0xD0] KEY_DN,
        [0xD1] KEY_PGDN,
        [0xD2] KEY_INS,
        [0xD3] KEY_DEL
};


extern void parse_command (char *);
static int index = 0;
char command_buffer[20] = {'\0',};


/* printable char */
static void pln(void) 
{
        unsigned char *map;
        
        
        if (scancode & 0x80)
                return;
        
        
        if ((mode & (RSHIFT | LSHIFT)) || (leds & CAPS_LOCK))
                map = shift_map;
        else
                map = normal_map;
        key = *(map + scancode);
        
        /* I have no meaning of the following code, so just ignore it */
#if 0 
        if (mode & (LCTRL | RCTRL | CAPS_STATE))
                if (key >= 'a' && key <= '}')
                        key -= 32;
        if (mode & (LCTRL | RCTRL))
                if (key >= 64 && key <= 64 + 26)
                        key -= 32;
#endif
        
        /* shell part */
        
        if (key == '\n') {
                command_buffer[index] = '\0';
                goto skip;
        } 
        
        if (key == '\b') {
                if (index) {
                        index --;
                        goto skip;
                }
                
                return;
        }

        command_buffer[index] = (char)key;
        index ++;

 skip:
        con_write((char *)&key,1);
        
        if (key == '\n') {
                                
                if ( index == 0) 
                        goto next;
                
                parse_command (command_buffer);
                memset(command_buffer, '\0', index);
                index = 0;
        next:
                puts("thunix $ ");
        }
        
}

static void unp(void)
{
        /* Just do nothing */
}

static void ctl(void)
{
        unsigned char temp;
        temp = LCTRL;
        
        if (e0 & E0)
                temp <<= 1;
        
        mode |= temp;
}

static void alt(void)
{
        unsigned char temp;
        temp = LALT;

        if (e0 & E0)
                temp <<= 1;
        
        mode |= temp;
}

static void sft(void)
{
        mode |= LSHIFT;
        if (scancode == 0x36)
                mode |= LSHIFT * 2;
}

static void unshift(void)
{
        if (scancode == 0xAA)
                mode &= ~LSHIFT;
        else if (scancode == 0xB6)
                mode &= ~RSHIFT;
}

static void cur(void)
{
        /*Not implemented yet*/
}

static void set_leds(void)
{
        kb_wait();
        outb(0xed, 0x60);   /* set leds command */
        kb_wait();
        outb(leds, 0x60);   /* send param */
}

static void sroll(void)
{
        leds ^= SCROLL_LOCK;
        set_leds();
}

static void num_lock(void)
{
        leds ^= NUM_LOCK;
        set_leds();
}

static void cap(void)
{
        leds ^= CAPS_LOCK;
        set_leds();
}

static void kb_wait(void)
{
        while (inb(0x64) & 0x02)
                ;
}

static void fun(void)
{
        /* Not implement */
}

/*
 *  key   make  break 
 *  home  0x47  0xc7
 *  up    0x48  0xc8
 *  pgup  0x49  0xc9
 *   -    0x4A  0xca
 *  left  0x4B  0xcb
 *  centr 0x4C  0xcc
 *  right 0x4D  0xcd
 *   +    0x4E  0xce
 *  end   0x4F  0xcf
 *  down  0x50  0xd0
 *  pgdn  0x51  0xd1
 *  ins   0x52  0xd2
 *  del   0x53  0xd3
 */

void (*kfun_table[0x80])(void) = {
        /*    0/8  1/9  2/a  3/b  4/c  5/d  6/e  7/f */
        /*00*/unp, unp, pln, pln, pln, pln, pln, pln,
        /*  */pln, pln, pln, pln, pln, pln, pln, pln,
        /*10*/pln, pln, pln, pln, pln, pln, pln, pln,
        /*  */pln, pln, pln, pln, pln, ctl, pln, pln,
        /*20*/pln, pln, pln, pln, pln, pln, pln, pln,
        /*  */pln, pln, sft, pln, pln, pln, pln, pln,
        /*30*/pln, pln, pln, pln, pln, pln, sft, pln,
        /*  */alt, pln, cap, fun, fun, fun, fun, fun,
        /*40*/fun, fun, fun, fun, fun, unp, unp, pln,
        /*  */pln, pln, pln, unp, pln, pln, pln, pln,
        /*50*/pln, pln, pln, pln, unp, unp, unp, fun,
        /*  */fun, unp, unp, unp, unp, unp, unp, unp,
        /*60*/unp, unp, unp, unp, unp, unp, unp, unp,
        /*  */unp, unp, unp, unp, unp, unp, unp, unp,
        /*70*/unp, unp, unp, unp, unp, unp, unp, unp,
        /*  */unp, unp, unp, unp, unp, unp, unp, unp,
};



/* looks like not works */
void wait_for_keypress()
{
        char code;

        code = inb(0x60);

        while ( (code == '\0') || (code & 0x80) )
                ;
}

void keyboard_interrupt () 
{
	int com;
       

	com = 0;
	
	scancode = inb(0x60);



	if (scancode == 0xAA || scancode == 0xB6) {
                unshift();
                goto end;
        }
	
        if (scancode == 0x2A || scancode == 0x36) {
                sft();
                goto end;
        }
        
        if (scancode == 0x3A || scancode == 0xB6) {
                cap();
                goto end;
        }

        /* 
         *   I don't know why i need hanle the left move here, 
         * may just because i don't know why the origin handle
         * can't work! what a sham day
         *
         *   And what make happy is that it works now! greate work.
         */
        if (scancode == 0x4b) {
                scancode = LF;
                con_write((char *)&scancode, 1);
                goto end;
        }
                
                
        /* we haven't implenmented the ESC codes now */
#if 0        
        if (scancode == 0xe0) {
                e0 = 1;
                goto end;
        }
        /* we must reset the e0 later */
        if (e0 == 1 && scancode & 0x80) {
                e0 = 1;
                goto end;
        }
#endif
	if (scancode & 0x80)
                goto end;
	else
	  (*kfun_table[scancode&0x7f])();
        /* key stroke has been handled */

 end:
	outb((com=inb(0x61))|0x80, 0x61);   /* disable it */
	outb(com&0x7f, 0x61);               /* enable it again */
	outb(0x20, 0x20);                   /* EOI */
 
}

void keyboard_init ()
{
        set_trap_gate(0x21,keyboard_interrupt);
        outb(inb(0x21)&0xfd, 0x21);
}
