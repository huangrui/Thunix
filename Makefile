AS	= as -Iinclude
CC	= gcc -nostdinc -Iinclude -Wall -Wno-unused-function
LD	= ld
LDFLAGS = --oformat binary -N

KERNEL_OBJS = boot/head.o init/init.o kernel/kernel.o fs/ext2_fs.o mm/mm.o

.PHONY :clean backup release



.c.s:
	${CC} -S -o $*.s $<
.s.o:
	${AS} -o $*.o $<
.c.o:
	${CC} -c -o $*.o $<


all: thunix.img doc

thunix.img: boot.img kernel.img
	cat boot.img kernel.img > thunix.img
	@wc -c thunix.img



boot/boot.o:
	(cd boot; make)
init/init.o:
	(cd init; make)
kernel/kernel.o:
	(cd kernel; make)
fs/ext2_fs.o:
	(cd fs; make)
mm/mm.o:
	(cd mm; make)

boot.img: boot/bootsect.o
	${LD} ${LDFLAGS} -e start -Ttext 0x7c00 -o $@ $<
kernel.img: ${KERNEL_OBJS}
	${LD} ${LDFLAGS} -e pm_mode -Ttext 0x0000 -o $@ ${KERNEL_OBJS}

doc:
	(cd doc; make)

release: 
	./release.sh

bochs:
	bochs -qf bochsrc 
clean:
	rm -f bochsout.txt boot.img kernel.img *~ include/*~
	(cd boot; make clean)
	(cd init; make clean)
	(cd kernel; make clean)
	(cd fs; make clean)
	(cd mm; make clean)
	(cd doc; make clean)

dep:
	(cd kernel; make dep)
	(cd fs; make dep)
