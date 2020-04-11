/*
 * Defines the state struct
 */

#pragma once

#include <machine/arch/registers_struct.h>
#include "breakpoint.h"
#include <libc/cvector.h>

enum state_enum {
    STATE_DEFAULT,
    STATE_REENABLE_BREAKPOINT, // in case of software breakpoints, if we want to keep the current bp enabled
                               // we need to put a temporary bp on the next opcode, and then reenable the original one
};

struct state {
    struct registers regs;
    cvector_vector_type(struct breakpoint) breakpoints;
    enum state_enum state;
};

extern struct state g_state;

