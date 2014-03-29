#ifndef __PROCESS_H__
#define __PROCESS_H__
#define KSTACK_SIZE 4096
typedef struct PCB {
	uint8_t kstack[KSTACK_SIZE];
	struct {
		void *tf;
		bool sleep;
	};
} PCB;

extern PCB *current;

#endif
