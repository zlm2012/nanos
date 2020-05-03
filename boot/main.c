/* start.S and boot.S has put the processor into protected 32-bit mode,
	 and set up the right segmentation. The layout of our hard
	 disk is shown below:
	 +----------+------------+------------+------------------.        .-----------------+
	 | bootsect | bootloader | DOS Compat | The kernel binary    ...     (in ELF format)|
	 +----------+------------+------------+------------------`        '-----------------+
	 So the task of the C code is to load the game binary into
	 correct memory location (0x100000), and jump to it. */

#include "boot.h"

#define SECTSIZE 512
#define KOFFSET  0xC0000000

typedef unsigned int size_t;

void readseg(unsigned char *, int, int);
void init_serial(void);
void serial_printc(char);
void init_floppy(void);
int fd_secsize(void);
void fd_readsect(void *dst, unsigned int lba);
void init_idt();

inline void _memcpy(void *dest, const void *src, size_t size) {
	asm volatile ("cld; rep movsb" : : "c"(size), "S"(src), "D"(dest));
}

void
bootmain(void) {
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	unsigned char* pa, *i;
	void (*entry)(void);
	unsigned int j;

	serial_printc('0');
	init_floppy();
	init_idt();

	serial_printc('1');

	/* The binary is in ELF format (please search the Internet).
	   0x8000 is just a scratch address. Anywhere would be fine. */
	elf = (struct ELFHeader*)0x8000;

	/* Read the first 4096 bytes into memory.
	   The first several bytes is the ELF header. */
	readseg((unsigned char*)elf, 4096, 0);
	serial_printc('2');

	/* Load each program segment */
	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)(ph->paddr); /* physical address */
		readseg(pa, ph->filesz, ph->off); /* load from disk */
		for (i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
	}
	serial_printc('3');

	/* Here we go! */
	entry = (void(*)(void))(elf->entry - KOFFSET);
	serial_printc('@');
	entry(); /* never returns */
}

void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0x40); /* Spin on disk until ready */
}

unsigned char ide_polling(unsigned int advanced_check) {
   int i;
   // (I) Delay 400 nanosecond for BSY to be set:
   // -------------------------------------------------
   for(i = 0; i < 4; i++)
      in_byte(0x3F8); // Reading the Alternate Status port wastes 100ns; loop four times.
   // (II) Wait for BSY to be cleared:
   // -------------------------------------------------
   while (in_byte(0x1F7) & 0x80)
      ; // Wait for BSY to be zero.
 
   if (advanced_check) {
      unsigned char state = in_byte(0x1F7); // Read Status Register.
 
      // (III) Check For Errors:
      // -------------------------------------------------
      if (state & 0x01)
         return 2; // Error.
 
      // (IV) Check If Device fault:
      // -------------------------------------------------
      if (state & 0x20)
         return 1; // Device Fault.
 
      // (V) Check DRQ:
      // -------------------------------------------------
      // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
      if ((state & 0x08) == 0)
         return 3; // DRQ should be set
 
   }
 
   return 0; // No Error.
 
}

/* Read a single sector (512B) from disk */
void
readsect(void *dst, unsigned int offset) {
	int i;
	int err;
	/* Issue command */
	waitdisk();
	out_byte(0x1F6, (offset >> 24) | 0xE0 & 0xEF);
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F7, 0x20);
	/* Fetch data */
	if (err = ide_polling(1)) {
		serial_printc('0' + err);
	}
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}

/* Read "count" bytes at "offset" from binary into physical address "pa". */
void
readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	//int secsize;
	//secsize = fd_secsize();
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 18;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
	/*
	pa -= offset & secsize;
	offset = (offset / secsize) + 18;
	for (; pa < epa; pa += secsize, offset ++)
		fd_readsect(pa, offset);
	*/
}

#define SERIAL_PORT  0x3F8

void
init_serial(void) {
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x80);
	out_byte(SERIAL_PORT + 0, 0x01);
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x03);
	out_byte(SERIAL_PORT + 2, 0xC7);
	out_byte(SERIAL_PORT + 4, 0x0B);
}

static inline int
serial_idle(void) {
	return (in_byte(SERIAL_PORT + 5) & 0x20) != 0;
}

void
serial_printc(char ch) {
	while (!serial_idle());
	out_byte(SERIAL_PORT, ch);
	while (!serial_idle());
	out_byte(SERIAL_PORT, '\n');
}

// idt

#define INTERRUPT_GATE_32   0xE
#define TRAP_GATE_32        0xF
static GateDesc idt[NR_IRQ];

static void set_trap(GateDesc *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	ptr->offset_15_0 = offset & 0xFFFF;
	ptr->segment = selector;
	ptr->pad0 = 0;
	ptr->type = TRAP_GATE_32;
	ptr->system = 0;
	ptr->privilege_level = dpl;
	ptr->present = 1;
	ptr->offset_31_16 = (offset >> 16) & 0xFFFF;
}

void irq_empty();

void init_idt() {
	int i;
	for (i = 0; i < NR_IRQ; i ++) {
		set_trap(idt + i, 1 << 3, (uint32_t)irq_empty, 0);
	}

	write_idtr(idt, sizeof(idt));
}

// floppy
// msr: rqm dio ndma cmdbsy drv3-0bsy

typedef struct{
  unsigned char steprate_headunload;
  unsigned char headload_ndma;
  unsigned char motor_delay_off; /*specified in clock ticks*/
  unsigned char bytes_per_sector;
  unsigned char sectors_per_track;
  unsigned char gap_length;
  unsigned char data_length; /*used only when bytes per sector == 0*/
  unsigned char format_gap_length;
  unsigned char filler;
  unsigned char head_settle_time; /*specified in milliseconds*/
  unsigned char motor_start_time; /*specified in 1/8 seconds*/
}__attribute__ ((packed)) floppy_parameters;

#define FD_CMD_SPECIFY 0x03
#define FD_CMD_SENSE_INTR_STAT 0x08
#define FD_CMD_RECALIBRATE 0x07
#define FD_CMD_READ_SECTOR 0x66
#define FD_CMD_SEEK 0x0F

#define FD_REGBASE_PRI 0x03F0

#define FD_REGOFF_SRA 0x0
#define FD_REGOFF_SRB 0x1
#define FD_REGOFF_DOR 0x2
#define FD_REGOFF_TDR 0x3
#define FD_REGOFF_MSR 0x4
#define FD_REGOFF_DSR 0x4
#define FD_REGOFF_DAT 0x5
#define FD_REGOFF_DIR 0x7
#define FD_REGOFF_CCR 0x7

#define DISK_PARAMETER_ADDRESS 0x000fefc7

floppy_parameters floppy_disk;

void wait_floppy_data(int base){
/*     read main status register of controller and wait until it signals that
     data is ready in the data register*/

  while(((in_byte(base+FD_REGOFF_MSR))&0xd0)!=0xd0);
 return;
}

void wait_floppy_ready(int base, char firstcmd){
/*     read main status register of controller and wait until it signals that
     it is ready for write to data register*/
  unsigned char mask = firstcmd ? 0xd0 : 0xc0;
  while(((in_byte(base+FD_REGOFF_MSR)) & mask)!=0x80);
 return;
}

void write_floppy_command(int base,char command, char firstcmd){
   wait_floppy_ready(base, firstcmd);
   out_byte(base+FD_REGOFF_DAT,command);
}

char read_floppy_data(int base) {
   wait_floppy_data(base);
   return in_byte(base+FD_REGOFF_DAT);
}

void check_interrupt_status(int base,unsigned char *st0,unsigned char *cylinder){
             write_floppy_command(base,FD_CMD_SENSE_INTR_STAT, 1);
             wait_floppy_data(base);
             *st0=in_byte(base+FD_REGOFF_DAT);
             wait_floppy_data(base);
              *cylinder=in_byte(base+FD_REGOFF_DAT);
  return;
}

void configure_drive(int base,char drive){
  write_floppy_command(base,FD_CMD_SPECIFY, 1);/*config/specify command*/
  write_floppy_command(base,floppy_disk.steprate_headunload, 0);
  write_floppy_command(base,floppy_disk.headload_ndma | 1, 0); /* set ndma */
  return;
}

void select_fdd(int base, char drive) {
  switch (drive) {
    case 0:
      out_byte(base + FD_REGOFF_DOR, 0x1c);
      break;
    case 1:
      out_byte(base + FD_REGOFF_DOR, 0x2d);
      break;
    case 2:
      out_byte(base + FD_REGOFF_DOR, 0x4e);
      break;
    case 3:
      out_byte(base + FD_REGOFF_DOR, 0x8f);
      break;
  }
}

void calibrate_drive(int base,char drive){
  char st0;
  unsigned char cylinder;
  select_fdd(base, drive); /*turns the motor of drive ON*/
  write_floppy_command(base,FD_CMD_RECALIBRATE, 1); /*Calibrate drive*/
  write_floppy_command(base,drive, 0);
  check_interrupt_status(base,&st0,&cylinder); /*check interrupt status and
                                                store results in global variables
                                                st0 and cylinder*/
}

void reset_floppy_controller(int base,char drive){
  char st0;
  unsigned char cylinder;
  out_byte((base+FD_REGOFF_DOR),0x00); /*disable controller*/
  out_byte((base+FD_REGOFF_DOR),0x0c); /*enable controller*/
  check_interrupt_status(base,&st0,&cylinder);
  out_byte(base+FD_REGOFF_CCR,0);
  configure_drive(base,drive);
  calibrate_drive(base,drive);
}

void init_floppy(void) {
	_memcpy(&floppy_disk, (unsigned char *)DISK_PARAMETER_ADDRESS, sizeof(floppy_parameters));
	reset_floppy_controller(FD_REGBASE_PRI, 0);
}

void fd_seek(int base, char drive, unsigned char cylinder,  unsigned char head) {
	char st0;
	unsigned char real_cylinder;
	write_floppy_command(base, FD_CMD_SEEK, 1);
	write_floppy_command(base, drive | (head << 2), 0);
	write_floppy_command(base, cylinder, 0);
	check_interrupt_status(base, &st0, &real_cylinder);
	if (real_cylinder != cylinder) {
		serial_printc('L');
	}
}

inline int fd_secsize(void) {
	return 128 * (1 << floppy_disk.bytes_per_sector);
}

void fd_readsect(void *dst, unsigned int lba) {
	unsigned char cyl, head, sector, spt;
	int bps, i;
	spt = floppy_disk.sectors_per_track;
	cyl = lba / (2 * spt);
	head = ((lba % (2 * spt)) / spt);
	sector = ((lba % (2 * spt)) % spt + 1);
	fd_seek(FD_REGBASE_PRI, 0, cyl, head);
	serial_printc('A');

	write_floppy_command(FD_REGBASE_PRI, FD_CMD_READ_SECTOR, 1);
	serial_printc('a');
	write_floppy_command(FD_REGBASE_PRI, head << 2, 0);
	serial_printc('b');
	write_floppy_command(FD_REGBASE_PRI, cyl, 0);
	serial_printc('c');
	write_floppy_command(FD_REGBASE_PRI, head, 0);
serial_printc('d');
	write_floppy_command(FD_REGBASE_PRI, sector, 0);
serial_printc('e');
	write_floppy_command(FD_REGBASE_PRI, floppy_disk.bytes_per_sector, 0);
serial_printc('f');
	write_floppy_command(FD_REGBASE_PRI, spt, 0);
serial_printc('g');
	write_floppy_command(FD_REGBASE_PRI, floppy_disk.gap_length, 0);
serial_printc('h');
	write_floppy_command(FD_REGBASE_PRI, floppy_disk.data_length, 0);
	serial_printc('B');

	bps = fd_secsize();
	for (i = 0; i < bps; i++) {
		((char *)dst)[i] = read_floppy_data(FD_REGBASE_PRI);
	}

	for (i = 7; i > 0; i--) {
		read_floppy_data(FD_REGBASE_PRI);
	}
}