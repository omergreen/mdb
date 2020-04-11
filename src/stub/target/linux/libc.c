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
 
char *sys_errlist[] = {
    [0]        = "Invalid error number",
    [EPERM]        = "Operation not permitted",
    [ENOENT]    = "No such file or directory",
    [ESRCH]        = "No such process",
    [EINTR]        = "Interrupted system call",
    [EIO]        = "I/O error",
    [ENXIO]        = "No such device or address",
    [E2BIG]        = "Argument list too long",
    [ENOEXEC]    = "Exec format error",
    [EBADF]        = "Bad file number",
    [ECHILD]    = "No child processes",
    [EAGAIN]    = "Operation would block",
    [ENOMEM]    = "Out of memory",
    [EACCES]    = "Permission denied",
    [EFAULT]    = "Bad address",
    [ENOTBLK]    = "Block device required",
    [EBUSY]        = "Device or resource busy",
    [EEXIST]    = "File exists",
    [EXDEV]        = "Cross-device link",
    [ENODEV]    = "No such device",
    [ENOTDIR]    = "Not a directory",
    [EISDIR]    = "Is a directory",
    [EINVAL]    = "Invalid argument",
    [ENFILE]    = "File table overflow",
    [EMFILE]    = "Too many open files",
    [ENOTTY]    = "Not a tty",
    [ETXTBSY]    = "Text file busy",
    [EFBIG]        = "File too large",
    [ENOSPC]    = "No space left on device",
    [ESPIPE]    = "Illegal seek",
    [EROFS]        = "Read-only file system",
    [EMLINK]    = "Too many links",
    [EPIPE]        = "Broken pipe",
    [EDOM]        = "Argument outside domain",
    [ERANGE]    = "Result not representable",
};
int sys_nerr = sizeof(sys_errlist) / sizeof(sys_errlist[0]);
void _perror(char *s) {
    int idx = errno;
    if (idx >= sys_nerr)
        idx = 0;
    ERROR("%s: %s\n", s, sys_errlist[idx]);
}

