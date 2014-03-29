#include "kernel.h"

//extern uint8_t stackPool[];
extern PCB* current;
extern void schedule();
int pcblen = 0;
PCB pcbpool[8];

void sleep(void) {
	current->sleep=true;
	wait_intr();
}

void wakeup(PCB *p) {
	p->sleep=false;
}

void A () { 
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("a");
            wakeup(&pcbpool[1]);
            sleep();
        }
        x ++;
    }
}
void B () { 
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("b");
            wakeup(&pcbpool[2]);
            sleep();
        }
        x ++;
    }
}
void C () { 
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("c");
            wakeup(&pcbpool[3]);
            sleep();
        }
        x ++;
    }
}
void D () { 
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("d");
            wakeup(&pcbpool[0]);
            sleep();
        }
        x ++;
    }
}

PCB*
create_kthread(void *fun) {
	TrapFrame *tf=(TrapFrame *)(pcbpool[pcblen].kstack+4096-sizeof(TrapFrame));
	pcbpool[pcblen].tf=tf;
	pcbpool[pcblen].sleep=false;
	tf->eip=(uint32_t)fun;
	tf->cs=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_CODE);
	tf->ds=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->es=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->fs=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->gs=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->esp=(uint32_t)&(tf->gs);
	tf->eflags=512;
	return &pcbpool[pcblen++];
}

void
init_proc() {
	create_kthread(&A);
	create_kthread(&B);
	create_kthread(&C);
	create_kthread(&D);
}

