#include <asm/unistd.h>
#include <sys/syscall.h>
#include <machine/target/syscall.h>
#include <machine/target/libc.h>

void *mmap2(void *addr, unsigned long len, int prot, int flags, int fildes, signed long off) {
  return syscall(__NR_mmap2, (long)addr, (long)len, (long)prot, (long)flags, (long)fildes, (long)off);
}
