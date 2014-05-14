CC      = gcc
LD      = ld
CFLAGS  = -m32 -static -ggdb -MD -Wall -Werror -I./include -O2 \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer
ASFLAGS = -m32 -MD -I./include
LDFLAGS = -melf_i386

CFILES  = $(shell find src/ -name "*.c")
SFILES  = $(shell find src/ -name "*.S")
OBJS    = $(CFILES:.c=.o) $(SFILES:.S=.o)

usr: $(OBJS)
	$(LD) $(LDFLAGS) -e _init -o usr $(OBJS)
	objdump -D usr > code.txt	# disassemble result
	readelf -a usr > elf.txt		# obtain more information about the executable

-include $(OBJS:.o=.d)

clean:
	rm -f usr $(OBJS) $(OBJS:.o=.d)
