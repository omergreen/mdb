#include <machine/target/target.h>
#include <machine/target/linux.h>
#include <machine/target/libc.h>
#include <core/ops.h>
#include <libc/malloc.h>
#include <sys/mman.h>
#include <fcntl.h>

#define PAGE_SIZE 4096

struct linux_data g_linux_data;

static void cleanup() {
    _close(g_linux_data.log_fd);
}

static void init_log() {
    /* int fd = _open("/tmp/stub_log.txt", O_WRONLY); */
    g_linux_data.log_fd = _open("/tmp/stub_log.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (g_linux_data.log_fd == -1) {
        char *d = 0;
        *d = 1;
    }
}

static void log(char *data, unsigned int length) {
    _write(g_linux_data.log_fd, (char *)data, length);
}

void target_init(void *args) {
    char *free_space = (char *)mmap2(0, PAGE_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    /* free_space[PAGE_SIZE * 2] = 0; */
    malloc_init();
    init_log();
    add_malloc_block(free_space, PAGE_SIZE);
    g_ops.log = &log;
    g_ops.cleanup = &cleanup;
}

