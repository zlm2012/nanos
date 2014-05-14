#include "x86/memory.h"
#include "string.h"

static bool* bitmap;
static int page_total;

void
init_bitmap() {
	page_total=((unsigned)*MEM_INF)>>12;
	if(!(bitmap=(bool*)kmalloc(page_total)) && page_total>0)panic("bitmap init failed\n");
	printk("bitmap address: %p\n", bitmap);
}

void*
get_page(size_t size) {
	int b=0, i;
ATMP_FAILED:
	while(b<page_total) {
		for (i=0; i<size; i++)
			if (bitmap[b+i]) {
				b+=i+1;
				goto ATMP_FAILED;
			}
		for (i=0; i<size; i++)
			bitmap[b+i]=true;
		return ((void*)(0x1000000+(b<<12)));
	}
	return 0;
}

void
free_page(void* pa, size_t size) {
	int b=((int)pa-0x1000000)>>12;
	if (page_total!=0)
		memset(bitmap+b, 0, size);
}

void
print_bitmap() {
	int i;
	for (i=0; i<page_total; i++)
		printk("%d ", (bitmap[i]>0)?1:0);
	printk("\nTail Pointer: %p", bitmap+page_total-1);
}

void
make_invalid_pde(PDE *p) {
	p->val = 0;
}
void
make_invalid_pte(PTE *p) {
	p->val = 0;
}

/* For simplicity, we make all pages readable and writable for all ring 3 processes.
 * In Lab3, you may set different flags for different pages to perform the whole 
 * page level protection. */

void
make_pde(PDE *p, void *addr, int us) {
	p->val = 0;
	p->page_frame = ((uint32_t)addr) >> 12;
	p->present = 1;
	p->read_write = 1;
	p->user_supervisor = us;
}

void
make_pte(PTE *p, void *addr, int us, int rw) {
	p->val = 0;
	p->page_frame = ((uint32_t)addr) >> 12;
	p->present = 1;
	p->read_write = rw;
	p->user_supervisor = us;
}

