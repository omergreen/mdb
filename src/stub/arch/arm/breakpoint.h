#pragma once

#include <stdbool.h>

struct breakpoint_arch_specific { // define here to avoid errors due to breakpoint.h and machine breakpoint.h referencing each other
    void *stub;
    unsigned char original_data[4];
};

#include <core/breakpoint.h>

bool jump_breakpoint_put(struct breakpoint *bp);
bool jump_breakpoint_disable(struct breakpoint *bp);


