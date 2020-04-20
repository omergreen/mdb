/*
 * The generic breakpoint struct & functions
 */

#pragma once

#include <stdbool.h>
#include <machine/arch/breakpoint_arch_specific_struct.h>

enum breakpoint_type {
    BREAKPOINT_TYPE_JUMP, 
    BREAKPOINT_TYPE_SOFTWARE,
    BREAKPOINT_TYPE_HARDWARE,

    BREAKPOINT_TYPE_JUMP_OR_SOFTWARE // used to indicate that either a jump or a software breakpoint 
                                     // generally should be used as the default breakpoint type
};

struct breakpoint {
    unsigned int address;
    bool enabled;
    bool temporary;
    enum breakpoint_type type;
    struct breakpoint_arch_specific arch_specific;
};


// Public breakpoint interface

// Entry point from the arch specific breakpoint handler
void breakpoint_handler();

// Add a new breakpoint at `address`, without enabling it.
// `temporary` - temporary breakpoints will be removed the
//               next time the stub is reentered
bool breakpoint_add(unsigned int address, bool temporary, enum breakpoint_type type);

// Remove breakpoint at `address`
void breakpoint_remove(unsigned int address);

// Returns if a breakpoint is enabled at `address`
bool breakpoint_exists(unsigned int address);

// When we access the memory we want to read the original opcodes
// and not our modified ones (for example when gdb requests to read)
void breakpoint_disable_all_temporarily();
void breakpoint_restore_all_temporarily();

// Flip all breakpoints marked with flip_on_next_stop, erasing the disabled
void breakpoint_remove_temporay();

