/*
 * Implements the entry point
 */

#include <arch/interface.h>
#include <target/interface.h>
#include <libc/libc.h>
#include "gdbstub.h"
#include "state.h"
#include "core.h"

extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;
extern unsigned long __START_OF_PROGRAM;
extern unsigned long __START_OF_PROGRAM_MASK;

struct state g_state = { .state = STATE_CONTINUE, .previous_pc_breakpoint = 0 };

static void fix_got();

#ifdef ARCH_MIPS
// per abicalls, _start2 expects that t9 will point to the start of the function
asm(".global _start;\
     .section .init;\
     .set noreorder;\
_start:\
     move $t0, $ra;\
     bal 1f;\
     addiu $t9, $ra, 8;\
\
  1: b _start2;\
     move $ra, $t0;\
    ");
extern void *_start;
__attribute__((used,section(".init"))) void _start2(void *args) {
#else
__attribute__((used,section(".init"))) void _start(void *args) {
#endif
    fix_got();
    target_init(args);
    arch_init();
}

__attribute__((always_inline,section(".init"))) static void fix_got() {
    register unsigned long start;

#ifdef ARCH_MIPS
    asm volatile(".set noreorder; bal 1f; addiu %0, $ra, _start - . - 4; 1:" : "=r" (start) :: "ra");
#else
    start = (unsigned long)&_start; // stackoverflow.com/q/8398755
#endif

    unsigned long *got = (unsigned long *)(start + (unsigned long)&__GOT_OFFSET);
    for (unsigned int i = 0; i < (unsigned long)&__GOT_LENGTH / sizeof(*got); ++i) {
        // we can't use variables for 0x12300000 here since it would be stored in the GOT
        // TODO then: find some better solution
        if ((got[i] & 0xfff00000) == 0x12300000) {
            got[i] += start - 0x12300000;
        }
    }
}

void core_loop() {
    // first, remove all temporary breakpoints and restore the breakpoint
    // on the previous pc if needed
    breakpoint_remove_temporay();
    if (g_state.previous_pc_breakpoint != 0) {
        breakpoint_add(g_state.previous_pc_breakpoint, false, BREAKPOINT_TYPE_JUMP_OR_SOFTWARE);
        g_state.previous_pc_breakpoint = 0;
    }

    bool should_stop_on_next_pc = false;

    if (breakpoint_exists(g_state.regs.pc) || g_state.state == STATE_SINGLE_STEP) {
        // TODO: give control to target
        breakpoint_disable_all_temporarily();
        enum action action = gdbstub();
        breakpoint_restore_all_temporarily();

        switch (action) {
            case ACTION_CONTINUE:
                g_state.state = STATE_CONTINUE;
                break;
            case ACTION_SINGLE_STEP:
                should_stop_on_next_pc = true;
                g_state.state = STATE_SINGLE_STEP;
                break;
        }
    }

    if (breakpoint_exists(g_state.regs.pc)) {
        breakpoint_remove(g_state.regs.pc); // disable it temporarily
        g_state.previous_pc_breakpoint = g_state.regs.pc; // mark the current pc so we'll restore the breakpoint at our next stop
        should_stop_on_next_pc = true;
    }
    
    if (should_stop_on_next_pc) {
        breakpoint_disable_all_temporarily();
        pc_list next_pcs = arch_get_next_pc();
        breakpoint_restore_all_temporarily();

        if (cvector_empty(next_pcs)) {
            ERROR("we don't know what's the next pc");
        }

        for (CORE_ADDR *next_pc = cvector_begin(next_pcs); next_pc != cvector_end(next_pcs); next_pc++) {
            if (!breakpoint_exists(*next_pc)) {
                breakpoint_add(*next_pc, true, BREAKPOINT_TYPE_JUMP_OR_SOFTWARE); // add a temporary breakpoint
            }
        }

        cvector_free(next_pcs);
    }
}

