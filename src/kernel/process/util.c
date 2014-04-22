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
int readytail=0, stalltail=0, readylen=0, stalllen=0;

static inline void
initProcQ(void) {
  list_init(&readyhead);
  list_init(&stallhead);
  memset(readypool, 0, sizeof(readypool));
  memset(stallpool, 0, sizeof(stallpool));
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
      current->sleep=true;
      asm volatile("int $0x80");
      current->sleep=false;
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

void A () { 
  int x = 0;
  while(1) {
    if(x % 100000 == 0) {
      printk("a");
      lock();
      x=0;
      wakeup(&pcbpool[1]);
      sleep();
      unlock();

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
  pcbpool[pcblen].lock=0;
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

#define NBUF 5
#define NR_PROD 3
#define NR_CONS 4
 
int buf[NBUF], f = 0, r = 0, g = 1;
int last = 0;
Sem empty, full, mutex;
 
void
test_producer(void) {
	while (1) {
		P(&empty);
		P(&mutex);
		if(g % 10000 == 0) {
			printk(".");	// tell us threads are really working
		}
		buf[f ++] = g ++;
		f %= NBUF;
		V(&mutex);
		V(&full);
	}
}
 
void
test_consumer(void) {
	int get;
	while (1) {
		P(&full);
		P(&mutex);
		get = buf[r ++];
		assert(last == get - 1);	// the products should be strictly increasing
		last = get;
		r %= NBUF;
		V(&mutex);
		V(&empty);
	}
}
 
void
test_setup(void) {
	create_sem(&full, 0);
	create_sem(&empty, NBUF);
	create_sem(&mutex, 1);
	int i;
	for(i = 0; i < NR_PROD; i ++) {
		wakeup(create_kthread(test_producer));
	}
	for(i = 0; i < NR_CONS; i ++) {
		wakeup(create_kthread(test_consumer));
	}
}

void
init_proc() {
  initProcQ();
  test_setup();
  //create_kthread(&A);
  //create_kthread(&B);
  //create_kthread(&C);
  //create_kthread(&D);
  //wakeup(&pcbpool[0]);
  //enterProcQ(false, create_kthread(&B), &readyhead);
  //enterProcQ(false, create_kthread(&C), &readyhead);
  //enterProcQ(false, create_kthread(&D), &readyhead);
}

