#include "process.h"
#include "string.h"
#include "memory.h"

void copy_from_kernel(PCB* pcb, void* dest, void* src, int len){
	memcpy(nndma_to_ka(pcb->cr3.val, (uint32_t)dest), src, len);
}

void copy_to_kernel(PCB* pcb, void* dest, void* src, int len){
	memcpy(dest, nndma_to_ka(pcb->cr3.val, (uint32_t)src), len);
}

void strcpy_to_kernel(PCB* pcb, char* dest, char* src){
	strcpy(dest, nndma_to_ka(pcb->cr3.val, (uint32_t)src));
}

void strcpy_from_kernel(PCB* pcb, char* dest, char* src){
	strcpy(nndma_to_ka(pcb->cr3.val, (uint32_t)dest), src);
}
