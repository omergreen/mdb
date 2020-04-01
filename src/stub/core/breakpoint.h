#pragma once

#include <stdbool.h>
#include <machine/arch/breakpoint_arch_specific_struct.h>

enum breakpoint_type {
    USER,
    TEMPORARY
};

struct breakpoint {
    unsigned int address;
    bool enabled;
    enum breakpoint_type type;
    struct breakpoint_arch_specific arch_specific;
};

