/*
 * Implements the entry point
 */

#include <arch/interface.h>
#include <target/interface.h>
#include <libc/libc.h>

extern unsigned long __GOT_OFFSET;
extern unsigned long __GOT_LENGTH;
extern unsigned long __START_OFFSET;

void fix_got();

static long syscall(long sysnum, long a, long b, long c) {
    register long _r7 __asm__("r7")=(long)(sysnum);
    register long _r2 __asm__("r2")=(long)(c);
    register long _r1 __asm__("r1")=(long)(b);
    register long _r0 __asm__("r0")=(long)(a);
    __asm__ __volatile__(
            "swi 0"
            : "=r"(_r0)
            : "r"(_r0), "r"(_r1), "r"(_r2),
            "r"(_r7)
            : "memory");
    if(_r0 >=(unsigned long) -4095) {
        long err = _r0;
        _r0=(unsigned long) -1;
    }
    return (long) _r0;
}
unsigned int core_write(int fd, char *buf, unsigned int length) {
#include <sys/syscall.h>
    return syscall(__NR_write, fd, buf, length);
}

void do_nabaz() {
    /* target_log("in a loop\n"); */

    int i = 0;

    while (1) {
        if ((i++ & (0x100-1)) == 0) {
            char c = (i>>8) + '0';
            core_write(1, &c, 1);
        }
    };
}

__attribute__((used,section(".init"))) void _start(void *args) {
    fix_got();
    arch_init();
    target_init(args);

    struct breakpoint bp;
    bp.address = (unsigned int)do_nabaz;// + 52;
    arch_jump_breakpoint_enable(&bp);

    do_nabaz();

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

