/*
 * Implements the entry point
 */

#include <arch/interface.h>
#include <target/interface.h>

extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;
extern unsigned long __START_OFFSET;

void fix_got();

void do_nabaz() {
    target_log("in a loop\n");

    int i = 0;

    while (1) {
        if ((i++ & (0x10000000-1)) == 0) {
            target_log("inside\n");
        }
    };
}

__attribute__((used,section(".init"))) void _start(void *args) {
    fix_got();
    arch_init();
    target_init(args);

    struct breakpoint bp;
    bp.address = (unsigned int)do_nabaz + 52;
    arch_jump_breakpoint_enable(&bp);

    do_nabaz();

    return;
}

void fix_got() {
    unsigned long start = (unsigned long)&_start;
    unsigned long *got = (unsigned long *)(start + (unsigned long)&__GOT_OFFSET); // stackoverflow.com/q/8398755
    for (unsigned int i = 0; i < ((unsigned long)&__GOT_LENGTH) / sizeof(*got); ++i) {
        if (i == 10)
            break;
        got[i] += start - (unsigned long)&__START_OFFSET;
    }
}

