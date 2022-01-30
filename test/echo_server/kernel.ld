ENTRY(_start)

SECTIONS
{

  . = 0x4000;

	low_kern_start = .;

	.boot : {
		*(.boot*)
		. = ALIGN(4096);
	}


	.text : {
		*(.text*)
		*(.gnu.linkonce.t*)
	}


	.bss : {
		*(.bss)
	}
	/* The data segment */
	.data : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.data)
	}


	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}
