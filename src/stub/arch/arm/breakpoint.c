/*
 * Implementation of the various breakpoint-related functions
 */

#include "breakpoint.h"
#include "registers.h"
#include "abort.h"
#include <core/log.h>
#include <libc/libc.h>
#include <core/state.h>
#include <arm_acle.h>
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

    jump_breakpoint_epilogue(g_state.regs.pc, sp);
}

static void fill_offset(void *stub, int offset, void *value) {
    *(void **)((unsigned int)(stub) + offset) = value;
}

/*
 * Replace the code at bp->address with a jump to jump_breakpoint_stub.
 * We allocate a stub for each breakpoint because we need some way to let ourselves know
 * which breakpoint was triggered - so for each stub we fill the bp_address with the breakpoint's address
 * jump_breakpoint_handler is called with that breakpoint and so we can pass the core the triggered breakpoint
 */
bool arch_jump_breakpoint_enable(struct breakpoint *bp) {
    void *stub = target_malloc(JUMP_BREAKPOINT_STUB_SIZE);
    if (stub == NULL) {
        ERROR("malloc for breakpoint stub returned NULL");
        return false;
    }

    unsigned int jump = build_jump(bp->address, (unsigned int)stub);
    if (jump == 0) {
        // if the stub is too far away from the breakpoint
        target_free(stub);
        return false;
    }

    memcpy(stub, &jump_breakpoint_stub, JUMP_BREAKPOINT_STUB_SIZE);
    // fill in the required stuff for the stub:
    // - bp_address so that the handler will know which breakpoint was triggered
    // - handler_func is technically static, but we can't fill it at compile time 
    //   due to PICness. We can initialize it once at init time instead of each
    //   time here, but does it really matter
    fill_offset(stub, JUMP_BREAKPOINT_STUB_BP_ADDRESS_OFFSET, (void *)bp->address);
    fill_offset(stub, JUMP_BREAKPOINT_STUB_HANDLER_FUNC_OFFSET, &jump_breakpoint_handler);

    cache_flush(stub, JUMP_BREAKPOINT_STUB_SIZE);

    bp->arch_specific.stub = stub; // save it for later so we could free it

    // TODO: add thumb support
    // finally, backup the code at bp->address, and replace it with the jump
    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
    *(unsigned int *)bp->address = jump;
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);

    return true;
}

/*
 * Replace the breakpoint at bp->addres with the original code
 */
void arch_jump_breakpoint_disable(struct breakpoint *bp) {
    if (bp->arch_specific.stub == NULL) {
        ERROR("disable on an already disabled breakpoint?");
    }
    else {
        target_free(bp->arch_specific.stub);
        bp->arch_specific.stub = NULL;
    }

    memcpy((void *)bp->address, bp->arch_specific.original_data, BREAKPOINT_LENGTH);
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);
}

// init the IVT abort handler if it's our first time
void ivt_breakpoint_init_handler() {
    if (!g_target_config.should_override_ivt) { // maybe this target works differently? allow for per-target override
        return;
    }

    install_abort_ivt_handlers();
}

static bool ivt_init = false;
bool arch_software_breakpoint_enable(struct breakpoint *bp) {
    if (!ivt_init) {
        ivt_breakpoint_init_handler();
        ivt_init = true;
    }

    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
#ifdef TARGET_TYPE_LINUX
    *(unsigned int *)bp->address = 0xffffffff; // qemu linux doesn't really work well with SIGTRAP (at least for arm) 
                                               // so we use SIGILL instead for debugging
#else
    *(unsigned int *)bp->address = convert_code_data_32(0xe1200070); // bkpt
#endif
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

