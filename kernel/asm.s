/*   Yeah, i finally figure it out with a half day debugging.
 * when an interrupt happened with no pri changed, process wouldn't 
 * push the OLD SS and OLD ESP in the following stack fragment. And
 * that also happened to WITH_ERROR_CODE interrupt.
 *
 *	 So, if when we at pri ZERO, and do a var_a = var_b / 0 that ivoke
 * a divide_error interrupt, and it just push the the flag regsiter and
 * and cs register with eip where the interrupt happened. There is NO
 * OLD SS and OLD ESP, then the final handler die() will put wrong 
 * information that the the value of SS:ESP is random and no rule.
 */
 

/* 
 * no ERROR_code fragment      
+--------+--------+
|        | old ss |
+--------+--------+
|    old   esp    |
+--------+--------+
|    old  eflags  |        
+--------+--------+
|        |  cs    |     
+--------+--------+
|       eip       |0x34         
+--------+--------+
|    c_fun_addr   |0x30
+--------+--------+
|       eax       |0x2c      
+--------+--------+        
|       ecx       |0x28      
+--------+--------+
|       edx       |0x24      
+--------+--------+
|       ebx       |0x20     
+--------+--------+
|       esi       |0x1c
+--------+--------+
|       edi       |0x18
+--------+--------+        
|       ebp       |0x14
+--------+--------+
|        |   ds   |0x10
+--------+--------+
|        |   es   |0x0c
+--------+--------+
|        |   fs   |0x08
+--------+--------+
| error_code(=0)  |0x04    that's the para needed to pass to handler function 
+--------+--------+
|    ptr_to_eip   |0x00    the protype of handler is do_xxx(eip_addr, err_code) 
+--------+--------+
 *
 */
.globl divide_error        
divide_error:
	pushl $do_divide_error
        
no_error_code:
        /* push all the reg first to protect data */
	pushl %eax      
	pushl %ecx      
	pushl %edx      
	pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ebp      
	pushl %ds        
	pushl %es        
	pushl %fs
        
	pushl $0        		# error code
        lea   0x30(%esp), %eax          # get the addr of eip as para ofdo_xx
        pushl %eax
        movl  0x30(%esp), %eax          # another para of do_xxx
        movl  $0x10,%edx
	movw %dx,%ds
	movw %dx,%es
	movw %dx,%fs
	call *%eax
	addl $8,%esp                    # pop the two para
        
	popl %fs
	popl %es
	popl %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %ebx
	popl %edx
	popl %ecx
	popl %eax                       
        
        addl $4,%esp                    # pop the handler address
	iret
        
.globl debug
debug:
	pushl $do_int3		# _do_debug
	jmp no_error_code

.globl nmi
nmi:
	pushl $do_nmi
	jmp no_error_code
        
.globl int3
int3:
	pushl $do_int3
	jmp no_error_code

.globl overflow        
overflow:
	pushl $do_overflow
	jmp no_error_code

.globl bounds        
bounds:
	pushl $do_bounds
	jmp no_error_code

.globl invalid_op       
invalid_op:
	pushl $do_invalid_op
	jmp no_error_code

.globl coprocessor_segment_overrun        
coprocessor_segment_overrun:
	pushl $do_coprocessor_segment_overrun
	jmp no_error_code

.globl reserved        
reserved:
	pushl $do_reserved
	jmp no_error_code

#irq13:
#	pushl %eax
#	xorb %al,%al
#	outb %al,$0xF0
#	movb $0x20,%al
#	outb %al,$0x20
#	jmp 1f
#1:	jmp 1f
#1:	outb %al,$0xA0
#	popl %eax
#	jmp coprocessor_error



/*
 * with ERROR_code fragment      
+--------+--------+
|        | old ss |
+--------+--------+
|    old   eps    |
+--------+--------+
|    old  eflags  |        
+--------+--------+
|        |  cs    |     
+--------+--------+
|       eip       |0x38         
+--------+--------+
|    error_code   |0x34
+--------+--------+        
|    c_fun_addr   |0x30
+--------+--------+
|       eax       |0x2c      
+--------+--------+        
|       ecx       |0x28      
+--------+--------+
|       edx       |0x24      
+--------+--------+
|       ebx       |0x20     
+--------+--------+
|       edi       |0x1c
+--------+--------+
|       esi       |0x18
+--------+--------+        
|       ebp       |0x14
+--------+--------+
|        |   ds   |0x10
+--------+--------+
|        |   es   |0x0c
+--------+--------+
|        |   fs   |0x08
+--------+--------+
|    error_code   |0x04    that's the para needed to pass to handler function 
+--------+--------+
|    ptr_to_eip   |0x00    the protype of handler is do_xxx(eip_addr, err_code) 
+--------+--------+
 *
 */ 

.globl double_fault        
double_fault:
	pushl $do_double_fault
error_code:

        pushl %eax
	pushl %ecx
	pushl %edx
        pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ebp
	pushl %ds
	pushl %es
	pushl %fs

        movl  0x30(%esp), %eax         # erro code
        pushl %eax
        lea   0x34(%esp), %eax         # ptr to eip       
        pushl %eax

	movl $0x10,%eax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
        movl 0x30(%esp), %eax
        call *%eax
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %ebx
	popl %edx
	popl %ecx
	popl %eax
        
        addl $8, %esp
	iret

.globl invalid_TSS        
invalid_TSS:
	pushl $do_invalid_TSS
	jmp error_code

.globl segment_not_present        
segment_not_present:
	pushl $do_segment_not_present
	jmp error_code

.globl stack_segment        
stack_segment:
	pushl $do_stack_segment
	jmp error_code

.globl general_protection        
general_protection:
	pushl $do_general_protection
	jmp error_code


.globl read_eip
read_eip:
  pop %eax
  jmp *%eax


/*   i'm tried to make the timer work correct, so i move the
   interrupt handle here. hope it works. :)
*/

.globl timer_interrupt
timer_interrupt:
        pushl %eax
	pushl %ecx
	pushl %edx
        pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ebp
	pushl %ds
	pushl %es
	pushl %fs

        incl timer_ticks
	movb $0x20,%al		# EOI to interrupt controller #1
	outb %al,$0x20

        call do_timer
        
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %ebx
	popl %edx
	popl %ecx
	popl %eax
              
	iret


/*
.globl floppy_interrupt
floppy_interrupt:
        pushl %eax
	pushl %ecx
	pushl %edx
        pushl %ebx
	pushl %edi
	pushl %esi
	pushl %ebp
	pushl %ds
	pushl %es
	pushl %fs

        movl $0x4142434445, %eax
        pushl %eax
        call printk
        addl $4, %esp
        
	movb $0x20,%al		# EOI to interrupt controller #1
	outb %al,$0x20

      	xorl %eax,%eax
	xchgl do_floppy,%eax
	testl %eax,%eax
	jne 1f
	movl $unexpected_floppy_interrupt,%eax
1:	call *%eax
        
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %ebx
	popl %edx
	popl %ecx
	popl %eax
              
	iret
        */