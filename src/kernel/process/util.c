#include "kernel.h"

extern uint8_t stackPool[];
int pcblen = 0;
PCB* pcbpool[8];

void A () { 
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {printk("a");}
        x ++;
    }
}
void B () { 
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {printk("b");}
        x ++;
    }
}

PCB*
create_kthread(void *fun) {
	TrapFrame *tf=(TrapFrame *)&stackPool[1024*(++pcblen)-sizeof(TrapFrame)];
	*(((int*)tf)-1)=(int)tf;
	pcbpool[pcblen-1]=((PCB*)tf)-1;
	tf->eip=(uint32_t)fun;
	tf->cs=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_CODE);
	tf->ds=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->es=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->fs=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->gs=(uint32_t)SELECTOR_KERNEL(SEG_KERNEL_DATA);
	tf->esp=(uint32_t)&(tf->gs);
	tf->eflags=512;
	printk("thread created: %d", pcblen);
	return pcbpool[pcblen-1];
}

void
init_proc() {
	create_kthread(&A);
	create_kthread(&B);
}

