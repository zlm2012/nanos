#include "x86/memory.h"
#include "string.h"

static uint8_t* bitmap;
static int page_total;

void
init_bitmap() {
	page_total=(((*MEM_SIZE_PTR) << 10) - 0x800000)>>12;
	printk("mem: %d, page total: %d\n", *MEM_SIZE_PTR, page_total);
	if(!(bitmap=(uint8_t*)kmalloc(page_total)) && page_total>0)panic("bitmap init failed\n");
	printk("bitmap address: %p\n", bitmap);
}

void
print_bitmap() {
	int i;
	for (i=0; i<10; i++)
		printk("%d ", bitmap[i]);
	printk("\nTail Pointer: %p\n", bitmap+page_total-1);
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
			bitmap[b+i]=1;
		printk("Page alloc: %x\n", (0x800000+(b<<12)));
		print_bitmap();
		return ((void*)(0x800000+(b<<12)));
	}
	return 0;
}

void
free_page(void* pa, size_t size) {
	int b=((int)pa-0x800000)>>12, i;
	printk("freepage: %p %d\nbefore: ", pa, b);
	print_bitmap();
	if (page_total!=0)
		for (i=0; i<size; i++)
			bitmap[b+i]--;
	printk("after: ");
	print_bitmap();
}

void
realloc_page(void* pa, size_t size) {
	printk("Page realloc: %x\n", pa);
	int b=((int)pa-0x800000)>>12, i;
	if (page_total!=0)
		for (i=0; i<size; i++)
			bitmap[b+i]++;
	print_bitmap();
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

