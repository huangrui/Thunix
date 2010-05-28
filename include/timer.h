#ifndef TIMER_H
#define TIMER_H

extern void timer_interrupt(void);

void timer_init(int);
void sleep(unsigned long );

#endif /* timer.h */
