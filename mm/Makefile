AS	= as --32 -I../include
CC	= gcc -m32 -g -nostdinc -I../include -Wall -Wno-unused
LD	= ld -m elf_i386
LDFLAGS = --oformat binary -N
CPP	=gcc -g -E -nostdinc -I../include

DIRNAME = mm

OBJS    = malloc.o 

mm.o: ${OBJS} 
	@printf "%8s   %s\n" "LD" ${DIRNAME}/$@
	${LD} -r -o $@ ${OBJS}
.c.o:
	@printf "%8s   %s\n" "CC" ${DIRNAME}/$@
	${CC} -c -o $*.o $<



clean:
	@rm -f *.o *.map *~ tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
