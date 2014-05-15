bootblock: start.S boot.S main.c asm.h boot.h
	gcc -c -m32 start.S
	ld -e start -Ttext 0x7c00 -m elf_i386 start.o
	objcopy -S -O binary -j .text a.out bootsect
	gcc -c -m32 boot.S
	gcc -c -m32 -O1 main.c
	ld -e boot -Ttext 0x9000 -m elf_i386 boot.o main.o
	objcopy -S -O binary -j .text a.out bootloader
	perl genboot.pl bootsect bootloader

clean:
	rm -rf bootblock a.out *.o
