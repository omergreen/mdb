#pragma once

#include <stdbool.h>
#include <core/breakpoint.h>

bool jump_breakpoint_put(struct breakpoint *bp);
bool jump_breakpoint_disable(struct breakpoint *bp);


// all defined in breakpoint_stub.S
#define JUMP_BREAKPOINT_STUB_SIZE ((unsigned int)&jump_breakpoint_stub_end - (unsigned int)&jump_breakpoint_stub)
#define JUMP_BREAKPOINT_STUB_BP_ADDRESS_OFFSET ((unsigned int)&jump_breakpoint_stub_bp_address - (unsigned int)&jump_breakpoint_stub)
#define JUMP_BREAKPOINT_STUB_HANDLER_FUNC_OFFSET ((unsigned int)&jump_breakpoint_stub_handler_func - (unsigned int)&jump_breakpoint_stub)
#define JUMP_BREAKPOINT_STUB_EPILOGUE_FUNC_OFFSET ((unsigned int)&jump_breakpoint_stub_epilogue_func - (unsigned int)&jump_breakpoint_stub)

extern void *jump_breakpoint_stub;
extern void *jump_breakpoint_stub_end;
extern void *jump_breakpoint_stub_bp_address;
extern void *jump_breakpoint_stub_handler_func;
extern void *jump_breakpoint_stub_epilogue_func;

