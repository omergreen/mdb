#pragma once

#include <core/breakpoint.h>
#include "registers_struct.h"

struct registers_from_stub {
   unsigned int cpsr;
   unsigned int sp;
   unsigned int r0_to_r12[13];
   unsigned int lr;
};

enum registers_enum {
    REG_R0,
    REG_R1,
    REG_R2,
    REG_R3,
    REG_R4,
    REG_R5,
    REG_R6,
    REG_R7,
    REG_R8,
    REG_R9,
    REG_R10,
    REG_R11,
    REG_R12,
    REG_SP,
    REG_LR,
    REG_PC,
    REG_CPSR,
};

void registers_print_all(struct breakpoint *bp);
void registers_get_from_stub(struct registers *regs, struct registers_from_stub *regs_stub, unsigned int pc);
void registers_update_to_stub(struct registers *regs, struct registers_from_stub *regs_stub);

