.global jump_breakpoint_stub
.global jump_breakpoint_stub_end
.global jump_breakpoint_stub_bp_address
.global jump_breakpoint_stub_handler_func
.global jump_breakpoint_stub_epilogue_func

jump_breakpoint_stub:
    PUSH {R0-R12,LR}
    MRS R0, cpsr
    ADD R1, SP, #4*14 // point SP at the start of R0-R12
    PUSH {R0,R1}

    LDR LR, jump_breakpoint_stub_epilogue_func
    LDR R0, jump_breakpoint_stub_bp_address
    MOV R1, SP
    LDR R2, jump_breakpoint_stub_handler_func
    BX R2

jump_breakpoint_stub_bp_address:
    .word 0
jump_breakpoint_stub_handler_func:
    .word 0
jump_breakpoint_stub_epilogue_func:
    .word 0

jump_breakpoint_stub_end:

