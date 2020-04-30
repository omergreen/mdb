/*
 * Implementation of the init and cleanup functions for MIPS
 */

#include "libc.h"
#include <target/interface.h>
#include "breakpoint.h"

void arch_init() {
    unsigned long addr = (unsigned long)target_malloc(0x2300);
    create_and_move_ivt((addr + 0x1000) & ~0xfff);
}

void arch_cleanup() {
}

