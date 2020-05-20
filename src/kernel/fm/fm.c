#include "kernel.h"
#define NR_MAX_FILE 8
#define NR_FILE_SIZE (6 * 1024)
 
pid_t FILEMAN;

static void fm_thread(void);

static inline int
do_read(int filename, uint8_t *buf, off_t offset, size_t len) {
  return dev_read("ram", current->pid, buf, filename*NR_FILE_SIZE+offset, len);
}

static inline int
do_write(int filename, uint8_t *buf, off_t offset, size_t len) {
  return dev_write("ram", current->pid, buf, filename*NR_FILE_SIZE+offset, len);
}

void
init_fm(void) {
  PCB *p = create_kthread("File Manager", fm_thread);
  FILEMAN = p->pid;
  wakeup(p);
}

static void
fm_thread(void) {
  static Msg m;

  while (true) {
    receive(ANY, &m);

    if (m.type == DO_READ) {
      m.ret=do_read(m.dev_id, m.buf, m.offset, m.len);
      m.dest = m.src;
      m.src = FILEMAN;
      send(m.dest, &m);
    } else if (m.type == DO_WRITE) {
      m.ret=do_write(m.dev_id, m.buf, m.offset, m.len);
      m.dest = m.src;
      m.src = FILEMAN;
      send(m.dest, &m);
    }
    else {
      assert(0);
    }
  }
}
