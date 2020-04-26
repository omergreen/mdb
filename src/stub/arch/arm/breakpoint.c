/*
 * Implementation of the various breakpoint-related functions
 */

#include "breakpoint.h"
#include "registers.h"
#include <core/log.h>
#include <libc/libc.h>
#include <core/state.h>
#include <arm_acle.h>
#include "get_next_pc/get_next_pc.h"

/*
 * Called directly from the breakpoint stub, this function parses the stack to get the register state
 * and then gives control to the core.
 */
__attribute__((noreturn)) static void jump_breakpoint_handler(unsigned int address, unsigned int sp) {
    DEBUG("breakpoint jumped from 0x%08x with sp 0x%08x", address, sp);

    struct registers_from_stub *regs = (struct registers_from_stub *)sp;
    registers_get_from_stub(&g_state.regs, (struct registers_from_stub *)sp, address);

    // give control to the core
    breakpoint_handler();

    registers_update_to_stub(&g_state.regs, regs);
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


void ivt_breakpoint_init_handler() {
    if (!g_target_config.should_override_ivt) {
        return;
    }

    // TODO: consider moving the vector table somewhere else if it's read only
    //
    unsigned long *ivt = (unsigned long *)determine_ivt();

    // we modify both the relevant vector (prefetch abort occurs on BKPT or debug registers)
    // and another one which is unused in all modes except hypervisor, to allow far jumps
    // also, we modify the data abort vector and the reset vector to allow us to catch data aborts
    // as well (for example, for making safe memcpys)
    ivt[0] = (unsigned long)&data_abort_interrupt_handler; // reset vector - hopefully no one will need this?
    ivt[5] = (unsigned long)&prefetch_abort_interrupt_handler; // not used

    ivt[3] = convert_code_data_32(0xe59ff000); // prefetch abort - ldr pc, [pc]
    ivt[4] = convert_code_data_32(0xe51ff018); // data abort - ldr pc, [pc-24]
}

static bool ivt_init = false;
bool arch_software_breakpoint_enable(struct breakpoint *bp) {
    if (!ivt_init) {
        ivt_breakpoint_init_handler();
        ivt_init = true;
    }

    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
#ifdef TARGET_TYPE_LINUX
    *(unsigned int *)bp->address = 0xffffffff; // qemu linux doesn't really work well with SIGTRAP (at least for arm) so we use SIGILL instead for debugging
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


// check that the FSR indicates a debug event
bool fsr_is_debug_event(unsigned long fsr) {
    if (fsr & (1 << 9)) { // long-description translation table
        int fault_status = fsr & 0x3f; // bits 5-0

        return fault_status == 0b100010; // debug event
    }
    else {
        int fault_status = ((fsr & (1 << 10)) >> 6) | (fsr & 0xf); // bit 10 + bit 3-0

        return fault_status == 0b00010; // debug event
    }
}

bool data_abort_handler(bool *skip_handling_abort) {
    unsigned long dfsr = __arm_mrc(15, 0, 5, 0, 0); // read dfsr

    if (!fsr_is_debug_event(dfsr)) { // validate_dfsr
        // did we get here while checking memory?
        if (false) {
            return false;
        }
        else {
            // we got here because of some fault that wasn't ours
            // TODO: jump to original handler
            target_log("uncaught data abort");
            while (1) ;
        }
    }

    return true;
}

bool prefetch_abort_handler(bool *skip_handling_abort) {
    unsigned long ifsr = __arm_mrc(15, 0, 5, 0, 1); // read ifsr

    if (!fsr_is_debug_event(ifsr)) { // validate_ifsr
        // we got here because of some fault that wasn't ours
        // TODO: jump to original handler
        target_log("uncaught prefetch abort\n");
        while (1) ;
    }

    return true;
}

unsigned long abort_handler(unsigned long address, unsigned long sp, bool is_prefetch_abort) {
    unsigned long ivt = determine_ivt();

    bool skip_handling_abort = false;
    if (is_prefetch_abort) {
        if (!prefetch_abort_handler(&skip_handling_abort)) {
            return address;
        }
    }
    else {
        if (!data_abort_handler(&skip_handling_abort)) {
            return address;
        }
    }

    if (!breakpoint_exists(address)) {
        target_log("debug event without a breakpoint (is_prefetch_abort=%d, address=0x%08x)\n", is_prefetch_abort, address);
        return address;
    }

    struct registers_from_stub *regs = (struct registers_from_stub *)sp;
    registers_get_from_stub(&g_state.regs, (struct registers_from_stub *)sp, address);

    // pass control to the core
    breakpoint_handler();

    registers_update_to_stub(&g_state.regs, regs);

    return g_state.regs.pc;
}

