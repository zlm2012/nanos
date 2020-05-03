.section .multiboot_header
	.align 4
header_start:
	.long 0x1BADB002
	.long 0x00000003
	.long -(0x1BADB002 + 0x3)

