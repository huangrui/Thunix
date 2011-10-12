AS	= as --32 -I../include
CC	= gcc -m32 -g -nostdinc -I../include -Wall -Wno-unused
LD	= ld -m elf_i386
LDFLAGS = --oformat binary -N
CPP	=gcc -g -E -nostdinc -I../include

DIRNAME = kernel

OBJS    = asm.o sys.o console.o kb.o mktime.o panic.o printk.o \
	 timer.o traps.o fd.o shell.o reboot.o halt.o hexdump.o
#user_progs = user-test

all: kernel.o ${user_progs}

kernel.o: ${OBJS}
	@printf '%8s   %s\n' 'LD' ${DIRNAME}/$@
	${LD} -r -o $@ ${OBJS}
.s.o:
	@printf '%8s   %s\n' 'AS' ${DIRNAME}/$@
	${AS} -o $*.o $<
.c.o:
	@printf '%8s   %s\n' 'CC' ${DIRNAME}/$@
	${CC} -c -o $*.o $<

clean:
	@rm -f *.o *.map *~ tmp_make

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do echo -n `echo $$i | sed 's,\.c,\.s,'`" "; \
		$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:
console.s console.o: console.c ../include/console.h ../include/asm/io.h \
  ../include/asm/system.h
fd2.s fd2.o: fd2.c ../include/fd.h ../include/timer.h ../include/thunix.h \
  ../include/asm/io.h ../include/asm/system.h
fd.s fd.o: fd.c ../include/fd.h ../include/timer.h ../include/thunix.h \
  ../include/asm/io.h ../include/asm/system.h
fork.s fork.o: fork.c ../include/sched.h ../include/head.h ../include/mm.h \
  ../include/asm/system.h
kb.s kb.o: kb.c ../include/asm/io.h ../include/asm/system.h \
  ../include/keyboard.h
kb (copy).s keyboard.s keyboard.o: keyboard.c ../include/keyboard.h ../include/asm/io.h \
  ../include/asm/system.h
mktime.s mktime.o: mktime.c ../include/time.h
panic.s panic.o: panic.c ../include/kernel.h ../include/types.h
printk.s printk.o: printk.c ../include/stdarg.h ../include/stddef.h \
  ../include/console.h
ramdisk.s sched.s sched.o: sched.c ../include/sched.h ../include/head.h ../include/mm.h \
  ../include/asm/system.h ../include/asm/io.h
timer.s timer.o: timer.c ../include/asm/io.h ../include/asm/system.h \
  ../include/timer.h ../include/types.h
traps.s traps.o: traps.c ../include/asm/system.h ../include/asm/io.h
