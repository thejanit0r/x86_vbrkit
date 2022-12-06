OUTPUT_FORMAT("binary")
ENTRY(ldr_entrypoint)
SECTIONS
{
	. = 0x1000;

	.init :
	{
		*(.init)
	}

	.text :
	{
		*(.text)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(.bss)
	}

	/* 1 kB of stack memory */
	. = ALIGN(16);
	. += 0x600; 
 	pmode_stack = .;
	rmode_stack = 0x7000;
}
