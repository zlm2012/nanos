#include "kernel.h"
#include "adt/list.h"
#include "string.h"

//extern uint8_t stackPool[];
extern PCB* current;
extern void schedule();
int pcblen = 0;
PCB pcbpool[20];
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

void A () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("a"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
void B () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	receive(4, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("b"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
void C () { 
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	receive(4, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("c"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
void D () { 
	Msg m1, m2;
	m1.src = current->pid;
	receive(4, &m2);
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("d"); 
			send(4, &m1);
			receive(4, &m2);
		}
		x ++;
	}
}
 
void E () {
	Msg m1, m2;
	m2.src = current->pid;
	char c;
	while(1) {
		receive(MSG_TRGT_ANY, &m1);
		if(m1.src == 0) {c = '|'; m2.dest = 1; }
		else if(m1.src == 1) {c = '/'; m2.dest = 2;}
		else if(m1.src == 2) {c = '-'; m2.dest = 3;}
		else if(m1.src == 3) {c = '\\';m2.dest = 0;}
		else assert(0);
 
		printk("\033[s\033[1000;1000H%c\033[u", c);
		send(m2.dest, &m2);
	}
 
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
  //test_setup();
  //wakeup(&pcbpool[0]);
  wakeup(create_kthread(A));
  wakeup(create_kthread(B));
  wakeup(create_kthread(C));
  wakeup(create_kthread(D));
  wakeup(create_kthread(E));
}

