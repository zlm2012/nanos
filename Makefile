CC      = gcc
LD      = ld
CFLAGS  = -m32 -std=gnu89 -static -ggdb -MD -Wall -Werror -I./include -O2 \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer
ASFLAGS = -m32 -MD -I./include
LDFLAGS = -melf_i386 --verbose
QEMU    = qemu-system-i386

CFILES  = $(shell find src/ -name "*.c")
SFILES  = $(shell find src/ -name "*.S")
OBJS    = $(CFILES:.c=.o) $(SFILES:.S=.o)

run: test.iso
	$(QEMU) -serial stdio -cdrom test.iso > serial_log

hdd: disk.img
	$(QEMU) -serial stdio disk.img

floppy: disk.img
	$(QEMU) -serial stdio -boot order=a -fda disk.img

debug: test.iso
	$(QEMU) -serial stdio -s -S -cdrom test.iso

debughdd: disk.img
	$(QEMU) -serial stdio -s -S -boot order=a -fda disk.img

test.iso: kernel
	@cp kernel floppy/boot/
	grub-mkrescue -o test.iso floppy/

disk.img: kernel
	@cd boot; make
	cat boot/bootsect boot/bootloader kernel > disk.img

kernel: $(OBJS)
	$(LD) $(LDFLAGS) -T linker.ld -o kernel $(OBJS)
#	$(LD) $(LDFLAGS) -e os_init -Ttext 0xC0100000 -o kernel $(OBJS)
	objdump -D kernel > code.txt	# disassemble result
	readelf -a kernel > elf.txt		# obtain more information about the executable

-include $(OBJS:.o=.d)

clean:
	@cd boot; make clean
	rm -f kernel disk.img $(OBJS) $(OBJS:.o=.d)
