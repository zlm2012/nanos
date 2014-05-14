#ifndef __COMMON_H__
#define __COMMON_H__

#include "types.h"
#include "const.h"

void printk(const char *ctl, ...);
void* kmalloc(size_t nbytes);
void kfree(void* ap);

#include "assert.h"

#define SYS_puts 101

#endif
