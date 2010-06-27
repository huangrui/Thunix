#include <stdio.h>
#include <asm/system.h>
#include <asm/io.h>
#include <thunix.h>
#include <malloc.h>

#include <syscall.h>

unsigned long long *idt = (unsigned long long  *)0x80000;

#define get_seg_byte(seg,addr) ({                                       \
                        register char __res;                            \
                        __asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
                                :"=a" (__res):"0" (seg),"m" (*(addr))); \
                        __res;})

#define get_seg_long(seg,addr) ({                                       \
                        register unsigned long __res;                   \
                        __asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
                                :"=a" (__res):"0" (seg),"m" (*(addr))); \
                        __res;})

#define _fs() ({                                                \
                        register unsigned short __res;          \
                        __asm__("mov %%fs,%%ax":"=a" (__res):); \
                        __res;})

#define _ds() ({                                                \
                        register unsigned short __res;          \
                        __asm__("mov %%ds,%%ax":"=a" (__res):); \
                        __res;})

#define _es() ({                                                \
                        register unsigned short __res;          \
                        __asm__("mov %%es,%%ax":"=a" (__res):); \
                        __res;})


/*int do_exit(long code);   *** Not finished *** */

void ipc_install(void);

void page_exception(void);
void divide_error(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
void page_fault(void);
void coprocessor_error(void);
void reserved(void);

static void die(char * str,long esp_ptr,long nr)
{
        long * esp = (long *) esp_ptr;
        int i;
        //unsigned long eip = esp[0];
  
        printk("\n");

        printk("%s: %04x\n\r",str,nr&0xffff);
        printk("EIP:\t%04x:%p\nEFLAGS:\t%p\nESP:\t%04x:%p\n",
               esp[1],esp[0],esp[2],esp[4],esp[3]);
        printk("fs: %04x\n",_fs());
        printk("ds: %04x\n",_ds());
        printk("es: %04x\n",_es());
        
        if (esp[4] == 0x17) {
                printk("Stack: ");
                for (i=0;i<4;i++)
                        printk("%p ",get_seg_long(0x17,i+(long *)esp[3]));
                printk("\n");
        }
       
        /*  
          if (strcmp(str, "divide error" == 0)) {
                  esp[0] += 0x03;
                  return;
          }
        */
        while(1)
                ;
}

void do_double_fault(long esp, long error_code)
{
        die("double fault",esp,error_code);
}

void do_general_protection(long esp, long error_code)
{
        die("general protection",esp,error_code);
}

void do_divide_error(long esp, long error_code)
{
        die("divide error",esp,error_code);
}

void do_int3(long * esp, long error_code,
	     long fs,long es,long ds,
	     long ebp,long esi,long edi,
	     long edx,long ecx,long ebx,long eax)
{
          
        /*	__asm__("str %%ax":"=a" (tr):"0" (0));*/
        printk("eax\t\tebx\t\tecx\t\tedx\n\r%8x\t%8x\t%8x\t%8x\n\r",
               eax,ebx,ecx,edx);
        printk("esi\t\tedi\t\tebp\t\tesp\n\r%8x\t%8x\t%8x\t%8x\n\r",
               esi,edi,ebp,(long) esp);
        printk("\n\rds\tes\tfs\ttr\n\r%4x\t%4x\t%4x\t\n\r",
               ds,es,fs/*,tr*/);
        printk("EIP: %8x   CS: %4x  EFLAGS: %8x\n\r",esp[0],esp[1],esp[2]);
  
        for (;;)
                ;
}

void do_nmi(long esp, long error_code)
{
        die("nmi",esp,error_code);
}

void do_debug(long esp, long error_code)
{
        die("debug",esp,error_code);
}

void do_overflow(long esp, long error_code)
{
        die("overflow",esp,error_code);
}

void do_bounds(long esp, long error_code)
{
        die("bounds",esp,error_code);
}

void do_invalid_op(long esp, long error_code)
{
        die("invalid operand",esp,error_code);
}

void do_device_not_available(long esp, long error_code)
{
        die("device not available",esp,error_code);
}

void do_coprocessor_segment_overrun(long esp, long error_code)
{
        die("coprocessor segment overrun",esp,error_code);
}

void do_invalid_TSS(long esp,long error_code)
{
        die("invalid TSS",esp,error_code);
}

void do_segment_not_present(long esp,long error_code)
{
        die("segment not present",esp,error_code);
}

void do_stack_segment(long esp,long error_code)
{
        die("stack segment",esp,error_code);
}

void do_coprocessor_error(long esp, long error_code)
{
        die("coprocessor error",esp,error_code);
}

void do_reserved(long esp, long error_code)
{
        die("reserved (15,17-31) error",esp,error_code);
}



void ipc_install(void)
{
        outb_p(0x11, 0x20);
        outb_p(0x11, 0xa0);

        outb_p(0x20, 0x21);
        outb_p(0x28, 0xa1);

        outb_p(0x04, 0x21);
        outb_p(0x02, 0xa1);

        outb_p(0x01, 0x21);
        outb_p(0x01, 0xa1);
  
        outb_p(0x11, 0x21);
        outb_p(0x11, 0xa1);
}


void trap_init(void)
{
        extern void keyboard_interrupt();
        int i;
        struct DESCR {
                unsigned short length;
                unsigned long long  *address;
        } __attribute__((packed)) idt_descr = {256*8-1, idt};
        
        ipc_install();

        set_trap_gate(0,&divide_error);
        set_trap_gate(1,&debug);
        set_trap_gate(2,&nmi);
        set_system_gate(3,&int3);  /*int3*/
        set_system_gate(4,&overflow);
        set_system_gate(5,&bounds);
        set_trap_gate(6,&invalid_op);
        set_trap_gate(7,&reserved);/*device_not_available*/
        set_trap_gate(8,&double_fault);
        set_trap_gate(9,&coprocessor_segment_overrun);
        set_trap_gate(10,&invalid_TSS);
        set_trap_gate(11,&segment_not_present);
        set_trap_gate(12,&stack_segment);
        set_trap_gate(13,&general_protection);
        set_trap_gate(14,&reserved);/*page_fault*/
        set_trap_gate(15,&reserved);
        set_trap_gate(16,&reserved); /*coprocessor_error*/
        
        for (i=17;i < 48;i++)
                set_trap_gate(i,&reserved);
        
        

        __asm__ ("lidt %0\n\t"::"m"(idt_descr));



        /*set_trap_gate(45,&irq13);*/
  
        outb_p(inb_p(0x21)&0xfb, 0x21);
        outb_p(inb_p(0xa1)&0xdf, 0xa1);
        
        /* set_trap_gate(39,&parallel_interrupt);*/


	/* Install system call handler */
        set_system_gate(0x80, &syscall_interrupt);
}
