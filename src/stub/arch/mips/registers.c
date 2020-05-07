#include "breakpoint.h"
#include "registers.h"
#include <core/log.h>
#include <libc/libc.h>
#include <core/state.h>

// COLORS, sorry for the mess

#define REGISTER_COLOR KBLU KBOLD
#define COLORIZE_REGISTER_ARG(...) COLORIZE_ARGS(REGISTER_COLOR, ##__VA_ARGS__)

static void print_colored_line(const char *s1, const char *s2, const char *s3, const char *s4, unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4) {
#define STRING_AND_VALUE_FORMAT COLORIZE_FORMAT("%-4s") " 0x%-8x  "

    target_log(STRING_AND_VALUE_FORMAT STRING_AND_VALUE_FORMAT STRING_AND_VALUE_FORMAT STRING_AND_VALUE_FORMAT "\n", 
              COLORIZE_REGISTER_ARG(s1), v1, COLORIZE_REGISTER_ARG(s2), v2, COLORIZE_REGISTER_ARG(s3), v3, COLORIZE_REGISTER_ARG(s4), v4);
}
void registers_print_all() {
}

