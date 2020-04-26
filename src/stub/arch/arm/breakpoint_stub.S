.global jump_breakpoint_stub
.global jump_breakpoint_stub_end
.global jump_breakpoint_stub_bp_address
.global jump_breakpoint_stub_handler_func
.global jump_breakpoint_epilogue
.global data_abort_interrupt_handler
.global prefetch_abort_interrupt_handler

// TODO: change SP to some pre-allocated different stack

jump_breakpoint_stub:
    PUSH {R0-R12,LR}
    MRS R0, cpsr
    ADD R1, SP, #4*14 // point SP at the start of R0-R12
    PUSH {R0,R1}

    LDR R0, jump_breakpoint_stub_bp_address
    MOV R1, SP
    LDR R2, jump_breakpoint_stub_handler_func
    BX R2

jump_breakpoint_stub_bp_address:
    .word 0
jump_breakpoint_stub_handler_func:
    .word 0

jump_breakpoint_stub_end:




jump_breakpoint_epilogue: // r0 - return address, r1 - sp
    STR r0, return_address
    MOV SP, R1
    POP {R0,R1}
    MSR cpsr, R0
    POP {R0-R12,LR}
    LDR PC, return_address

return_address: .word 0


.equ MODE_USR, 0x10
.equ MODE_FIQ, 0x11
.equ MODE_IRQ, 0x12
.equ MODE_SVC, 0x13
.equ MODE_ABT, 0x17
.equ MODE_UND, 0x1B
.equ MODE_SYS, 0x1F
.equ MODE_MASK, 0x1f

prefetch_abort_interrupt_handler:
    SUB LR, #4 // on prefetch abort, LR points to PC + 4
    PUSH {R0}
    MOV R0, #1
    STR R0, abort_interrupt_handler_is_prefetch_abort
    POP {R0}
    B abort_interrupt_handler

data_abort_interrupt_handler:
    SUB LR, #8 // on data abort, LR points to PC + 8
    PUSH {R0}
    MOV R0, #0
    STR R0, abort_interrupt_handler_is_prefetch_abort
    POP {R0}
    B abort_interrupt_handler

// TODO: check if sp is 0 (i.e. no one initialized our stack) and do something
abort_interrupt_handler:
    STR LR, abort_interrupt_handler_abort_pc

    // remember that SP here points to the _abort_ stack
    PUSH {R0, R1} // backup r0 and r1 since we use them as temp registers

    MRS R0, spsr // read original cpsr
    BIC R0, #~MODE_MASK // clear anything but the mode
    // if we came from user mode, switch to system instead (which shared registers
    // with user mode) to avoid losing privilege
    CMP R0, #MODE_USR
    MOVEQ R0, #MODE_SYS

    // we want to keep cpsr the same, but switch the mode back to the one we came from
    // to access its' LR and SP (since they are banked)
    MRS R1, cpsr
    BIC R1, #MODE_MASK // clear the previous mode
    ORR R0, R1 // cpsr = (cpsr without mode) | old mode
    MRS R1, cpsr // backup cpsr so we can restore it afterwards
    MSR cpsr, R0

    // now we are in the original mode
    STR LR, abort_interrupt_handler_system_lr
    STR SP, abort_interrupt_handler_system_sp

    // switch back
    MSR cpsr, R1

    POP {R0, R1} // restore r0 and r1
    LDR LR, abort_interrupt_handler_system_lr
    PUSH {R0-R12, LR}
    MRS R0, spsr // read original cpsr
    LDR R1, abort_interrupt_handler_system_sp
    PUSH {R0, R1}
    
    LDR R0, abort_interrupt_handler_abort_pc // address of breakpoint
    MOV R1, SP
    LDR R2, abort_interrupt_handler_is_prefetch_abort
    BL abort_handler // returns the address we want to return to
    STR R0, abort_interrupt_handler_return_address

    POP {R0, R1}
    POP {R0-R12, LR}
    LDR LR, abort_interrupt_handler_return_address
    SUBS PC, LR, #0 // restores original cpsr, switches back to normal mode with original LR

// banked registers per mode are {lr, sp}
abort_interrupt_handler_abort_pc: .word 0
abort_interrupt_handler_system_lr: .word 0
abort_interrupt_handler_system_sp: .word 0
abort_interrupt_handler_return_address: .word 0
abort_interrupt_handler_is_prefetch_abort: .word 0

