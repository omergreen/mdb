/*
 * Implementation of the various breakpoint-related functions
 */

#include "breakpoint.h"
#include "registers.h"
#include <core/log.h>
#include <libc/libc.h>
#include "get_next_pc/get_next_pc.h"


/*
 * Restore the CPU state and jump back to the original code
 */
__attribute__((noreturn)) static void jump_breakpoint_epilogue(unsigned int return_address, unsigned int sp) {
    // we replace in runtime the word at trampoline with a jump opcode to the original PC
    extern unsigned int trampoline;
    *&trampoline = build_jump((unsigned int)&trampoline, return_address);
    cache_flush(&trampoline, 4);

    // sp is restored "automatically" because we do the same amount of pops as we did pushes
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

struct breakpoint global_bp1;
struct breakpoint global_bp2;
/*
 * Called directly from the breakpoint stub, this function parses the stack to get the register state
 * and then gives control to the core.
 */
__attribute__((noreturn)) static void jump_breakpoint_handler(struct breakpoint *bp, unsigned int sp) {
    DEBUG("breakpoint jumped from 0x%08x with sp 0x%08x", bp->address, sp);

    struct registers_from_stub *regs = (struct registers_from_stub *)sp;
    registers_get_from_stub(&bp->arch_specific.regs, (struct registers_from_stub *)sp, bp->address);

    extern void *core_write;
    if (bp->arch_specific.regs.pc == (unsigned int)&core_write) {
        registers_print_all(bp);
        ERROR("going to write \"%s\"", (char *)bp->arch_specific.regs.r1); 
    }

    pc_list next_pcs = arch_get_next_pc(bp);

    struct breakpoint *next_bp = (bp == &global_bp1 ? &global_bp2 : &global_bp1);

    next_bp->address = *cvector_begin(next_pcs);
    if (next_pcs == NULL) {
        ERROR("next_pcs is NULL???");
    }
    arch_jump_breakpoint_enable(next_bp);

    cvector_free(next_pcs);

    registers_update_to_stub(&bp->arch_specific.regs, (struct registers_from_stub *)sp);
    arch_jump_breakpoint_disable(bp);
    jump_breakpoint_epilogue(bp->address, sp);
}

static void fill_offset(void *stub, int offset, void *value) {
    *(void **)((unsigned int)(stub) + offset) = value;
}

/*
 * Replace the code at bp->address with a jump to jump_breakpoint_stub.
 * We allocate a stub for each breakpoint because we need some way to let ourselves know
 * which breakpoint was triggered - so for each stub we fill the bp_address with the relevant breakpoint struct pointer
 * jump_breakpoint_handler is called with that breakpoint and so we can pass the core the triggered breakpoint
 */
bool arch_jump_breakpoint_enable(struct breakpoint *bp) {
    void *stub = target_malloc(JUMP_BREAKPOINT_STUB_SIZE);
    if (stub == NULL) {
        ERROR("malloc for breakpoint stub returned NULL");
        return false;
    }

    /* DEBUG("stub for bp at 0x%08x allocated at 0x%08x", bp->address, stub); */

    memcpy(stub, &jump_breakpoint_stub, JUMP_BREAKPOINT_STUB_SIZE);
    // fill in the required stuff for the stub:
    // - bp_address so that the handler will know which breakpoint was triggered
    // - handler_func and epilogue_func are technically static, but we can't fill them 
    //   at compile time due to PICness. We can initialize them once at init time instead of each
    //   time here, but does it really matter
    fill_offset(stub, JUMP_BREAKPOINT_STUB_BP_ADDRESS_OFFSET, bp);
    fill_offset(stub, JUMP_BREAKPOINT_STUB_HANDLER_FUNC_OFFSET, &jump_breakpoint_handler);
    fill_offset(stub, JUMP_BREAKPOINT_STUB_EPILOGUE_FUNC_OFFSET, &jump_breakpoint_epilogue);

    cache_flush(stub, JUMP_BREAKPOINT_STUB_SIZE);

    bp->arch_specific.stub = stub; // save it for later so we could free it

    // finally, backup the code at bp->address, and replace it with the jump
    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));
    *(unsigned int *)bp->address = build_jump(bp->address, (unsigned int)stub);
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);

    return true;
}

/*
 * Replace the breakpoint at bp->addres with the original code
 */
bool arch_jump_breakpoint_disable(struct breakpoint *bp) {
    /* DEBUG("disabling bp at 0x%08x", bp->address); */

    target_free(bp->arch_specific.stub);
    memcpy((void *)bp->address, bp->arch_specific.original_data, BREAKPOINT_LENGTH);
    cache_flush((void *)bp->address, BREAKPOINT_LENGTH);

    return true;
}

