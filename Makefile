LD	= ld
OBJCOPY = objcopy
LDFLAGS = -N
MAKEFLAGS +=  --no-print-directory

ifeq ("$(origin V)", "command line")
	MAKEFLAGS +=
else
	MAKEFLAGS += --quiet
endif

KERNEL_OBJS = boot/head.o init/init.o kernel/kernel.o mm/mm.o fs/fs.o lib/lib.a

.PHONY: all ${KERNEL_OBJS} clean backup release

all: thunix.img user-progs

thunix.img: boot.img kernel.img user-progs
	@printf "\n%8s   %s\n" "MK" $@
	cat boot.img kernel.img > thunix.img
	(./gen-test.sh >/dev/null)

%.img: %.elf
	${OBJCOPY} -O binary $< $@

boot.elf: boot/bootsect.o
	@printf "%8s   %s\n" "LD" $@
	${LD} ${LDFLAGS} -e start -Ttext 0x7c00 -o $@ $<

kernel.elf: ${KERNEL_OBJS}
	@printf "%8s   %s\n" "LD" $@
	${LD} ${LDFLAGS} -e pm_mode -Ttext 0x0000 -o $@ ${KERNEL_OBJS} -M > thunix.map

boot/bootsect.o:
	${MAKE} --directory=boot
init/init.o:
	${MAKE} --directory=init
kernel/kernel.o:
	${MAKE} --directory=kernel
fs/fs.o:
	${MAKE} --directory=fs
mm/mm.o:
	${MAKE} --directory=mm
lib/lib.a:
	${MAKE} --directory=lib

doc:
	${MAKE} --directory=doc


user-progs:
	@printf "\n"
	${MAKE} clean --directory=user
	${MAKE} --directory=user

release: 
	./release.sh

bochs:
	bochs -qf bochsrc 

qemu:
	qemu -fda thunix.img -boot a

clean:
	@printf "%8s   *.o *~ boot.img kernle.img *.elf\n" "RM" 
	rm -f bochsout.txt boot.img kernel.img *~ include/*~ *.elf *.map user-test
	(cd boot; make clean)
	(cd init; make clean)
	(cd kernel; make clean)
	(cd fs; make clean)
	(cd mm; make clean)
	(cd lib; make clean)
	(cd doc; make clean)
	(cd user; make clean)

dep:
	(cd kernel; make dep)
	(cd fs; make dep)
