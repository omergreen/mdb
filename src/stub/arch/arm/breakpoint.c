#include "breakpoint.h"
#include <core/ops.h>

#include <libc/libc.h>

/* #define BREAKPOINT_LENGTH (4) */
#define JUMP_BREAKPOINT_STUB_SIZE (jump_breakpoint_stub_end - jump_breakpoint_stub)
/* #define BREAKPOINT_STUB_TRAMPOLINE_OFFSET (jump_breakpoint_stub_trampoline - jump_breakpoint_stub) */
#define JUMP_BREAKPOINT_STUB_PTR_OFFSET (jump_breakpoint_stub_ptr - jump_breakpoint_stub)

__attribute__((naked)) void jump_breakpoint_epilogue(unsigned int return_address) {
    asm("\
    MOV R1, R0;\
    ADR R0, trampoline;\
    BL build_jump;\
\
    STR R0, trampoline;\
    POP {R0,R1};\
    MSR cpsr, R0;\
    POP {R0-R12,LR};\
\
trampoline:\
    .word 0;\
    ");
}
void breakpoint_handler() {
}

/* __attribute__((naked)) void jump_breakpoint_stub() { */
asm("\
.global jump_breakpoint_stub;\
.global jump_breakpoint_stub_end;\
.global jump_breakpoint_stub_ptr;\
jump_breakpoint_stub:\
    PUSH {R0-R12,LR};\
    MRS R0, cpsr;\
    ADD R1, SP, #4*14;\
    PUSH {R0,R1};\
\
    ADR LR, jump_breakpoint_epilogue;\
    LDR R0, jump_breakpoint_stub_ptr;\
    MOV R1, SP;\
    B breakpoint_handler;\
\
jump_breakpoint_stub_ptr:\
    .word 0;\
\
jump_breakpoint_stub_end:\
    ");
/* } */

extern void *jump_breakpoint_stub;
extern void *jump_breakpoint_stub_end;
extern void *jump_breakpoint_stub_ptr;


bool jump_breakpoint_put(struct breakpoint *bp) {
    void *stub = g_ops.malloc(JUMP_BREAKPOINT_STUB_SIZE);
    memcpy(stub, jump_breakpoint_stub, JUMP_BREAKPOINT_STUB_SIZE);
    /* unsigned int *trampoline_address = (unsigned char *)stub + BREAKPOINT_STUB_TRAMPOLINE_OFFSET; */
    /* *trampoline_address = build_jump((unsigned int)trampoline_address, bp->address + 4); */
    struct breakpoint **ptr_address = (struct breakpoint **)((unsigned char*)stub + JUMP_BREAKPOINT_STUB_PTR_OFFSET);

    bp->arch_specific.stub = stub;
    memcpy(bp->arch_specific.original_data, (void *)bp->address, sizeof(bp->arch_specific.original_data));

    *(unsigned int *)bp->address = build_jump((unsigned int)bp->address, (unsigned int)stub);

    g_ops.cache_flush();

    return true;
}

bool jump_breakpoint_disable(struct breakpoint *bp) {
    if (!bp->enabled) { // TODO: move..
        return false;
    }

    g_ops.free(bp->arch_specific.stub);
    memcpy((void *)bp->address, bp->arch_specific.original_data, sizeof(bp->arch_specific.original_data));
    g_ops.cache_flush();
    /* bp->enabled = false; */

    return true;
}

