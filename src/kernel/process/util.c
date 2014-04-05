#include "kernel.h"
#include "adt/list.h"
#include "string.h"

//extern uint8_t stackPool[];
extern PCB* current;
extern void schedule();
int pcblen = 0;
PCB pcbpool[8];
typedef struct PCBQ {
  PCB* pcb;
  ListHead li;
  bool va;
} PCBQ;

PCBQ readypool[100], stallpool[100];
ListHead readyhead, stallhead;
static int readytail=0, stalltail=0, readylen=0, stalllen=0;

static inline void
initProcQ(void) {
  list_init(&readyhead);
  list_init(&stallhead);
  memset(readypool, 0, sizeof(readypool));
  memset(stallpool, 0, sizeof(stallpool));
}

void
enterProcQ(bool stall, PCB* pcb) {
  ListHead* i;
  if(stall) {
    if (stalllen==100)
      panic("Stall Queue Overflowed!\n");
    i=stallhead.prev;
    for (; stallpool[stalltail].va; stalltail++, stalltail%=100);
    stallpool[stalltail].va=1;
    stallpool[stalltail].pcb=pcb;
    list_add_after(i, &stallpool[stalltail].li);
    stalllen++;
  } else {
    if (readylen==100)
      panic("Ready Queue Overflowed!\n");
    i=readyhead.prev;
    for (; readypool[readytail].va; readytail++, readytail%=100);
    readypool[readytail].va=1;
    readypool[readytail].pcb=pcb;
    list_add_after(i, &readypool[readytail].li);
    readylen++;
  }
}

PCB*
leaveProcQ(bool stall) {
  ListHead* i;
  if (stall) {
    if (list_empty(&stallhead))
      return NULL;
    i=stallhead.next;
    list_del(i);
    stalllen--;
    list_entry(i, PCBQ, li)->va=0;
    return list_entry(i, PCBQ, li)->pcb;
  } else {
    if (list_empty(&readyhead))
      return NULL;
    i=readyhead.next;
    list_del(i);
    list_entry(i, PCBQ, li)->va=0;
    readylen--;
    return list_entry(i, PCBQ, li)->pcb;
  }
}

void sleep(void) {
  ListHead* i;
  list_foreach(i, &readyhead)
    if (list_entry(i, PCBQ, li)->pcb==current){
      readylen--;
      list_entry(i, PCBQ, li)->va=0;
      list_del(i);
      enterProcQ(true, current);
      asm volatile("int $0x80");
      break;
    }
}

void wakeup(PCB *p) {
  ListHead* i;
  list_foreach(i, &stallhead)
    if (list_entry(i, PCBQ, li)->pcb==p) {
      stalllen--;
      list_entry(i, PCBQ, li)->va=0;
      list_del(i);
      enterProcQ(false, p);
      break;
    }
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
  tf->xxx=(uint32_t)&(tf->gs);
  tf->eflags=512;
  return &pcbpool[pcblen++];
}

void
init_proc() {
  initProcQ();
  enterProcQ(false, create_kthread(&A));
  enterProcQ(false, create_kthread(&B));
  enterProcQ(false, create_kthread(&C));
  enterProcQ(false, create_kthread(&D));
}

