AS	= as -I../include
CC	= gcc -g -nostdinc -I../include -Wall -Wno-unused

DIRNAME = init

all: init.o

.c.o:
	@printf "%8s   %s\n" "CC" ${DIRNAME}/$@
	${CC} -c -o $*.o $<


clean:
	@rm -f *.o *.map *~
