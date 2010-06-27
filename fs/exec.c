/* 
 * The thunix execve system call implementaion
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fs.h>
#include <err.h>


/*
 * Count how many arguments we hare
 */
static inline int count(char **argv)
{
	int i = 0;
	if (argv) {
		while (argv[i])
			i++;
	}
	return i;
}

int sys_execve(const char *file, char **argv)
{
	int fd = sys_open(file, 0);

	/*
 	 * The current exec plan is: load the binary file to 16M,
 	 * then jump to this address.
 	 *
 	 * I will try to make a good global thunix memory allocation
 	 * map when I get time.
 	 */
	char * exec_buf = (char *)0x1600000;
	char *p = exec_buf;
	int bytes_read;
	int argc;
	int (*entry)(int, char **);

	if (fd < 0)
		return fd;

	/*
	 * Try to read 4K at a time 
	 *
	 * Note: Since we don't support demand loading now, the current exec plan is
	 * to load the whole program into
	 * memory once. 
	 */
	while ((bytes_read = sys_read(fd, p, 4096)) > 0)
		p += bytes_read;
	sys_close(fd);

	argc = count(argv);

	entry = (int(*)(int, char **))(exec_buf);
	return entry(argc, argv);
}
