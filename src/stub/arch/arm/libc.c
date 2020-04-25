/*
 * Implements general functions related to the ARM architecture
 */

#include <target/interface.h>
#include <core/log.h>
#include <arm_acle.h>
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

// find the vector table location
// important - works only for arm-a
unsigned long determine_ivt() {
    if (__arm_mrc(15, 0, 1, 0, 0) & (1 << 13)) { // read Control Register and check the V bit (high vectors)
        return 0xffff0000; 
    }

    if (__arm_mrc(15, 0, 0, 1, 1) & 0xf0) { // read Processor Feature Register 1 and check if trust zone is enabled
        /* from qemu source code */
        /* if (new_mode == ARM_CPU_MODE_SMC || */
        /*     (env->uncached_cpsr & CPSR_M) == ARM_CPU_MODE_SMC) { */
        /*     addr += env->cp15.c12_mvbar; */
        /* } else { */
        return __arm_mrc(15, 0, 12, 0, 0);
    }

    return 0;
}

void arch_cache_flush() {
    // for new ARM (7+) we can try dsb + isb (https://blog.senr.io/blog/why-is-my-perfectly-good-shellcode-not-working-cache-coherency-on-mips-and-arm)
    __arm_mcr(15, 0, 0, 7, 10, 4); // ensure all memory stores are complete
    __arm_mcr(15, 0, 0, 7, 5, 4); // flush entire icache
}

