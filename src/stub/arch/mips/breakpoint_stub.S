.global jump_breakpoint_stub
.global jump_breakpoint_stub_end
.global jump_breakpoint_stub_bp_address
.global jump_breakpoint_stub_handler_func
.global jump_breakpoint_stub_epilogue_func

.set noat
.set noreorder

jump_breakpoint_stub:
    /* PUSH {R0-R12,LR} */
    sw $zero, 0($sp);
    sw $at, 4($sp);
    sw $v0, 8($sp);
    sw $v1, 12($sp);
    sw $a0, 16($sp);
    sw $a1, 20($sp);
    sw $a2, 24($sp);
    sw $a3, 28($sp);
    sw $t0, 32($sp);
    sw $t1, 36($sp);
    sw $t2, 40($sp);
    sw $t3, 44($sp);
    sw $t4, 48($sp);
    sw $t5, 52($sp);
    sw $t6, 56($sp);
    sw $t7, 60($sp);
    sw $s0, 64($sp);
    sw $s1, 68($sp);
    sw $s2, 72($sp);
    sw $s3, 76($sp);
    sw $s4, 80($sp);
    sw $s5, 84($sp);
    sw $s6, 88($sp);
    sw $s7, 92($sp);
    sw $t8, 96($sp);
    sw $t9, 100($sp);
    sw $k0, 104($sp);
    sw $k1, 108($sp);
    sw $gp, 112($sp);
    sw $sp, 116($sp);
    sw $fp, 120($sp);
    sw $ra, 124($sp);

    mflo $v0;
    sw $v0, 128($sp);
    mfhi $v0;
    sw $v0, 132($sp);

    addiu $sp, 136;
    /* sw $pc, 136($sp); */

    lw $a0, jump_breakpoint_stub_bp_address;
    move $a1, $sp;
    lw $t0, jump_breakpoint_stub_handler_func;
    jr $t0;
    nop;

jump_breakpoint_stub_bp_address:
    .word 0
jump_breakpoint_stub_handler_func:
    .word 0

jump_breakpoint_stub_end:

