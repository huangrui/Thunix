/*
 * page.s contains the low-level page-exception code.
 * the real work is done in mm.c
 */

.globl page_fault
page_fault:
	popal
	push %ds
	push %es
	push %fs

	movl $0x10, %edx
	mov  %dx, %ds
	mov  %dx, %es
	mov  %dx, %fs
	pushl 48(%esp)
	movl %cr2,%edx
	pushl %edx
	movl 52(%esp), %eax
	pushl %eax
	testl $1, %eax
	jne   1f
	call do_no_page
	jmp 2f
1:	call do_wp_page
2:	addl $12, %esp
	pop %fs
	pop %es
	pop %ds
	popal
	iret
	



	/*
page_fault:
	xchgl %eax,(%esp)
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	movl %cr2,%edx
	pushl %edx
	testl $1,%eax
	jne 1f
	call do_no_page
	jmp 2f
1:	call do_wp_page
2:	addl $4,%esp
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret*/
