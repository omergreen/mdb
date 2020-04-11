/*
 * The generic breakpoint struct & functions
 */

#pragma once

#include <stdbool.h>
#include <machine/arch/breakpoint_arch_specific_struct.h>

enum breakpoint_type {
    USER,
    TEMPORARY // we remove any breakpoint marked as "temporary" on the next time we're up
};

struct breakpoint {
    unsigned int address;
    bool enabled;
    enum breakpoint_type type;
    struct breakpoint_arch_specific arch_specific;
};


// Public breakpoint interface

// Entry point from the arch specific breakpoint handler
void breakpoint_handler(unsigned int address);

// Add a new breakpoint, without enabling it
void breakpoint_add(unsigned int address, enum breakpoint_type type);

// Enable an existing disabled breakpoint
void breakpoint_enable(unsigned int address);

// Disable an existing enabled breakpoint
void breakpoint_disable(unsigned int address);


