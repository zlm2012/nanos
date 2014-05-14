#include "kernel.h"
#include "common.h"
#include "adt/list.h"

PCB idle, *current = &idle;

extern int pcblen;
extern ListHead readyhead;
extern int readylen;
extern void enterProcQ(bool, PCB*, ListHead*);
extern PCB* leaveProcQ(ListHead*, int*);

void
schedule(void) {
  if (list_empty(&readyhead)) {
    current = &idle;
    return;
  }
  current=leaveProcQ(&readyhead, &readylen);
  enterProcQ(false, current, &readyhead);
  write_cr3(&(current->cr3));
}
