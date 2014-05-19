#ifndef __FILE_H__
#define __FILE_H__

#define NR_FILE_SIZE (128 * 1024)

static inline void read_file(int filename, void* buf, off_t offset, size_t len) {
  dev_read("ram", current->pid, buf, filename*NR_FILE_SIZE+offset, len);
}

#endif
