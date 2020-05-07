/* Target-dependent code for the MIPS architecture, for GDB, the GNU Debugger.

   Copyright (C) 1988-2020 Free Software Foundation, Inc.

   Contributed by Alessandro Forin(af@cs.cmu.edu) at CMU
   and by Per Bothner(bothner@cs.wisc.edu) at U.Wisconsin.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "gdb_get_next_pc.h"
#include <libc/libc.h>

/* These are the fields of 32 bit mips instructions.  */
#define mips32_op(x) (x >> 26)
#define itype_op(x) (x >> 26)
#define itype_rs(x) ((x >> 21) & 0x1f)
#define itype_rt(x) ((x >> 16) & 0x1f)
#define itype_immediate(x) (x & 0xffff)

#define jtype_op(x) (x >> 26)
#define jtype_target(x) (x & 0x03ffffff)

#define rtype_op(x) (x >> 26)
#define rtype_rs(x) ((x >> 21) & 0x1f)
#define rtype_rt(x) ((x >> 16) & 0x1f)
#define rtype_rd(x) ((x >> 11) & 0x1f)
#define rtype_shamt(x) ((x >> 6) & 0x1f)
#define rtype_funct(x) (x & 0x3f)

/* MicroMIPS instruction fields.  */
#define micromips_op(x) ((x) >> 10)

/* 16-bit/32-bit-high-part instruction formats, B and S refer to the lowest
   bit and the size respectively of the field extracted.  */
#define b0s4_imm(x) ((x) & 0xf)
#define b0s5_imm(x) ((x) & 0x1f)
#define b0s5_reg(x) ((x) & 0x1f)
#define b0s7_imm(x) ((x) & 0x7f)
#define b0s10_imm(x) ((x) & 0x3ff)
#define b1s4_imm(x) (((x) >> 1) & 0xf)
#define b1s9_imm(x) (((x) >> 1) & 0x1ff)
#define b2s3_cc(x) (((x) >> 2) & 0x7)
#define b4s2_regl(x) (((x) >> 4) & 0x3)
#define b5s5_op(x) (((x) >> 5) & 0x1f)
#define b5s5_reg(x) (((x) >> 5) & 0x1f)
#define b6s4_op(x) (((x) >> 6) & 0xf)
#define b7s3_reg(x) (((x) >> 7) & 0x7)

/* 32-bit instruction formats, B and S refer to the lowest bit and the size
   respectively of the field extracted.  */
#define b0s6_op(x) ((x) & 0x3f)
#define b0s11_op(x) ((x) & 0x7ff)
#define b0s12_imm(x) ((x) & 0xfff)
#define b0s16_imm(x) ((x) & 0xffff)
#define b0s26_imm(x) ((x) & 0x3ffffff)
#define b6s10_ext(x) (((x) >> 6) & 0x3ff)
#define b11s5_reg(x) (((x) >> 11) & 0x1f)
#define b12s4_op(x) (((x) >> 12) & 0xf)

/* Table to translate 3-bit register field to actual register number.  */
static const signed char mips_reg3_to_reg[8] = { 16, 17, 2, 3, 4, 5, 6, 7 };

/* Decoding the next place to set a breakpoint is irregular for the
   mips 16 variant, but fortunately, there fewer instructions.  We have
   to cope ith extensions for 16 bit instructions and a pair of actual
   32 bit instructions.  We dont want to set a single step instruction
   on the extend instruction either.  */

/* Lots of mips16 instruction formats */
/* Predicting jumps requires itype,ritype,i8type
   and their extensions      extItype,extritype,extI8type.  */
enum mips16_inst_fmts
{
  itype,			/* 0  immediate 5,10 */
  ritype,			/* 1   5,3,8 */
  rrtype,			/* 2   5,3,3,5 */
  rritype,			/* 3   5,3,3,5 */
  rrrtype,			/* 4   5,3,3,3,2 */
  rriatype,			/* 5   5,3,3,1,4 */
  shifttype,			/* 6   5,3,3,3,2 */
  i8type,			/* 7   5,3,8 */
  i8movtype,			/* 8   5,3,3,5 */
  i8mov32rtype,			/* 9   5,3,5,3 */
  i64type,			/* 10  5,3,8 */
  ri64type,			/* 11  5,3,3,5 */
  jalxtype,			/* 12  5,1,5,5,16 - a 32 bit instruction */
  exiItype,			/* 13  5,6,5,5,1,1,1,1,1,1,5 */
  extRitype,			/* 14  5,6,5,5,3,1,1,1,5 */
  extRRItype,			/* 15  5,5,5,5,3,3,5 */
  extRRIAtype,			/* 16  5,7,4,5,3,3,1,4 */
  EXTshifttype,			/* 17  5,5,1,1,1,1,1,1,5,3,3,1,1,1,2 */
  extI8type,			/* 18  5,6,5,5,3,1,1,1,5 */
  extI64type,			/* 19  5,6,5,5,3,1,1,1,5 */
  extRi64type,			/* 20  5,6,5,5,3,3,5 */
  extshift64type		/* 21  5,5,1,1,1,1,1,1,5,1,1,1,3,5 */
};
/* I am heaping all the fields of the formats into one structure and
   then, only the fields which are involved in instruction extension.  */
struct upk_mips16
{
  CORE_ADDR offset;
  unsigned int regx;		/* Function in i8 type.  */
  unsigned int regy;
};

static LONGEST
mips32_relative_offset (ULONGEST inst)
{
  return ((itype_immediate (inst) ^ 0x8000) - 0x8000) << 2;
}

LONGEST
regcache_raw_get_signed (struct regcache *regcache, int regnum) {
  return ((unsigned int*)regcache->regs)[regnum];
}

/* Return true if the OP represents the Octeon's BBIT instruction.  */
static int
is_octeon_bbit_op (int op, struct gdbarch *gdbarch)
{
  /* if (!is_octeon (gdbarch)) */
  /*   return 0; */
  /* BBIT0 is encoded as LWC2: 110 010.  */
  /* BBIT032 is encoded as LDC2: 110 110.  */
  /* BBIT1 is encoded as SWC2: 111 010.  */
  /* BBIT132 is encoded as SDC2: 111 110.  */
  if (op == 50 || op == 54 || op == 58 || op == 62)
    return 1;
  return 0;
}

// TODO floating point support
/* Determine the address of the next instruction executed after the INST
   floating condition branch instruction at PC.  COUNT specifies the
   number of the floating condition bits tested by the branch.  */

/* static CORE_ADDR */
/* mips32_bc1_pc (struct gdbarch *gdbarch, struct regcache *regcache, */
/* 	       ULONGEST inst, CORE_ADDR pc, int count) */
/* { */
/*   int fcsr = mips_regnum (gdbarch)->fp_control_status; */
/*   int cnum = (itype_rt (inst) >> 2) & (count - 1); */
/*   int tf = itype_rt (inst) & 1; */
/*   int mask = (1 << count) - 1; */
/*   ULONGEST fcs; */
/*   int cond; */
/*  */
/*   if (fcsr == -1) */
/*     #<{(| No way to handle; it'll most likely trap anyway.  |)}># */
/*     return pc; */
/*  */
/*   fcs = regcache_raw_get_unsigned (regcache, fcsr); */
/*   cond = ((fcs >> 24) & 0xfe) | ((fcs >> 23) & 0x01); */
/*  */
/*   if (((cond >> cnum) & mask) != mask * !tf) */
/*     pc += mips32_relative_offset (inst); */
/*   else */
/*     pc += 4; */
/*  */
/*   return pc; */
/* } */

/* ULONGEST */
/* extract_unsigned_integer (const gdb_byte *addr, int len, enum bfd_endian byte_order) */
/* { */
/*   ULONGEST retval = 0; */
/*   const unsigned char *p; */
/*   const unsigned char *startaddr = addr; */
/*   const unsigned char *endaddr = startaddr + len; */
/*  */
/*   #<{(| Start at the most significant end of the integer, and work towards */
/*      the least significant.  |)}># */
/*   if (byte_order == BFD_ENDIAN_BIG) */
/*     { */
/*       p = startaddr; */
/*       for (; p < endaddr; ++p) */
/* 	retval = (retval << 8) | *p; */
/*     } */
/*   else */
/*     { */
/*       p = endaddr - 1; */
/*       for (; p >= startaddr; --p) */
/* 	retval = (retval << 8) | *p; */
/*     } */
/*   return retval; */
/* } */

static CORE_ADDR
unmake_compact_addr (CORE_ADDR addr)
{
  return ((addr) & ~(CORE_ADDR) 1);
}

static ULONGEST
mips_fetch_instruction (struct gdbarch *gdbarch,
			enum mips_isa isa, CORE_ADDR addr, int *errp)
{
  /* enum bfd_endian byte_order = gdbarch_byte_order (gdbarch); */
  gdb_byte buf[MIPS_INSN32_SIZE];
  int instlen;
  int err;

  switch (isa)
    {
    case ISA_MICROMIPS:
    case ISA_MIPS16:
      instlen = MIPS_INSN16_SIZE;
      addr = unmake_compact_addr (addr);
      return (ULONGEST)(*(unsigned short *)addr);
      break;
    case ISA_MIPS:
      instlen = MIPS_INSN32_SIZE;
      return (ULONGEST)(*(unsigned int *)addr);
      break;
    default:
      break;
    }
  /* memcpy ((void *)addr, buf, instlen); */
  /* return extract_unsigned_integer (buf, instlen, byte_order); */
}

/* Only call this function if you know that this is an extendable
   instruction.  It won't malfunction, but why make excess remote memory
   references?  If the immediate operands get sign extended or something,
   do it after the extension is performed.  */
/* FIXME: Every one of these cases needs to worry about sign extension
   when the offset is to be used in relative addressing.  */

static unsigned int
fetch_mips_16 (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  /* enum bfd_endian byte_order = gdbarch_byte_order (gdbarch); */
  /* gdb_byte buf[8]; */

  pc = unmake_compact_addr (pc);	/* Clear the low order bit.  */
  return (unsigned int)(*(unsigned short *)pc);
  /* target_read_memory (pc, buf, 2); */
  /* return extract_unsigned_integer (buf, 2, byte_order); */
}

/* The EXT-I, EXT-ri nad EXT-I8 instructions all have the same format
   for the bits which make up the immediate extension.  */

static CORE_ADDR
extended_offset (unsigned int extension)
{
  CORE_ADDR value;

  value = (extension >> 16) & 0x1f;	/* Extract 15:11.  */
  value = value << 6;
  value |= (extension >> 21) & 0x3f;	/* Extract 10:5.  */
  value = value << 5;
  value |= extension & 0x1f;		/* Extract 4:0.  */

  return value;
}

/* Calculate the destination of a branch whose 16-bit opcode word is at PC,
   and having a signed 16-bit OFFSET.  */

static CORE_ADDR
add_offset_16 (CORE_ADDR pc, int offset)
{
  return pc + (offset << 1) + 2;
}

static void
unpack_mips16 (struct gdbarch *gdbarch, CORE_ADDR pc,
	       unsigned int extension,
	       unsigned int inst,
	       enum mips16_inst_fmts insn_format, struct upk_mips16 *upk)
{
  CORE_ADDR offset;
  int regx;
  int regy;
  switch (insn_format)
  {
    case itype:
      {
        CORE_ADDR value;
        if (extension)
        {
          value = extended_offset ((extension << 16) | inst);
          value = (value ^ 0x8000) - 0x8000;		/* Sign-extend.  */
        }
        else
        {
          value = inst & 0x7ff;
          value = (value ^ 0x400) - 0x400;		/* Sign-extend.  */
        }
        offset = value;
        regx = -1;
        regy = -1;
      }
      break;
    case ritype:
    case i8type:
      {				/* A register identifier and an offset.  */
        /* Most of the fields are the same as I type but the
           immediate value is of a different length.  */
        CORE_ADDR value;
        if (extension)
        {
          value = extended_offset ((extension << 16) | inst);
          value = (value ^ 0x8000) - 0x8000;		/* Sign-extend.  */
        }
        else
        {
          value = inst & 0xff;			/* 8 bits */
          value = (value ^ 0x80) - 0x80;		/* Sign-extend.  */
        }
        offset = value;
        regx = (inst >> 8) & 0x07;			/* i8 funct */
        regy = -1;
        break;
      }
    case jalxtype:
      {
        unsigned long value;
        unsigned int nexthalf;
        value = ((inst & 0x1f) << 5) | ((inst >> 5) & 0x1f);
        value = value << 16;
        nexthalf = mips_fetch_instruction (gdbarch, ISA_MIPS16, pc + 2, NULL);
        /* Low bit still set.  */
        value |= nexthalf;
        offset = value;
        regx = -1;
        regy = -1;
        break;
      }
    default:
      break;
  }
  upk->offset = offset;
  upk->regx = regx;
  upk->regy = regy;
}

/* enum bfd_endian */
/* gdbarch_byte_order (struct gdbarch *gdbarch) */
/* { */
/*   return gdbarch->byte_order; */
/* } */

CORE_ADDR
mips32_next_pc (struct regcache *regcache, CORE_ADDR pc)
{
  struct gdbarch *gdbarch = regcache->arch;
  unsigned long inst;
  int op;
  inst = mips_fetch_instruction (gdbarch, ISA_MIPS, pc, NULL);
  op = itype_op (inst);
  if ((inst & 0xe0000000) != 0)		/* Not a special, jump or branch
                                     instruction.  */
  {
    if (op >> 2 == 5)
      /* BEQL, BNEL, BLEZL, BGTZL: bits 0101xx */
    {
      switch (op & 0x03)
      {
        case 0:		/* BEQL */
          goto equal_branch;
        case 1:		/* BNEL */
          goto neq_branch;
        case 2:		/* BLEZL */
          goto less_branch;
        case 3:		/* BGTZL */
          goto greater_branch;
        default:
          pc += 4;
      }
    }
    // TODO floating point support
    /* else if (op == 17 && itype_rs (inst) == 8) */
      /* BC1F, BC1FL, BC1T, BC1TL: 010001 01000 */
    /*   pc = mips32_bc1_pc (gdbarch, regcache, inst, pc + 4, 1); */
    /* else if (op == 17 && itype_rs (inst) == 9 */
    /*     && (itype_rt (inst) & 2) == 0) */
      /* BC1ANY2F, BC1ANY2T: 010001 01001 xxx0x */
      /* pc = mips32_bc1_pc (gdbarch, regcache, inst, pc + 4, 2); */
    /* else if (op == 17 && itype_rs (inst) == 10 */
    /*     && (itype_rt (inst) & 2) == 0) */
      /* BC1ANY4F, BC1ANY4T: 010001 01010 xxx0x */
      /* pc = mips32_bc1_pc (gdbarch, regcache, inst, pc + 4, 4); */
    else if (op == 29)
      /* JALX: 011101 */
      /* The new PC will be alternate mode.  */
    {
      unsigned long reg;

      reg = jtype_target (inst) << 2;
      /* Add 1 to indicate 16-bit mode -- invert ISA mode.  */
      pc = ((pc + 4) & ~(CORE_ADDR) 0x0fffffff) + reg + 1;
    }
    else if (is_octeon_bbit_op (op, gdbarch))
    {
      int bit, branch_if;

      branch_if = op == 58 || op == 62;
      bit = itype_rt (inst);

      /* Take into account the *32 instructions.  */
      if (op == 54 || op == 62)
        bit += 32;

      if (((regcache_raw_get_signed (regcache,
                itype_rs (inst)) >> bit) & 1)
          == branch_if)
        pc += mips32_relative_offset (inst) + 4;
      else
        pc += 8;        /* After the delay slot.  */
    }

    else
      pc += 4;		/* Not a branch, next instruction is easy.  */
  }
  else
  {				/* This gets way messy.  */

    /* Further subdivide into SPECIAL, REGIMM and other.  */
    switch (op & 0x07)	/* Extract bits 28,27,26.  */
    {
      case 0:		/* SPECIAL */
        op = rtype_funct (inst);
        switch (op)
        {
          case 8:		/* JR */
          case 9:		/* JALR */
            /* Set PC to that address.  */
            pc = regcache_raw_get_signed (regcache, rtype_rs (inst));
            break;
          // TODO syscall
          /* case 12:            #<{(| SYSCALL |)}># */
          /*   { */
          /*     struct gdbarch_tdep *tdep; */
          /*  */
          /*     tdep = gdbarch_tdep (gdbarch); */
          /*     if (tdep->syscall_next_pc != NULL) */
          /*       pc = tdep->syscall_next_pc (get_current_frame ()); */
          /*     else */
          /*       pc += 4; */
          /*   } */
          /*   break; */
          default:
            pc += 4;
        }

        break;		/* end SPECIAL */
      case 1:			/* REGIMM */
        {
          op = itype_rt (inst);	/* branch condition */
          switch (op)
          {
            case 0:		/* BLTZ */
            case 2:		/* BLTZL */
            case 16:		/* BLTZAL */
            case 18:		/* BLTZALL */
less_branch:
              if (regcache_raw_get_signed (regcache, itype_rs (inst)) < 0)
                pc += mips32_relative_offset (inst) + 4;
              else
                pc += 8;	/* after the delay slot */
              break;
            case 1:		/* BGEZ */
            case 3:		/* BGEZL */
            case 17:		/* BGEZAL */
            case 19:		/* BGEZALL */
              if (regcache_raw_get_signed (regcache, itype_rs (inst)) >= 0)
                pc += mips32_relative_offset (inst) + 4;
              else
                pc += 8;	/* after the delay slot */
              break;
            // TODO wtf is this?
            /* case 0x1c:	#<{(| BPOSGE32 |)}># */
            /* case 0x1e:	#<{(| BPOSGE64 |)}># */
            /*   pc += 4; */
            /*   if (itype_rs (inst) == 0) */
            /*   { */
            /*     unsigned int pos = (op & 2) ? 64 : 32; */
            /*     int dspctl = mips_regnum (gdbarch)->dspctl; */
            /*  */
            /*     if (dspctl == -1) */
            /*       #<{(| No way to handle; it'll most likely trap anyway.  |)}># */
            /*       break; */
            /*  */
            /*     if ((regcache_raw_get_unsigned (regcache, */
            /*             dspctl) & 0x7f) >= pos) */
            /*       pc += mips32_relative_offset (inst); */
            /*     else */
            /*       pc += 4; */
            /*   } */
              break;
              /* All of the other instructions in the REGIMM category */
            default:
              pc += 4;
          }
        }
        break;		/* end REGIMM */
      case 2:		/* J */
      case 3:		/* JAL */
        {
          unsigned long reg;
          reg = jtype_target (inst) << 2;
          /* Upper four bits get never changed...  */
          pc = reg + ((pc + 4) & ~(CORE_ADDR) 0x0fffffff);
        }
        break;
      case 4:		/* BEQ, BEQL */
equal_branch:
        if (regcache_raw_get_signed (regcache, itype_rs (inst)) ==
            regcache_raw_get_signed (regcache, itype_rt (inst)))
          pc += mips32_relative_offset (inst) + 4;
        else
          pc += 8;
        break;
      case 5:		/* BNE, BNEL */
neq_branch:
        if (regcache_raw_get_signed (regcache, itype_rs (inst)) !=
            regcache_raw_get_signed (regcache, itype_rt (inst)))
          pc += mips32_relative_offset (inst) + 4;
        else
          pc += 8;
        break;
      case 6:		/* BLEZ, BLEZL */
        if (regcache_raw_get_signed (regcache, itype_rs (inst)) <= 0)
          pc += mips32_relative_offset (inst) + 4;
        else
          pc += 8;
        break;
      case 7:
      default:
greater_branch:	/* BGTZ, BGTZL */
        if (regcache_raw_get_signed (regcache, itype_rs (inst)) > 0)
          pc += mips32_relative_offset (inst) + 4;
        else
          pc += 8;
        break;
    }			/* switch */
  }				/* else */
  return pc;
}				/* mips32_next_pc */

static CORE_ADDR
extended_mips16_next_pc (struct regcache *regcache, CORE_ADDR pc,
			 unsigned int extension, unsigned int insn)
{
  struct gdbarch *gdbarch = regcache->arch;
  int op = (insn >> 11);
  switch (op)
  {
    case 2:			/* Branch */
      {
        struct upk_mips16 upk;
        unpack_mips16 (gdbarch, pc, extension, insn, itype, &upk);
        pc = add_offset_16 (pc, upk.offset);
        break;
      }
    case 3:			/* JAL , JALX - Watch out, these are 32 bit
                   instructions.  */
      {
        struct upk_mips16 upk;
        unpack_mips16 (gdbarch, pc, extension, insn, jalxtype, &upk);
        pc = ((pc + 2) & (~(CORE_ADDR) 0x0fffffff)) | (upk.offset << 2);
        if ((insn >> 10) & 0x01)	/* Exchange mode */
          pc = pc & ~0x01;	/* Clear low bit, indicate 32 bit mode.  */
        else
          pc |= 0x01;
        break;
      }
    case 4:			/* beqz */
      {
        struct upk_mips16 upk;
        int reg;
        unpack_mips16 (gdbarch, pc, extension, insn, ritype, &upk);
        reg = regcache_raw_get_signed (regcache, mips_reg3_to_reg[upk.regx]);
        if (reg == 0)
          pc = add_offset_16 (pc, upk.offset);
        else
          pc += 2;
        break;
      }
    case 5:			/* bnez */
      {
        struct upk_mips16 upk;
        int reg;
        unpack_mips16 (gdbarch, pc, extension, insn, ritype, &upk);
        reg = regcache_raw_get_signed (regcache, mips_reg3_to_reg[upk.regx]);
        if (reg != 0)
          pc = add_offset_16 (pc, upk.offset);
        else
          pc += 2;
        break;
      }
    case 12:			/* I8 Formats btez btnez */
      {
        struct upk_mips16 upk;
        int reg;
        unpack_mips16 (gdbarch, pc, extension, insn, i8type, &upk);
        /* upk.regx contains the opcode */
        /* Test register is 24 */
        reg = regcache_raw_get_signed (regcache, 24);
        if (((upk.regx == 0) && (reg == 0))	/* BTEZ */
            || ((upk.regx == 1) && (reg != 0)))	/* BTNEZ */
          pc = add_offset_16 (pc, upk.offset);
        else
          pc += 2;
        break;
      }
    case 29:			/* RR Formats JR, JALR, JALR-RA */
      {
        struct upk_mips16 upk;
        /* upk.fmt = rrtype; */
        op = insn & 0x1f;
        if (op == 0)
        {
          int reg;
          upk.regx = (insn >> 8) & 0x07;
          upk.regy = (insn >> 5) & 0x07;
          if ((upk.regy & 1) == 0)
            reg = mips_reg3_to_reg[upk.regx];
          else
            reg = 31;		/* Function return instruction.  */
          pc = regcache_raw_get_signed (regcache, reg);
        }
        else
          pc += 2;
        break;
      }
    case 30:
      /* This is an instruction extension.  Fetch the real instruction
         (which follows the extension) and decode things based on
         that.  */
      {
        pc += 2;
        pc = extended_mips16_next_pc (regcache, pc, insn,
            fetch_mips_16 (gdbarch, pc));
        break;
      }
    default:
      {
        pc += 2;
        break;
      }
  }
  return pc;
}

static CORE_ADDR
mips16_next_pc (struct regcache *regcache, CORE_ADDR pc)
{
  struct gdbarch *gdbarch = regcache->arch;
  unsigned int insn = fetch_mips_16 (gdbarch, pc);
  return extended_mips16_next_pc (regcache, pc, 0, insn);
}

