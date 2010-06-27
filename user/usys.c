#include <syscall.h>
#include <unistd.h>

_syscall1(int, conwrite, char *, str);
_syscall1(int, test1, int, nr);
_syscall2(int, test2, int, nr, char *, str);


