/*
 * Implements the entry point
 */

#include <arch/interface.h>
#include <target/interface.h>
#include "test_app.h"
#include <libc/libc.h>

extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;
extern unsigned long __START_OFFSET;

void fix_got();

__attribute__((used,section(".init"))) void _start(void *args) {
    fix_got();
    arch_init();
    target_init(args);

    struct breakpoint bp;
    bp.address = (unsigned int)test_app;// + 52;
    arch_jump_breakpoint_enable(&bp);

    test_app();

    return;
}

void fix_got() {
    unsigned long start = (unsigned long)&_start;
    unsigned long *got = (unsigned long *)(start + (unsigned long)&__GOT_OFFSET); // stackoverflow.com/q/8398755
    for (unsigned int i = 0; i < ((unsigned long)&__GOT_LENGTH) / sizeof(*got); ++i) {
        got[i] += start - (unsigned long)&__START_OFFSET;
    }
}

