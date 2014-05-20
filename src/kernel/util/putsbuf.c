#include "kernel.h"

static char buf[256];
static int buflen=0;

const char* get_current_tty(void);

void flush() {
  char buff[256];
  int i=buflen;
  memcpy(buff, buf, 256);
  buflen=0;
  dev_write(get_current_tty(), current->pid, buff, 0, i);
}

void putsbuf(char c) {
  buf[buflen++]=c;
  if (buflen==256) flush();
}
