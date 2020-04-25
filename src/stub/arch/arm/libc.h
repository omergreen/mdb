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

// find the vector table location
// important - works only for arm-a, assumes we are in secure mode
unsigned long determine_ivt();

void cache_flush();


#define IS_THUMB_ADDR(addr)    ((addr) & 1)
#define MAKE_THUMB_ADDR(addr)  ((addr) | 1)
#define UNMAKE_THUMB_ADDR(addr) ((addr) & ~1)

#define ENDIAN_BIG (0)
#define ENDIAN_LITTLE (1)

#ifdef ARM_BIG_ENDIAN
    #define DATA_ENDIAN ENDIAN_BIG
    #define CODE_ENDIAN ENDIAN_BIG
#endif
#ifdef ARM_LITTLE_ENDIAN
    #define DATA_ENDIAN ENDIAN_LITTLE
    #define CODE_ENDIAN ENDIAN_LITTLE
#endif
#ifdef ARM_BE8_ENDIAN
    #define DATA_ENDIAN ENDIAN_BIG
    #define CODE_ENDIAN ENDIAN_LITTLE
#endif
#ifndef DATA_ENDIAN
    #error "no endianness"
#endif

