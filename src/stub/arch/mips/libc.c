/*
 * Implements general functions related to the ARM architecture
 */

#include <target/interface.h>
#include <core/log.h>
#include "libc.h"

unsigned int build_jump(unsigned int from, unsigned int to) {
    unsigned int jump = 0xea000000 + (((to - (from + 8)) / 4) & 0xffffff);
    return jump;
}

void raise(int sig) { // needed for the ARM div functions
    target_log("sig %d was raised\n", sig);
    asm(".word 0xffffffff");
}

bool arch_test_address(unsigned long address, bool write) {
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

