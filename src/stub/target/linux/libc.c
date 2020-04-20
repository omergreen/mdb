/*
 * Implements Linux-related libc stuff
 */

#include <asm/unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdarg.h>
#include "syscall.h"
#include <core/log.h>
#include <libc/libc.h>

#ifdef ARCH_MIPS
#include <asm/cachectl.h>
#endif

long errno;

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

ssize_t _read(int fd, void *buf, size_t length) {
    return syscall(__NR_read, fd, buf, length);
}

ssize_t _write(int fd, const void *buf, size_t length) {
    return syscall(__NR_write, fd, buf, length);
}

int _close(int fd) {
    return syscall(__NR_close, fd);
}

int _socket(int domain, int type, int protocol) {
    return syscall(__NR_socket, domain, type, protocol);
}

int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return syscall(__NR_bind, sockfd, addr, addrlen);
}

int _listen(int sockfd, int backlog) {
    return syscall(__NR_listen, sockfd, backlog);
}

int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    return syscall(__NR_accept, sockfd, addr, addrlen);
}

int _setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    return syscall(__NR_setsockopt, sockfd, level, optname, optval, optlen);
}

void target_cache_flush(void *start, unsigned int length) { // https://github.com/llvm-mirror/compiler-rt/blob/master/lib/builtins/clear_cache.c
#ifdef ARCH_ARM
    syscall(__ARM_NR_cacheflush, start, start + length, 0);
#elif ARCH_MIPS
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
 
void _perror(char *s) {
    int sys_nerr = 35;
    char *sys_errlist[sys_nerr];
    // initialize in-place because of PICness
    sys_errlist[0] = "Invalid error number";
    sys_errlist[EPERM] = "Operation not permitted";
    sys_errlist[ENOENT] = "No such file or directory";
    sys_errlist[ESRCH] = "No such process";
    sys_errlist[EINTR] = "Interrupted system call";
    sys_errlist[EIO] = "I/O error";
    sys_errlist[ENXIO] = "No such device or address";
    sys_errlist[E2BIG] = "Argument list too long";
    sys_errlist[ENOEXEC] = "Exec format error";
    sys_errlist[EBADF] = "Bad file number";
    sys_errlist[ECHILD] = "No child processes";
    sys_errlist[EAGAIN] = "Operation would block";
    sys_errlist[ENOMEM] = "Out of memory";
    sys_errlist[EACCES] = "Permission denied";
    sys_errlist[EFAULT] = "Bad address";
    sys_errlist[ENOTBLK] = "Block device required";
    sys_errlist[EBUSY] = "Device or resource busy";
    sys_errlist[EEXIST] = "File exists";
    sys_errlist[EXDEV] = "Cross-device link";
    sys_errlist[ENODEV] = "No such device";
    sys_errlist[ENOTDIR] = "Not a directory";
    sys_errlist[EISDIR] = "Is a directory";
    sys_errlist[EINVAL] = "Invalid argument";
    sys_errlist[ENFILE] = "File table overflow";
    sys_errlist[EMFILE] = "Too many open files";
    sys_errlist[ENOTTY] = "Not a tty";
    sys_errlist[ETXTBSY] = "Text file busy";
    sys_errlist[EFBIG] = "File too large";
    sys_errlist[ENOSPC] = "No space left on device";
    sys_errlist[ESPIPE] = "Illegal seek";
    sys_errlist[EROFS] = "Read-only file system";
    sys_errlist[EMLINK] = "Too many links";
    sys_errlist[EPIPE] = "Broken pipe";
    sys_errlist[EDOM] = "Argument outside domain";
    sys_errlist[ERANGE] = "Result not representable";

    int idx = errno;
    if (idx >= sys_nerr)
        idx = 0;
    ERROR("%s: %s\n", s, sys_errlist[idx]);
}

