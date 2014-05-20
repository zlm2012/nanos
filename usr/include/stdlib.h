#ifndef __STDLIB_H__
#define __STDLIB_H__

int syscall(int id, ...);
void exit(int);
int fork(void);
void exec(int, int, char**);

#define SYS_puts 201
#define SYS_printf 202
#define SYS_exit 100
#define SYS_fork 101
#define SYS_exec 102

#endif
