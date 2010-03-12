#ifndef KEYBOARD_H
#define KEYBOADR_H

/* macro of mode */
#define LSHIFT      (1 << 0)
#define RSHIFT      (1 << 1)
#define LCTRL       (1 << 2)
#define RCTRL       (1 << 3)
#define LALT        (1 << 4)
#define RALT        (1 << 5)
#define CAPS_STATE  (1 << 6)
#define CAPS        (1 << 7)

/* macro of leds */
#define SCROLL_LOCK (1 << 0)
#define NUM_LOCK    (1 << 1)
#define CAPS_LOCK   (1 << 2)


/* macro of eo */
#define E0          0x01
#define E1          0x02



/* Special keycodes  */
#define KEY_HOME        0xE0
#define KEY_END         0xE1
#define KEY_UP          0xE2
#define KEY_DN          0xE3
#define KEY_LF          0xE4
#define KEY_RT          0xE5
#define KEY_PGUP        0xE6
#define KEY_PGDN        0xE7
#define KEY_INS         0xE8
#define KEY_DEL         0xE9


#define HOME            KEY_HOME
#define END             KEY_END
#define UP              KEY_UP
#define DN              KEY_DN
#define LF              KEY_LF
#define RT              KEY_RT
#define PGUP            KEY_PGUP
#define PGDN            KEY_PGDN  
#define INS             KEY_INS 
#define DEL             KEY_DEL



void keyboard_interrupt(void);			
		      


#endif  /*keyboard.h*/
