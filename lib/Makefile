CC	= gcc -g -nostdinc -I../include -Wall -Wno-unused
LD	= ld
LDFLAGS = -m elf_i386 -N

DIRNAME = lib


OBJS = string.o bitopts.o

.PHONY: all clean

all: lib.a

lib.a: ${OBJS}
	@printf "%8s   %s\n" "AR" ${DIRNAME}/$@
	rm -f $@
	ar cq $@ ${OBJS}

%.o: %.c
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $*.o $<

clean:
	rm -f *.o *~ lib.a
