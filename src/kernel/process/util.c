#include "kernel.h"
#include "adt/list.h"
#include "string.h"

//extern uint8_t stackPool[];
extern PCB* current;
extern void A();
extern void B();
extern void C();
extern void D();
extern void E();
extern void test_setup();
extern void testForTimer();
extern void testIDE();
extern void testRamdisk();

int pcblen = 0, msglen=0;
PCB pcbpool[50];
MsgPU msgpool[500];
typedef struct PCBQ {
  PCB* pcb;
  ListHead li;
  bool va;
} PCBQ;

PCBQ readypool[100], stallpool[100];
ListHead readyhead, stallhead;
int readytail=0, stalltail=0, readylen=0, stalllen=0;

static inline void
initProcQ(void) {
  list_init(&readyhead);
  list_init(&stallhead);
  memset(readypool, 0, sizeof(readypool));
  memset(stallpool, 0, sizeof(stallpool));
  memset(msgpool, 0, sizeof(msgpool));
}

void
enterProcQ(bool stall, PCB* pcb, ListHead* q) {
  ListHead* i;
  if(stall) {
    if (stalllen==100)
      panic("Stall Queue Overflowed!\n");
    i=q->prev;
    for (; stallpool[stalltail].va; stalltail++, stalltail%=100);
    stallpool[stalltail].va=1;
    stallpool[stalltail].pcb=pcb;
    list_add_after(i, &stallpool[stalltail].li);
    stalllen++;
  } else {
    if (readylen==100)
      panic("Ready Queue Overflowed!\n");
    i=q->prev;
    for (; readypool[readytail].va; readytail++, readytail%=100);
    readypool[readytail].va=1;
    readypool[readytail].pcb=pcb;
    list_add_after(i, &readypool[readytail].li);
    readylen++;
  }
}

PCB*
leaveProcQ(ListHead* q, int* len) {
  ListHead* i;
  if (list_empty(q))
    return NULL;
  i=q->next;
  list_del(i);
  (*len)--;
  list_entry(i, PCBQ, li)->va=0;
  return list_entry(i, PCBQ, li)->pcb;
}

void sleep(void) {
  lock();
  ListHead* i;
  list_foreach(i, &readyhead)
    if (list_entry(i, PCBQ, li)->pcb==current){
      readylen--;
      list_entry(i, PCBQ, li)->va=0;
      list_del(i);
      enterProcQ(true, current, &stallhead);
      asm volatile("int $0x80");
      break;
    }
  unlock();
}

void wakeup(PCB *p) {
  lock();
  ListHead* i;
  bool new=true;
  list_foreach(i, &stallhead)
    if (list_entry(i, PCBQ, li)->pcb==p) {
      stalllen--;
      list_entry(i, PCBQ, li)->va=0;
      list_del(i);
      enterProcQ(false, p, &readyhead);
      new=false;
      break;
    }
  if (new) enterProcQ(false, p, &readyhead);
  unlock();
}

void P(Sem *s) {
  lock();
  if (s->token>0) {
    s->token--;
  } else {
    enterProcQ(true, current, &(s->block));
    sleep();
  }
  unlock();
}

void V(Sem *s) {
  lock();
  if (list_empty(&(s->block))) {
    s->token++;
  } else {
    wakeup(leaveProcQ(&(s->block), &stalllen));
  }
  unlock();
}

PCB*
create_kthread(void *fun) {
  TrapFrame *tf=(TrapFrame *)(pcbpool[pcblen].kstack+4096-sizeof(TrapFrame));
  pcbpool[pcblen].tf=tf;
  pcbpool[pcblen].lock=0;
  pcbpool[pcblen].lockif=0;
  pcbpool[pcblen].pid=pcblen;
  list_init(&(pcbpool[pcblen].msgq));
  create_sem(&(pcbpool[pcblen].msem), 0);
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
  //test_setup();
  //wakeup(&pcbpool[0]);
  wakeup(create_kthread(A));
  wakeup(create_kthread(B));
  wakeup(create_kthread(C));
  wakeup(create_kthread(D));
  wakeup(create_kthread(E));
  wakeup(create_kthread(testIDE));
  wakeup(create_kthread(testForTimer));
  wakeup(create_kthread(testRamdisk));
}

