/*
 * Defines the state struct
 */

#pragma once

#include <machine/arch/registers_struct.h>
#include "breakpoint.h"
#include <libc/cvector.h>

enum state_enum {
    STATE_CONTINUE,
    STATE_SINGLE_STEP
};

struct state {
    struct registers regs;
    cvector_vector_type(struct breakpoint) breakpoints;
    enum state_enum state;
    unsigned int previous_pc_breakpoint;
};

extern struct state g_state;

