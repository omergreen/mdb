#pragma once

#include <core/breakpoint.h>
#include <stdbool.h>

enum registers_enum;

bool arch_jump_breakpoint_put(struct breakpoint *bp);
bool arch_jump_breakpoint_disable(struct breakpoint *bp);
void arch_cache_flush(void *start, unsigned int length);
unsigned long (*reg_read)(enum registers_enum reg);
void (*reg_write)(enum registers_enum reg, unsigned long value);

void arch_init();
void arch_cleanup();

