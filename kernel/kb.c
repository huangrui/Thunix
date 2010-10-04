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


/*
 * As I said before, I now get the passion and time to fix this mess up!
 * A re-write plan is undergoing, and hope this time it will work perfectly:)
 *
 * 		    Aleaxander, 2010, 10.1
 */

#include <asm/io.h>
#include <asm/system.h>
#include <keyboard.h>
#include <console.h>
#include <string.h>
#include <timer.h>

static unsigned char scancode;
static unsigned char mode = 0;
static unsigned char e0   = 2;  /* num lock is opened by default */
static unsigned char leds;

extern void con_write(char *, int);

//#define DEBUG_KBD 1
#ifdef DEBUG_KBD
	#define KBD_PRINTK(fmt, args...)  printk("KBD: "fmt, ##args)
#else
	#define KBD_PRINTK(fmt, args...)  
#endif
	

#define NO	0x0
#define ESC	0x1b

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

static unsigned char normal_map[256] =
{
        NO,   ESC,  '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
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
        'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
        'o',  'p',  '{',  '}',  '\n', NO,   'a',  's',
        'd',  'f',  'g',  'h',  'j',  'k',  'l',  ':',  // 0x20
        '"',  '~',  NO,   '|',  'z',  'x',  'c',  'v',
        'b',  'n',  'm',  '<',  '>',  '?',  NO,   '*',  // 0x30
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

static unsigned char ctrl_map[256] = {
        NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
        NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
        C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
        C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
        C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
        NO,      ' ',     NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
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



#define KBD_BUF_SIZE	1024
static unsigned char kbd_ring_buffer[KBD_BUF_SIZE];
static int ri = 0;
static int wi = 0;

/*
 * Revert char: turn upper to lower, lower to upper
 */
static void revert_char(unsigned char *ch)
{
	if (*ch >= 'a' && *ch <= 'z')
		*ch -= 0x20;
	else if (*ch >= 'A' && *ch <= 'Z')
		*ch += 0x20;
}


/* printable char */
static void pln(void) 
{
        unsigned char *map;
	unsigned char *maps[] = {
		shift_map, shift_map, /*Left, right */
		ctrl_map,  ctrl_map,
		NULL,	   NULL,      /* alt */
	};
	unsigned char key;
	int xmode = mode & 0x3f;
        
        
	/* It's a break scancode */
        if (scancode & 0x80)
                return;
	
	/* Use the normal mode map */
	if (xmode == 0) {
		map = normal_map;
	} else if (((xmode - 1) | xmode) == (2 * xmode - 1)) {
		/* If just contains one mode(aka, just one bit) */
		map = maps[xmode - 1];
		if (!map)
			return;
	} else {
		/* Several mode key pressed, do nothing */
		return;
	}
        
        key = *(map + scancode);
	KBD_PRINTK("raw key: 0x%x ", key);
	if (mode & (RSHIFT | LSHIFT)) {
		KBD_PRINTK(" (SHIFT_MAP) ");
		revert_char(&key);
	}

	if (leds & CAPS_LOCK) {
		KBD_PRINTK(" (CAPS) ");
		/* CTRL-x keys will be ignored! */
		revert_char(&key);
	}
	KBD_PRINTK("| final key: 0x%x\n", key);

	kbd_ring_buffer[wi] = key;
	wi = (wi + 1) % KBD_BUF_SIZE;
	return;
}

static int key_left = 0;

static void do_get_key(void)
{
	ri = wi;
	while (ri == wi)
		;
	
	/* Bochs need some delay, and seems that even 100 ms still not enough */
	sleep(10);
	
	key_left = wi - ri;
}

unsigned char get_key(void)
{
	while (1) {
		if (key_left--)
			return kbd_ring_buffer[ri++];

		/* No key left, do get more keys */
		do_get_key();
	}
}


static void unp(void)
{
        /* Just do nothing */
}


static inline void set_clear_mode(int flag)
{
	if (scancode & 0x80) {
		KBD_PRINTK("About cleaning flag %x(before) ", mode);
		mode &= ~flag;
		KBD_PRINTK("%x(after)\n", mode);
	}
	else {
		KBD_PRINTK("About setting flag, %x(before) ", mode);
		mode |= flag;
		KBD_PRINTK("%x(after)\n", mode);
	}
}

/* 
 * left CTRL and left ALT are extend scancode, 
 * they are followed by a '0xe0' scancode.
 */
static void ctrl(int got_e0)
{
	int flag = got_e0 ? RCTRL : LCTRL;

	set_clear_mode(flag);
}

static void alt(int got_e0)
{
	int flag = got_e0 ? RALT : LALT;

	set_clear_mode(flag);
}

static void shift(void)
{
	int flag = ((scancode & 0x7f) == 0x2A) ? LSHIFT : RSHIFT;

	set_clear_mode(flag);
}

static void cur(void)
{
        /*Not implemented yet*/
}

static void kb_wait(void)
{
        while (inb(0x64) & 0x02)
                ;
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
	if (scancode & 0x80)
		return;

	KBD_PRINTK("leds before: %x\n", leds);
        leds ^= CAPS_LOCK;
	KBD_PRINTK("leds after: %x\n", leds);
        set_leds();
}


static void fun(void)
{
        /* Not implement */
}

void keyboard_interrupt(void) 
{
	int com = 0;
	static int got_e0 = 0;
	
	
	scancode = inb(0x60);
	if(scancode == 0xE0) {
		got_e0 = 1;
		goto end;
	}


	switch (scancode & 0x7f) {
	case 0x36:
	case 0x2A:
		shift();
		break;
	case 0x3A:
		cap();
		break;
	case 0x1D:
		ctrl(got_e0);
		got_e0 = 0;
		break;
	case 0x38:
		alt(got_e0);
		got_e0 = 0;
		break;
	default:
		pln();
		break;
	}

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
