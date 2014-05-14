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
      m.src=current->pid;
      m.dest=FILEMAN;
      m.type=DO_READ;
      m.buf=buf;
      m.offset=0;
      m.len=512;
      send(FILEMAN, &m);
      receive(FILEMAN, &m);
      ph=(ProgHeader*)(buf + elf->phoff);
      eph = ph + elf->phnum;
      for(; ph < eph; ph ++) {
	va = (unsigned char*)(ph->vaddr);
	if(va==0)continue;
	m.src=current->pid;
	m.dest=MEMMAN;
	m.type=NEW_PAGE;
	m.req_pid=p->pid;
	m.len=ph->memsz;
	m.buf=va;
	send(MEMMAN, &m);
	receive(MEMMAN, &m);
	ka=(void*)m.ret;
	m.src=current->pid;
	m.dest=FILEMAN;
	m.type=DO_READ;
	m.buf=ka;
	m.offset=ph->off;
	m.len=ph->filesz;
	send(FILEMAN, &m);
	receive(FILEMAN, &m);
	for (i = ka + ph->filesz; i < ka + ph->memsz; *i ++ = 0);
      }
      m.src=current->pid;
      m.dest=MEMMAN;
      m.type=NEW_PAGE;
      m.req_pid=p->pid;
      m.len=4096;
      m.buf=(void*)0xBFFFF000;
      send(MEMMAN, &m);
      receive(MEMMAN, &m);
      TrapFrame *tf=(TrapFrame *)(m.ret+0x1000-sizeof(TrapFrame));
      p->tf=tf;
      p->lock=0;
      p->lockif=0;
      list_init(&(p->msgq));
      create_sem(&(p->msem), 0);
      tf->eip=elf->entry;
      tf->cs=(uint32_t)SELECTOR_USER(SEG_USER_CODE);
      tf->ds=(uint32_t)SELECTOR_USER(SEG_USER_DATA);
      tf->es=(uint32_t)SELECTOR_USER(SEG_USER_DATA);
      tf->fs=(uint32_t)SELECTOR_USER(SEG_USER_DATA);
      tf->gs=(uint32_t)SELECTOR_USER(SEG_USER_DATA);
      tf->esp=0xbfffffff;
      tf->eflags=512;
      tf->ss=(uint32_t)SELECTOR_USER(SEG_USER_DATA);
      wakeup(p);
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
    else {
      assert(0);
    }
  }
}
