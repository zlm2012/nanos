#ifndef __PROCESS_H__
#define __PROCESS_H__
#define KSTACK_SIZE 4096
#include "adt/list.h"
typedef union PCB {
  uint8_t kstack[KSTACK_SIZE];
  struct {
    void *tf;
    int lock;
    volatile bool sleep;
  };
} PCB;

typedef struct Semaphore {
  int token;
  ListHead block;
}Sem;

extern PCB *current;

static inline void
create_sem(Sem* sem, int tok) {
  sem->token=tok;
  list_init(&(sem->block));
}

static inline void
lock() {
  //  cli();
  (current->lock)++;
}

static inline void
unlock() {
  (current->lock)--;
  //  if(!(current->lock)) sti();
}



#endif
