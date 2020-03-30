SECTIONS {
  . = 0x0;
  .note.gnu.build-id : {*(.note.gnu.build-id*) } /* gdb has a bug when .text (_start) begins on 0, so this hack fixes that by compensating later */
  .text : { *(.text) }
  .data : { *(.data*) *(.rodata*) *(.bss*) *(COMMON) }
  .got : {*(.got*)}
  __GOT_OFFSET = SIZEOF(.text) + SIZEOF(.data);
  __GOT_LENGTH = SIZEOF(.got);
  __START_OFFSET = SIZEOF(.note.gnu.build-id);
}
