#include "breakpoint.h"
#include "registers.h"
#include <core/log.h>
#include <libc/libc.h>
#include <core/ops.h>

void registers_get_from_stub(struct registers *regs, struct registers_from_stub *regs_stub, unsigned int pc) {
    memcpy(&regs->r0, regs_stub->r0_to_r12, sizeof(regs_stub->r0_to_r12)); // copy r0-r12
    regs->sp = regs_stub->sp;
    regs->lr = regs_stub->lr;
    regs->pc = pc; // do we need to add 8 to it or something? not sure
    regs->cpsr.packed = regs_stub->cpsr;
}

void registers_update_to_stub(struct registers *regs, struct registers_from_stub *regs_stub) {
    memcpy(&regs_stub->r0_to_r12, &regs->r0, sizeof(regs_stub->r0_to_r12)); // copy r0-r12
    regs_stub->sp = regs->sp;
    regs_stub->lr = regs->lr;
    /* regs_stub->pc = regs->pc; // do we need to add 8 to it or something? not sure */   // pc needs special treatment - argument of jump_breakpoint_epilogue
    regs_stub->cpsr = regs->cpsr.packed;
}


// COLORS, sorry for the mess

#define REGISTER_COLOR KBLU KBOLD
#define COLORIZE_REGISTER_ARG(...) COLORIZE_ARGS(REGISTER_COLOR, ##__VA_ARGS__)

static void print_colored_line(const char *s1, const char *s2, const char *s3, const char *s4, unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4) {
#define STRING_AND_VALUE_FORMAT COLORIZE_FORMAT("%-4s") " 0x%-8x  "

    g_ops.log(STRING_AND_VALUE_FORMAT STRING_AND_VALUE_FORMAT STRING_AND_VALUE_FORMAT STRING_AND_VALUE_FORMAT "\n", 
              COLORIZE_REGISTER_ARG(s1), v1, COLORIZE_REGISTER_ARG(s2), v2, COLORIZE_REGISTER_ARG(s3), v3, COLORIZE_REGISTER_ARG(s4), v4);
}
static void print_cpsr(union cpsr *cpsr) {
#define BOLDIFY_FLAG(value) COLORIZE_ARGS(value ? KBOLD : KDIM)

    g_ops.log(COLORIZE_FORMAT("cpsr") " 0x%-8x  [ " COLORIZE_FORMAT("thumb ") 
              COLORIZE_FORMAT("overflow ") COLORIZE_FORMAT("carry ") 
              COLORIZE_FORMAT("zero ") COLORIZE_FORMAT("negative ") "]\n", 

              COLORIZE_REGISTER_ARG(), cpsr->packed, BOLDIFY_FLAG(cpsr->bits.thumb), 
              BOLDIFY_FLAG(cpsr->bits.overflow), BOLDIFY_FLAG(cpsr->bits.carry), 
              BOLDIFY_FLAG(cpsr->bits.zero), BOLDIFY_FLAG(cpsr->bits.negative));
}
void registers_print_all(struct breakpoint *bp) {
    print_colored_line("r0", "r1", "r2", "r3", bp->arch_specific.regs.r0, bp->arch_specific.regs.r1, bp->arch_specific.regs.r2, bp->arch_specific.regs.r3);
    print_colored_line("r4", "r5", "r6", "r7", bp->arch_specific.regs.r4, bp->arch_specific.regs.r5, bp->arch_specific.regs.r6, bp->arch_specific.regs.r7);
    print_colored_line("r8", "r9", "r10", "r11", bp->arch_specific.regs.r8, bp->arch_specific.regs.r9, bp->arch_specific.regs.r10, bp->arch_specific.regs.r11);
    print_colored_line("r12", "sp", "lr", "pc", bp->arch_specific.regs.r12, bp->arch_specific.regs.sp, bp->arch_specific.regs.lr, bp->arch_specific.regs.pc);
    print_cpsr(&bp->arch_specific.regs.cpsr);
}

