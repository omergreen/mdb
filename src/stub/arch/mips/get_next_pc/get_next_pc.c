/*
 * This is our own interface to the GDB get_next_pc code, which is implemented in gdb_get_next_pc.*
 */

#include <libc/libc.h>
#include <arch/interface.h>
#include "arch/mips/get_next_pc/gdb_get_next_pc.h"
#include "get_next_pc.h"
#include <core/state.h>

pc_list arch_get_next_pc() {
    pc_list next_pcs = NULL;
    struct regcache regcache;
    struct gdbarch arch;
    regcache.regs = &g_state.regs;
    regcache.arch = &arch;
    CORE_ADDR next_pc = mips32_next_pc (&regcache, g_state.regs.pc);
    cvector_push_back(next_pcs, next_pc);

   return next_pcs;
}

