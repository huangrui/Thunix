LD	= ld
LDFLAGS = --oformat binary -N
MAKEFLAGS +=  --no-print-directory

ifeq ("$(origin V)", "command line")
	MAKEFLAGS +=
else
	MAKEFLAGS += --quiet
endif

KERNEL_OBJS = boot/head.o init/init.o kernel/kernel.o mm/mm.o fs/fs.o

.PHONY :clean backup release

all: thunix.img doc

thunix.img: boot.img kernel.img
	@printf "%8s   %s\n" "MK" $@
	cat boot.img kernel.img > thunix.img
	(./gen-test.sh >/dev/null)

boot.img: boot/bootsect.o
	@printf "%8s   %s\n" "LD" $@
	${LD} ${LDFLAGS} -e start -Ttext 0x7c00 -o $@ $<
kernel.img: ${KERNEL_OBJS}
	@printf "%8s   %s\n" "LD" $@
	${LD} ${LDFLAGS} -e pm_mode -Ttext 0x0000 -o $@ ${KERNEL_OBJS}

boot/bootsect.o:
	(cd boot; make)
init/init.o:
	(cd init; make)
kernel/kernel.o:
	(cd kernel; make)
fs/fs.o:
	(cd fs; make)
mm/mm.o:
	(cd mm; make)

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
