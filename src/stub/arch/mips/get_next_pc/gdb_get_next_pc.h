#pragma once

#include <machine/arch/registers_struct.h>

typedef unsigned long CORE_ADDR; // changed this to long instead of long long since we don't deal with 64 bit
typedef long LONGEST;
typedef unsigned long ULONGEST;
typedef unsigned char gdb_byte;

/* Base and compressed MIPS ISA variations.  */
enum mips_isa
  {
    ISA_MIPS = -1,		/* mips_compression_string depends on it.  */
    ISA_MIPS16,
    ISA_MICROMIPS
  };

// enum bfd_endian { BFD_ENDIAN_BIG, BFD_ENDIAN_LITTLE, BFD_ENDIAN_UNKNOWN };

struct gdbarch {
  // enum bfd_endian byte_order;
};

struct regcache {
  struct registers *regs;
  struct gdbarch *arch;
};

enum
{
  MIPS_INSN16_SIZE = 2,
  MIPS_INSN32_SIZE = 4,
  /* The number of floating-point or integer registers.  */
  MIPS_NUMREGS = 32
};

CORE_ADDR
mips32_next_pc (struct regcache *regcache, CORE_ADDR pc);

