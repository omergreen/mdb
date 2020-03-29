#pragma once

struct breakpoint; // define here first to avoid errors due to breakpoint.h and machine breakpoint.h referencing each other

#include <stdbool.h>
#include <machine/arch/breakpoint.h>

enum breakpoint_type {
	USER,
	TEMPORARY
};

#define BREAKPOINT_ORIGINAL_DATA_LENGTH (10)
struct breakpoint {
    unsigned int address;
    bool enabled;
	enum breakpoint_type type;
	struct breakpoint_arch_specific arch_specific;
};

