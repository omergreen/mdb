SECTIONS {
    __START_OF_PROGRAM = 0x12300000;
    __START_OF_PROGRAM_MASK = 0xfff00000;

    . = __START_OF_PROGRAM; /* some random value that we will be able to check for when fixing GOT */
    .init : { *(.init*) }
    .text : { *(.text*) }
    .data : { *(.data*) *(.rodata*) *(.bss*) *(COMMON) *(.sdata*) }

    . = ALIGN(16);
    HIDDEN(_gp = . + 0x7ff0); /* required for mips */

    HIDDEN(__JUST_BEFORE_GOT = .);
    .got : { *(.got*) }

    __GOT_OFFSET = __JUST_BEFORE_GOT - __START_OF_PROGRAM;
    __GOT_LENGTH = SIZEOF(.got);

    /DISCARD/ : { *(.rld_map*) *(.note.gnu.build-id*) *(.rel*) *(.dynamic*) *(.reginfo*) *(.MIPS.abiflags*) *(.dynsym*) *(.dynstr*) *(.hash*) }
}

