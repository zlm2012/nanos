#include "kernel.h"
 
pid_t PROCMAN;
extern pid_t FILEMAN;
extern pid_t MEMMAN;
extern PDE updir[][NR_PDE];

static void pm_thread(void);

void
init_pm(void) {
  PCB *p = create_kthread(pm_thread);
  PROCMAN = p->pid;
  wakeup(p);
}

void* new_page(pid_t req_id, size_t len, void* va) {
  Msg m;
  m.src=current->pid;
  m.dest=MEMMAN;
  m.type=NEW_PAGE;
  m.req_pid=req_id;
  m.len=len;
  m.buf=va;
  send(MEMMAN, &m);
  receive(MEMMAN, &m);
  return (void*)m.ret;
}

static void
pm_thread(void) {
  static Msg m;
  uint8_t *i, *va, *ka;
  pid_t osrc;
  char buf[512];
  ELFHeader *elf=(ELFHeader*)buf;
  ProgHeader *ph, *eph;
  PCB* p;

  while (true) {
    receive(ANY, &m);

    if (m.type == NEW_PROC) {
      osrc=m.src;
      p=new_pcb();
      p->cr3.val=0;
      p->cr3.page_directory_base=((uint32_t)va_to_pa(updir[p->pid-13]))>>12;
      read_file(m.dev_id, buf, 0, 512);
      ph=(ProgHeader*)(buf + elf->phoff);
      eph = ph + elf->phnum;
      for(; ph < eph; ph ++) {
	va = (unsigned char*)(ph->vaddr);
	if(va==0)continue;
	ka=new_page(p->pid, ph->memsz, va);
        read_file(m.dev_id, ka, ph->off, ph->filesz);
	for (i = ka + ph->filesz; i < ka + ph->memsz; *i ++ = 0);
      }
      ka=new_page(p->pid, 4096, (void*)0xbffff000);
      new_proc(p, (TrapFrame *)(ka+0x1000-sizeof(TrapFrame)), 0xbfffffff, elf->entry);
      m.dest=osrc;
      m.src=PROCMAN;
      m.ret=1;
      printk("User Process %d Created.\n", p->pid);
      if(m.dest>=0) send(m.dest, &m);
    } else if (m.type == DSTRY_PROC) {
      m.src=PROCMAN;
      m.dest=MEMMAN;
      m.type=FREE_PAGE;
      send(MEMMAN, &m);
      receive(MEMMAN, &m);
      free_pcb(m.req_pid);
    }
    else if (m.type == EXEC_PROC) {
      
    } else {
      assert(0);
    }
  }
}
