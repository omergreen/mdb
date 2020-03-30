#include <asm/unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdarg.h>
#include <machine/target/syscall.h>
#include <machine/target/libc.h>

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
