#include "kernel.h"

static char buf[256];
static int buflen=0;

const char* get_current_tty(void);

void flush() {
  printk("-----\nflush: %s\n--------\n", buf);
  dev_write(get_current_tty(), current->pid, buf, 0, buflen);
  buflen=0;
  memset(buf, 0, 256);
}

void putsbuf(char c) {
  buf[buflen++]=c;
  if (buflen==256) flush();
}
