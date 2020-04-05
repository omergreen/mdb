#include <asm/unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdarg.h>
#include "syscall.h"
#include <libc/libc.h>

void *mmap2(void *addr, unsigned long len, int prot, int flags, int fildes, signed long off) {
    return (void *)syscall(__NR_mmap2, (long)addr, (long)len, (long)prot, (long)flags, (long)fildes, (long)off);
}

int _open(const char *filename, int flags, ...) {
    mode_t mode = 0;

    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }

    return syscall(__NR_open, filename, flags, mode);
}

unsigned int _write(int fd, char *buf, unsigned int length) {
    return syscall(__NR_write, fd, buf, length);
}

int _close(int fd) {
    return syscall(__NR_close, fd);
}

void target_cache_flush(void *start, unsigned int length) { // https://github.com/llvm-mirror/compiler-rt/blob/master/lib/builtins/clear_cache.c
#if ARCH == arm
    syscall(__ARM_NR_cacheflush, start, start + length, 0);
#elif ARCH == mips
    syscall(__NR_cacheflush, start, length, BCACHE);
#else
#error "bassa"
#endif
}

void *target_malloc(size_t size) {
    return malloc(size);
}

void target_free(void *ptr) {
    free(ptr);
}

void *target_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

