/*
 * This is our own interface to the GDB get_next_pc code, which is implemented in gdb_get_next_pc.*
 */

#include <libc/libc.h>
#include <arch/interface.h>
#include "get_next_pc.h"
#include <core/state.h>

static ULONGEST read_mem_uint(CORE_ADDR memaddr, int len, int byte_order) {
    unsigned short val16;
    unsigned int val32;

    switch (len) {
        case 1:
            return *(unsigned char *)memaddr;
            break;
        case 2:
            val16 = *(unsigned short *)memaddr;
            if (byte_order == CODE_ENDIAN) { 
                return convert_code_data_16(val16);
            } 
            else {
                return val16;
            }
        case 4:
            val32 = *(unsigned int *)memaddr;
            if (byte_order == CODE_ENDIAN) {
                return convert_code_data_32(val32);
            } 
            else {
                return val32;
            }
        default:
            assert("bad length");
            return 0;
    }
}

static CORE_ADDR syscall_next_pc(struct arm_get_next_pcs *self) {
    return self->regs->pc + 4;
}

static CORE_ADDR addr_bits_remove(struct arm_get_next_pcs *self, CORE_ADDR val) {
    return UNMAKE_THUMB_ADDR(val);
}

pc_list arch_get_next_pc() {
   struct arm_get_next_pcs_ops ops = {.read_mem_uint=read_mem_uint, .syscall_next_pc=syscall_next_pc, .addr_bits_remove=addr_bits_remove, .fixup=NULL};
   struct arm_get_next_pcs self;

   arm_get_next_pcs_ctor(&self, &ops, DATA_ENDIAN, CODE_ENDIAN, 0, &g_state.regs);
   pc_list next_pcs = arm_get_next_pcs(&self, g_state.regs.cpsr.bits.thumb);

   return next_pcs;
}

