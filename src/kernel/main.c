#include "common.h"
#include "x86/x86.h"
#include "memory.h"
//#include "multiboot2.h"
#include "multiboot.h"

void init_page(void);
void init_serial(void);
void init_segment(void);
void init_idt(void);
void init_intr(void);
void init_proc(void);
void init_driver(void);
void init_fm(void);
void init_mm(void);
void init_pm(void);
void init_bitmap();
void init_kthread();
void print_bitmap();
void init_idle();
void welcome(void);

void os_init_cont(void);

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

void
os_init(uint32_t magic, uint32_t addr) {
	multiboot_info_t *mbi;

	cli();

	assert(magic == MULTIBOOT_BOOTLOADER_MAGIC);
	mbi = (multiboot_info_t *) addr;

	assert(CHECK_FLAG(mbi->flags, 0));
	*MEM_SIZE_PTR = mbi->mem_lower + mbi->mem_upper;

	assert(CHECK_FLAG(mbi->flags, 4) ^ CHECK_FLAG(mbi->flags, 5));
	assert(CHECK_FLAG(mbi->flags, 5));
	*ELF_INFO_PTR = &(mbi->u.elf_sec);
/*
	struct multiboot_tag *tag;
	//size_t size;
	assert(magic == MULTIBOOT2_BOOTLOADER_MAGIC);
	assert((addr & 7) == 0);

	//size = *(size_t *) addr;
	for (tag = (struct multiboot_tag *) (addr + 8);
	     tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			*MEM_SIZE_PTR = ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower +
			                ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper;
			break;
		case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
			*ELF_INFO_PTR = tag;
			break;
		}
	}
*/
		
	//printk("test\n");
	
	
	/* We must set up kernel virtual memory first because our kernel
	   thinks it is located in 0xC0000000.
	   Before setting up correct paging, no global variable can be used. */
	init_page();

	/* After paging is enabled, we can jump to the high address to keep 
	 * consistent with virtual memory, although it is not necessary. */
	asm volatile (" addl %0, %%esp\n\t\
					jmp *%1": : "r"(KOFFSET), "r"(os_init_cont));

	assert(0);	// should not reach here
}

void
os_init_cont(void) {
	/* Reset the GDT. Although user processes in Nanos run in Ring 0,
	   they have their own virtual address space. Therefore, the
	   old GDT located in physical address 0x7C00 cannot be used again. */
	init_segment();

	/* Initialize the serial port. After that, you can use printk() */
	init_serial();
	printk("Serial inited\n");

	/* Set up interrupt and exception handlers,
	   just as we did in the game. */
	init_idt();

	/* Initialize the intel 8259 PIC. */
	init_intr();
/*
#define PORT_TIME 0x40
#define FREQ_8253 1193182
#define HZ        100000
 
int count = FREQ_8253 / HZ;
assert(count < 65536);
out_byte(PORT_TIME + 3, 0x34);
out_byte(PORT_TIME    , count % 256);
out_byte(PORT_TIME    , count / 256);
*/
	
	/* Initialize processes. You should fill this. */
	init_proc();

	init_driver();

	init_fm();

	init_mm();

	init_pm();

	init_bitmap();

	init_kthread();

	welcome();

	init_idle();

	sti();

	/* This context now becomes the idle process. */
	while (1) {
		wait_intr();
	}
}

void os_idle(void) {
	while (1) {
		wait_intr();
	}
}

void
welcome(void) {
	printk("Hello, OS World!\n");
	printk("%x\n", *((uint32_t *)pa_to_va(0x10000)));
	printk("%x\n", *((uint8_t *)pa_to_va(0x11000)));
}
