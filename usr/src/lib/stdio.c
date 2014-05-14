#include "stdlib.h"

int puts(char* s) {
	return syscall(SYS_puts, s);
}
