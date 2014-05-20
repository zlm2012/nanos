	.globl _init
	.extern exit
	.extern main

_init:  pushl %esp
        pushl %eax
        call main
	pushl %eax
	call exit
