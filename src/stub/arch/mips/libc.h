/*
 * General functions related to the ARM architecture
 */

#pragma once

/*
 * Return a jump opcode from `from` to `to`
 */
unsigned int build_jump(unsigned int from, unsigned int to);

// find the vector table location
unsigned long determine_ivt();

void cache_flush();


#define IS_MIPS16_ADDR(addr)    ((addr) & 1)
#define MAKE_MIPS16_ADDR(addr)  ((addr) | 1)
#define UNMAKE_MIPS16_ADDR(addr) ((addr) & ~1)

#ifdef MIPS_BIG_ENDIAN
    #define DATA_ENDIAN ENDIAN_BIG
    #define CODE_ENDIAN ENDIAN_BIG
#endif
#ifdef MIPS_LITTLE_ENDIAN
    #define DATA_ENDIAN ENDIAN_LITTLE
    #define CODE_ENDIAN ENDIAN_LITTLE
#endif
#ifndef DATA_ENDIAN
    #error "no endianness"
#endif

