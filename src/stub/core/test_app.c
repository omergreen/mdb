/*
 * A simple test application to test the debugger
 */

#include <time.h>


static long syscall(long sysnum, long a, long b, long c, long d, long e, long f) {
    int ret;
#ifdef ARCH_ARM
    register long _r7 __asm__("r7")=(long)(sysnum);
    register long _r5 __asm__("r5")=(long)(f);
    register long _r4 __asm__("r4")=(long)(e);
    register long _r3 __asm__("r3")=(long)(d);
    register long _r2 __asm__("r2")=(long)(c);
    register long _r1 __asm__("r1")=(long)(b);
    register long _r0 __asm__("r0")=(long)(a);
    __asm__ __volatile__(
            "swi 0"
            : "=r"(_r0)
            : "r"(_r0), "r"(_r1), "r"(_r2), "r"(_r3),
            "r"(_r4), "r"(_r5), "r"(_r7)
            : "memory");

    ret = _r0;
#elif ARCH_MIPS
    register long r4 __asm__("$4") = a;
	register long r5 __asm__("$5") = b;
	register long r6 __asm__("$6") = c;
	register long r7 __asm__("$7") = d;
	register long r8 __asm__("$8") = e;
	register long r9 __asm__("$9") = f;
	register long r2 __asm__("$2");
	__asm__ __volatile__ (
		"subu $sp,$sp,32 ; sw $8,16($sp) ; sw $9,20($sp) ; "
		"addu $2,$0,%4 ; syscall ;"
		"addu $sp,$sp,32"
		: "=&r"(r2), "+r"(r7), "+r"(r8), "+r"(r9)
		: "ir"(sysnum), "0"(r2), "r"(r4), "r"(r5), "r"(r6)
		: "$1", "$3", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "hi", "lo", "memory");

	/* return r7 && r2>0 ? -r2 : r2; */
    ret = r2;
#endif
    if(ret >=(unsigned long) -4095) {
        ret=(unsigned long) -1;
    }
    return (long) ret;
}
unsigned int core_write(int fd, char *buf, unsigned int length) {
#include <sys/syscall.h>
    return syscall(__NR_write, fd, (long)buf, length, 0, 0, 0);
}
int nanosleep(const struct timespec *req, struct timespec *rem) {
    return syscall(__NR_nanosleep, (long)req, (long)rem, 0, 0, 0, 0);
}

void test_app() {
    int i = 0;

    struct timespec req;
    req.tv_sec = 1;
    req.tv_nsec = 0;

    while (1) {
        char c = i++ + '0';
        core_write(1, &c, 1);
        nanosleep(&req, NULL);
    };
}

