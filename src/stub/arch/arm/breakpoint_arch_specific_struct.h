#pragma once

#include "registers_struct.h"
#define BREAKPOINT_LENGTH (4)

struct breakpoint_arch_specific { 
    void *stub;
    unsigned char original_data[BREAKPOINT_LENGTH];
};

