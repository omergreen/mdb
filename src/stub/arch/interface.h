/*
 * List of functions that implement the architecture interface.
 * arch_cache_flush may be overrided by target_cache_flush if needed (for example, in Linux)
 */

#pragma once

#include <core/breakpoint.h>
#include <stdbool.h>

bool arch_jump_breakpoint_enable(struct breakpoint *bp);
bool arch_jump_breakpoint_disable(struct breakpoint *bp);

void arch_cache_flush(void *start, unsigned int length);

void arch_init();
void arch_cleanup();

