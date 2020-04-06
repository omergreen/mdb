/*
 * The ARM register set
 */

#pragma once

#define CPSR_M_USR 0x10 // PL0 - user mode
#define CPSR_M_FIQ 0x11 // PL1 - on FIQ interrupt
#define CPSR_M_IRQ 0x12 // PL1 - on IRQ interrupt
#define CPSR_M_SVC 0x13 // PL1 - on supervisor call
#define CPSR_M_ABT 0x17 // PL1 - on data abort or prefetch abort
#define CPSR_M_MON 0x16 // PL1 - on secure monitor call exception
#define CPSR_M_HYP 0x1a // PL2 - virtualization extensions something something
#define CPSR_M_UND 0x1b // PL1 - undefined mode - we get here by instruction-related exceptions (including executing UNDEFINED)
#define CPSR_M_SYS 0x1f // PL1 - system mode

union cpsr { // TODO: check the endianness of bits with both BE and LE
    unsigned int packed; 
    struct __attribute__((packed)) { // https://www.keil.com/pack/doc/CMSIS/Core_A/html/group__CMSIS__CPSR.html
        unsigned int mode : 5; // current privilege mode
        unsigned int thumb : 1; // T 
        unsigned int firq : 1; // F - when set, disable fast interrupt requests
        unsigned int irq : 1; // I - when set, disable interrupt requests
        unsigned int async_abort : 1; // A - when set, disable imprecise aborts
        unsigned int endianness : 1; // E - 0 for little endian, 1 for big
        unsigned int IT7_2 : 6; // If-Then execution state bits for the Thumb IT (If-Then) instruction
        unsigned int GE : 4; // greater than or equal flags, related to SIMD
        unsigned int _hole1 : 4;
        unsigned int jazelle : 1; // something about java bytecode
        unsigned int IT1_0 : 2;
        unsigned int Q : 1; // Q - cumulative saturation bit, not interesting, related to DSP
        unsigned int overflow : 1; // V - oVerflow condition code flag
        unsigned int carry : 1; // C - Carry condition code flag
        unsigned int zero : 1; // Z - Zero condition code flag
        unsigned int negative : 1; // N - Negative condition code flag
    } bits;        
};

struct registers {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r4;
    unsigned int r5;
    unsigned int r6;
    unsigned int r7;
    unsigned int r8;
    unsigned int r9;
    unsigned int r10;
    unsigned int r11;
    unsigned int r12;
    unsigned int sp;
    unsigned int lr;
    unsigned int pc;
    union cpsr cpsr;
};

