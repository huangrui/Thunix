AS	= as --32
LD	= ld -m elf_i386
LDFLAGS = --oformat binary -N

all: boot.img

.s.o:
	${AS} -o $*.o $<

boot.img: boot.o
	@printf "%8s   %s\n" "LD" $@
	${LD} ${LDFLAGS} -e start -Ttext 0x7c00 -o $@ $<


bochs:
	bochs -qf bochsrc

clean:
	rm -rf *.o *.img *.txt
