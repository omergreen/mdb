/*
 * A simple test application to test the debugger
 */

#include <time.h>

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
    return syscall(__NR_write, fd, (long)buf, length);
}
int nanosleep(const struct timespec *req, struct timespec *rem) {
    return syscall(__NR_nanosleep, (long)req, (long)rem, 0);
}

void test_app() {
    /* target_log("in a loop\n"); */

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

