SECTIONS {
    . = 0x12300000;
    HIDDEN(__START_OF_PROGRAM = .);
    .note.gnu.build-id : {*(.note.gnu.build-id*) } /* gdb has a bug when .text (_start) begins on 0, so this hack fixes that by compensating later */
    HIDDEN(__INIT_OFFSET = .);
    .init : { *(.init*) }
    .text : { *(.text*) }
    .data : { *(.data*) *(.rodata*) *(.bss*) *(COMMON) }

    . = ALIGN(16);
    HIDDEN(_gp = . + 0x7ff0);

    __JUST_BEFORE_GOT = .;

    .got : { *(.got*) }
    .sdata : { *(.sdata*) }

    .rel : { *(.rel*) }
    .dynamic : { *(.dynamic) }

    __GOT_OFFSET = __JUST_BEFORE_GOT - __INIT_OFFSET; /*SIZEOF(.init) + SIZEOF(.text) + SIZEOF(.data);*/
    __GOT_LENGTH = SIZEOF(.got);
    __START_OFFSET = __INIT_OFFSET - __START_OF_PROGRAM;

    /DISCARD/ : { *(.rld_map*) }
}

