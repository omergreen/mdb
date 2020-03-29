
#define MAX_OPCODE_LENGTH (10)
struct breakpoint_arch_specific {
    unsigned char original_opcode[MAX_OPCODE_LENGTH];
    unsigned char original_opcode_length;
}
