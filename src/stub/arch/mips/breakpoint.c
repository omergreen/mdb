/*
 * Implementation of the various breakpoint-related functions
 */

#include "breakpoint.h"
#include "registers.h"
/* #include "abort.h" */
#include <core/log.h>
#include <libc/libc.h>
#include <core/state.h>
#include "get_next_pc/get_next_pc.h"

void arm_breakpoint_handler(unsigned int address, struct registers_from_stub *regs) {
    registers_get_from_stub(&g_state.regs, regs, address);

    // pass control to the core
    breakpoint_handler();

    registers_update_to_stub(&g_state.regs, regs);
}

/*
 * Called directly from the breakpoint stub, this function parses the stack to get the register state
 * and then gives control to the core.
 */
__attribute__((noreturn)) static void jump_breakpoint_handler(unsigned int address, unsigned int sp) {
    DEBUG("breakpoint jumped from 0x%08x with sp 0x%08x", address, sp);

    arm_breakpoint_handler(address, (struct registers_from_stub *)sp);

    /* jump_breakpoint_epilogue(g_state.regs.pc, sp); */
}

bool arch_jump_breakpoint_enable(struct breakpoint *bp) {
    return false;
}

void arch_jump_breakpoint_disable(struct breakpoint *bp) {
    ERROR("arch_jump_breakpoint_disable called when jump breakpoints aren't supported\n");
}

void ivt_breakpoint_init_handler() {
    if (!g_target_config.should_override_ivt) {
        return;
    }

    /* install_abort_ivt_handlers(); */
}

static bool ivt_init = false;
bool arch_software_breakpoint_enable(struct breakpoint *bp) {
    if (!ivt_init) {
        ivt_breakpoint_init_handler();
        ivt_init = true;
    }

    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
    *(unsigned int *)bp->address = 0xe1200070; // bkpt
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);
    return true;
}

void arch_software_breakpoint_disable(struct breakpoint *bp) {
    memcpy((void *)bp->address, bp->arch_specific.original_data, sizeof(bp->arch_specific.original_data));
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);
}

bool arch_hardware_breakpoint_enable(struct breakpoint *bp) {
    return false;
}

void arch_hardware_breakpoint_disable(struct breakpoint *bp) {
}

