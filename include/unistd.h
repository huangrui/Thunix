#ifndef UNISTD_H
#define UNISTD_H


#define __NR_test	0
#define __NR_test1	1
#define __NR_test2	2
#define __NR_test3	3

struct sys_test {
	int id;
	char name[12];
} __attribute__((packed));


int test(void);
int test1(int);
int test2(int, char *);
int test3(int, char *, struct sys_test *);




#endif /* unistd.h */
