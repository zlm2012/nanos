# Booting procedure: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7C00 and starts executing in real mode
# with %CS=0 %IP=7C00.

#include "asm.h"
.code16
.globl start
start:
	cli                       # BIOS enabled interrupts; disable

	# Zero data segment registers DS, ES, and SS
	xorw    %ax, %ax          # %AX = 0
	movw    %ax, %ds          # %DS = AX (data segment)
	movw    %ax, %es          # %ES = AX (extra segment)
	movw    %ax, %ss          # %SS = AX (stack segment)

	# Physical address line A20 is tied to zero so that the first PCs 
	# with 2 MB would run software that assumed 1 MB - nobody wants it,
	# but it continues to haunt us.
	movw    $0x2401, %ax      # Enable A20 line by BIOS
	int     $0x15

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

# GDT
.p2align 2                    # force 4 byte alignment
gdt:
	SEG_NULLASM                    # empty segment
	SEG_ASM(0xA, 0x0, 0xffffffff)  # code
	SEG_ASM(0x2, 0x0, 0xffffffff)  # data

gdtdesc:                      # descriptor
	.word   (gdtdesc - gdt - 1)    # limit = sizeof(gdt) - 1
	.long   gdt                    # address of GDT

