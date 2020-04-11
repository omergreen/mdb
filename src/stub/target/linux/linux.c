/*
 * Implements Linux-related functions
 */

#include <libc/libc.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <core/log.h>
#include <target/interface.h>

#include "linux.h"

#define PAGE_SIZE 4096

struct linux_data g_linux_data;

void target_cleanup() {
    /* close(g_linux_data.log_fd); */
}

static void init_log() {
    g_linux_data.log_fd = 1;
    /* g_linux_data.log_fd = _open("/tmp/stub_log.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666); */
    /* if (g_linux_data.log_fd == -1) { */
    /*     char *d = 0; */
    /*     *d = 1; */
    /* } */
}

void target_log(const char *format, ...) {
    char buffer[1024];

    va_list ap;
    va_start(ap, format);
    vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);

    write(g_linux_data.log_fd, buffer, strlen(buffer) + 1);
}

void target_init(void *args) {
    char *free_space = (char *)mmap2(0, PAGE_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    /* free_space[PAGE_SIZE * 2] = 0; */
    malloc_init();
    init_log();
    add_malloc_block(free_space, PAGE_SIZE);
}

