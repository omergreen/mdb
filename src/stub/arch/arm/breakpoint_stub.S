breakpoint_stub:
    PUSH {R0-R12,LR}
    MRS R0, cpsr
    ADD R1, SP, 4*14
    PUSH {R0,R1}

    BL breakpoint_handler

    POP {R0,R1}
    MSR cpsr, R0
    POP {R0-R12,LR}
    
breakpoint_stub_trampoline:
    .word 0

breakpoint_stub_end:

