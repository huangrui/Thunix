	movw $0x0, %ax
	movw %ax, %ds

	movw $str, %si
	call print_str
	jmp hang

#
# Print the string in SI register
#
# INPUT: SI = The string to print 
#
print_str:
	movb $0x0e, %ah
	movb $0x0f, %bh
	movb $0x00, %bl
.next_char:
	lodsb
	or %al, %al
	jz .ret
	int $0x10
	jmp .next_char
.ret:
	ret

hang:
	jmp hang

str:
	.string "Hello World\n"

.org 0x1fe, 0x90
.word 0xaa55        	# bootsector flag
