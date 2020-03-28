SECTIONS {
  . = 0x0;
  .text : { *(.text) }
  .data : { *(.data*) *(.rodata*) *(.bss*) *(COMMON) }
  .got : {*(.got*)}
  __GOT_OFFSET = SIZEOF(.text) + SIZEOF(.data);
  __GOT_LENGTH = SIZEOF(.got);
}
