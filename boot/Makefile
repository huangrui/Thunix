AS	= as -I../include
CC	= gcc -nostdinc -I../include -Wall
LD	= ld
LDFLAGS = --oformat binary -N

OBJS	 = bootsect.o head.o

stop: bootsect.o head.o

.c.s:
	${CC} -S -o $*.s $<
.s.o:
	${AS} -o $*.o $<
.c.o:
	${CC} -c -o $*.o $<

clean:
	rm -f *.o *.map *~