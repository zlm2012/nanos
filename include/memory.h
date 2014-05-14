#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "x86/x86.h"

#define KOFFSET 0xC0000000
#define NEW_PAGE 201
#define FREE_PAGE 202

inline CR3* get_kcr3();
inline PDE* get_kpdir();
inline PTE* get_kptable();

void make_invalid_pde(PDE *);
void make_invalid_pte(PTE *);
void make_pde(PDE *, void *);
void make_pte(PTE *, void *);

void* get_page(size_t size);
void free_page(void* pa, size_t size);

static inline void*
nndma_to_ka(uint32_t cr3, uint32_t va) {
	uint32_t pde = ((uint32_t *)(cr3 & ~0xfff))[va >> 22];
	uint32_t pte = ((uint32_t *)(pde & ~0xfff))[(va >> 12) & 0x3ff];
	uint32_t pa = (pte & ~0xfff) | (va & 0xfff);
	return (void*)(pa+KOFFSET);
}

#define va_to_pa(addr) \
	((void*)(((uint32_t)(addr)) - KOFFSET))

#define pa_to_va(addr) \
	((void*)(((uint32_t)(addr)) + KOFFSET))



/* the maxinum kernel size is 16MB */
#define KMEM    (16 * 1024 * 1024)

/* Nanos has 128MB physical memory  */
#define PHY_MEM   (128 * 1024 * 1024)

#endif
