ENTRY(multiboot_start)

SECTIONS {
	. = 0xC0100000;

	.boot :
	AT ( ADDR(.boot) - 0xC0000000 )
	{
		KEEP(*(.multiboot_header))
	}
	
	.text :
	AT ( ADDR(.text) - 0xC0000000 )
	{
		*(.text)
		*(.text.*)
	}

	.rodata :
	AT ( ADDR(.rodata) - 0xC0000000 )
	{
		*(.rodata)
		*(.rodata.*)
	}

	. = 0xC0110000;

	.data :
	AT ( ADDR(.data) - 0xC0000000 )
	{
		*(.data)
	}

	.bss :
	AT ( ADDR(.bss) - 0xC0000000 )
	{
		*(.bss)
	}
	/DISCARD/ :
	{
		*(.eh_frame)
	}
}
