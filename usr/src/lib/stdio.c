#include "stdlib.h"

int puts(char* s) {
	return syscall(SYS_puts, s);
}

int printf(const char* fmt, ...) {
	void **args = (void **)&fmt + 1;
	return syscall(SYS_printf, fmt, args);
}
