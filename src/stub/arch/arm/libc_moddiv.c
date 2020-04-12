#include <stdint.h>
#include <core/log.h>

unsigned long __udivmoddi4(uint64_t num, uint64_t den, uint64_t *rem_p) {
    unsigned long quot = 0, qbit = 1;

    if ( den == 0 ) {
        assert(0);
    }

    /* Left-justify denominator and count shift */
    while ( (long)den >= 0 ) {
        den <<= 1;
        qbit <<= 1;
    }

    while ( qbit ) {
        if ( den <= num ) {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    if ( rem_p )
        *rem_p = num;

    return quot;
}

uint32_t __udivmodsi4(uint32_t num, uint32_t den, uint32_t *rem_p) {
    uint32_t quot = 0, qbit = 1;

    if ( den == 0 ) {
        assert(0);
    }

    /* Left-justify denominator and count shift */
    while ( (int32_t)den >= 0 ) {
        den <<= 1;
        qbit <<= 1;
    }

    while ( qbit ) {
        if ( den <= num ) {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    if ( rem_p )
        *rem_p = num;

    return quot;
}

// struct { uint64_t quot, uint64_t rem}
//        __aeabi_uldivmod(uint64_t numerator, uint64_t denominator) {
//   uint64_t rem, quot;
//   quot = __udivmoddi4(numerator, denominator, &rem);
//   return {quot, rem};
// }
__attribute__((naked)) void __aeabi_uldivmod() {
    asm("push {r11, lr};\
         sub sp, sp, #16;\
         add r12, sp, #8;\
         str r12, [sp];\
         bl __udivmoddi4;\
         ldr r2, [sp, #8];\
         ldr r3, [sp, #12];\
         add sp, sp, #16;\
         pop {r11, pc}");
}

__attribute__((naked)) void __aeabi_uidivmod() {
    asm("push {lr};\
         sub sp, sp, #4;\
         mov r2, sp;\
         bl __udivmodsi4;\
         ldr r1, [sp];\
         add sp, sp, #4;\
         pop { pc }");
}

uint32_t __aeabi_uidiv(uint32_t numerator, uint32_t denominator) {
    uint32_t rem, quot;
    quot = __udivmodsi4(numerator, denominator, &rem);
    return quot;
}

