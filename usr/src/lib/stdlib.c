#include "stdlib.h"

int __attribute__((__noinline__))
syscall(int id, ...) {
	int ret;
	int *args = &id;
	asm volatile("int $0x80": "=r"(ret) : "a"(args[0]), "b"(args[1]), "c"(args[2]), "d"(args[3]));
	return ret;
}

void exit(int status) {
	syscall(SYS_exit, status);
}

int fork() {
	return syscall(SYS_fork);
}

void exec(int filename, int argc, char** argv) {
  syscall(SYS_exec, filename, argc, argv);
}
