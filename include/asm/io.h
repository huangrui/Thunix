#ifndef IO_H
#define IO_H


#define outb(value, port)			\
  __asm__ ("outb %%al, %%dx\n\t"		\
	   ::"a"(value),"d"(port))

#define inb(port) ({				\
      unsigned char _v;					 \
      __asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port));	\
      _v;							\
    })


#define outb_p(value, port)			\
  __asm__ ("outb %%al, %%dx\n\t"		\
	   "jmp 1f\n\t"				\
	   "1: jmp 1f\n\t"			\
	   "1:" ::"a"(value),"d"(port))

#define inb_p(port) ({				\
      unsigned char _v;				\
      __asm__ volatile ("inb %%dx, %%al\n\t"	\
			"jmp 1f\n\t"		\
			"1:jmp 1f\n\t"			\
			"1:":"=a"(_v):"d"(port));	\
      _v;						\
    })


#define insl(port, buf, nr)			\
  __asm__ ("cld;rep;insl\n\t"			\
	   ::"d"(port), "D"(buf), "c"(nr))

#define outsl(buf, nr, port)			\
  __asm__ ("cld;rep;outsl\n\t"			\
	   ::"d"(port), "S" (buf), "c" (nr))

#endif /* _IO_H */
