#ifndef UNISTD_H
#define UNISTD_H


/*
 * The first system call is used for user program writing date
 * on the screen, aka, console. Since we didn't implement console
 * write/read now 
 */
#define __NR_conwrite	0 
#define __NR_test1	1
#define __NR_test2	2

#define __NR_syscall    3


int conwrite(char *);
int test1(int);
int test2(int, char *);




#endif /* unistd.h */
