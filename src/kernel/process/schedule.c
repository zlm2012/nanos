#include "kernel.h"
#include "common.h"

PCB idle, *current = &idle;

extern int pcblen;
extern PCB pcbpool[];
static int i = 0;
void
schedule(void) {
	if(!pcblen) {
		current=&idle;
		return;
	}
	while(pcbpool[i].sleep && i<pcblen) i++;
	if(i==pcblen) {
		i=0;
		current=&idle;
		return;
	}
	current=&pcbpool[i];
	i++;
	i%=pcblen;
}
