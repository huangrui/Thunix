AS	= as -I../include
CC	= gcc -nostdinc -I../include #-Wall
LD	= ld
LDFLAGS = --oformat binary -N
CPP	=gcc -E -nostdinc -I../include

OBJS    =bitmap.o balloc.o ialloc.o inode.o namei.o ext2.o \
	ramdisk.o ram_mke2fs.o dir.o ls.o 


ext2_fs.o: ${OBJS} 
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

### Dependencies:
balloc.s balloc.o: balloc.c ../include/fs_ext2.h ../include/types.h \
  ../include/rd.h
bitmap.s bitmap.o: bitmap.c
ext2.s ext2.o: ext2.c ../include/stdarg.h ../include/fs_ext2.h \
  ../include/types.h ../include/rd.h
ialloc.s ialloc.o: ialloc.c ../include/fs_ext2.h ../include/types.h \
  ../include/rd.h
inode.s inode.o: inode.c ../include/thunix.h ../include/fs_ext2.h \
  ../include/types.h ../include/rd.h
ls.s ls.o: ls.c ../include/fs_ext2.h ../include/types.h ../include/rd.h
namei.s namei.o: namei.c ../include/fs_ext2.h ../include/types.h ../include/rd.h
ramdisk.s ramdisk.o: ramdisk.c ../include/fs_ext2.h ../include/types.h \
  ../include/rd.h ../include/kernel.h
ram_mke2fs.s ram_mke2fs.o: ram_mke2fs.c ../include/fs_ext2.h ../include/types.h \
  ../include/rd.h ../include/thunix.h
