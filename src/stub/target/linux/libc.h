/*
 * Defines Linux-related libc stuff
 */

void *mmap2(void *addr, unsigned long len, int prot, int flags, int fildes, signed long off);
int _open(const char *filename, int flags, ...);
unsigned int _write(int fd, char *buf, unsigned int length);
int _close(int fd);
void cache_flush_linux(void *start, unsigned int length);

#define open _open
#define write _write
#define close _close

