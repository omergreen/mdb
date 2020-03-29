.extern sw_breakpoint_handler

sw_breakpoint_prologue:
    pushad
	pushfd
    jmp sw_breakpoint_handler

_sw_breakpoint_epilogue:
    popfd
    popad

	.byte 0xff
sw_breakpoint_epilogue_trampoline_address:
	.word 0

