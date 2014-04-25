#ifndef __PROCESS_H__
#define __PROCESS_H__
#define KSTACK_SIZE 4096
#define MSG_TRGT_ANY -1
#include "adt/list.h"
#include "common.h"
#include "string.h"

typedef struct Semaphore {
  int token;
  ListHead block;
}Sem;

typedef union PCB {
  uint8_t kstack[KSTACK_SIZE];
  struct {
    void *tf;
    volatile int lock;
    volatile long lockif;
    ListHead msgq;
    Sem msem;
    pid_t pid;
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

extern PCB *current;

static inline void
create_sem(Sem* sem, int tok) {
  sem->token=tok;
  list_init(&(sem->block));
}

static inline void
lock() {
  long ifl=(read_eflags()&512);
  cli();
  assert(current->lock<64);
  current->lockif|=ifl<<(current->lock);
  (current->lock)++;
}

static inline void
unlock() {
  (current->lock)--;
  assert(current->lock>=0);
  if(((current->lockif)>>current->lock)&1) sti();
}

extern void P(Sem*);
extern void V(Sem*);

static inline void
send(pid_t dest, Msg *m) {
  lock();
  list_add_after(pcbpool[dest].msgq.prev, &(m->list));
  V(&(pcbpool[dest].msem));
  unlock();
}

static inline void
receive(pid_t src, Msg *m) {
  int walked;
  ListHead *i;
  lock();
  walked=0;
  while(1) {
    P(&(current->msem));
    list_foreach(i, &(current->msgq))
      if(src==MSG_TRGT_ANY || list_entry(i, Msg, list)->src==src) {
	memcpy(m, list_entry(i, Msg, list), sizeof(Msg));
        list_del(i);
	while (walked--) V(&(current->msem));
	unlock();
	return;
      }
    walked++;
  }
}

#endif
