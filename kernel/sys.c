#include <stdio.h>

#include <unistd.h>
#include <syscall.h>
#include <hexdump.h>

struct syscall_params *scp;


int sys_test(void)
{
	printk("Hello test with no arguments\n");
	return 0;
}

int sys_test1(void)
{
	printk("Hello test with 1 argument: %d\n", (int)scp->first);
	return 0;
}

int sys_test2(void)
{
	printk("Hello test with 2 arguments: %d\t %s\n", (int)scp->first, (char *)scp->second);
	return 0;
}


int sys_test3(void)
{
	struct sys_test *st = (struct sys_test *)scp->third;
	printk("Hello test with 3 arguments:\n%d\n%s\n", (int)scp->first, (char *)scp->second);
	printk("id: %d\t\tname: %s\n", st->id, st->name);
	return 0;
}

static int (*syscall_table[])(void) = {
	sys_test,
	sys_test1,
	sys_test2,
	sys_test3
};

_syscall0(int, test);
_syscall1(int, test1, int, nr);
_syscall2(int, test2, int, nr, char *, str);
_syscall3(int, test3, int, nr, char *, str, struct sys_test *, test);


int syscall(int inr)
{
	printk("system call:%d\n", inr);
	if (inr > 5)
		return -1;
	return syscall_table[inr]();
}
