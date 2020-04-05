#include <target/interface.h>

unsigned int build_jump(unsigned int from, unsigned int to) {
    return 0xea000000 + (((to - (from + 8)) / 4) & 0xffffff);
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

