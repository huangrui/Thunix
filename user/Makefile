CC	= gcc -g -nostdinc -I../include -Wall -Wno-unused -fno-builtin
LD	= ld
LDFLAGS = -m elf_i386 -N --oformat binary 

DIRNAME = user

LIB = usys.o printf.o vsprintf.o string.o

OBJS = user-test.o

.PHONY: all clean

all: user-test

user-test: ${LIB} ${OBJS}
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} ${LDFLAGS} -e main -Ttext 0x1600000 -o $@ user-test.o ${LIB} -M > user.map
	cp user-test ../

%.o: %.c
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $*.o $<

clean:
	rm -f *.o *.map user-test *~
