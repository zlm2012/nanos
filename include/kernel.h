#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "common.h"
#include "x86/x86.h"
#include "memory.h"
#include "process.h"
#include "hal.h"

#define NEW_TIMER 100
#define DO_READ 101
#define DO_WRITE 102

void copy_from_kernel(PCB* pcb, void* dest, void* src, int len);
void copy_to_kernel(PCB* pcb, void* dest, void* src, int len);
void strcpy_to_kernel(PCB* pcb, char* dest, char* src);
void strcpy_from_kernel(PCB* pcb, char* dest, char* src);

#endif
