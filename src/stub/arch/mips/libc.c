/*
 * Implements general functions related to the mips architecture
 */

#include <target/interface.h>
#include "breakpoint.h"
#include <core/log.h>
#include <libc/libc.h>
#include <stddef.h>
#include "libc.h"
#include "cp0.h"

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
    unsigned long config = MFC0(CP0_CONFIG);
    return ((config >> 10) & 7) == 0; // AR - bits 12:10, 0 is release 1, 1 is release 2-6
}

unsigned long get_ivt_base() {
    unsigned long status = MFC0(CP0_STATUS);
    unsigned long ebase = MFC0(CP0_EBASE);

    if (status & ST0_BEV) { // BEV == 1 -> bootstrap
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
    unsigned long ebase = MFC0(CP0_EBASE);
    if (ebase & EBASE_WG) {
        return true;
    }
    
    // try to write back ebase with WG on, and test if it changed
    ebase |= EBASE_WG;
    MTC0(CP0_EBASE, ebase);
    return MFC0(CP0_EBASE) & EBASE_WG;
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
    else if (MFC0(CP0_STATUS) & ST0_BEV) { // TODO: turn off BEV and move?
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

    unsigned long old_ebase = MFC0(CP0_EBASE);

    // if we needed to use the write gate then it's already enabled, so we only need to update the address
    MTC0(CP0_EBASE, (old_ebase & 0xfff) | new_addr);

    return old_ebase & ~0xfff;
}

bool test_for_bus_error(unsigned long address, bool write) {
    asm volatile("di; ehb");
    g_memory_test_active = true;
    g_memory_test_got_fault = false;

    unsigned char *ptr = (unsigned char *)address;
    unsigned char val = *ptr;
    if (write) {
        *ptr = val;
    }

    asm volatile("ei; ehb");

    return !g_memory_test_got_fault;    
}

bool arch_test_address(unsigned long address, bool write) {
    unsigned long asid = MFC0(CP0_ENTRYHI) & (MIPS_ENTRYHI_ASID | MIPS_ENTRYHI_ASIDX);
    MTC0(CP0_ENTRYHI, (address & 0xffffe000) | asid); // page / 2 + asid (asid is some kind of page identifier)
    asm volatile("ehb; tlbp; ehb"); // probe TLB for address
    unsigned long index = MFC0(CP0_INDEX); 

    if (address >= 0x80000000 && address < 0xc0000000) { // those addresses are always unmapped
        return test_for_bus_error(address, write);
    }

    if (index & 0x80000000) { // P bit
        // probe failure
        return false;
    }

    asm volatile("tlbr; ehb"); // read the found entry

    unsigned long entry;

    if (address & 0x1000) { // odd page
        entry = MFC0(CP0_ENTRYLO1);
    }
    else { // even page
        entry = MFC0(CP0_ENTRYLO0);
    }

    if (!(entry & ENTRYLO_V)) { // valid == false
        return false;
    }

    if (write && !(entry & ENTRYLO_D)) { // if we want to write and dirty == fakse
        return false;
    }

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

