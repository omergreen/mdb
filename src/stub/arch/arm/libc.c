/*
 * Implements general functions related to the ARM architecture
 */

#include <target/interface.h>
#include <core/log.h>
#include "libc.h"

unsigned int convert_code_data_32(unsigned int val) {
#if CODE_ENDIAN != DATA_ENDIAN
    val = swap32(val);
#endif
    return val;
}

unsigned short convert_code_data_16(unsigned short val) {
#if CODE_ENDIAN != DATA_ENDIAN
    val = swap16(val);
#endif
    return val;
}

unsigned int build_jump(unsigned int from, unsigned int to) {
    int delta = ((int)(to - (from + 8))) >> 2;
    if (delta > 0xffffff || delta < -0xffffff) {
        ERROR("jump from 0x%x to 0x%x is too far", from, to);
        assert(1);
    }

    unsigned int jump = 0xea000000 + (delta & 0xffffff);
    return convert_code_data_32(jump);
}

void raise(int sig) { // needed for the ARM div functions
    target_log("sig %d was raised\n", sig);
    asm(".word 0xffffffff");
}

void arch_cache_flush() {
    asm("\
    MOV    r0, #0;\
    MCR    p15, 0, r0, c7, c5, 0; # Flush entire ICache\
    ");
}

