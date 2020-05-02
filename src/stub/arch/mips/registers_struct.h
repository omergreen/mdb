/*
 * The MIPS register set
 */

#pragma once

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

        // cp0
        unsigned int status;
        unsigned int badvaddr;
        unsigned int cause;
         
        // floating point
        unsigned int floating_point_registers[32];
        unsigned int fscr, fir;

        // dsp        
        unsigned int hi1;
        unsigned int lo1;
        unsigned int hi2;
        unsigned int lo2;
        unsigned int hi3;
        unsigned int lo3;
        unsigned int dspctl;
};

#define REGISTERS_LENGTH (sizeof(struct registers) / sizeof(unsigned int))

