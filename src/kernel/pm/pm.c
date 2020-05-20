#include "kernel.h"
 
pid_t PROCMAN;
extern pid_t FILEMAN;
extern pid_t MEMMAN;
extern PDE updir[][NR_PDE];

static void pm_thread(void);

static ListHead wpid_list[NR_PROC];
typedef struct wpid_unit {
  PCB* p;
  ListHead h;
}wpu;

void
init_pm(void) {
  int i;
  PCB *p = create_kthread("Process Manager", pm_thread);
  PROCMAN = p->pid;
  wakeup(p);
  for (i=0; i<NR_PROC; i++)
    list_init(&wpid_list[i]);
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

uint32_t proc_from_file(int filename, PCB* p) {
  uint8_t *i, *va, *ka;
  char buf[512];
  ELFHeader *elf=(ELFHeader*)buf;
  ProgHeader *ph, *eph;
  read_file(filename, buf, 0, 512);
  if (elf->magic!=0x464c457f)return -1;
  ph=(ProgHeader*)(buf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph ++) {
    va = (unsigned char*)(ph->vaddr);
    if(va==0)continue;
    ka=new_page(p->pid, ph->memsz, va);
    read_file(filename, ka, ph->off, ph->filesz);
    for (i = ka + ph->filesz; i < ka + ph->memsz; *i ++ = 0);
  }
  return elf->entry;
}

static void
pm_thread(void) {
  static Msg m;
  uint8_t *ka;
  char* esp;
  uint32_t entry, argc, i;
  pid_t osrc;
  PCB* p;
  ListHead *itr;
  wpu* wp;
  char **kargv, **dargv;

  while (true) {
    receive(ANY, &m);

    if (m.type == NEW_PROC) {
      osrc=m.src;
      p=new_pcb();
      p->cr3.val=0;
      p->cr3.page_directory_base=((uint32_t)va_to_pa(updir[p->pid-13]))>>12;
      entry=proc_from_file(m.dev_id, p);
      if (entry==-1) {
        m.dest=osrc;
        m.src=PROCMAN;
        m.ret=-1;
        printk("Not an elf...\n");
        continue;
      }
      ka=new_page(p->pid, 4096, (void*)0xbffff000);
      new_proc(p, (TrapFrame *)(ka+0x1000-sizeof(TrapFrame)), 0xbfffffff, entry);
      m.dest=osrc;
      m.src=PROCMAN;
      m.ret=1;
      printk("User Process %d Created.\n", p->pid);
      if(m.dest>=0) send(m.dest, &m);
    } else if (m.type == WAIT_PROC) {
      wp=(wpu*)kmalloc(sizeof(wpu));
      wp->p=fetch_pcb(m.src);
      list_add_before(&wpid_list[m.req_pid], &(wp->h));
    } else if (m.type == DSTRY_PROC) {
      m.src=PROCMAN;
      m.dest=MEMMAN;
      m.type=FREE_PAGE;
      send(MEMMAN, &m);
      receive(MEMMAN, &m);
      free_pcb(m.req_pid);
      list_foreach(itr, &wpid_list[m.req_pid]) {
        wp=list_entry(itr, wpu, h);
        m.src=PROCMAN;
        m.dest=wp->p->pid;
        send(m.dest, &m);
        list_del(itr);
        kfree(wp);
      }
    }
    else if (m.type == EXEC_PROC) {
      osrc=m.src;
      p=fetch_pcb(m.req_pid);
      argc=m.len;
      dargv=(char**)kmalloc(argc*sizeof(char*));
      kargv=(char**)m.buf;
      free_page(p->paged.caddr, p->paged.csize);
      free_page(p->paged.daddr, p->paged.dsize);
      entry=proc_from_file(m.dev_id, p);
      if (entry==-1) {
        m.dest=osrc;
        m.src=PROCMAN;
        m.ret=-1;
        printk("Not an elf...\n");
        continue;
      }
      ka=nndma_to_ka(p->cr3.val, (uint32_t)p->paged.saddr);
      new_proc(p, (TrapFrame *)(ka+0x1000-sizeof(TrapFrame)), 0xbfffffff, entry);
      p->tf->eax=argc;
      esp=(char*)p->tf->esp;
      //printk("Now init argc, argv...\n");
      for (i=0; i<argc; i++) {
        esp-=strlen(kargv[i]);
        strcpy_from_kernel(p, esp, kargv[i]);
        kfree(kargv[i]);
        dargv[i]=esp;
        esp--;
      }
      esp-=argc*sizeof(char*)-1;
      copy_from_kernel(p, esp, dargv, argc*sizeof(char*));
      p->tf->esp=(uint32_t)esp;
      kfree(kargv);
      kfree(dargv);
    } else {
      assert(0);
    }
  }
}
