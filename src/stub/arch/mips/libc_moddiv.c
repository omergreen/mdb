/*
 * Helper functions for / and % operations
 */

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

unsigned long long __udivdi3(unsigned long long n, unsigned long long d) {
  return __udivmoddi4(n, d, (unsigned long long *) 0);
}

unsigned long long __umoddi3(unsigned long long u, unsigned long long v) {
  unsigned long long w;
  __udivmoddi4(u ,v, &w);
  return w;
}

