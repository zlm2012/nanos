	.globl _init
	.extern exit
	.extern main

_init:	call main
	pushl %eax
	call exit