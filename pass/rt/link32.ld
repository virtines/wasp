ENTRY(_start)

OUTPUT_FORMAT("elf32-i386", "elf32-i386",
              "elf32-i386")
OUTPUT_ARCH(i386)

SECTIONS
{
  . = 0x1000;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}
