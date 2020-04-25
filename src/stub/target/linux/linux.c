/*
 * Implements Linux-related functions
 */

#include <libc/libc.h>
#include <core/state.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <core/log.h>
#include <target/interface.h>
#include <netinet/tcp.h>

#include "test_app.h"
#include "linux.h"

#define PAGE_SIZE 4096

struct linux_data g_linux_data;
struct target_config g_target_config = { .supports_real_breakpoints = true, .should_override_ivt = false };

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

void init_gdb() {
    g_linux_data.gdb_fd = -1;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("unable to create gdb socket");
        return;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in sa = {.sin_family = AF_INET, .sin_port = htons(1337), .sin_addr = 0};
    if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("unable to bind gdb socket");
        close(sock);
        return;
    }

    if (listen(sock, 1) < 0) {
        perror("unable to listen gdb socket");
        close(sock);
        return;
    }

    socklen_t sa_size = sizeof(sa);
    g_linux_data.gdb_fd = accept(sock, (struct sockaddr *)&sa, &sa_size);
    if (g_linux_data.gdb_fd == -1) {
        perror("unable to accept gdb socket");
        close(sock);
        return;
    }

    setsockopt(g_linux_data.gdb_fd, SOL_TCP, TCP_NODELAY, &(int){1}, sizeof(int));

    DEBUG("got connection from %d.%d.%d.%d:%d", sa.sin_addr.s_addr >> 24, (sa.sin_addr.s_addr >> 16) & 0xff, (sa.sin_addr.s_addr >> 8) & 0xff, sa.sin_addr.s_addr & 0xff, ntohs(sa.sin_port));
}

unsigned int target_recv(char *output, unsigned int length) {
    if (g_linux_data.gdb_fd == -1) {
        ERROR("gdb connection down");
        return -1;
    }

    return read(g_linux_data.gdb_fd, output, length);
}

unsigned int target_send(const char *data, unsigned int length) { 
    if (g_linux_data.gdb_fd == -1) {
        ERROR("gdb connection down");
        return -1;
    }

    return write(g_linux_data.gdb_fd, data, length);
}

void sigaction_handler(int sig, siginfo_t *info, void *ucontext) {
    struct ucontext_t *c = ucontext;

#ifdef ARCH_ARM
    memcpy(&g_state.regs.r0, &c->uc_mcontext.arm_r0, sizeof(g_state.regs));
#else
#error "unimplemented"
#endif

    breakpoint_handler();
}

void target_init(void *args) {
    char *free_space = (char *)mmap2(target_init, PAGE_SIZE, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0); // addr is target_init - small hack to make the heap be close to the code so that the jumps will work
    malloc_init();
    init_log();
    init_gdb();

    add_malloc_block(free_space, PAGE_SIZE);

    struct_sigaction action = { .sa_handler = NULL, .sa_sigaction = sigaction_handler, .sa_flags = SA_SIGINFO };
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGILL, &action, NULL) < 0) {
        perror("sigaction");
    }

    unsigned int bp_address = (unsigned int)test_app;
    breakpoint_add(bp_address, false, BREAKPOINT_TYPE_JUMP_OR_SOFTWARE);

    test_app();
}

