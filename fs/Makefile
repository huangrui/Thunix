CC	= gcc -g -nostdinc -I../include -Wall -Wno-unused
LD	= ld

DIRNAME = fs

OBJS    =  inode.o namei.o file.o super.o dir.o fslib.o cache.o exec.o

.PHONY: fs.o tfs/tfs.o

all: fs.o

fs.o: tfs/tfs.o ${OBJS}
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} -r -o $@  tfs/tfs.o ${OBJS}

tfs/tfs.o:
	${MAKE} --directory=tfs

%.o: %.c
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $@ $<

clean:
	@rm -f *.o
	@cd tfs; make clean
