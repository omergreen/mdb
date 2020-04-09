/*
 * This (and gdb_get_next_pc.h) was taken almost directly from GDB's source code.
 * I wrote our own interface to that in get_next_pc.c
 */

/* Common code for ARM software single stepping support.

   Copyright (C) 1988-2020 Free Software Foundation, Inc.

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

#include "get_next_pc.h"
#include <libc/cvector.h>

static int
count_one_bits (unsigned long val)
{
	int nbits;
	for (nbits = 0; val != 0; nbits++)
		val &= val - 1;     /* Delete rightmost 1-bit in val.  */
	return nbits;
}

/* Support routines for instruction parsing.  */
#define submask(x) ((1L << ((x) + 1)) - 1)
#define bits(obj,st,fn) (((obj) >> (st)) & submask ((fn) - (st)))
#define bit(obj,st) (((obj) >> (st)) & 1)
#define sbits(obj,st,fn) \
  ((long) (bits(obj,st,fn) | ((long) bit(obj,fn) * ~ submask (fn - st))))
#define BranchDest(addr,instr) \
  ((CORE_ADDR) (((unsigned long) (addr)) + 8 + (sbits (instr, 0, 23) << 2)))

enum gdb_regnum {
  ARM_A1_REGNUM = 0,		/* first integer-like argument */
  ARM_A4_REGNUM = 3,		/* last integer-like argument */
  ARM_AP_REGNUM = 11,
  ARM_IP_REGNUM = 12,
  ARM_SP_REGNUM = 13,		/* Contains address of top of stack */
  ARM_LR_REGNUM = 14,		/* address to return to from a function call */
  ARM_PC_REGNUM = 15,		/* Contains program counter */
  /* F0..F7 are the fp registers for the (obsolete) FPA architecture.  */
  ARM_F0_REGNUM = 16,		/* first floating point register */
  ARM_F3_REGNUM = 19,		/* last floating point argument register */
  ARM_F7_REGNUM = 23, 		/* last floating point register */
  ARM_FPS_REGNUM = 24,		/* floating point status register */
  ARM_PS_REGNUM = 25,		/* Contains processor status */
  ARM_WR0_REGNUM,		/* WMMX data registers.  */
  ARM_WR15_REGNUM = ARM_WR0_REGNUM + 15,
  ARM_WC0_REGNUM,		/* WMMX control registers.  */
  ARM_WCSSF_REGNUM = ARM_WC0_REGNUM + 2,
  ARM_WCASF_REGNUM = ARM_WC0_REGNUM + 3,
  ARM_WC7_REGNUM = ARM_WC0_REGNUM + 7,
  ARM_WCGR0_REGNUM,		/* WMMX general purpose registers.  */
  ARM_WCGR3_REGNUM = ARM_WCGR0_REGNUM + 3,
  ARM_WCGR7_REGNUM = ARM_WCGR0_REGNUM + 7,
  ARM_D0_REGNUM,		/* VFP double-precision registers.  */
  ARM_D31_REGNUM = ARM_D0_REGNUM + 31,
  ARM_FPSCR_REGNUM,

  ARM_NUM_REGS,

  /* Other useful registers.  */
  ARM_FP_REGNUM = 11,		/* Frame register in ARM code, if used.  */
  THUMB_FP_REGNUM = 7,		/* Frame register in Thumb code, if used.  */
  ARM_NUM_ARG_REGS = 4, 
  ARM_LAST_ARG_REGNUM = ARM_A4_REGNUM,
  ARM_NUM_FP_ARG_REGS = 4,
  ARM_LAST_FP_ARG_REGNUM = ARM_F3_REGNUM
};

/* Enum describing the different kinds of breakpoints.  */
enum arm_breakpoint_kinds
{
   ARM_BP_KIND_THUMB = 2,
   ARM_BP_KIND_THUMB2 = 3,
   ARM_BP_KIND_ARM = 4,
};

/* Supported Arm FP hardware types.  */
enum arm_fp_type {
   ARM_FP_TYPE_NONE = 0,
   ARM_FP_TYPE_VFPV2,
   ARM_FP_TYPE_VFPV3,
   ARM_FP_TYPE_IWMMXT,
   ARM_FP_TYPE_INVALID
};

/* Supported M-profile Arm types.  */
enum arm_m_profile_type {
   ARM_M_TYPE_M_PROFILE,
   ARM_M_TYPE_VFP_D16,
   ARM_M_TYPE_WITH_FPA,
   ARM_M_TYPE_INVALID
};

/* Instruction condition field values.  */
#define INST_EQ		0x0
#define INST_NE		0x1
#define INST_CS		0x2
#define INST_CC		0x3
#define INST_MI		0x4
#define INST_PL		0x5
#define INST_VS		0x6
#define INST_VC		0x7
#define INST_HI		0x8
#define INST_LS		0x9
#define INST_GE		0xa
#define INST_LT		0xb
#define INST_GT		0xc
#define INST_LE		0xd
#define INST_AL		0xe
#define INST_NV		0xf

#define FLAG_N		0x80000000
#define FLAG_Z		0x40000000
#define FLAG_C		0x20000000
#define FLAG_V		0x10000000

#define CPSR_T		0x20

#define XPSR_T		0x01000000

/* Size of registers.  */

#define ARM_INT_REGISTER_SIZE		4
/* IEEE extended doubles are 80 bits.  DWORD aligned they use 96 bits.  */
#define ARM_FP_REGISTER_SIZE		12
#define ARM_VFP_REGISTER_SIZE		8
#define IWMMXT_VEC_REGISTER_SIZE	8

/* Size of register sets.  */

/* r0-r12,sp,lr,pc,cpsr.  */
#define ARM_CORE_REGS_SIZE (17 * ARM_INT_REGISTER_SIZE)
/* f0-f8,fps.  */
#define ARM_FP_REGS_SIZE (8 * ARM_FP_REGISTER_SIZE + ARM_INT_REGISTER_SIZE)
/* d0-d15,fpscr.  */
#define ARM_VFP2_REGS_SIZE (16 * ARM_VFP_REGISTER_SIZE + ARM_INT_REGISTER_SIZE)
/* d0-d31,fpscr.  */
#define ARM_VFP3_REGS_SIZE (32 * ARM_VFP_REGISTER_SIZE + ARM_INT_REGISTER_SIZE)
/* wR0-wR15,fpscr.  */
#define IWMMXT_REGS_SIZE (16 * IWMMXT_VEC_REGISTER_SIZE \
			  + 6 * ARM_INT_REGISTER_SIZE)

/* See arm-get-next-pcs.h.  */
int
thumb_insn_size (unsigned short inst1)
{
  if ((inst1 & 0xe000) == 0xe000 && (inst1 & 0x1800) != 0)
    return 4;
  else
    return 2;
}

/* See arm.h.  */

int
condition_true (unsigned long cond, unsigned long status_reg)
{
  if (cond == INST_AL || cond == INST_NV)
    return 1;

  switch (cond)
    {
    case INST_EQ:
      return ((status_reg & FLAG_Z) != 0);
    case INST_NE:
      return ((status_reg & FLAG_Z) == 0);
    case INST_CS:
      return ((status_reg & FLAG_C) != 0);
    case INST_CC:
      return ((status_reg & FLAG_C) == 0);
    case INST_MI:
      return ((status_reg & FLAG_N) != 0);
    case INST_PL:
      return ((status_reg & FLAG_N) == 0);
    case INST_VS:
      return ((status_reg & FLAG_V) != 0);
    case INST_VC:
      return ((status_reg & FLAG_V) == 0);
    case INST_HI:
      return ((status_reg & (FLAG_C | FLAG_Z)) == FLAG_C);
    case INST_LS:
      return ((status_reg & (FLAG_C | FLAG_Z)) != FLAG_C);
    case INST_GE:
      return (((status_reg & FLAG_N) == 0) == ((status_reg & FLAG_V) == 0));
    case INST_LT:
      return (((status_reg & FLAG_N) == 0) != ((status_reg & FLAG_V) == 0));
    case INST_GT:
      return (((status_reg & FLAG_Z) == 0)
	      && (((status_reg & FLAG_N) == 0)
		  == ((status_reg & FLAG_V) == 0)));
    case INST_LE:
      return (((status_reg & FLAG_Z) != 0)
	      || (((status_reg & FLAG_N) == 0)
		  != ((status_reg & FLAG_V) == 0)));
    }
  return 1;
}


/* See arm.h.  */

int
thumb_advance_itstate (unsigned int itstate)
{
  /* Preserve IT[7:5], the first three bits of the condition.  Shift
     the upcoming condition flags left by one bit.  */
  itstate = (itstate & 0xe0) | ((itstate << 1) & 0x1f);

  /* If we have finished the IT block, clear the state.  */
  if ((itstate & 0x0f) == 0)
    itstate = 0;

  return itstate;
}

/* See arm.h.  */

int
arm_instruction_changes_pc (unsigned int this_instr)
{
  if (bits (this_instr, 28, 31) == INST_NV)
    /* Unconditional instructions.  */
    switch (bits (this_instr, 24, 27))
      {
      case 0xa:
      case 0xb:
	/* Branch with Link and change to Thumb.  */
	return 1;
      case 0xc:
      case 0xd:
      case 0xe:
	/* Coprocessor register transfer.  */
        if (bits (this_instr, 12, 15) == 15)
	  ERROR("Invalid update to pc in instruction");
	return 0;
      default:
	return 0;
      }
  else
    switch (bits (this_instr, 25, 27))
      {
      case 0x0:
	if (bits (this_instr, 23, 24) == 2 && bit (this_instr, 20) == 0)
	  {
	    /* Multiplies and extra load/stores.  */
	    if (bit (this_instr, 4) == 1 && bit (this_instr, 7) == 1)
	      /* Neither multiplies nor extension load/stores are allowed
		 to modify PC.  */
	      return 0;

	    /* Otherwise, miscellaneous instructions.  */

	    /* BX <reg>, BXJ <reg>, BLX <reg> */
	    if (bits (this_instr, 4, 27) == 0x12fff1
		|| bits (this_instr, 4, 27) == 0x12fff2
		|| bits (this_instr, 4, 27) == 0x12fff3)
	      return 1;

	    /* Other miscellaneous instructions are unpredictable if they
	       modify PC.  */
	    return 0;
	  }
	/* Data processing instruction.  */
	/* Fall through.  */

      case 0x1:
	if (bits (this_instr, 12, 15) == 15)
	  return 1;
	else
	  return 0;

      case 0x2:
      case 0x3:
	/* Media instructions and architecturally undefined instructions.  */
	if (bits (this_instr, 25, 27) == 3 && bit (this_instr, 4) == 1)
	  return 0;

	/* Stores.  */
	if (bit (this_instr, 20) == 0)
	  return 0;

	/* Loads.  */
	if (bits (this_instr, 12, 15) == ARM_PC_REGNUM)
	  return 1;
	else
	  return 0;

      case 0x4:
	/* Load/store multiple.  */
	if (bit (this_instr, 20) == 1 && bit (this_instr, 15) == 1)
	  return 1;
	else
	  return 0;

      case 0x5:
	/* Branch and branch with link.  */
	return 1;

      case 0x6:
      case 0x7:
	/* Coprocessor transfers or SWIs can not affect PC.  */
	return 0;

      default:
    ERROR("bad value in switch");
    return 0;
      }
}

/* See arm.h.  */

int
thumb_instruction_changes_pc (unsigned short inst)
{
  if ((inst & 0xff00) == 0xbd00)	/* pop {rlist, pc} */
    return 1;

  if ((inst & 0xf000) == 0xd000)	/* conditional branch */
    return 1;

  if ((inst & 0xf800) == 0xe000)	/* unconditional branch */
    return 1;

  if ((inst & 0xff00) == 0x4700)	/* bx REG, blx REG */
    return 1;

  if ((inst & 0xff87) == 0x4687)	/* mov pc, REG */
    return 1;

  if ((inst & 0xf500) == 0xb100)	/* CBNZ or CBZ.  */
    return 1;

  return 0;
}


/* See arm.h.  */

int
thumb2_instruction_changes_pc (unsigned short inst1, unsigned short inst2)
{
  if ((inst1 & 0xf800) == 0xf000 && (inst2 & 0x8000) == 0x8000)
    {
      /* Branches and miscellaneous control instructions.  */

      if ((inst2 & 0x1000) != 0 || (inst2 & 0xd001) == 0xc000)
	{
	  /* B, BL, BLX.  */
	  return 1;
	}
      else if (inst1 == 0xf3de && (inst2 & 0xff00) == 0x3f00)
	{
	  /* SUBS PC, LR, #imm8.  */
	  return 1;
	}
      else if ((inst2 & 0xd000) == 0x8000 && (inst1 & 0x0380) != 0x0380)
	{
	  /* Conditional branch.  */
	  return 1;
	}

      return 0;
    }

  if ((inst1 & 0xfe50) == 0xe810)
    {
      /* Load multiple or RFE.  */

      if (bit (inst1, 7) && !bit (inst1, 8))
	{
	  /* LDMIA or POP */
	  if (bit (inst2, 15))
	    return 1;
	}
      else if (!bit (inst1, 7) && bit (inst1, 8))
	{
	  /* LDMDB */
	  if (bit (inst2, 15))
	    return 1;
	}
      else if (bit (inst1, 7) && bit (inst1, 8))
	{
	  /* RFEIA */
	  return 1;
	}
      else if (!bit (inst1, 7) && !bit (inst1, 8))
	{
	  /* RFEDB */
	  return 1;
	}

      return 0;
    }

  if ((inst1 & 0xffef) == 0xea4f && (inst2 & 0xfff0) == 0x0f00)
    {
      /* MOV PC or MOVS PC.  */
      return 1;
    }

  if ((inst1 & 0xff70) == 0xf850 && (inst2 & 0xf000) == 0xf000)
    {
      /* LDR PC.  */
      if (bits (inst1, 0, 3) == 15)
	return 1;
      if (bit (inst1, 7))
	return 1;
      if (bit (inst2, 11))
	return 1;
      if ((inst2 & 0x0fc0) == 0x0000)
	return 1;

      return 0;
    }

  if ((inst1 & 0xfff0) == 0xe8d0 && (inst2 & 0xfff0) == 0xf000)
    {
      /* TBB.  */
      return 1;
    }

  if ((inst1 & 0xfff0) == 0xe8d0 && (inst2 & 0xfff0) == 0xf010)
    {
      /* TBH.  */
      return 1;
    }

  return 0;
}

/* See arm.h.  */
unsigned long regcache_raw_get_unsigned(struct registers *regs, int regnum) {
    return ((unsigned long *)regs)[regnum];
}

unsigned long
shifted_reg_val (struct registers *regs, unsigned long inst,
		 int carry, unsigned long pc_val, unsigned long status_reg)
{
  unsigned long res, shift;
  int rm = bits (inst, 0, 3);
  unsigned long shifttype = bits (inst, 5, 6);

  if (bit (inst, 4))
    {
      int rs = bits (inst, 8, 11);
      shift = (rs == 15
	       ? pc_val + 8
	       : regcache_raw_get_unsigned (regs, rs)) & 0xFF;
    }
  else
    shift = bits (inst, 7, 11);

  res = (rm == ARM_PC_REGNUM
	 ? (pc_val + (bit (inst, 4) ? 12 : 8))
	 : regcache_raw_get_unsigned (regs, rm));

  switch (shifttype)
    {
    case 0:			/* LSL */
      res = shift >= 32 ? 0 : res << shift;
      break;

    case 1:			/* LSR */
      res = shift >= 32 ? 0 : res >> shift;
      break;

    case 2:			/* ASR */
      if (shift >= 32)
	shift = 31;
      res = ((res & 0x80000000L)
	     ? ~((~res) >> shift) : res >> shift);
      break;

    case 3:			/* ROR/RRX */
      shift &= 31;
      if (shift == 0)
	res = (res >> 1) | (carry ? 0x80000000L : 0);
      else
	res = (res >> shift) | (res << (32 - shift));
      break;
    }

  return res & 0xffffffff;
}


void
arm_get_next_pcs_ctor (struct arm_get_next_pcs *self,
        struct arm_get_next_pcs_ops *ops,
        int byte_order,
        int byte_order_for_code,
        int has_thumb2_breakpoint,
        struct registers *regs)
{
    self->ops = ops;
    self->byte_order = byte_order;
    self->byte_order_for_code = byte_order_for_code;
    self->has_thumb2_breakpoint = has_thumb2_breakpoint;
    self->regs = regs;
}

/* Checks for an atomic sequence of instructions beginning with a LDREX{,B,H,D}
   instruction and ending with a STREX{,B,H,D} instruction.  If such a sequence
   is found, attempt to step through it.  The end of the sequence address is
   added to the next_pcs list.  */

static pc_list
thumb_deal_with_atomic_sequence_raw (struct arm_get_next_pcs *self)
{
    int byte_order_for_code = self->byte_order_for_code;
    CORE_ADDR breaks[2] = {CORE_ADDR_MAX, CORE_ADDR_MAX};
    CORE_ADDR pc = self->regs->pc;
    CORE_ADDR loc = pc;
    unsigned short insn1, insn2;
    int insn_count;
    int index;
    int last_breakpoint = 0; /* Defaults to 0 (no breakpoints placed).  */
    const int atomic_sequence_length = 16; /* Instruction sequence length.  */
    ULONGEST status, itstate;

    /* We currently do not support atomic sequences within an IT block.  */
    status = self->regs->cpsr.packed;
    itstate = ((status >> 8) & 0xfc) | ((status >> 25) & 0x3);
    if (itstate & 0x0f)
        return NULL; // empty vector

    /* Assume all atomic sequences start with a ldrex{,b,h,d} instruction.  */
    insn1 = self->ops->read_mem_uint (loc, 2, byte_order_for_code);

    loc += 2;
    if (thumb_insn_size (insn1) != 4)
        return NULL;

    insn2 = self->ops->read_mem_uint (loc, 2, byte_order_for_code);

    loc += 2;
    if (!((insn1 & 0xfff0) == 0xe850
                || ((insn1 & 0xfff0) == 0xe8d0 && (insn2 & 0x00c0) == 0x0040)))
        return NULL;

    /* Assume that no atomic sequence is longer than "atomic_sequence_length"
       instructions.  */
    for (insn_count = 0; insn_count < atomic_sequence_length; ++insn_count)
    {
        insn1 = self->ops->read_mem_uint (loc, 2,byte_order_for_code);
        loc += 2;

        if (thumb_insn_size (insn1) != 4)
        {
            /* Assume that there is at most one conditional branch in the
               atomic sequence.  If a conditional branch is found, put a
               breakpoint in its destination address.  */
            if ((insn1 & 0xf000) == 0xd000 && bits (insn1, 8, 11) != 0x0f)
            {
                if (last_breakpoint > 0)
                    return NULL; /* More than one conditional branch found,
                                  fallback to the standard code.  */

                breaks[1] = loc + 2 + (sbits (insn1, 0, 7) << 1);
                last_breakpoint++;
            }

            /* We do not support atomic sequences that use any *other*
               instructions but conditional branches to change the PC.
               Fall back to standard code to avoid losing control of
               execution.  */
            else if (thumb_instruction_changes_pc (insn1))
                return NULL;
        }
        else
        {
            insn2 = self->ops->read_mem_uint (loc, 2, byte_order_for_code);

            loc += 2;

            /* Assume that there is at most one conditional branch in the
               atomic sequence.  If a conditional branch is found, put a
               breakpoint in its destination address.  */
            if ((insn1 & 0xf800) == 0xf000
                    && (insn2 & 0xd000) == 0x8000
                    && (insn1 & 0x0380) != 0x0380)
            {
                int sign, j1, j2, imm1, imm2;
                unsigned int offset;

                sign = sbits (insn1, 10, 10);
                imm1 = bits (insn1, 0, 5);
                imm2 = bits (insn2, 0, 10);
                j1 = bit (insn2, 13);
                j2 = bit (insn2, 11);

                offset = (sign << 20) + (j2 << 19) + (j1 << 18);
                offset += (imm1 << 12) + (imm2 << 1);

                if (last_breakpoint > 0)
                    return NULL; /* More than one conditional branch found,
                                  fallback to the standard code.  */

                breaks[1] = loc + offset;
                last_breakpoint++;
            }

            /* We do not support atomic sequences that use any *other*
               instructions but conditional branches to change the PC.
               Fall back to standard code to avoid losing control of
               execution.  */
            else if (thumb2_instruction_changes_pc (insn1, insn2))
                return NULL;

            /* If we find a strex{,b,h,d}, we're done.  */
            if ((insn1 & 0xfff0) == 0xe840
                    || ((insn1 & 0xfff0) == 0xe8c0 && (insn2 & 0x00c0) == 0x0040))
                break;
        }
    }

    /* If we didn't find the strex{,b,h,d}, we cannot handle the sequence.  */
    if (insn_count == atomic_sequence_length)
        return NULL;

    /* Insert a breakpoint right after the end of the atomic sequence.  */
    breaks[0] = loc;

    /* Check for duplicated breakpoints.  Check also for a breakpoint
       placed (branch instruction's destination) anywhere in sequence.  */
    if (last_breakpoint
            && (breaks[1] == breaks[0]
                || (breaks[1] >= pc && breaks[1] < loc)))
        last_breakpoint = 0;

    pc_list next_pcs = NULL;

    /* Adds the breakpoints to the list to be inserted.  */
    for (index = 0; index <= last_breakpoint; index++)
        cvector_push_back(next_pcs, MAKE_THUMB_ADDR(breaks[index]));

    return next_pcs;
}

/* Checks for an atomic sequence of instructions beginning with a LDREX{,B,H,D}
   instruction and ending with a STREX{,B,H,D} instruction.  If such a sequence
   is found, attempt to step through it.  The end of the sequence address is
   added to the next_pcs list.  */

pc_list
arm_deal_with_atomic_sequence_raw (struct arm_get_next_pcs *self)
{
    int byte_order_for_code = self->byte_order_for_code;
    CORE_ADDR breaks[2] = {CORE_ADDR_MAX, CORE_ADDR_MAX};
    CORE_ADDR pc = self->regs->pc;
    CORE_ADDR loc = pc;
    unsigned int insn;
    int insn_count;
    int index;
    int last_breakpoint = 0; /* Defaults to 0 (no breakpoints placed).  */
    const int atomic_sequence_length = 16; /* Instruction sequence length.  */

    /* Assume all atomic sequences start with a ldrex{,b,h,d} instruction.
       Note that we do not currently support conditionally executed atomic
       instructions.  */
    insn = self->ops->read_mem_uint (loc, 4, byte_order_for_code);

    loc += 4;
    if ((insn & 0xff9000f0) != 0xe1900090)
        return NULL;

    /* Assume that no atomic sequence is longer than "atomic_sequence_length"
       instructions.  */
    for (insn_count = 0; insn_count < atomic_sequence_length; ++insn_count)
    {
        insn = self->ops->read_mem_uint (loc, 4, byte_order_for_code);

        loc += 4;

        /* Assume that there is at most one conditional branch in the atomic
           sequence.  If a conditional branch is found, put a breakpoint in
           its destination address.  */
        if (bits (insn, 24, 27) == 0xa)
        {
            if (last_breakpoint > 0)
                return NULL; /* More than one conditional branch found, fallback
                              to the standard single-step code.  */

            breaks[1] = BranchDest (loc - 4, insn);
            last_breakpoint++;
        }

        /* We do not support atomic sequences that use any *other* instructions
           but conditional branches to change the PC.  Fall back to standard
           code to avoid losing control of execution.  */
        else if (arm_instruction_changes_pc (insn))
            return NULL;

        /* If we find a strex{,b,h,d}, we're done.  */
        if ((insn & 0xff9000f0) == 0xe1800090)
            break;
    }

    /* If we didn't find the strex{,b,h,d}, we cannot handle the sequence.  */
    if (insn_count == atomic_sequence_length)
        return NULL;

    /* Insert a breakpoint right after the end of the atomic sequence.  */
    breaks[0] = loc;

    /* Check for duplicated breakpoints.  Check also for a breakpoint
       placed (branch instruction's destination) anywhere in sequence.  */
    if (last_breakpoint
            && (breaks[1] == breaks[0]
                || (breaks[1] >= pc && breaks[1] < loc)))
        last_breakpoint = 0;

    pc_list next_pcs = NULL;

    /* Adds the breakpoints to the list to be inserted.  */
    for (index = 0; index <= last_breakpoint; index++)
        cvector_push_back(next_pcs, breaks[index]);

    return next_pcs;
}

/* Find the next possible PCs for thumb mode.  */

pc_list
thumb_get_next_pcs_raw (struct arm_get_next_pcs *self)
{
    int byte_order = self->byte_order;
    int byte_order_for_code = self->byte_order_for_code;
    CORE_ADDR pc = self->regs->pc;
    unsigned long pc_val = ((unsigned long) pc) + 4;	/* PC after prefetch */
    unsigned short inst1;
    CORE_ADDR nextpc = pc + 2;		/* Default is next instruction.  */
    ULONGEST status, itstate;
    pc_list next_pcs = NULL;

    nextpc = MAKE_THUMB_ADDR (nextpc);
    pc_val = MAKE_THUMB_ADDR (pc_val);

    inst1 = self->ops->read_mem_uint (pc, 2, byte_order_for_code);

    /* Thumb-2 conditional execution support.  There are eight bits in
       the CPSR which describe conditional execution state.  Once
       reconstructed (they're in a funny order), the low five bits
       describe the low bit of the condition for each instruction and
       how many instructions remain.  The high three bits describe the
       base condition.  One of the low four bits will be set if an IT
       block is active.  These bits read as zero on earlier
       processors.  */
    status = self->regs->cpsr.packed;
    itstate = ((status >> 8) & 0xfc) | ((status >> 25) & 0x3);

    /* If-Then handling.  On GNU/Linux, where this routine is used, we
       use an undefined instruction as a breakpoint.  Unlike BKPT, IT
       can disable execution of the undefined instruction.  So we might
       miss the breakpoint if we set it on a skipped conditional
       instruction.  Because conditional instructions can change the
       flags, affecting the execution of further instructions, we may
       need to set two breakpoints.  */

    if (self->has_thumb2_breakpoint)
    {
        if ((inst1 & 0xff00) == 0xbf00 && (inst1 & 0x000f) != 0)
        {
            /* An IT instruction.  Because this instruction does not
               modify the flags, we can accurately predict the next
               executed instruction.  */
            itstate = inst1 & 0x00ff;
            pc += thumb_insn_size (inst1);

            while (itstate != 0 && ! condition_true (itstate >> 4, status))
            {
                inst1 = self->ops->read_mem_uint (pc, 2,byte_order_for_code);
                pc += thumb_insn_size (inst1);
                itstate = thumb_advance_itstate (itstate);
            }

            cvector_push_back(next_pcs, MAKE_THUMB_ADDR(pc));
            return next_pcs;
        }
        else if (itstate != 0)
        {
            /* We are in a conditional block.  Check the condition.  */
            if (! condition_true (itstate >> 4, status))
            {
                /* Advance to the next executed instruction.  */
                pc += thumb_insn_size (inst1);
                itstate = thumb_advance_itstate (itstate);

                while (itstate != 0 && ! condition_true (itstate >> 4, status))
                {
                    inst1 = self->ops->read_mem_uint (pc, 2, byte_order_for_code);

                    pc += thumb_insn_size (inst1);
                    itstate = thumb_advance_itstate (itstate);
                }

                cvector_push_back(next_pcs, MAKE_THUMB_ADDR(pc));
                return next_pcs;
            }
            else if ((itstate & 0x0f) == 0x08)
            {
                /* This is the last instruction of the conditional
                   block, and it is executed.  We can handle it normally
                   because the following instruction is not conditional,
                   and we must handle it normally because it is
                   permitted to branch.  Fall through.  */
            }
            else
            {
                int cond_negated;

                /* There are conditional instructions after this one.
                   If this instruction modifies the flags, then we can
                   not predict what the next executed instruction will
                   be.  Fortunately, this instruction is architecturally
                   forbidden to branch; we know it will fall through.
                   Start by skipping past it.  */
                pc += thumb_insn_size (inst1);
                itstate = thumb_advance_itstate (itstate);

                /* Set a breakpoint on the following instruction.  */
                assert((itstate & 0x0f) != 0);
                cvector_push_back(next_pcs, MAKE_THUMB_ADDR(pc));

                cond_negated = (itstate >> 4) & 1;

                /* Skip all following instructions with the same
                   condition.  If there is a later instruction in the IT
                   block with the opposite condition, set the other
                   breakpoint there.  If not, then set a breakpoint on
                   the instruction after the IT block.  */
                do
                {
                    inst1 = self->ops->read_mem_uint (pc, 2, byte_order_for_code);
                    pc += thumb_insn_size (inst1);
                    itstate = thumb_advance_itstate (itstate);
                }
                while (itstate != 0 && ((itstate >> 4) & 1) == cond_negated);

                cvector_push_back(next_pcs, MAKE_THUMB_ADDR(pc));

                return next_pcs;
            }
        }
    }
    else if (itstate & 0x0f)
    {
        /* We are in a conditional block.  Check the condition.  */
        int cond = itstate >> 4;

        if (! condition_true (cond, status))
        {
            /* Advance to the next instruction.  All the 32-bit
               instructions share a common prefix.  */
            cvector_push_back(next_pcs, MAKE_THUMB_ADDR(pc + thumb_insn_size(inst1)));
        }

        return next_pcs;

        /* Otherwise, handle the instruction normally.  */
    }

    if ((inst1 & 0xff00) == 0xbd00)	/* pop {rlist, pc} */
    {
        CORE_ADDR sp;

        /* Fetch the saved PC from the stack.  It's stored above
           all of the other registers.  */
        unsigned long offset
            = count_one_bits (bits (inst1, 0, 7)) * ARM_INT_REGISTER_SIZE;
        sp = regcache_raw_get_unsigned (self->regs, ARM_SP_REGNUM);
        nextpc = self->ops->read_mem_uint (sp + offset, 4, byte_order);
    }
    else if ((inst1 & 0xf000) == 0xd000)	/* conditional branch */
    {
        unsigned long cond = bits (inst1, 8, 11);
        if (cond == 0x0f)  /* 0x0f = SWI */
        {
            nextpc = self->ops->syscall_next_pc (self);
        }
        else if (cond != 0x0f && condition_true (cond, status))
            nextpc = pc_val + (sbits (inst1, 0, 7) << 1);
    }
    else if ((inst1 & 0xf800) == 0xe000)	/* unconditional branch */
    {
        nextpc = pc_val + (sbits (inst1, 0, 10) << 1);
    }
    else if (thumb_insn_size (inst1) == 4) /* 32-bit instruction */
    {
        unsigned short inst2;
        inst2 = self->ops->read_mem_uint (pc + 2, 2, byte_order_for_code);

        /* Default to the next instruction.  */
        nextpc = pc + 4;
        nextpc = MAKE_THUMB_ADDR (nextpc);

        if ((inst1 & 0xf800) == 0xf000 && (inst2 & 0x8000) == 0x8000)
        {
            /* Branches and miscellaneous control instructions.  */

            if ((inst2 & 0x1000) != 0 || (inst2 & 0xd001) == 0xc000)
            {
                /* B, BL, BLX.  */
                int j1, j2, imm1, imm2;

                imm1 = sbits (inst1, 0, 10);
                imm2 = bits (inst2, 0, 10);
                j1 = bit (inst2, 13);
                j2 = bit (inst2, 11);

                unsigned long offset = ((imm1 << 12) + (imm2 << 1));
                offset ^= ((!j2) << 22) | ((!j1) << 23);

                nextpc = pc_val + offset;
                /* For BLX make sure to clear the low bits.  */
                if (bit (inst2, 12) == 0)
                    nextpc = nextpc & 0xfffffffc;
            }
            else if (inst1 == 0xf3de && (inst2 & 0xff00) == 0x3f00)
            {
                /* SUBS PC, LR, #imm8.  */
                nextpc = regcache_raw_get_unsigned (self->regs, ARM_LR_REGNUM);
                nextpc -= inst2 & 0x00ff;
            }
            else if ((inst2 & 0xd000) == 0x8000 && (inst1 & 0x0380) != 0x0380)
            {
                /* Conditional branch.  */
                if (condition_true (bits (inst1, 6, 9), status))
                {
                    int sign, j1, j2, imm1, imm2;

                    sign = sbits (inst1, 10, 10);
                    imm1 = bits (inst1, 0, 5);
                    imm2 = bits (inst2, 0, 10);
                    j1 = bit (inst2, 13);
                    j2 = bit (inst2, 11);

                    unsigned long offset
                        = (sign << 20) + (j2 << 19) + (j1 << 18);
                    offset += (imm1 << 12) + (imm2 << 1);

                    nextpc = pc_val + offset;
                }
            }
        }
        else if ((inst1 & 0xfe50) == 0xe810)
        {
            /* Load multiple or RFE.  */
            int rn, offset, load_pc = 1;

            rn = bits (inst1, 0, 3);
            if (bit (inst1, 7) && !bit (inst1, 8))
            {
                /* LDMIA or POP */
                if (!bit (inst2, 15))
                    load_pc = 0;
                offset = count_one_bits (inst2) * 4 - 4;
            }
            else if (!bit (inst1, 7) && bit (inst1, 8))
            {
                /* LDMDB */
                if (!bit (inst2, 15))
                    load_pc = 0;
                offset = -4;
            }
            else if (bit (inst1, 7) && bit (inst1, 8))
            {
                /* RFEIA */
                offset = 0;
            }
            else if (!bit (inst1, 7) && !bit (inst1, 8))
            {
                /* RFEDB */
                offset = -8;
            }
            else
                load_pc = 0;

            if (load_pc)
            {
                CORE_ADDR addr = regcache_raw_get_unsigned (self->regs, rn);
                nextpc = self->ops->read_mem_uint	(addr + offset, 4, byte_order);
            }
        }
        else if ((inst1 & 0xffef) == 0xea4f && (inst2 & 0xfff0) == 0x0f00)
        {
            /* MOV PC or MOVS PC.  */
            nextpc = regcache_raw_get_unsigned (self->regs, bits (inst2, 0, 3));
            nextpc = MAKE_THUMB_ADDR (nextpc);
        }
        else if ((inst1 & 0xff70) == 0xf850 && (inst2 & 0xf000) == 0xf000)
        {
            /* LDR PC.  */
            CORE_ADDR base;
            int rn, load_pc = 1;

            rn = bits (inst1, 0, 3);
            base = regcache_raw_get_unsigned (self->regs, rn);
            if (rn == ARM_PC_REGNUM)
            {
                base = (base + 4) & ~(CORE_ADDR) 0x3;
                if (bit (inst1, 7))
                    base += bits (inst2, 0, 11);
                else
                    base -= bits (inst2, 0, 11);
            }
            else if (bit (inst1, 7))
                base += bits (inst2, 0, 11);
            else if (bit (inst2, 11))
            {
                if (bit (inst2, 10))
                {
                    if (bit (inst2, 9))
                        base += bits (inst2, 0, 7);
                    else
                        base -= bits (inst2, 0, 7);
                }
            }
            else if ((inst2 & 0x0fc0) == 0x0000)
            {
                int shift = bits (inst2, 4, 5), rm = bits (inst2, 0, 3);
                base += regcache_raw_get_unsigned (self->regs, rm) << shift;
            }
            else
                /* Reserved.  */
                load_pc = 0;

            if (load_pc)
                nextpc
                    = self->ops->read_mem_uint (base, 4, byte_order);
        }
        else if ((inst1 & 0xfff0) == 0xe8d0 && (inst2 & 0xfff0) == 0xf000)
        {
            /* TBB.  */
            CORE_ADDR tbl_reg, table, offset, length;

            tbl_reg = bits (inst1, 0, 3);
            if (tbl_reg == 0x0f)
                table = pc + 4;  /* Regcache copy of PC isn't right yet.  */
            else
                table = regcache_raw_get_unsigned (self->regs, tbl_reg);

            offset = regcache_raw_get_unsigned (self->regs, bits (inst2, 0, 3));
            length = 2 * self->ops->read_mem_uint (table + offset, 1, byte_order);
            nextpc = pc_val + length;
        }
        else if ((inst1 & 0xfff0) == 0xe8d0 && (inst2 & 0xfff0) == 0xf010)
        {
            /* TBH.  */
            CORE_ADDR tbl_reg, table, offset, length;

            tbl_reg = bits (inst1, 0, 3);
            if (tbl_reg == 0x0f)
                table = pc + 4;  /* Regcache copy of PC isn't right yet.  */
            else
                table = regcache_raw_get_unsigned (self->regs, tbl_reg);

            offset = 2 * regcache_raw_get_unsigned (self->regs, bits (inst2, 0, 3));
            length = 2 * self->ops->read_mem_uint (table + offset, 2, byte_order);
            nextpc = pc_val + length;
        }
    }
    else if ((inst1 & 0xff00) == 0x4700)	/* bx REG, blx REG */
    {
        if (bits (inst1, 3, 6) == 0x0f)
            nextpc = UNMAKE_THUMB_ADDR (pc_val);
        else
            nextpc = regcache_raw_get_unsigned (self->regs, bits (inst1, 3, 6));
    }
    else if ((inst1 & 0xff87) == 0x4687)	/* mov pc, REG */
    {
        if (bits (inst1, 3, 6) == 0x0f)
            nextpc = pc_val;
        else
            nextpc = regcache_raw_get_unsigned (self->regs, bits (inst1, 3, 6));

        nextpc = MAKE_THUMB_ADDR (nextpc);
    }
    else if ((inst1 & 0xf500) == 0xb100)
    {
        /* CBNZ or CBZ.  */
        int imm = (bit (inst1, 9) << 6) + (bits (inst1, 3, 7) << 1);
        ULONGEST reg = regcache_raw_get_unsigned (self->regs, bits (inst1, 0, 2));

        if (bit (inst1, 11) && reg != 0)
            nextpc = pc_val + imm;
        else if (!bit (inst1, 11) && reg == 0)
            nextpc = pc_val + imm;
    }

    cvector_push_back(next_pcs, nextpc);

    return next_pcs;
}

/* Get the raw next possible addresses.  PC in next_pcs is the current program
   counter, which is assumed to be executing in ARM mode.

   The values returned have the execution state of the next instruction
   encoded in it.  Use IS_THUMB_ADDR () to see whether the instruction is
   in Thumb-State, and gdbarch_addr_bits_remove () to get the plain memory
   address in GDB and arm_addr_bits_remove in GDBServer.  */

pc_list
arm_get_next_pcs_raw (struct arm_get_next_pcs *self)
{
    int byte_order = self->byte_order;
    int byte_order_for_code = self->byte_order_for_code;
    unsigned long pc_val;
    unsigned long this_instr = 0;
    unsigned long status;
    CORE_ADDR nextpc;
    CORE_ADDR pc = self->regs->pc;
    pc_list next_pcs = NULL;

    pc_val = (unsigned long) pc;
    this_instr = self->ops->read_mem_uint (pc, 4, byte_order_for_code);

    status = self->regs->cpsr.packed;
    nextpc = (CORE_ADDR) (pc_val + 4);	/* Default case */

    if (bits (this_instr, 28, 31) == INST_NV)
        switch (bits (this_instr, 24, 27))
        {
            case 0xa:
            case 0xb:
                {
                    /* Branch with Link and change to Thumb.  */
                    nextpc = BranchDest (pc, this_instr);
                    nextpc |= bit (this_instr, 24) << 1;
                    nextpc = MAKE_THUMB_ADDR (nextpc);
                    break;
                }
            case 0xc:
            case 0xd:
            case 0xe:
                /* Coprocessor register transfer.  */
                if (bits (this_instr, 12, 15) == 15)
                    ERROR("Invalid update to pc in instruction");
                break;
        }
    else if (condition_true (bits (this_instr, 28, 31), status))
    {
        switch (bits (this_instr, 24, 27))
        {
            case 0x0:
            case 0x1:			/* data processing */
            case 0x2:
            case 0x3:
                {
                    unsigned long operand1, operand2, result = 0;
                    unsigned long rn;
                    int c;

                    if (bits (this_instr, 12, 15) != 15)
                        break;

                    if (bits (this_instr, 22, 25) == 0
                            && bits (this_instr, 4, 7) == 9)	/* multiply */
                        ERROR("Invalid update to pc in instruction");

                    /* BX <reg>, BLX <reg> */
                    if (bits (this_instr, 4, 27) == 0x12fff1
                            || bits (this_instr, 4, 27) == 0x12fff3)
                    {
                        rn = bits (this_instr, 0, 3);
                        nextpc = ((rn == ARM_PC_REGNUM)
                                ? (pc_val + 8)
                                : regcache_raw_get_unsigned (self->regs, rn));

                        cvector_push_back(next_pcs, nextpc);
                        return next_pcs;
                    }

                    /* Multiply into PC.  */
                    c = (status & FLAG_C) ? 1 : 0;
                    rn = bits (this_instr, 16, 19);
                    operand1 = ((rn == ARM_PC_REGNUM)
                            ? (pc_val + 8)
                            : regcache_raw_get_unsigned (self->regs, rn));

                    if (bit (this_instr, 25))
                    {
                        unsigned long immval = bits (this_instr, 0, 7);
                        unsigned long rotate = 2 * bits (this_instr, 8, 11);
                        operand2 = ((immval >> rotate) | (immval << (32 - rotate)))
                            & 0xffffffff;
                    }
                    else		/* operand 2 is a shifted register.  */
                        operand2 = shifted_reg_val (self->regs, this_instr, c,
                                pc_val, status);

                    switch (bits (this_instr, 21, 24))
                    {
                        case 0x0:	/*and */
                            result = operand1 & operand2;
                            break;

                        case 0x1:	/*eor */
                            result = operand1 ^ operand2;
                            break;

                        case 0x2:	/*sub */
                            result = operand1 - operand2;
                            break;

                        case 0x3:	/*rsb */
                            result = operand2 - operand1;
                            break;

                        case 0x4:	/*add */
                            result = operand1 + operand2;
                            break;

                        case 0x5:	/*adc */
                            result = operand1 + operand2 + c;
                            break;

                        case 0x6:	/*sbc */
                            result = operand1 - operand2 + c;
                            break;

                        case 0x7:	/*rsc */
                            result = operand2 - operand1 + c;
                            break;

                        case 0x8:
                        case 0x9:
                        case 0xa:
                        case 0xb:	/* tst, teq, cmp, cmn */
                            result = (unsigned long) nextpc;
                            break;

                        case 0xc:	/*orr */
                            result = operand1 | operand2;
                            break;

                        case 0xd:	/*mov */
                            /* Always step into a function.  */
                            result = operand2;
                            break;

                        case 0xe:	/*bic */
                            result = operand1 & ~operand2;
                            break;

                        case 0xf:	/*mvn */
                            result = ~operand2;
                            break;
                    }
                    nextpc = self->ops->addr_bits_remove (self, result);
                    break;
                }

            case 0x4:
            case 0x5:		/* data transfer */
            case 0x6:
            case 0x7:
                if (bits (this_instr, 25, 27) == 0x3 && bit (this_instr, 4) == 1)
                {
                    /* Media instructions and architecturally undefined
                       instructions.  */
                    break;
                }

                if (bit (this_instr, 20))
                {
                    /* load */
                    if (bits (this_instr, 12, 15) == 15)
                    {
                        /* rd == pc */
                        unsigned long rn;
                        unsigned long base;

                        if (bit (this_instr, 22))
                            ERROR("Invalid update to pc in instruction");

                        /* byte write to PC */
                        rn = bits (this_instr, 16, 19);
                        base = ((rn == ARM_PC_REGNUM)
                                ? (pc_val + 8)
                                : regcache_raw_get_unsigned (self->regs, rn));

                        if (bit (this_instr, 24))
                        {
                            /* pre-indexed */
                            int c = (status & FLAG_C) ? 1 : 0;
                            unsigned long offset =
                                (bit (this_instr, 25)
                                 ? shifted_reg_val (self->regs, this_instr, c,
                                     pc_val, status)
                                 : bits (this_instr, 0, 11));

                            if (bit (this_instr, 23))
                                base += offset;
                            else
                                base -= offset;
                        }
                        nextpc
                            = (CORE_ADDR) self->ops->read_mem_uint ((CORE_ADDR) base,
                                    4, byte_order);
                    }
                }
                break;

            case 0x8:
            case 0x9:		/* block transfer */
                if (bit (this_instr, 20))
                {
                    /* LDM */
                    if (bit (this_instr, 15))
                    {
                        /* loading pc */
                        int offset = 0;
                        CORE_ADDR rn_val_offset = 0;
                        unsigned long rn_val
                            = regcache_raw_get_unsigned (self->regs,
                                    bits (this_instr, 16, 19));

                        if (bit (this_instr, 23))
                        {
                            /* up */
                            unsigned long reglist = bits (this_instr, 0, 14);
                            offset = count_one_bits (reglist) * 4;
                            if (bit (this_instr, 24))		/* pre */
                                offset += 4;
                        }
                        else if (bit (this_instr, 24))
                            offset = -4;

                        rn_val_offset = rn_val + offset;
                        nextpc = (CORE_ADDR) self->ops->read_mem_uint (rn_val_offset,
                                4, byte_order);
                    }
                }
                break;

            case 0xb:		/* branch & link */
            case 0xa:		/* branch */
                {
                    nextpc = BranchDest (pc, this_instr);
                    break;
                }

            case 0xc:
            case 0xd:
            case 0xe:		/* coproc ops */
                break;
            case 0xf:		/* SWI */
                {
                    nextpc = self->ops->syscall_next_pc (self);
                }
                break;

            default:
                ERROR("Bad bit-field extraction");
                return next_pcs;
        }
    }

    cvector_push_back(next_pcs, nextpc);

    return next_pcs;
}

/* See arm-get-next-pcs.h.  */

pc_list
arm_get_next_pcs (struct arm_get_next_pcs *self, bool is_thumb)
{
    pc_list next_pcs = NULL;

    if (is_thumb)
    {
        next_pcs = thumb_deal_with_atomic_sequence_raw (self);
        if (cvector_empty(next_pcs))
            next_pcs = thumb_get_next_pcs_raw (self);
    }
    else
    {
        next_pcs = arm_deal_with_atomic_sequence_raw (self);
        if (cvector_empty(next_pcs))
            next_pcs = arm_get_next_pcs_raw (self);
    }

    if (self->ops->fixup != NULL)
    {
        for (CORE_ADDR *pc_ref = cvector_begin(next_pcs); pc_ref != cvector_end(next_pcs); pc_ref++)
            *pc_ref = self->ops->fixup (self, *pc_ref);
    }

    return next_pcs;
}

