/*
 * Implements general functions related to the ARM architecture
 */

#include <target/interface.h>
#include <core/log.h>
#include <libc/libc.h>
#include <stddef.h>
#include "libc.h"

#define MFC0(specifier) ({ int out; asm volatile("mfc0 %0, $" specifier : "=r"(out)); out; })
#define MTC0(specifier, val) asm volatile("mtc0 %0, $" specifier :: "r"(val));
#define C0_EBASE "15, 1"
#define C0_STATUS "12, 0"
#define C0_CONFIG "16, 1"
#define EBASE_WG (1 << 11)
#define STATUS_BEV (1 << 22)

unsigned int build_jump(unsigned int from, unsigned int to) {
    unsigned int jump = 0xea000000 + (((to - (from + 8)) / 4) & 0xffffff);
    return jump;
}

int raise(int sig) { // needed for the ARM div functions
    target_log("sig %d was raised\n", sig);
    asm(".word 0xffffffff");
    return 0;
}

bool is_release1() {
    unsigned long config = MFC0(C0_CONFIG);
    return ((config >> 10) & 7) == 0; // AR - bits 12:10, 0 is release 1, 1 is release 2-6
}

unsigned long get_ivt_base() {
    unsigned long status = MFC0(C0_STATUS);
    unsigned long ebase = MFC0(C0_EBASE);

    if (status & STATUS_BEV) { // BEV == 1 -> bootstrap
        return 0xbfc00200;
    }
    else { // BEV == 0 - we take bits 31-12 of EBase
        if (is_release1()) {
            return 0x80000000;
        }
        else {
            return ebase & 0xfffff000;
        }
    }
}

bool is_ebase_writegate_enabled() {
    unsigned long ebase = MFC0(C0_EBASE);
    if (ebase & EBASE_WG) {
        return true;
    }
    
    // try to write back ebase with WG on, and test if it changed
    ebase |= EBASE_WG;
    MTC0(C0_EBASE, ebase);
    return MFC0(C0_EBASE) & EBASE_WG;
}

unsigned long move_ivt(unsigned long new_addr) {
    unsigned long ivt_base = get_ivt_base();
    char *error_reason = NULL;

    // can we move the ivt to the new addr?
    //
    // TODO: what the hell does this mean
    /*
     * "In Release 2 of the Architecture (and subsequent releases), software must guarantee that EBase15..12 contains zeros in
     * all bit positions less than or equal to the most significant bit in the vector offset. This situation can only occur when a
     * vector offset greater than 0xFFF is generated when an interrupt occurs with VI or EIC interrupt mode enabled. The
     * operation of the processor is UNDEFINED if this condition is not met."
     */
    if (is_release1()) {
        error_reason = "version is release 1";
    }
    else if (MFC0(C0_STATUS) & STATUS_BEV) { // TODO: turn off BEV and move?
        error_reason = "status.BEV is 1";
    }
    else if ((new_addr & 0xfff) != 0) {
        error_reason = "new_addr isn't a multiply of 0x1000";
    }
    else if (!is_ebase_writegate_enabled() && (new_addr & 0xc0000000) != 0) { // we can move the IVT to those places only with write gate
        error_reason = "write gate isn't enabled and new_addr isn't in kseg0";
    }
    if (error_reason != NULL) {
        ERROR("unable to move IVT to 0x%08x - %s", new_addr, error_reason);
        assert(0);
        return 0;
    }

    unsigned long old_ebase = MFC0(C0_EBASE);

    // if we needed to use the write gate then it's already enabled, so we only need to update the address
    MTC0(C0_EBASE, (old_ebase & 0xfff) | new_addr);

    return old_ebase & ~0xfff;
}

void create_and_move_ivt(unsigned long addr, void *breakpoint_interrupt_handler) {
    unsigned long old_ivt = MFC0(C0_EBASE);

    // TLB refill at 0, cache error on 0x100, general exception on 0x180, interrupt on 0x200
    // we store old_addr - (new_addr + 12) in a word after hop_back_opcodes so that we will jump
    // to the exact same spot we would be in if we didn't switch the IVT - since stuff like cache error
    // jumps to kseg1 instead of kseg0 (0xa0000000 instead of 0x80000000)
    unsigned int hop_back_opcodes[] = { 
                                        0x03e0d025, // move k0, ra - backup ra since we modify it
                                        0x04110001, // bal 0xc
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
                                                 0x401a6800, // mfc0 k0, c0_cause
                                                 0x001ad082, // srl k0, k0, 0x2
                                                 0x335a001f, // andi k0, k0, 0x1f
                                                 0x241b0009, // li k1, 9 (breakpoint)
                                                 0x175b0006, // bne k0, k1, not_bp
                                                 0x03e0d025, // move k0, ra
                                                 0x04110001, // bal 1f
                                                 0x8ffb0008, // lw k1, 8(ra)
                                                 0x03600008, // 1: jr k1
                                                 0x0340f825, // move ra, k0
                                                             // .word stub_address
                                                             // not_bp:
                                               };

    // TLB Refill at 0, cache error at 0x100, general exception at 0x180, interrupts at 0x200-0x8000
    // the number of interrupt vectors depends on IntCTL_VS, but we prepare for the worst
    for (int offset = 0; offset <= 0x8000; offset += 0x20) {
        unsigned long hop_back_address = addr + offset;
        int hop_back_offset = 12; // the bal is the second instruction -> ra = start + 12

        if (offset == 0x180) {  // general exception
            memcpy((void *)hop_back_address, general_exception_opcodes, sizeof(general_exception_opcodes));
            *(unsigned long *)(hop_back_address + sizeof(general_exception_opcodes)) = (unsigned long)breakpoint_interrupt_handler;
            hop_back_address += sizeof(general_exception_opcodes) + sizeof(unsigned int);
            hop_back_offset += sizeof(general_exception_opcodes) + sizeof(unsigned int);
        }

        if (offset == 0x80) continue; // nothing there
        if (offset < 0x200 && (offset & 0x60) != 0) continue; // below 0x200 there's a handler every 0x80 

        memcpy((void *)hop_back_address, hop_back_opcodes, sizeof(hop_back_opcodes));
        *(unsigned int *)(hop_back_address + sizeof(hop_back_opcodes)) = old_ivt - (addr + hop_back_offset);
    }

    cache_flush((void *)addr, 0x8020);
    move_ivt(addr);
}

bool arch_test_address(unsigned long address, bool write) {
    return true;
}

void arch_cache_flush(void *start, unsigned int length) {
    /* unsigned long config1; */
    /* asm volatile ("mfc0 %0, 16, 1" :: "r" (config1)); */

    /* int i_cache_size = (config1 >> 19) & 0b111; // bits 21:19 */

    /* if (i_cache_size == 0) { */
    /*     // no i-cache */
    /*     return; */
    /* } */
    /* else if (i_cache_size == 7) { */
    /*     ERROR("got reserved i_cache?"); */
    /*     return; */
    /* } */

    /* i_cache_size = 2 << i_cache_size; // i_cache_size=1 means 4 bytes */

    for (void *address = start; address < start + length; address++) {
        __builtin_mips_cache(0x11, address); // hit invalidate I
    }
}

