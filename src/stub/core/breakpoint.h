/*
 * The generic breakpoint struct & functions
 */

#pragma once

#include <stdbool.h>
#include <machine/arch/breakpoint_arch_specific_struct.h>

struct breakpoint {
    unsigned int address;
    bool enabled;
    bool temporary;
    struct breakpoint_arch_specific arch_specific;
};


// Public breakpoint interface

// Entry point from the arch specific breakpoint handler
void breakpoint_handler();

// Add a new breakpoint at `address`, without enabling it
void breakpoint_add(unsigned int address, bool temporary);

// Remove breakpoint at `address`
void breakpoint_remove(unsigned int address);

// Returns if a breakpoint is enabled at `address`
bool breakpoint_exists(unsigned int address);

void breakpoint_disable_all_temporarily();
void breakpoint_restore_all_temporarily();

// Flip all breakpoints marked with flip_on_next_stop, erasing the disabled
void breakpoint_remove_temporay();

