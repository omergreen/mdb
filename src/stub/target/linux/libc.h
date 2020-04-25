/*
 * Defines Linux-related libc stuff
 */

#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

extern long errno;
typedef struct sigaction struct_sigaction; // small hack since we #define sigaction as _sigaction

void *mmap2(void *addr, unsigned long len, int prot, int flags, int fildes, signed long off);
int _open(const char *filename, int flags, ...);
ssize_t _read(int fd, void *buf, size_t length);
ssize_t _write(int fd, const void *buf, size_t length);
int _close(int fd);
int _socket(int domain, int type, int protocol);
int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int _listen(int sockfd, int backlog);
int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int _setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int _sigaction(int signum, const struct_sigaction *act, struct_sigaction *oldact);
int _sigemptyset(sigset_t *set);
void _perror(char *s);

void cache_flush_linux(void *start, unsigned int length);

#define open _open
#define read _read
#define write _write
#define close _close
#define socket _socket
#define bind _bind
#define listen _listen
#define accept _accept
#define setsockopt _setsockopt
#define sigaction _sigaction
#define sigemptyset _sigemptyset
#define perror _perror

#define EPERM         1    /* Operation not permitted */
#define ENOENT        2    /* No such file or directory */
#define ESRCH         3    /* No such process */
#define EINTR         4    /* Interrupted system call */
#define EIO           5    /* I/O error */
#define ENXIO         6    /* No such device or address */
#define E2BIG         7    /* Argument list too long */
#define ENOEXEC       8    /* Exec format error */
#define EBADF         9    /* Bad file number */
#define ECHILD        10    /* No child processes */
#define EAGAIN        11    /* Try again */
#define ENOMEM        12    /* Out of memory */
#define EACCES        13    /* Permission denied */
#define EFAULT        14    /* Bad address */
#define ENOTBLK       15    /* Block device required */
#define EBUSY         16    /* Device or resource busy */
#define EEXIST        17    /* File exists */
#define EXDEV         18    /* Cross-device link */
#define ENODEV        19    /* No such device */
#define ENOTDIR       20    /* Not a directory */
#define EISDIR        21    /* Is a directory */
#define EINVAL        22    /* Invalid argument */
#define ENFILE        23    /* File table overflow */
#define EMFILE        24    /* Too many open files */
#define ENOTTY        25    /* Not a typewriter */
#define ETXTBSY       26    /* Text file busy */
#define EFBIG         27    /* File too large */
#define ENOSPC        28    /* No space left on device */
#define ESPIPE        29    /* Illegal seek */
#define EROFS         30    /* Read-only file system */
#define EMLINK        31    /* Too many links */
#define EPIPE         32    /* Broken pipe */
#define EDOM          33    /* Math argument out of domain of func */
#define ERANGE        34    /* Math result not representable */

