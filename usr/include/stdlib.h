#ifndef __STDLIB_H__
#define __STDLIB_H__

int syscall(int id, ...);
void exit(int);

#define SYS_puts 201
#define SYS_printf 202
#define SYS_exit 100

#endif
