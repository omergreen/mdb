/*
 * This is our own interface to the GDB get_next_pc code, which is implemented in gdb_get_next_pc.*
 */

#pragma once

#include <libc/cvector.h>
#include "gdb_get_next_pc.h"

typedef cvector_vector_type(CORE_ADDR) pc_list;

/*
 * Given the current set of registers, determine the list of possible next PC's
 */
pc_list arch_get_next_pc(struct breakpoint *bp);

