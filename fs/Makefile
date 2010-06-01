AS	= as -I../include
CC	= gcc -nostdinc -I../include -Wall
LD	= ld
LDFLAGS = --oformat binary -N
CPP	=gcc -E -nostdinc -I../include


fs.o:
	(cd tfs; make)
	(cp tfs/tfs.o ./fs.o)


clean:
	rm -f *.o
	cd tfs; make clean
