/*
 * Implements the entry point
 */

#include <arch/interface.h>
#include <target/interface.h>
#include "test_app.h"
#include <libc/libc.h>
#include "gdbstub.h"
#include "state.h"
#include "core.h"

extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;
extern unsigned long __START_OFFSET;

struct state g_state;

void fix_got();

__attribute__((used,section(".init"))) void _start(void *args) {
    fix_got();
    arch_init();
    target_init(args);

    unsigned int bp_address = (unsigned int)test_app;
    breakpoint_add(bp_address, USER);
    breakpoint_enable(bp_address);

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

void core_loop(struct breakpoint *bp) {
   enum action action = gdbstub();
   switch (action) {
       case ACTION_CONTINUE:
           DEBUG("got continue");
           break;
       case ACTION_SINGLE_STEP:
           DEBUG("got single step");
           break;
   }
}

