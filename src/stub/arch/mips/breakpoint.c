/*
 * Implementation of the various breakpoint-related functions
 */

#include "breakpoint.h"
#include <core/breakpoint.h>
#include "registers.h"
/* #include "abort.h" */
#include <core/log.h>
#include "cp0.h"
#include <libc/libc.h>
#include <core/state.h>
#include "get_next_pc/get_next_pc.h"

bool g_memory_test_active = false;
bool g_memory_test_got_fault = false;

bool init_general_exception_handler();

bool arch_jump_breakpoint_enable(struct breakpoint *bp) {
    return false;
}

void arch_jump_breakpoint_disable(struct breakpoint *bp) {
    ERROR("arch_jump_breakpoint_disable called when jump breakpoints aren't supported\n");
}

static bool ivt_init = false;
bool arch_software_breakpoint_enable(struct breakpoint *bp) {
    if (!ivt_init) {
        init_general_exception_handler();
        ivt_init = true;
    }

    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
#ifdef TARGET_TYPE_LINUX
    *(unsigned int *)bp->address = 0xf0000000; // some illegal instruction, gdb doesn't like us using break
#else
    *(unsigned int *)bp->address = 0x0000000d; // break
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

// high-level handler for general exceptions
// returns whether we have handled the exception ourselves - if false
// is returned, we expect that we'll let the original IVT handle it
bool general_exception_handler_high(struct registers *regs) {
    int exccode = (regs->cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;

    if (exccode == EXCCODE_DBE) { // data bus error
        if (g_memory_test_active) {
            if (regs->cause & CAUSEF_BD) {
                ERROR("memory test got bus error in delay slot");
                return false; // let original handle it
            }

            g_memory_test_got_fault = true;
            regs->pc += 4;
            return true; // we handled it
        }
    
        return false; // let original handle it
    }
    else if (exccode == EXCCODE_BP) { // breakpoint
        memcpy(&g_state.regs, regs, sizeof(*regs));
        breakpoint_handler();
        return true; // we handled it
    }
    else {
        ERROR("we shouldn't have got here");
        return false;
    }
}

void create_and_move_ivt(unsigned long addr) {
    unsigned long old_ivt = MFC0(CP0_EBASE);

    // TLB refill at 0, cache error on 0x100, general exception on 0x180, interrupt on 0x200
    // we store old_addr - (new_addr + 12) in a word after hop_back_opcodes so that we will jump
    // to the exact same spot we would be in if we didn't switch the IVT - since stuff like cache error
    // jumps to kseg1 instead of kseg0 (0xa0000000 instead of 0x80000000)
    unsigned int hop_back_opcodes[] = { 
                                        0x03e0d025, // move k0, ra - backup ra since we modify it
                                        0x04110001, // bal . + 8
                                        0x8ffb000c, // lw k1, 12(ra)
                                        0x037fd821, // addu k1, k1, ra
                                        0x03600008, // jr k1
                                        0x0340f825, // move ra, k0
                                        // .word old_addr - (new_addr + 12)
                                      };
    assert(sizeof(hop_back_opcodes) + sizeof(unsigned int) <= 0x20);

    // for general exception, we first want to check if a breakpoint triggered, and if so jump
    // to our stub. otherwise, fall back to hop_back_opcodes
    unsigned int general_exception_opcodes[] = { 
                                                 0x03e0d025, // move k0, ra - backup ra since we modify it
                                                 0x04110001, // bal . + 8
                                                 0x8ffb0008, // lw k1, 8(ra)
                                                 0x03600008, // jr k1
                                                 0x0340f825, // move ra, k0
                                                 (unsigned int)&general_exception_handler_low,
                                               };

    // TLB Refill at 0, cache error at 0x100, general exception at 0x180, interrupts at 0x200-0x8000
    // the number of interrupt vectors depends on IntCTL_VS, but we prepare for the worst
    for (int offset = 0; offset <= 0x8000; offset += 0x20) {
        unsigned long hop_back_address = addr + offset;
        int hop_back_offset = 12; // the bal is the second instruction -> ra = start + 12

        if (offset == 0x180) {  // general exception
            memcpy((void *)hop_back_address, general_exception_opcodes, sizeof(general_exception_opcodes));
            *&general_exception_original_ivt_handler = old_ivt + 0x180;
            continue;
        }

        if (offset == 0x80) continue; // nothing there
        if (offset < 0x200 && (offset & 0x60) != 0) continue; // below 0x200 there's a handler every 0x80 

        memcpy((void *)hop_back_address, hop_back_opcodes, sizeof(hop_back_opcodes));
        *(unsigned int *)(hop_back_address + sizeof(hop_back_opcodes)) = old_ivt - (addr + hop_back_offset);
    }

    cache_flush((void *)addr, 0x8020);
    move_ivt(addr);
}


bool init_general_exception_handler() {
    if (!g_target_config.should_override_ivt) {
        return true;
    }

    unsigned long addr = (unsigned long)target_malloc(0x2300 + 0x8000);
    void *stack = target_malloc(0x4000);
    if (addr == 0) {
        ERROR("unable to malloc IVT for general exceptions");
        return false;
    }
    if (stack == NULL) {
        ERROR("unable to allocate stack for general exceptions");
        return false;
    }

    *&general_exception_stack = (unsigned long)stack;
    create_and_move_ivt((addr + 0x1000) & ~0xfff); // addr needs to be aligned to a page, and of at least 0x8020 bytes

    return true;
}

