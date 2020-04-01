#pragma once


union cpsr { // TODO: check the endianness of bits with both BE and LE
    unsigned int packed; 
    struct __attribute__((packed)) { // https://www.keil.com/pack/doc/CMSIS/Core_A/html/group__CMSIS__CPSR.html
        unsigned int mode : 5; // current privilege mode
        unsigned int thumb : 1; // T
        unsigned int firq : 1; // F - enable/disable fast interrupt requests
        unsigned int irq : 1; // I
        unsigned int async_abort : 1; // A
        unsigned int endianness : 1; // E - 0 for little endian, 1 for big
        unsigned int IT7_2 : 6; // If-Then execution state bits for the Thumb IT (If-Then) instruction
        unsigned int GE : 4; // greater than or equal flags
        unsigned int _hole1 : 4;
        unsigned int jazelle : 1; // something about java bytecode
        unsigned int IT1_0 : 2;
        unsigned int underflow : 1; // Q - cumulative saturation bit
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

