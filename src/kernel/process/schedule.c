#include "kernel.h"
#include "common.h"
#include "adt/list.h"

PCB idle, *current = &idle;

extern int pcblen;
extern PCB pcbpool[];
extern ListHead readyhead;
extern void enterProcQ(bool, PCB*);
extern PCB* leaveProcQ(bool);

void
schedule(void) {
  if (list_empty(&readyhead)) {
    current = &idle;
    return;
  }
  current=leaveProcQ(false);
  enterProcQ(false, current);
}
