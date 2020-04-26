/*
 * List of functions that implement the architecture interface.
 * arch_cache_flush may be overrided by target_cache_flush if needed (for example, in Linux)
 */

#pragma once

#include <core/breakpoint.h>
#include <stdbool.h>
#include <libc/cvector.h>
#include <machine/arch/get_next_pc/get_next_pc.h>

typedef unsigned long CORE_ADDR;

typedef cvector_vector_type(CORE_ADDR) pc_list;

bool arch_jump_breakpoint_enable(struct breakpoint *bp);
void arch_jump_breakpoint_disable(struct breakpoint *bp);
bool arch_software_breakpoint_enable(struct breakpoint *bp);
void arch_software_breakpoint_disable(struct breakpoint *bp);
bool arch_hardware_breakpoint_enable(struct breakpoint *bp);
void arch_hardware_breakpoint_disable(struct breakpoint *bp);

void arch_cache_flush(void *start, unsigned int length);

// test if `address` is readable, and if `write` is true then also if it's writeable
// technically an address can be writeable and not readable but... we don't want to override
// it with something we don't know during testing
bool arch_test_address(unsigned long address, bool write); 

void arch_init();
void arch_cleanup();

/*
 * Given the current set of registers, determine the list of possible next PC's
 */
pc_list arch_get_next_pc();

