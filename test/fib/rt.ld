ENTRY(_start)

OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64",
              "elf64-x86-64")

SECTIONS
{
  . = 0x8000;
	__stack_base = .;
  .text : { *(.text) }
  .data : { *(.data) }
  .paging : ALIGN(4096) { *(.paging) }

	.rodata : {
		__init_array_start = .;
        KEEP (*(.ctors))
		KEEP (*(.init_array*))
		__init_array_end = .;
		__fini_array_start = .;
        KEEP (*(.dtors))
		KEEP (*(.fini_array*))
		__fini_array_end = .;
      *(.rodata)
    }

  .bss : { *(.bss) }




	. = ALIGN(4096);
	__heap_start = .;
}
