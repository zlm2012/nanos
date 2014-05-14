#include "kernel.h"

#define NR_IRQ_HANDLE 32

/* In Nanos, there are no more than 16(actually, 3) hardward interrupts. */
#define NR_HARD_INTR 16

/* Structures below is a linked list of function pointers indicating the
   work to be done for each interrupt. Routines will be called one by
   another when interrupt comes.
   For example, the timer interrupts are used for alarm clocks, so
   device driver of timer will use add_irq_handle to attach a simple
   alarm checker. Also, the IDE driver needs to write back dirty cache
   slots periodically. So the IDE driver also calls add_irq_handle
   to register a handler. */

const char* get_current_tty(void);
char* bare_syscall_test="Bare Syscall Test...\n";

struct IRQ_t {
	void (*routine)(void);
	struct IRQ_t *next;
};

static struct IRQ_t handle_pool[NR_IRQ_HANDLE];
static struct IRQ_t *handles[NR_HARD_INTR];
static int handle_count = 0;

void
add_irq_handle(int irq, void (*func)(void) ) {
	struct IRQ_t *ptr;
	assert(irq < NR_HARD_INTR);
	if (handle_count > NR_IRQ_HANDLE) {
		panic("Too many irq registrations!");
	}
	ptr = &handle_pool[handle_count ++]; /* get a free handler */
	ptr->routine = func;
	ptr->next = handles[irq]; /* insert into the linked list */
	handles[irq] = ptr;
}
void schedule();

void irq_handle(TrapFrame *tf) {
	int irq = tf->irq;

	if (irq < 0) {
		panic("Unhandled exception!");
	}

	if (irq == 0x80) {
		switch(tf->eax) {
		case 0:
			break;
		case 1:
			tf->eax=dev_write(get_current_tty(), current->pid, bare_syscall_test, 0, strlen(bare_syscall_test));
			current->tf=tf;
			return;
			break;
		case SYS_puts:
			tf->eax=dev_write(get_current_tty(), current->pid, (void*)(tf->ebx), 0, strlen((const char*)(tf->ebx)));
			current->tf=tf;
			return;
			break;
		default:
			assert(0);
		}
	} else if (irq < 1000) {
		extern uint8_t logo[];
		printk("Errorcode: %d\n", tf->error_code);
		panic("Unexpected exception #%d\n\33[1;31mHint: The machine is always right! For more details about exception #%d, see\n%s\n\33[0m", irq, irq, logo);
	} else if (irq >= 1000) {
		/* The following code is to handle external interrupts.
		 * You will use this code in Lab2.  */
		int irq_id = irq - 1000;
		assert(irq_id < NR_HARD_INTR);
		struct IRQ_t *f = handles[irq_id];
		while (f != NULL) { /* call handlers one by one */
			f->routine(); 
			f = f->next;
		}
	}

	current->tf = tf;
	schedule();
}

