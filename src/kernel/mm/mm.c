#include "kernel.h"
#include "string.h"
 
pid_t MEMMAN;

PDE updir[NR_PROC-13][NR_PDE] align_to_page;
PTE uptable[NR_PROC-13][2*NR_PTE] align_to_page;

static void mm_thread(void);

void
init_mm(void) {
  int pdir_idx;
  PTE *ptable=(PTE*)va_to_pa(get_kptable());
  for (pdir_idx=0; pdir_idx<PHY_MEM/PD_SIZE; pdir_idx++) {
    make_pde(&updir[0][pdir_idx+KOFFSET/PD_SIZE], ptable);
    ptable+=NR_PTE;
  }
  for (pdir_idx=1; pdir_idx<NR_PROC-13; pdir_idx++)
    memcpy(updir[pdir_idx], updir[0], NR_PDE*sizeof(PDE));
  memset(uptable, 0, sizeof(uptable));
  for (pdir_idx=0; pdir_idx<NR_PROC-13; pdir_idx++) {
    make_pde(&updir[pdir_idx][0x08048000/PD_SIZE], va_to_pa(uptable[pdir_idx]));
    make_pde(&updir[pdir_idx][KOFFSET/PD_SIZE-1], va_to_pa(&uptable[pdir_idx][NR_PTE]));
  }
  PCB *p = create_kthread(mm_thread);
  MEMMAN = p->pid;
  wakeup(p);
}

static void
mm_thread(void) {
  static Msg m;
  int i, j, len;
  void* pa, *ipa;
  memdist* md;

  while (true) {
    receive(ANY, &m);

    if (m.type == NEW_PAGE) {
      pa=0;
      j=(((uint32_t)m.buf)&0x3ff000)>>12;
      if (((uint32_t)m.buf)&0xfff)
        len=(m.len-(((j+1)<<12)-(((uint32_t)m.buf)&0x3fffff))+PAGE_SIZE-1)/PAGE_SIZE+1;
      else len=(m.len+PAGE_SIZE-1)/PAGE_SIZE;
      if(((uint32_t)m.buf)/PD_SIZE==0x08048000/PD_SIZE) {
	while(uptable[m.req_pid-13][j].present&&len>0)len--, j++;
	if(len)pa=get_page(len);
	if(j==0) {
	  fetch_pcb(m.req_pid)->paged.caddr=pa;
	  fetch_pcb(m.req_pid)->paged.csize=len;
	} else {
	  fetch_pcb(m.req_pid)->paged.daddr=pa;
	  fetch_pcb(m.req_pid)->paged.dsize=len;
	}
	for(ipa=pa,i=0; i<len; i++, j++, ipa+=PAGE_SIZE)
	  make_pte(&uptable[m.req_pid-13][j], ipa);
      } else if (((uint32_t)m.buf)/PD_SIZE==KOFFSET/PD_SIZE-1) {
	if (uptable[m.req_pid-13][2*NR_PTE-1].val==0) {
	  make_pte(&uptable[m.req_pid-13][2*NR_PTE-1], pa=get_page(1));
	  fetch_pcb(m.req_pid)->paged.saddr=pa;
	}
      } else panic("MM failed: %p, %d\n", m.buf, m.len);
      m.dest = m.src;
      m.src = MEMMAN;
      m.ret = (uint32_t)pa_to_va(pa);
      send(m.dest, &m);
    } else if (m.type == FREE_PAGE) {
      memset(uptable[m.req_pid], 0, 2*NR_PTE*sizeof(PTE));
      md=&(fetch_pcb(m.req_pid)->paged);
      free_page(md->caddr, md->csize);
      free_page(md->daddr, md->dsize);
      free_page(md->saddr, 1);
      m.ret=0;
      m.dest = m.src;
      m.src = MEMMAN;
      send(m.dest, &m);
    }
    else {
      panic("MM assertion failed. Src: %d, Type: %d\n", m.src, m.type);
    }
  }
}
