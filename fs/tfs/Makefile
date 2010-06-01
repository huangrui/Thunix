AS	= as -I../../include
CC	= gcc -nostdinc -I../../include -Wall -Wno-unused
LD	= ld
LDFLAGS = --oformat binary -N
CPP	=gcc -E -nostdinc -I../../include

all: tfs.o #mktfs

OBJS    = balloc.o cache.o dir.o file.o ialloc.o inode.o super.o tfs_diskio.o


tfs.o: ${OBJS} 
	${LD} -r -o $@ ${OBJS}
.c.s:
	${CC} -S -o $*.s $<
.s.o:
	${AS} -o $*.o $<
.c.o:
	${CC} -c -o $*.o $<

mktfs: balloc.c cache.c dir.c file.c ialloc.c inode.c mktfs.c utils.c
	gcc -Wall -Wno-unused -o mktfs balloc.c cache.c dir.c file.c ialloc.c inode.c utils.c


clean:
	rm -f *.o *.map *~ tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
