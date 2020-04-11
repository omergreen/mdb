#pragma once

#include "breakpoint.h"

/*
 * Return value for the GDB stub and (possibly) target specific stub, specify what to do next
 */
enum action {
    ACTION_CONTINUE,
    ACTION_SINGLE_STEP
};

/*
 * Entry point into the "core" of the debugger (for example, gdb interface)
 */
void core_loop(struct breakpoint *bp);

