#define ASM_FILE 1
#include "multiboot.h"
#define STACK_SIZE 4096
#define MULTIBOOT_HEADER_FLAGS                  MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
.section .multiboot_header, "a", @progbits
.align 4
header_start:
	.long MULTIBOOT_HEADER_MAGIC
	.long MULTIBOOT_HEADER_FLAGS
	.long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	.long   0
        .long   0
        .long   0
        .long   0
        .long   0
	.long 0
        .long 1024
        .long 768
        .long 32
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