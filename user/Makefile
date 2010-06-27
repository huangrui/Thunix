CC	= gcc -g -nostdinc -I../include -Wall -Wno-unused
LD	= ld
LDFLAGS = -m elf_i386 --oformat binary -N

DIRNAME = user

LIB = usys.o

OBJS = user-test.o arg.o

.PHONY: all clean

all: user-test arg

user-test: ${LIB} ${OBJS}
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} ${LDFLAGS} -e main -Ttext 0x1600000 -o $@ user-test.o ${LIB}

arg: ${LIB} ${OBJS}
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} ${LDFLAGS} -e main -Ttext 0x1600000 -o $@ arg.o ${LIB}


%.o: %.c
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $*.o $<

clean:
	rm -f *.o user-test *~
