#include <asm/unistd.h>
#include <sys/syscall.h>
#include <machine/target/syscall.h>
#include <machine/target/libc.h>

void *mmap2(void *addr, unsigned long len, int prot, int flags, int fildes, signed long off) {
  return syscall(__NR_mmap2, (long)addr, (long)len, (long)prot, (long)flags, (long)fildes, (long)off);
}

int _open(char *fname, int flags) {
  return syscall(__NR_open, fname, flags);
}

unsigned int _write(int fd, char *buf, unsigned int length) {
  return syscall(__NR_write, fd, buf, length);
}

int _close(int fd) {
  return syscall(__NR_close, fd);
}
