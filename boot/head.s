##***************************************************************
##     What's done here is very simple, assign the segments,    *
##  move the rest of kernel down to low address again. Then     *
##  call the kernel initial function init().                    *
##     Yeah, we put the all the rest job of our kernel on it.   *
##     Hope it don't let's down. :)                             *
##                                                              *
##                  Copyright (C) 2007-2008  Aleaxander         *
##                      Email: Aleaxander@gmail.com             *
##***************************************************************        
	        .text
		.globl	pm_mode
		.include "kernel.inc"
		.org 0
pg_dir:		
pm_mode:
		movl	$DATA_SEL,%eax
		movw	%ax,	%ds
		movw	%ax,	%es
		movw	%ax,	%fs
		movw	%ax,	%gs
		movw	%ax,	%ss
		movl	$STACK_BOT,%esp

		cld
		movl	$0x10200,%esi
		movl	$0x200,	%edi
		movl	$(KERNEL_SECT-1)<<7,%ecx
		rep
		movsl

                call	init    # we count on it.

