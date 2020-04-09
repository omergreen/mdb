/*
 * General functions related to the ARM architecture
 */

#pragma once

/*
 * If needed, reverse the endianness of a 32-bit integer to convert
 * from code to data
 */
unsigned int convert_code_data_32(unsigned int val);

/*
 * If needed, reverse the endianness of a 16-bit integer to convert
 * from code to data
 */
unsigned short convert_code_data_16(unsigned short val);

/*
 * Return a jump opcode from `from` to `to`
 */
unsigned int build_jump(unsigned int from, unsigned int to);

void cache_flush();


#define IS_THUMB_ADDR(addr)    ((addr) & 1)
#define MAKE_THUMB_ADDR(addr)  ((addr) | 1)
#define UNMAKE_THUMB_ADDR(addr) ((addr) & ~1)

enum endian {
    BIG,
    LITTLE
};

#ifdef BIG_ENDIAN
    #define DATA_ENDIAN (LITTLE)
    #define CODE_ENDIAN (LITTLE)
#endif
#ifdef LITTLE_ENDIAN
    #define DATA_ENDIAN (BIG)
    #define CODE_ENDIAN (BIG)
#endif
#ifdef BE8_ENDIAN
    #define DATA_ENDIAN (BIG)
    #define CODE_ENDIAN (LITTLE)
#endif
#ifndef DATA_ENDIAN
    #error "no endianness"
#endif

