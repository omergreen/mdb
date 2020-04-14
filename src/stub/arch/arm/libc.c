/*
 * Implements general functions related to the ARM architecture
 */

#include <target/interface.h>
#include "libc.h"

unsigned int convert_code_data_32(unsigned int val) {
    unsigned int swapped;

// swap if needed
#if CODE_ENDIAN != DATA_ENDIAN
    swapped = ((val>>24) & 0xff)     |
              ((val<<8)  & 0xff0000) |
              ((val>>8)  & 0xff00)   |
              ((val<<24) & 0xff000000);
#else
    swapped = val;
#endif

    return swapped;
}

unsigned short convert_code_data_16(unsigned short val) {
    unsigned short swapped;

// swap if needed
#if CODE_ENDIAN != DATA_ENDIAN
    swapped = (val>>8) | (val<<8);
#else
    swapped = val;
#endif

    return swapped;
}


unsigned int build_jump(unsigned int from, unsigned int to) {
    unsigned int jump = 0xea000000 + (((to - (from + 8)) / 4) & 0xffffff);
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

