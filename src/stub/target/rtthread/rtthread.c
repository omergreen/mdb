#include <target/interface.h>
#include <core/breakpoint.h>
#include <stdarg.h>
#include <libc/libc.h>

struct target_config g_target_config = { .supports_real_breakpoints = true, .should_override_ivt = true };

int (*serial2_read)(char *buffer, unsigned int length);
int (*serial2_write)(const char *buffer, unsigned int length);
int (*vprintf)(const char *format, va_list args);
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
#ifdef ARCH_ARM
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#elif ARCH_MIPS
    // qemu mipssim only has one serial port so we have to multiplex over it
    va_list args;
    va_start(args, format);
    tfp_vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    target_send(buf, strlen(buf));
#else
#error "not implemented"
#endif
}
void *target_malloc(unsigned int size) {
    return _malloc(size);
}
void *target_realloc(void *ptr, unsigned int size) {
    return _realloc(ptr, size);
}
void target_free(void *addr) {
    _free(addr);
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
    vprintf = args_ptr[2];
    _malloc = args_ptr[3];
    _realloc = args_ptr[4];
    _free = args_ptr[5];

    /* breakpoint_add((unsigned long)args_ptr[6], false, BREAKPOINT_TYPE_SOFTWARE); */

    wait_for_connection();
}

void target_cleanup() {
}

