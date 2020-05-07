/*
 * General functions related to the mips architecture
 */

#pragma once
#include "cp0.h"

/*
 * Return a jump opcode from `from` to `to`
 */
unsigned int build_jump(unsigned int from, unsigned int to);

// find the vector table location
unsigned long determine_ivt();

void cache_flush();

unsigned long move_ivt(unsigned long new_addr);

#define MFC0(specifier) ({ int out; asm volatile("mfc0 %0, " specifier : "=r"(out)); out; })
#define MTC0(specifier, val) asm volatile("mtc0 %0, " specifier :: "r"(val));
#define EBASE_WG (1 << 11)
#define STATUS_BEV (1 << 22)

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

