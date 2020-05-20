#define ASM_FILE 1
#include "multiboot2.h"
#define STACK_SIZE 0x8000
.section .multiboot_header, "a", @progbits
.align 8
header_start:
	.long MULTIBOOT2_HEADER_MAGIC
	.long GRUB_MULTIBOOT_ARCHITECTURE_I386
	.long header_end - header_start
	.long -(0xe85250d6 + 0 + (header_end - header_start))
	.short 0
	.short 0
	.long 8
header_end:

.globl multiboot_start
.text
.code32
multiboot_start:
	movl $(stack + STACK_SIZE - 0xC0000000), %esp
	pushl $0
	popf
	pushl %ebx
	pushl %eax
	call os_init

	.comm stack, STACK_SIZE