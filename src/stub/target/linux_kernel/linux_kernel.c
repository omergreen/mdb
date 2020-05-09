#include <target/interface.h>
#include <core/breakpoint.h>
#include <stdarg.h>
#include <libc/libc.h>
#include <libc/malloc.h>

struct target_config g_target_config = { .supports_real_breakpoints = true, .should_override_ivt = true };

int (*serial2_read)(char *buffer, unsigned int length);
int (*serial2_write)(const char *buffer, unsigned int length);
void (*puts)(const char *s);
void *(*_malloc)(unsigned int size);
void *(*_realloc)(void *ptr, unsigned int size);
void (*_free)(void *addr);

int state = 0;
static char first_char;
unsigned int target_recv(char *output, unsigned int length) {
    int got = 0;
    while (length) {
        int bytes = serial2_read(output, length);
        if (bytes <= 0) {
            break;
        }

        got += bytes;
        length -= bytes;
        output += bytes;
    }

    return got;
}
unsigned int target_send(const char *data, unsigned int length) {
    int got = 0;
    while (length) {
        int bytes = serial2_write(data, length);
        if (bytes <= 0) {
            break;
        }

        got += bytes;
        length -= bytes;
        data += bytes;
    }

    return got;
}
void target_log(const char *format, ...) {
    char buf[4096];
    va_list args;
    va_start(args, format);
    tfp_vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    puts(buf);
}
void *target_malloc(unsigned int size) {
    return malloc(size);
}
void *target_realloc(void *ptr, unsigned int size) {
    return realloc(ptr, size);
}
void target_free(void *addr) {
    free(addr);
}

void wait_for_connection() {
    /* while (target_recv(&first_char, 1) == 0) { */
    /* } */
    /* state = 1; */
}

void target_init(void *args) {
    void **args_ptr = args;
    serial2_read = args_ptr[0];
    serial2_write = args_ptr[1];
    puts = args_ptr[2];
    void *(*big_alloc)(size_t) = args_ptr[3];

    /* DEBUG("allocing for malloc"); */
    puts("test\n");
    return;
    malloc_init();
    /* void *malloc_block = big_alloc(0x10000); */
    DEBUG("malloc_block = 0x%08x", 123);
    /* add_malloc_block(malloc_block, 0x10000); */

    /* breakpoint_add((unsigned long)args_ptr[6], false, BREAKPOINT_TYPE_SOFTWARE); */

    wait_for_connection();
}

void target_cleanup() {
}

