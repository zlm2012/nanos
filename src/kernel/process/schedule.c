#include "kernel.h"
#include "common.h"

PCB idle, *current = &idle;

extern int pcblen;
extern PCB* pcbpool[];
static int i = 0;
void
schedule(void) {
	current=pcbpool[i%pcblen];
	i++;
}
