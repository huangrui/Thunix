#include <stdio.h>

#include <unistd.h>
#include <syscall.h>
#include <hexdump.h>

struct syscall_params *scp;


static int sys_test(void)
{
	printk("Hello test with no arguments\n");
	return 0;
}

static int sys_test1(int inr)
{
	printk("Hello test with 1 argument: %d\n", (int)scp->first);
	return 0;
}

static int sys_test2(int inr , char *str)
{
	printk("Hello test with 2 arguments: %d\t %s\n", inr, str);
	return 0;
}


static int sys_test3(int inr, char *str, struct sys_test *st)
{
	printk("Hello test with 3 arguments:\n%d\n%s\n", inr, str);
	printk("id: %d\t\tname: %s\n", st->id, st->name);
	return 0;
}

int (*syscall_table[])() = {
	sys_test,
	sys_test1,
	sys_test2,
	sys_test3
};

int syscall(int inr)
{
	printk("system call:%d\n", inr);
	if (inr > 5)
		return -1;
	return syscall_table[inr]();
}
