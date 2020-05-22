#include "kernel.h"
#include "common.h"
#include "adt/list.h"

PCB idle, *current = &idle;

extern ListHead readyhead;
extern int readylen;
extern void enterProcQ(bool, PCB*, ListHead*);
extern PCB* leaveProcQ(ListHead*, int*);
extern inline void set_tss_esp0(uint32_t esp);

void
schedule(void) {
  if (list_empty(&readyhead)) {
    current = &idle;
  } else {
    current=leaveProcQ(&readyhead, &readylen);
    if (current->pid != -1) {
      enterProcQ(false, current, &readyhead);
    }
  }
  write_cr3(&(current->cr3));
  //printk("written cr3: %x\n", current->cr3.val);
  set_tss_esp0((uint32_t)(current->kstack+4095));
  //printk("change to pid %d belongs to pcb %p\n", current->pid, current);
}
