CC	= gcc -g -nostdinc -I../include -Wall -Wno-unused
LD	= ld
LDFLAGS = -m elf_i386 --oformat binary -N

DIRNAME = user

OBJS = user-test.o usys.o

all: user-test

user-test: ${OBJS}
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} ${LDFLAGS} -e main -Ttext 0x1600000 -o $@ ${OBJS}

.c.o:
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $*.o $<

clean:
	rm -f *.o user-test *~
