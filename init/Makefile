AS	= as --32 -I../include
CC	= gcc -m32 -g -nostdinc -I../include -Wall -Wno-unused

DIRNAME = init

all: init.o

.c.o:
	@printf "%8s   %s\n" "CC" ${DIRNAME}/$@
	${CC} -c -o $*.o $<


clean:
	@rm -f *.o *.map *~
