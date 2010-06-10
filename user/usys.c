#include <syscall.h>
#include <unistd.h>

_syscall0(int, test);
_syscall1(int, test1, int, nr);
_syscall2(int, test2, int, nr, char *, str);
_syscall3(int, test3, int, nr, char *, str, struct sys_test *, test);


