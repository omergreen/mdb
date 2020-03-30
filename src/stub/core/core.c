/* #include <machine/arch/machine.h> */
#include <machine/target/target.h>
#include <core/ops.h>

struct ops g_ops;

/* extern unsigned long __GOT_START; */
/* extern unsigned long __GOT_END; */
/* extern unsigned long __START; */
extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;
extern unsigned long __START_OFFSET;

void fix_got();

__attribute__((externally_visible,used)) void _start(void *args) {
    fix_got();
    target_init(args);
    g_ops.log("123asd", 6);
    return;
}

void fix_got() {
    unsigned long start = (unsigned long)&_start;
    unsigned long *got = (unsigned long *)(start + (unsigned long)&__GOT_OFFSET); // stackoverflow.com/q/8398755
    for (unsigned int i = 0; i < ((unsigned long)&__GOT_LENGTH) / sizeof(*got); ++i) {
        if (i == 10)
            break;
        got[i] += start - (unsigned long)&__START_OFFSET;
    }
}

