/*
 * Inner functions required for ARM breakpoints
 * jump_breakpoint_stub* are implemented in assembly in breakpoint_stub.S
 */

#pragma once

#include <stdbool.h>
#include <core/breakpoint.h>
#include "registers.h"

// all defined in breakpoint_stub.S
#define JUMP_BREAKPOINT_STUB_SIZE ((unsigned int)&jump_breakpoint_stub_end - (unsigned int)&jump_breakpoint_stub)
#define JUMP_BREAKPOINT_STUB_BP_ADDRESS_OFFSET ((unsigned int)&jump_breakpoint_stub_bp_address - (unsigned int)&jump_breakpoint_stub)
#define JUMP_BREAKPOINT_STUB_HANDLER_FUNC_OFFSET ((unsigned int)&jump_breakpoint_stub_handler_func - (unsigned int)&jump_breakpoint_stub)

extern void *jump_breakpoint_stub;
extern void *jump_breakpoint_stub_end;
extern void *jump_breakpoint_stub_bp_address;
extern void *jump_breakpoint_stub_handler_func;


/*
 * Restore the CPU state and jump back to the original code
 */
extern __attribute__((noreturn)) void (jump_breakpoint_epilogue)(unsigned long return_address, unsigned long sp);

/*
 * Generic function that parses the registers and propagates execution to the
 * core. `address` should contain the address of the instruction that was about
 * to be executed, and `regs` should point to a registers_from_stub struct containing
 * the register state (R0-R12, LR, SP, CPSR)
 */
void arm_breakpoint_handler(unsigned int address, struct registers_from_stub *regs);

