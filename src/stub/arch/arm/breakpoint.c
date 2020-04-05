#include "breakpoint.h"
#include "registers.h"
#include <core/log.h>
#include <libc/libc.h>

__attribute__((noreturn)) void jump_breakpoint_epilogue(unsigned int return_address, unsigned int sp) {
    extern unsigned int trampoline;
    *&trampoline = build_jump((unsigned int)&trampoline, return_address);
    cache_flush(&trampoline, BREAKPOINT_LENGTH);

    // from this function we're returning straight to the place where the breakpoint was before
    asm("\
    .global trampoline;\
\
    MOV SP, %0;\
    POP {R0,R1};\
    MSR cpsr, R0;\
    POP {R0-R12,LR};\
\
trampoline:\
    .word 0;\
    "
    :: "r" (sp)
    ); // well, we can't really avoid this inline assembly

    __builtin_unreachable();
}

__attribute__((noreturn)) void jump_breakpoint_handler(struct breakpoint *bp, unsigned int sp) {
    target_log("breakpoint jumped from 0x%08x with sp 0x%08x\n", bp->address, sp);

    struct registers_from_stub *regs = (struct registers_from_stub *)sp;
    registers_get_from_stub(&bp->arch_specific.regs, (struct registers_from_stub *)sp, bp->address);

    registers_print_all(bp);

    registers_update_to_stub(&bp->arch_specific.regs, (struct registers_from_stub *)sp);
    arch_jump_breakpoint_disable(bp);
    jump_breakpoint_epilogue(bp->address, sp);
}

static void fill_offset(void *stub, int offset, void *value) {
    *(void **)((unsigned int)(stub) + offset) = value;
}

bool arch_jump_breakpoint_put(struct breakpoint *bp) {
    void *stub = target_malloc(JUMP_BREAKPOINT_STUB_SIZE);
    if (stub == NULL) {
        ERROR("malloc for breakpoint stub returned NULL");
        return false;
    }

    DEBUG("stub for bp at 0x%08x allocated at 0x%08x", bp->address, stub);

    memcpy(stub, &jump_breakpoint_stub, JUMP_BREAKPOINT_STUB_SIZE);
    cache_flush(stub, JUMP_BREAKPOINT_STUB_SIZE);

    // fill in the required stuff for the stub:
    // - bp_address so that the handler will know which breakpoint was triggered
    // - handler_func and epilogue_func are technically static, but we can't fill them 
    //   at compile time due to PICness. We can initialize them once at init time instead of each
    //   time here, but does it really matter
    fill_offset(stub, JUMP_BREAKPOINT_STUB_BP_ADDRESS_OFFSET, bp);
    fill_offset(stub, JUMP_BREAKPOINT_STUB_HANDLER_FUNC_OFFSET, &jump_breakpoint_handler);
    fill_offset(stub, JUMP_BREAKPOINT_STUB_EPILOGUE_FUNC_OFFSET, &jump_breakpoint_epilogue);

    bp->arch_specific.stub = stub; // save it for later so we could free it

    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
    *(unsigned int *)bp->address = build_jump(bp->address, (unsigned int)stub);
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);

    return true;
}

bool arch_jump_breakpoint_disable(struct breakpoint *bp) {
    DEBUG("disabling bp at 0x%08x", bp->address);

    target_free(bp->arch_specific.stub);
    memcpy((void *)bp->address, bp->arch_specific.original_data, BREAKPOINT_LENGTH);
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);

    return true;
}

