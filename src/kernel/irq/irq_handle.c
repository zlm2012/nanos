#include "kernel.h"

#define NR_IRQ_HANDLE 32

/* In Nanos, there are no more than 16(actually, 3) hardward interrupts. */
#define NR_HARD_INTR 16

/* Structures below is a linked list of function pointers indicating the
   work to be done for each interrupt. Routines will be called one by
   another when interrupt comes.
   For example, the timer interrupts are used for alarm clocks, so
   device driver of timer will use add_irq_handle to attach a simple
   alarm checker. Also, the IDE driver needs to write back dirty cache
   slots periodically. So the IDE driver also calls add_irq_handle
   to register a handler. */

const char* get_current_tty(void);
void flush();
void putsbuf(char);
char* bare_syscall_test="Bare Syscall Test...\n";
char* seg_fault="Segmentation Fault\n";

struct IRQ_t {
  void (*routine)(void);
  struct IRQ_t *next;
};

static struct IRQ_t handle_pool[NR_IRQ_HANDLE];
static struct IRQ_t *handles[NR_HARD_INTR];
static int handle_count = 0;

void
add_irq_handle(int irq, void (*func)(void) ) {
  struct IRQ_t *ptr;
  assert(irq < NR_HARD_INTR);
  if (handle_count > NR_IRQ_HANDLE) {
    panic("Too many irq registrations!");
  }
  ptr = &handle_pool[handle_count ++]; /* get a free handler */
  ptr->routine = func;
  ptr->next = handles[irq]; /* insert into the linked list */
  handles[irq] = ptr;
}
void schedule();

void do_fork(TrapFrame* tf) {
  PCB* p;
  int i, j, len;
  void *pa, *ipa;
  p=new_pcb();
  tf->eax=p->pid;
  current->tf=tf;
  pcbfork(p, current);
  (p->tf)->eax=0;
  pa=current->paged.caddr;
  len=current->paged.csize;
  realloc_page(pa, len);
  for(ipa=pa,i=0, j=0x48; i<len; i++, j++, ipa+=PAGE_SIZE)
    make_pte(&uptable[p->pid-13][j], ipa, 1, 0);
  pa=current->paged.daddr;
  len=current->paged.dsize;
  realloc_page(pa, len);
  for(ipa=pa,i=0; i<len; i++, j++, ipa+=PAGE_SIZE)
    make_pte(&uptable[p->pid-13][j], ipa, 1, 1);
  make_pte(&uptable[p->pid-13][2*NR_PTE-1], pa=get_page(1), 1, 1);
  p->paged.saddr=pa;
  printk("original cr3: %x, new cr3: %x", current->cr3.val, p->cr3.val);
  printk(", original ka: %p, new ka: %p\n", nndma_to_ka(p->cr3.val, 0xbffff000), nndma_to_ka(current->cr3.val, 0xbffff000));
  memcpy(nndma_to_ka(p->cr3.val, 0xbffff000), nndma_to_ka(current->cr3.val, 0xbffff000), 4096);
  wakeup(p);
}

void do_exec(int filename, int argc, char** argv) {
  Msg m;
  char** kargv;
  int i;
  //printk("argc: %d, argv: %p\n", argc, argv);
  kargv=(char**)kmalloc(argc*sizeof(char*));
  //printk("kargv: %p, dargv: %p\n", kargv, dargv);
  for (i=0; i<argc; i++) {
    kargv[i]=kmalloc(strlen(argv[i])+1);
    strcpy_to_kernel(current, kargv[i], argv[i]);
    //printk("%s\n", kargv[i]);
  }
  //printk("Start exec proc...\n");
  m.src=current->pid;
  m.dest=PROCMAN;
  m.req_pid=current->pid;
  m.type=EXEC_PROC;
  m.buf=kargv;
  m.len=argc;
  m.dev_id=filename;
  send(PROCMAN, &m);
  sleep();
}

void irq_handle(TrapFrame *tf) {
  int irq = tf->irq;

  if (irq < 0) {
    panic("Unhandled exception!");
  }

  if (irq == 0x80) {
    switch(tf->eax) {
    case 0:
      break;
    case 1:
      tf->eax=dev_write(get_current_tty(), current->pid, bare_syscall_test, 0, strlen(bare_syscall_test));
      current->tf=tf;
      return;
      break;
    case SYS_puts:
      tf->eax=dev_write(get_current_tty(), current->pid, (void*)(tf->ebx), 0, strlen((const char*)(tf->ebx)));
      current->tf=tf;
      return;
      break;
    case SYS_exit:
      do_exit(tf->ebx, current->pid);
      break;
    case SYS_printf:
      lock();
      printk("%s", (const char*)tf->ebx);
      tf->eax=vfprintf(putsbuf, (const char*)tf->ebx, (void**)tf->ecx);
      flush();
      unlock();
      break;
    case SYS_fork:
      do_fork(tf);
      return;
      break;
    case SYS_exec:
      printk("start exec...\n");
      do_exec(tf->ebx, tf->ecx, (void*)tf->edx);
      printk("exec executed...\n");
      return;
      break;
    default:
      assert(0);
    }
  } else if (irq < 1000) {
    extern uint8_t logo[];
    printk("Errorcode: %x\n", tf->error_code);
    printk("Unexpected exception #%d\n\33[1;31mHint: The machine is always right! For more details about exception #%d, see\n%s\n\33[0m", irq, irq, logo);
    if (current->cr3.val!=get_kcr3()->val) {
      dev_write(get_current_tty(), current->pid, seg_fault, 0, strlen(seg_fault));
      do_exit(-1, current->pid);
    } else assert(0);
  } else if (irq >= 1000) {
    /* The following code is to handle external interrupts.
     * You will use this code in Lab2.  */
    int irq_id = irq - 1000;
    assert(irq_id < NR_HARD_INTR);
    struct IRQ_t *f = handles[irq_id];
    while (f != NULL) { /* call handlers one by one */
      f->routine(); 
      f = f->next;
    }
  }

  current->tf = tf;
  schedule();
}

