SECTIONS {
  . = 0x0;
  .text : { *(.text) }
  .data : { *(.data) *(.data.rel.local) }
  .bss : { *(.bss) }
}
