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
	movw	$0x1000, %di
	movb	%dl, %es:0(%di)
	jmp	init_serial

serial_print:
	movw	$0x3fd, %dx
sp_loop:
	in	(%dx), %al
	test	$0x20, %al
	je	sp_loop
	movb	$0xf8, %dl
	mov	%cl, %al
	out	%al, (%dx)
	ret

init_serial:
	movw	$0x3f9, %dx
	xorw	%ax, %ax
	out	%al, (%dx)
	movb	$0xfb, %dl
	movb	$0x80, %al
	out	%al, (%dx)
	movb	$0xf8, %dl
	movb	$0x1, %al
	out	%al, (%dx)
	movb	$0xf9, %dl
	xorw	%ax, %ax
	out	%al, (%dx)
	movb	$0xfb, %dl
	movb	$0x3, %al
	out	%al, (%dx)
	movb	$0xfa, %dl
	movb	$0xc7, %al
	out	%al, (%dx)
	movb	$0xfc, %dl
	movb	$0xb, %al
	out	%al, (%dx)

try_serial:
	movb	$0x2a, %cl
	call	serial_print
	movb	$0xa, %cl
	call	serial_print

load_loader:
	xorw	%dx, %dx
	movb	%es:0(%di), %dl
	xorb	%ah, %ah
	int	$0x13

	xorw	%dx, %dx
	movb	%es:0(%di), %dl
	movw	$0x0002, %cx
	movw	$0x0000, %ax
	movw	%ax, %es
	movw	$0x9000, %bx
	movb	$0x02, %ah
	movb	$0x08, %al
	int	$0x13
	jnc	go
	movb	$0x21, %cl
	call	serial_print
	movb	$0xa, %cl
	call	serial_print
	hlt

go:
	ljmp	$0x0, $0x9000

