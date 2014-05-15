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
load_loader:
	xorb	%ah, %ah
	movb	$0x80, %dl
	int	$0x13

	xorw	%dx, %dx
	movb	$0x80, %dl
	movw	$0x0002, %cx
	movw	$0x0000, %ax
	movw	%ax, %es
	movw	$0x9000, %bx
	movb	$0x02, %ah
	movb	$0x08, %al
	int	$0x13
	jnc	go
	hlt

go:
	jmp	0x9000

