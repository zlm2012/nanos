#include "kernel.h"
#include "hal.h"
#include "time.h"
#define NR_MAX_FILE 8
#define NR_FILE_SIZE (128 * 1024)
 
static uint8_t file[NR_MAX_FILE][NR_FILE_SIZE]= {
  {0x12, 0x34, 0x56, 0x78},	// the first file '0'
  {"Hello World!\n"},		// the second file '1'
  {0x7f, 0x45, 0x4c, 0x46}	// the third file '2'
};
static uint8_t *disk = (void*)file;
pid_t RAMDISK;

static void ramdisk_driver_thread(void);

void
init_ramdisk(void) {
  PCB *p = create_kthread(ramdisk_driver_thread);
  RAMDISK = p->pid;
  hal_register("ram", RAMDISK, 0);
  wakeup(p);
}

static void
ramdisk_driver_thread(void) {
  static Msg m;

  while (true) {
    receive(ANY, &m);

    if (m.type == DEV_READ) {
      uint32_t i;
      uint8_t data;
      for (i = 0; i < m.len; i ++) {
	data = disk[m.offset + i];
	copy_from_kernel(fetch_pcb(m.req_pid), m.buf + i, &data, 1);
      }
      m.ret = i;
      m.dest = m.src;
      m.src = RAMDISK;
      send(m.dest, &m);
    } else if (m.type == DEV_WRITE) {
      uint32_t i;
      uint8_t data;
      for (i = 0; i < m.len; i ++) {
	copy_to_kernel(fetch_pcb(m.req_pid), &data, m.buf + i, 1);
	memcpy(&(disk[m.offset + i]), &data, 1);
      }
      m.ret = i;
      m.dest = m.src;
      m.src = RAMDISK;
      send(m.dest, &m);
    }
    else {
      assert(0);
    }
  }
}
