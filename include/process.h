#ifndef __PROCESS_H__
#define __PROCESS_H__
#define KSTACK_SIZE 4096
typedef union PCB {
	uint8_t kstack[KSTACK_SIZE];
	struct {
		void *tf;
	};
} PCB;

extern PCB *current;

#endif
