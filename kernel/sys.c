#include <stdio.h>
#include <string.h>
#include <console.h>

#include <unistd.h>
#include <syscall.h>
#include <hexdump.h>


static int sys_conwrite(char *str)
{
	con_write(str, strlen(str));
	return 0;
}

static int sys_test1(int nr)
{
	printk("Hello test with 1 argument: %d\n", nr);
	return 0;
}

static int sys_test2(int nr , char *str)
{
	printk("Hello test with 2 arguments: %d\t %s\n", nr, str);
	return 0;
}

int (*syscall_table[])() = {
	sys_conwrite,
	sys_test1,
	sys_test2,
};
