#include "breakpoint.h"
#include <stdbool.h>

#define BREAKPOINT_LENGTH (5)

bool sw_breakpoint_enable(struct breakpoint *bp) {
    if (bp->enabled) { // TODO: move into generic sw_breakpoint_enable
        DEBUG("breakpoint already enabled");
        return false;
    }

    memcpy(bp->original_data, bp->address, BREAKPOINT_LENGTH);
    ((unsigned char *)bp->address)[0] = '\xff';
    
    memcpy((unsigned char *)bp->address + 1, breakpoint_prologue, 4);

    cache_flush();

    bp->enabled = true;

    return true;
}

bool sw_breakpoint_disable(struct breakpoint *bp) {
    if (!bp->enabled) { // TODO: move..
        return false;
    }

    memcpy(bp->address, bp->original_data, BREAKPOINT_LENGTH);
    cache_flush();
    bp->enabled = false;

    return true;
}

void sw_breakpoint_epilogue(struct breakpoint *bp) {
	*sw_breakpoint_epilogue_trampoline_address = bp->address;
	cache_flush();
	_sw_breakpoint_epilogue();
}

