/*
 * This file deals with the higher-level interrupt handling, after the low-level
 * stub in abort.S
 */

#include <arm_acle.h>
#include <core/state.h>
#include "breakpoint.h"
#include "registers.h"
#include "abort.h"
#include <stdbool.h>
#include <core/log.h>

bool g_abort_memory_test_active = false;
bool g_abort_memory_test_got_fault = false;

// TODO: restore vector table on arch_cleanup
void install_abort_ivt_handlers() {
    // TODO: consider moving the vector table somewhere else if it's read only
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

bool data_abort_handler(unsigned long *address) {
    unsigned long dfsr = __arm_mrc(15, 0, 5, 0, 0); // read dfsr

    if (!fsr_is_debug_event(dfsr)) {
        // did we get here while checking memory?
        if (g_abort_memory_test_active) {
            g_abort_memory_test_got_fault = true; // mark that the memory test failed and return without 
                                                  // passing control to core
            *address += 4; // skip the instruction
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

bool prefetch_abort_handler() {
    unsigned long ifsr = __arm_mrc(15, 0, 5, 0, 1); // read ifsr

    if (!fsr_is_debug_event(ifsr)) {
        // we got here because of some fault that wasn't ours
        // TODO: jump to original handler
        target_log("uncaught prefetch abort\n");
        while (1) ;
    }

    return true;
}

// this function is called from abort.S's abort_interrupt_handler.
// if needed it calls arm_breakpoint_handler to pass the control upwards to the core
unsigned long abort_handler(unsigned long address, unsigned long sp, bool is_prefetch_abort) {
    unsigned long ivt = determine_ivt();

    // prefetch_abort_handler and data_abort_handler return true if we should continue and propagate
    // the event upward, and false if we should just return now.
    // data_abort_handler can change address since when we test memory and we got data abort, we want
    // to skip the faulty instruction.
    if (is_prefetch_abort) {
        if (!prefetch_abort_handler()) {
            return address;
        }
    }
    else {
        if (!data_abort_handler(&address)) {
            return address;
        }
    }

    // TODO: deal with this higher up
    if (!breakpoint_exists(address)) {
        target_log("debug event without a breakpoint (is_prefetch_abort=%d, address=0x%08x)\n", is_prefetch_abort, address);
        return address;
    }

    arm_breakpoint_handler(address, (struct registers_from_stub *)sp);

    return g_state.regs.pc;
}

