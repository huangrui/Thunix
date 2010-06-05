AS	= as -I../include
OBJS	 = bootsect.o head.o

DIRNAME = boot

all: bootsect.o head.o

.s.o:
	@printf "%8s   %s\n" "AS" ${DIRNAME}/$@
	${AS} -o $*.o $<

clean:
	@rm -f *.o *.map *~
