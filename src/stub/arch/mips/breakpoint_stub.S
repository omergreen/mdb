#define __ASSEMBLY__
#include "cp0.h"

.global general_exception_handler_low
.global general_exception_stack

.set noat
.set noreorder

.macro LOAD_FROM_LOCAL ret var
    move \ret, $ra
    bal 1f
    lw $ra, %lo(\var - 1f)($ra)

1:  xor \ret, $ra // swap ret and $ra ( x ^= y, y ^= x, x ^= y)
    xor $ra, \ret
    xor \ret, $ra
.endm
.macro SAVE_TO_LOCAL val var tmp
    move \tmp, $ra
    bal 1f
    sw \val, %lo(\var - 1f)($ra)

1:  move $ra, \tmp
.endm

general_exception_stack: .word 0
load_general_exception_stack: // temp = $sp; $sp = general_exception_stack; return temp (in k0);
    LOAD_FROM_LOCAL $k1, general_exception_stack
    move $k0, $sp
    jr $ra
    move $sp, $k1

nest_count: .word 0
nest_count_up: // return nest_count++;
    LOAD_FROM_LOCAL $k0 nest_count
    addiu $k0, 1
    SAVE_TO_LOCAL $k0 nest_count $k1
    jr $ra
    addiu $k0, -1 // return original nest_count

nest_count_down: // nest_count--;
    LOAD_FROM_LOCAL $k0 nest_count
    addiu $k0, -1
    SAVE_TO_LOCAL $k0 nest_count $k1
    jr $ra
    nop

original_ra: .word 0
general_exception_handler_low:
    // prepare by moving to our own designated stack
    move $k1, $ra // we can't SAVE_TO_LOCAL directly with $ra...
    SAVE_TO_LOCAL $k1 original_ra $k0

1:  bal nest_count_up
    nop

    bne $k0, 0, save_regs // don't load our stack if we nest_count > 0
    move $k0, $sp // we expect k0 to contain the frame SP later on

    bal load_general_exception_stack
    nop

save_regs:
    LOAD_FROM_LOCAL $k1 original_ra
    addiu $sp, -148 - 41*4 // we don't implement floating points & dsp for now
    sw $zero,  0($sp) // obviously it's zero but gdb needs it
    sw $at,    4($sp)
    sw $v0,    8($sp)
    sw $v1,   12($sp)
    sw $a0,   16($sp)
    sw $a1,   20($sp)
    sw $a2,   24($sp)
    sw $a3,   28($sp)
    sw $t0,   32($sp)
    sw $t1,   36($sp)
    sw $t2,   40($sp)
    sw $t3,   44($sp)
    sw $t4,   48($sp)
    sw $t5,   52($sp)
    sw $t6,   56($sp)
    sw $t7,   60($sp)
    sw $s0,   64($sp)
    sw $s1,   68($sp)
    sw $s2,   72($sp)
    sw $s3,   76($sp)
    sw $s4,   80($sp)
    sw $s5,   84($sp)
    sw $s6,   88($sp)
    sw $s7,   92($sp)
    sw $t8,   96($sp)
    sw $t9,  100($sp)
    sw $zero,104($sp) // k0 - meaningless
    sw $zero,108($sp) // k1 - meaningless
    sw $gp,  112($sp)
    sw $k0,  116($sp) // sp
    sw $fp,  120($sp)
    sw $k1,  124($sp) // ra

    mflo $a0
    sw $a0,  128($sp) // lo
    mfhi $a0
    sw $a0,  132($sp) // hi

    mfc0 $a0, CP0_EPC
    sw $a0,  136($sp) // pc
    mfc0 $a0, CP0_STATUS
    sw $a0,  140($sp)
    mfc0 $a0, CP0_BADVADDR
    sw $a0,  144($sp)
    mfc0 $a0, CP0_CAUSE
    sw $a0,  148($sp)

    // enable interrupts - this code is taken from the linux kernel (arch/mips/include/asm/stackframe.h, macro STI)
    // not really sure what we are really supposed to do here? probably depends on platform. 
    // currently this zeros out bits 1-4 of status (UM, R0, ERL, EXL) which puts us in kernel mode, normal level,
    // and turns on bits IE and CU0 to enable interrupts and mark that we are currently in the kernel
    mfc0 $t0, CP0_STATUS
    li $t1, ST0_CU0 | 0x1f
    or $t0, $t1
    xori $t0, 0x1f & ~1
    mtc0 $t0, CP0_STATUS
    ehb // interrupt hazard something - do we really need this?

    bal get_general_exception_handler_high_address
    nop

    move $t9, $v0
    bal general_exception_handler_high
    move $a0, $sp

    bal nest_count_down
    nop

    // restore all the regs
    lw $a0, 128($sp) // lo
    mtlo $a0
    lw $a0, 132($sp) // hi
    mthi $a0

    // taken from linux kernel
    mfc0 $a0, CP0_STATUS
    ori $a0, 0x1f
    xori $a0, 0x1f
    mtc0 $a0, CP0_STATUS // turn off interrupts
    li $v1, ST0_CU1 | ST0_FR | ST0_IM
    and $a0, $v1 // extract ST0_CU1, ST0_FR and ST0_IM from the current status
    lw $v0, 140($sp)
    nor $v1, $0, $v1
    and $v0, $v1
    or $v0, $a0 // apply them to the new status
    mtc0 $v0, CP0_STATUS

    lw $a0, 136($sp) // pc
    mtc0 $a0, CP0_EPC
    lw $a0, 144($sp)
    mtc0 $a0, CP0_BADVADDR
    lw $a0, 148($sp)
    mtc0 $a0, CP0_CAUSE

    /* lw $zero,0($sp) */
    lw $at,  4($sp)
    lw $v0,  8($sp)
    lw $v1, 12($sp)
    lw $a0, 16($sp)
    lw $a1, 20($sp)
    lw $a2, 24($sp)
    lw $a3, 28($sp)
    lw $t0, 32($sp)
    lw $t1, 36($sp)
    lw $t2, 40($sp)
    lw $t3, 44($sp)
    lw $t4, 48($sp)
    lw $t5, 52($sp)
    lw $t6, 56($sp)
    lw $t7, 60($sp)
    lw $s0, 64($sp)
    lw $s1, 68($sp)
    lw $s2, 72($sp)
    lw $s3, 76($sp)
    lw $s4, 80($sp)
    lw $s5, 84($sp)
    lw $s6, 88($sp)
    lw $s7, 92($sp)
    lw $t8, 96($sp)
    lw $t9, 100($sp)
    /* lw $zero, 104($sp) // k0 - meaningless */
    /* lw $zero, 108($sp) // k1 - meaningless */
    lw $gp, 112($sp)
    lw $k1, 116($sp) // sp
    lw $fp, 120($sp)
    lw $ra, 124($sp) // ra

    addiu $sp, 148 + 41*4 // not really necessary, but for completeness's sake

    move $sp, $k1
    eret
    nop // does eret even have a delay slot? not sure


general_exception_handler_high_branch: b general_exception_handler_high
get_general_exception_handler_high_address: // I hate mips.. we don't have any good way to calculate offsets
                                            // since it won't let us subtract symbols in different object files
                                            // even not with %lo
    move $a0, $ra
    bal a
    lw $v0, %lo(general_exception_handler_high_branch - a)($ra)

a:  andi $v0, 0xffff
    sll $v0, 16
    sra $v0, 14 // extract offset and sign extend
    addiu $ra, %lo(general_exception_handler_high_branch - a)  + 4
    jr $a0
    addu $v0, $ra

