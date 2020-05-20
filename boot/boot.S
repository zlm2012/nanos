#include "asm.h"
.code16
.globl boot
boot:
	jmp	boot2
	movw	$0x1000, %ax
	movw	%ax, %es
	movw	$0, %es:0x110(%di)
	movb	$18, %es:0x112(%di)
fetch_loop:
	movw	%es:0x110(%di), %ax
	movw	%ax, %es:0x104(%di)
	movw	%ax, %es:0x106(%di)
	divb	%es:0x112(%di)
	movb	%ah, %es:0x102(%di)
	addb	$1, %es:0x102(%di)
	movb	%al, %ah
	andb	$1, %al
	movb	%al, %es:0x101(%di)
	movb	%ah, %es:0x100(%di)
	shrb	$1, %es:0x100(%di)
	addb	$1, %es:0x100(%di)
	shlw	$9, %es:0x104(%di)
	shrw	$7, %es:0x106(%di)
	shlw	$12, %es:0x106(%di)
	addw	$0x2000, %es:0x106(%di)
	call	read_sectors
	addw	$2, %es:0x110(%di)
	movw	%es:0x110(%di), %ax
	cmpw	$0x400, %ax
	jl	fetch_loop
boot2:
	movw	$0x1000, %ax
	movw	%ax, %es
	movw	$0x0000, %di
meme801:
	stc					# fix to work around buggy
	xorw	%cx,%cx				# BIOSes which dont clear/set
	xorw	%dx,%dx				# carry on pass/error of
						# e801h memory size call
						# or merely pass cx,dx though
						# without changing them.
	movw	$0xe801, %ax
	int	$0x15

	cmpw	$0x0, %cx			# Kludge to handle BIOSes
	jne	e801usecxdx			# which report their extended
	cmpw	$0x0, %dx			# memory in AX/BX rather than
	jne	e801usecxdx			# CX/DX.  The spec I have read
	movw	%ax, %cx			# seems to indicate AX/BX 
	movw	%bx, %dx			# are more reasonable anyway...

e801usecxdx:
	andl	$0xffff, %edx			# clear sign extend
	shll	$16, %edx			# and go from 64k to 1k chunks
	movl	%edx, %es:0(%di)			# store extended memory size
	andl	$0xffff, %ecx
	shll	$10, %ecx
	addl	$1048576, %ecx			# clear sign extend
 	movl	%ecx, %es:4(%di)			# and add lower memory into
						# total size.
	# Make an "identical mapping" from virtual address to physical address.
	# We have already prepared a GDT in the 
	lgdt    gdtdesc           # See i386 manual for more information
	movl    %cr0, %eax        # %CR0 |= PROTECT_ENABLE_BIT
	orl     $0x1, %eax
	movl    %eax, %cr0

	# Complete transition to 32-bit protected mode by using long jmp
	# to reload %CS and %EIP.  The segment descriptors are set up with no
	# translation, so that the mapping is still the identity mapping.
	ljmp    $GDT_ENTRY(1), $start32

read_sectors:
	movw	%bx, %es:0x200(%di)
	movw	%cx, %es:0x202(%di)
	movw	%dx, %es:0x204(%di)
	movw	$3, %cx
try_read:
	movw	%cx, %es:0x206(%di)
	xorw	%dx, %dx
	movb	%es:0(%di), %dl
	movb	%es:0x101(%di), %dh
	movb	%es:0x100(%di), %ch
	movb	%es:0x102(%di), %cl
	movw	%es:0x104(%di), %bx
	movw	%es:0x106(%di), %ax
	movw	%ax, %es
	movw	$0x0202, %ax
	int	$0x13
	jnc	read_finish
	movw	$0x1000, %ax
	movw	%ax, %es
	movw	%es:0x206(%di), %cx
	loop	try_read
	jmp	failed
read_finish:
	movw	$0x1000, %ax
	movw	%ax, %es
	movw	%es:0x200(%di), %bx
	movw	%es:0x202(%di), %cx
	movw	%es:0x204(%di), %dx
	ret

failed:
	hlt

.code32
start32:
	# Set up the protected-mode data segment registers
	movw    $GDT_ENTRY(2), %ax
	movw    %ax, %ds          # %DS = %AX
	movw    %ax, %es          # %ES = %AX
	movw    %ax, %ss          # %SS = %AX

	# Set up a stack for C code. Anywhere would be fine.
	movl    $0x8000, %esp     # %ESP = $0x8000
	call    bootmain          # Go to the C code, should never return!

.globl irq_empty; irq_empty:iret

# GDT
.p2align 2                    # force 4 byte alignment
gdt:
	SEG_NULLASM                    # empty segment
	SEG_ASM(0xA, 0x0, 0xffffffff)  # code
	SEG_ASM(0x2, 0x0, 0xffffffff)  # data

gdtdesc:                      # descriptor
	.word   (gdtdesc - gdt - 1)    # limit = sizeof(gdt) - 1
	.long   gdt                    # address of GDT
