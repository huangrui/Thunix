CC	= gcc -m32 -g -nostdinc -I../../include -Wall -Wno-unused
LD	= ld -m elf_i386

DIRNAME = fs/tfs

OBJS    = balloc.o dir.o file.o ialloc.o inode.o super.o tfs_diskio.o tfs_cmd.o

all: tfs.o

tfs.o: ${OBJS} 
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} -r -o $@ ${OBJS}

%.o: %.c
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $@ $<


clean:
	@rm -f *.o *.map *~ tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
