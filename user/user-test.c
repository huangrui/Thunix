#include <unistd.h>

int main(void)
{
	struct sys_test st = {50, "Aleaxander"};
	test3(512, "Hello thunix system call", &st);

	return 0;
}
