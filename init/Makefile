AS	= as -I../include
CC	= gcc -nostdinc -I../include -Wall -Wno-unused
LD	= ld
LDFLAGS = --oformat binary -N

OBJS	 = init.o

init.o: init.o

.c.s:
	${CC} -S -o $*.s $<
.s.o:
	${AS} -o $*.o $<
.c.o:
	${CC} -c -o $*.o $<


clean:
	rm -f *.o *.map *~
