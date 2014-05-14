#ifndef __PROCESS_H__
#define __PROCESS_H__
#define KSTACK_SIZE 4096
#define MSG_TRGT_ANY -1
#define MSG_HARD_INTR -2
#define ANY -1
#define NR_PROC 50
#define NEW_PROC 301
#define DSTRY_PROC 302
#include "adt/list.h"
#include "common.h"
#include "string.h"

pid_t PROCMAN;

typedef struct Semaphore {
  int token;
  ListHead block;
}Sem;

typedef struct memdist {
  void* caddr;
  uint32_t csize;
  void* daddr;
  uint32_t dsize;
  void* saddr;
}memdist;

typedef union PCB {
  uint8_t kstack[KSTACK_SIZE];
  struct {
    void *tf;
    volatile int lock;
    volatile bool lockif;
    ListHead msgq;
    Sem msem;
    pid_t pid;
    memdist paged;
    CR3 cr3;
    bool va;
  };
} PCB;

extern PCB pcbpool[];

typedef struct Message {
  pid_t src, dest;
  union {
    int type;
    int ret;
  };
  union {
    int i[5];
    struct {
      pid_t req_pid;
      int dev_id;
      void *buf;
      off_t offset;
      size_t len;
    };
  };
  ListHead list;
} Msg;

typedef struct MsgPU {
  Msg m;
  bool va;
} MsgPU;

extern PCB *current;
extern MsgPU msgpool[];
extern int msglen;

static inline void
create_sem(Sem* sem, int tok) {
  sem->token=tok;
  list_init(&(sem->block));
}

static inline void
lock() {
  volatile bool ifl=!!(read_eflags()&512);
  cli();
  if(current->lock==0)
  current->lockif=ifl;
  (current->lock)++;
}

static inline void
unlock() {
  if(current->lock)(current->lock)--;
  assert(current->lock>=0);
  if(current->lockif && current->lock==0) sti();
}

extern void P(Sem*);
extern void V(Sem*);
extern PCB* create_kthread(void*);
extern void sleep(void);
extern void wakeup(PCB*);

static inline PCB*
fetch_pcb(pid_t p) {
  return &pcbpool[p];
}

static inline void
send(pid_t dest, Msg *m) {
  int i;
  lock();
  assert(msglen<500);
  if (pcbpool[dest].va==false)return;
  i=0;
  while(msgpool[i].va) i++;
  msgpool[i].va=true;
  memcpy(&(msgpool[i]), m, sizeof(Msg));
  msglen++;
  list_add_before(&(pcbpool[dest].msgq), &(msgpool[i].m.list));
  V(&(pcbpool[dest].msem));
  unlock();
}

static inline void
receive(pid_t src, Msg *m) {
  //volatile int walked;
  ListHead *i;
  Msg *mitr;
  lock();
  //walked=0;
  while(1) {
    if(list_empty(&(current->msgq))) goto RCV_FAILED;
    list_foreach(i, &(current->msgq)) {
      mitr=list_entry(i, Msg, list);
      if(src==ANY || mitr->src==src) {
        memcpy(m, mitr, sizeof(Msg));
        list_del(i);
        ((MsgPU*)mitr)->va=false;
        msglen--;
	//while (walked--) V(&(current->msem));
	unlock();
	return;
      }
    }
RCV_FAILED:
    P(&(current->msem));
    //walked++;
  }
}

static inline PCB*
new_pcb(void) {
  int i;
  for(i=0; pcbpool[i].va && i<NR_PROC; i++);
  assert(i<NR_PROC);
  pcbpool[i].pid=i;
  pcbpool[i].va=true;
  return &pcbpool[i];
}

static inline void
do_exit(int status, pid_t pid) {
  Msg m;
  m.src=pid;
  m.dest=PROCMAN;
  m.type=DSTRY_PROC;
  m.req_pid=pid;
  send(PROCMAN, &m);
  receive(PROCMAN, &m);
  assert(0);
}

void free_pcb(pid_t pid);

#endif
