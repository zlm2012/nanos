#ifndef __STDLIB_H__
#define __STDLIB_H__

int syscall(int id, ...);
void exit(int);

#define SYS_puts 101
#define SYS_exit 100

#endif
