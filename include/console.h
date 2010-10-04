#ifndef CONSOLE_H
#define CONSOLE_H

#define ORIG_X           (0)
#define ORIG_Y           (0)
#define ORIG_VIDEO_COLS  (80)
#define ORIG_VIDEO_LINES (25)

void get_cursor(int *, int *);
void set_cursor(void);
void con_init(void);
void con_write(char *buf, int nr);

extern int puts(char *);
extern int putchar(int c);
extern char getchar();
extern void wait_key_press(void);


#endif /* console.h */
