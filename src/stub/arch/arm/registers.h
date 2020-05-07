/*
 * Functions dealing with the ARM registers (print all registers to log, get next PC, etc)
 */

#pragma once

#include <core/breakpoint.h>
#include "registers_struct.h"

// TODO: rearrange stubs so we don't need registers_from_stub, like in mips

struct registers_from_stub {
   unsigned int cpsr;
   unsigned int sp;
   unsigned int r0_to_r12[13];
   unsigned int lr;
};

void registers_print_all();
void registers_get_from_stub(struct registers *regs, struct registers_from_stub *regs_stub, unsigned int pc);
void registers_update_to_stub(struct registers *regs, struct registers_from_stub *regs_stub);

