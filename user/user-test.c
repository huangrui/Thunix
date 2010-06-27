#define  __USER 1
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i = 0;
	
	printf("number of arguments: %d\nAnd they are:\n", argc);
	for (i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}

	test2(512, "Hello thunix system call");

	return 0;
}
