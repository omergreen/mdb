/*
 * The MIPS register set
 */

#pragma once

/* union cpsr { // TODO: check the endianness of bits with both BE and LE */
/*     unsigned int packed; */ 
/*     struct __attribute__((packed)) { // https://www.keil.com/pack/doc/CMSIS/Core_A/html/group__CMSIS__CPSR.html */
/*     } bits; */        
/* }; */

struct registers {
        unsigned int zero;
        unsigned int at;
        unsigned int v0;
        unsigned int v1;
        unsigned int a0;
        unsigned int a1;
        unsigned int a2;
        unsigned int a3;
        unsigned int t0;
        unsigned int t1;
        unsigned int t2;
        unsigned int t3;
        unsigned int t4;
        unsigned int t5;
        unsigned int t6;
        unsigned int t7;
        unsigned int s0;
        unsigned int s1;
        unsigned int s2;
        unsigned int s3;
        unsigned int s4;
        unsigned int s5;
        unsigned int s6;
        unsigned int s7;
        unsigned int t8;
        unsigned int t9;
        unsigned int k0;
        unsigned int k1;
        unsigned int gp;
        unsigned int sp;
        unsigned int fp;
        unsigned int ra;
        unsigned int lo;
        unsigned int hi;
        unsigned int pc;
        /* <reg name="status" bitsize="32" regnum="32"/> */
        /* <reg name="badvaddr" bitsize="32" regnum="35"/> */
        /* <reg name="cause" bitsize="32" regnum="36"/> */
};

#define REGISTERS_LENGTH (35)

