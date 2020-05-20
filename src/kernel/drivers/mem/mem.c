#include "kernel.h"
void init_ramdisk();
void init_rand();
static void mem_driver_thread();
pid_t MEMDR;

void init_mem() {
  PCB *p = create_kthread("mem driver", mem_driver_thread);
  init_rand();
  init_ramdisk();
  MEMDR = p->pid;
  hal_register("mem", MEMDR, 0);
  hal_register("kmem", MEMDR, 1);
  hal_register("null", MEMDR, 2);
  hal_register("zero", MEMDR, 3);
  wakeup(p);
}

static void
mem_driver_thread(void) {
  static Msg m;
  int i;

  while (true) {
    receive(ANY, &m);

    if (m.type == DEV_READ) {
      switch(m.dev_id) {
      case 0:
	copy_to_kernel(fetch_pcb(m.req_pid), m.buf, (void *)(m.offset+0xc0000000), m.len);
	m.ret = m.len;
	break;
      case 1:
	copy_to_kernel(fetch_pcb(m.req_pid), m.buf, (void *)m.offset, m.len);
	m.ret = m.len;
	break;
      case 2:
	m.ret = 0;
	break;
      case 3:
	for (i=0; i<m.len; i++)
	  *((uint8_t*)(m.buf+i))=0;
	m.ret = i;
	break;
      default:
	assert(0);
      }
      m.dest = m.src;
      m.src = MEMDR;
      send(m.dest, &m);
    } else if (m.type == DEV_WRITE) {
      switch (m.dev_id) {
      case 0:
	copy_to_kernel(fetch_pcb(m.req_pid), (void *)(m.offset+0xc0000000), m.buf, m.len);
	m.ret=m.len;
	break;
      case 1:
	copy_to_kernel(fetch_pcb(m.req_pid), (void *)m.offset, m.buf, m.len);
	m.ret=m.len;
	break;
      case 2:
      case 3:
	break;
      default:
	assert(0);
      }
      m.dest = m.src;
      m.src = MEMDR;
      send(m.dest, &m);
    }
    else {
      assert(0);
    }
  }
}
