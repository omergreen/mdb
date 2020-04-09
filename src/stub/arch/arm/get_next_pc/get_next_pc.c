#include "get_next_pc.h"
#include <libc/libc.h>
#include <arch/interface.h>

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

// TODO: should use state
pc_list arch_get_next_pc(struct breakpoint *bp) {
    // we start by disabling all the breakpoints so they won't interfere when we try to read the memory
    // TODO: fix this for later
    arch_jump_breakpoint_disable(bp);

   struct arm_get_next_pcs_ops ops = {.read_mem_uint=read_mem_uint, .syscall_next_pc=syscall_next_pc, .addr_bits_remove=addr_bits_remove, .fixup=NULL};
   struct arm_get_next_pcs self;

   arm_get_next_pcs_ctor(&self, &ops, DATA_ENDIAN, CODE_ENDIAN, 0, &bp->arch_specific.regs);
   pc_list next_pcs = arm_get_next_pcs(&self, bp->arch_specific.regs.cpsr.bits.thumb);

    // TODO: fix this for later
   arch_jump_breakpoint_enable(bp);
   return next_pcs;
}

