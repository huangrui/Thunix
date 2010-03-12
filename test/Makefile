AS	= as 
LD	= ld
LDFLAGS = --oformat binary -N

all: boot.img

.s.o:
	${AS} -o $*.o $<

boot.img: boot.o
	${LD} ${LDFLAGS} -e start -Ttext 0x7c00 -o $@ $<


bochs:
	bochs -qf bochsrc

clean:
	rm -rf *.o *.img *.txt
