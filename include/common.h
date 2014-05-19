#ifndef __COMMON_H__
#define __COMMON_H__

#include "types.h"
#include "const.h"

void printk(const char *ctl, ...);
int vfprintf(void (*printer)(char), const char *fmt0, void ** args);
void* kmalloc(size_t nbytes);
void kfree(void* ap);

#include "assert.h"

#define SYS_puts 201
#define SYS_printf 202
#define SYS_exit 100
#define SYS_fork 101

#endif
