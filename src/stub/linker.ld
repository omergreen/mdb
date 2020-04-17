SECTIONS {
    . = 0x12300000;
    .note.gnu.build-id : {*(.note.gnu.build-id*) } /* gdb has a bug when .text (_start) begins on 0, so this hack fixes that by compensating later */
    .init : { *(.init*) }
    .text : { *(.text*) }
    .data : { *(.data*) *(.rodata*) *(.bss*) *(COMMON) }
    .got : { *(.got*) }
    __GOT_OFFSET = SIZEOF(.init) + SIZEOF(.text) + SIZEOF(.data);
    __GOT_LENGTH = SIZEOF(.got);
    __START_OFFSET = SIZEOF(.note.gnu.build-id);

    .rel : { *(.rel*) }
    .dynamic : { *(.dynamic) }
}

