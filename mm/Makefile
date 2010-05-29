AS	= as -I../include
CC	= gcc -nostdinc -I../include -Wall -Wno-unused
LD	= ld
LDFLAGS = --oformat binary -N
CPP	=gcc -E -nostdinc -I../include

OBJS    = malloc.o 

mm.o: ${OBJS} 
	${LD} -r -o $@ ${OBJS}
.c.s:
	${CC} -S -o $*.s $<
.s.o:
	${AS} -o $*.o $<
.c.o:
	${CC} -c -o $*.o $<



clean:
	rm -f *.o *.map *~ tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
