#ifdef __ASSEMBLY__
#define _ULCAST_
#define REG_WRAPPER(...) __VA_ARGS__
#else
#define _ULCAST_ (unsigned long)
#define REG_WRAPPER(...) "" # __VA_ARGS__ // stringify it for "asm" directive
#endif

#define CP0_INDEX REG_WRAPPER($0)
#define CP0_RANDOM REG_WRAPPER($1)
#define CP0_ENTRYLO0 REG_WRAPPER($2)
#define CP0_ENTRYLO1 REG_WRAPPER($3)
#define CP0_CONF REG_WRAPPER($3)
#define CP0_GLOBALNUMBER REG_WRAPPER($3, 1)
#define CP0_CONTEXT REG_WRAPPER($4)
#define CP0_PAGEMASK REG_WRAPPER($5)
#define CP0_PAGEGRAIN REG_WRAPPER($5, 1)
#define CP0_SEGCTL0 REG_WRAPPER($5, 2)
#define CP0_SEGCTL1 REG_WRAPPER($5, 3)
#define CP0_SEGCTL2 REG_WRAPPER($5, 4)
#define CP0_WIRED REG_WRAPPER($6)
#define CP0_INFO REG_WRAPPER($7)
#define CP0_HWRENA REG_WRAPPER($7)
#define CP0_BADVADDR REG_WRAPPER($8)
#define CP0_BADINSTR REG_WRAPPER($8, 1)
#define CP0_COUNT REG_WRAPPER($9)
#define CP0_ENTRYHI REG_WRAPPER($10)
#define CP0_GUESTCTL1 REG_WRAPPER($10, 4)
#define CP0_GUESTCTL2 REG_WRAPPER($10, 5)
#define CP0_GUESTCTL3 REG_WRAPPER($10, 6)
#define CP0_COMPARE REG_WRAPPER($11)
#define CP0_GUESTCTL0EXT REG_WRAPPER($11, 4)
#define CP0_STATUS REG_WRAPPER($12)
#define CP0_GUESTCTL0 REG_WRAPPER($12, 6)
#define CP0_GTOFFSET REG_WRAPPER($12, 7)
#define CP0_CAUSE REG_WRAPPER($13)
#define CP0_EPC REG_WRAPPER($14)
#define CP0_PRID REG_WRAPPER($15)
#define CP0_EBASE REG_WRAPPER($15, 1)
#define CP0_CMGCRBASE REG_WRAPPER($15, 3)
#define CP0_CONFIG REG_WRAPPER($16)
#define CP0_CONFIG3 REG_WRAPPER($16, 3)
#define CP0_CONFIG5 REG_WRAPPER($16, 5)
#define CP0_CONFIG6 REG_WRAPPER($16, 6)
#define CP0_LLADDR REG_WRAPPER($17)
#define CP0_WATCHLO REG_WRAPPER($18)
#define CP0_WATCHHI REG_WRAPPER($19)
#define CP0_XCONTEXT REG_WRAPPER($20)
#define CP0_FRAMEMASK REG_WRAPPER($21)
#define CP0_DIAGNOSTIC REG_WRAPPER($22)
#define CP0_DEBUG REG_WRAPPER($23)
#define CP0_DEPC REG_WRAPPER($24)
#define CP0_PERFORMANCE REG_WRAPPER($25)
#define CP0_ECC REG_WRAPPER($26)
#define CP0_CACHEERR REG_WRAPPER($27)
#define CP0_TAGLO REG_WRAPPER($28)
#define CP0_TAGHI REG_WRAPPER($29)
#define CP0_ERROREPC REG_WRAPPER($30)
#define CP0_DESAVE REG_WRAPPER($31)

// CP0_STATUS bits
#define ST0_IM 0x0000ff00
#define STATUSB_IP0 8
#define STATUSF_IP0 (_ULCAST_(1) << 8)
#define STATUSB_IP1 9
#define STATUSF_IP1 (_ULCAST_(1) << 9)
#define STATUSB_IP2 10
#define STATUSF_IP2 (_ULCAST_(1) << 10)
#define STATUSB_IP3 11
#define STATUSF_IP3 (_ULCAST_(1) << 11)
#define STATUSB_IP4 12
#define STATUSF_IP4 (_ULCAST_(1) << 12)
#define STATUSB_IP5 13
#define STATUSF_IP5 (_ULCAST_(1) << 13)
#define STATUSB_IP6 14
#define STATUSF_IP6 (_ULCAST_(1) << 14)
#define STATUSB_IP7 15
#define STATUSF_IP7 (_ULCAST_(1) << 15)
#define STATUSB_IP8 0
#define STATUSF_IP8 (_ULCAST_(1) << 0)
#define STATUSB_IP9 1
#define STATUSF_IP9 (_ULCAST_(1) << 1)
#define STATUSB_IP10 2
#define STATUSF_IP10 (_ULCAST_(1) << 2)
#define STATUSB_IP11 3
#define STATUSF_IP11 (_ULCAST_(1) << 3)
#define STATUSB_IP12 4
#define STATUSF_IP12 (_ULCAST_(1) << 4)
#define STATUSB_IP13 5
#define STATUSF_IP13 (_ULCAST_(1) << 5)
#define STATUSB_IP14 6
#define STATUSF_IP14 (_ULCAST_(1) << 6)
#define STATUSB_IP15 7
#define STATUSF_IP15 (_ULCAST_(1) << 7)
#define ST0_CH 0x00040000
#define ST0_NMI 0x00080000
#define ST0_SR 0x00100000
#define ST0_TS 0x00200000
#define ST0_BEV 0x00400000
#define ST0_RE 0x02000000
#define ST0_FR 0x04000000
#define ST0_CU 0xf0000000
#define ST0_CU0 0x10000000
#define ST0_CU1 0x20000000
#define ST0_CU2 0x40000000
#define ST0_CU3 0x80000000

/*
 * Bitfields and bit numbers in the coprocessor 0 cause register.
 *
 * Refer to your MIPS R4xx0 manual, chapter 5 for explanation.
 */
#define CAUSEB_EXCCODE 2
#define CAUSEF_EXCCODE (_ULCAST_(31) << 2)
#define CAUSEB_IP 8
#define CAUSEF_IP (_ULCAST_(255) << 8)
#define CAUSEB_IP0 8
#define CAUSEF_IP0 (_ULCAST_(1) << 8)
#define CAUSEB_IP1 9
#define CAUSEF_IP1 (_ULCAST_(1) << 9)
#define CAUSEB_IP2 10
#define CAUSEF_IP2 (_ULCAST_(1) << 10)
#define CAUSEB_IP3 11
#define CAUSEF_IP3 (_ULCAST_(1) << 11)
#define CAUSEB_IP4 12
#define CAUSEF_IP4 (_ULCAST_(1) << 12)
#define CAUSEB_IP5 13
#define CAUSEF_IP5 (_ULCAST_(1) << 13)
#define CAUSEB_IP6 14
#define CAUSEF_IP6 (_ULCAST_(1) << 14)
#define CAUSEB_IP7 15
#define CAUSEF_IP7 (_ULCAST_(1) << 15)
#define CAUSEB_FDCI 21
#define CAUSEF_FDCI (_ULCAST_(1) << 21)
#define CAUSEB_WP 22
#define CAUSEF_WP (_ULCAST_(1) << 22)
#define CAUSEB_IV 23
#define CAUSEF_IV (_ULCAST_(1) << 23)
#define CAUSEB_PCI 26
#define CAUSEF_PCI (_ULCAST_(1) << 26)
#define CAUSEB_DC 27
#define CAUSEF_DC (_ULCAST_(1) << 27)
#define CAUSEB_CE 28
#define CAUSEF_CE (_ULCAST_(3) << 28)
#define CAUSEB_TI 30
#define CAUSEF_TI (_ULCAST_(1) << 30)
#define CAUSEB_BD 31
#define CAUSEF_BD (_ULCAST_(1) << 31)

/*
 * Cause.ExcCode trap codes.
 */
#define EXCCODE_INT 0 /* Interrupt pending */
#define EXCCODE_MOD 1 /* TLB modified fault */
#define EXCCODE_TLBL 2 /* TLB miss on load or ifetch */
#define EXCCODE_TLBS 3 /* TLB miss on a store */
#define EXCCODE_ADEL 4 /* Address error on a load or ifetch */
#define EXCCODE_ADES 5 /* Address error on a store */
#define EXCCODE_IBE 6 /* Bus error on an ifetch */
#define EXCCODE_DBE 7 /* Bus error on a load or store */
#define EXCCODE_SYS 8 /* System call */
#define EXCCODE_BP 9 /* Breakpoint */
#define EXCCODE_RI 10 /* Reserved instruction exception */
#define EXCCODE_CPU 11 /* Coprocessor unusable */
#define EXCCODE_OV 12 /* Arithmetic overflow */
#define EXCCODE_TR 13 /* Trap instruction */
#define EXCCODE_MSAFPE 14 /* MSA floating point exception */
#define EXCCODE_FPE 15 /* Floating point exception */
#define EXCCODE_TLBRI 19 /* TLB Read-Inhibit exception */
#define EXCCODE_TLBXI 20 /* TLB Execution-Inhibit exception */
#define EXCCODE_MSADIS 21 /* MSA disabled exception */
#define EXCCODE_MDMX 22 /* MDMX unusable exception */
#define EXCCODE_WATCH 23 /* Watch address reference */
#define EXCCODE_MCHECK 24 /* Machine check */
#define EXCCODE_THREAD 25 /* Thread exceptions (MT) */
#define EXCCODE_DSPDIS 26 /* DSP disabled exception */
#define EXCCODE_GE 27 /* Virtualized guest exception (VZ) */

/* Generic EntryLo bit definitions */
#define ENTRYLO_G		(_ULCAST_(1) << 0)
#define ENTRYLO_V		(_ULCAST_(1) << 1)
#define ENTRYLO_D		(_ULCAST_(1) << 2)
#define ENTRYLO_C_SHIFT		3
#define ENTRYLO_C		(_ULCAST_(7) << ENTRYLO_C_SHIFT)

/* MIPS32/64 EntryHI bit definitions */
#define MIPS_ENTRYHI_EHINV	(_ULCAST_(1) << 10)
#define MIPS_ENTRYHI_ASIDX	(_ULCAST_(0x3) << 8)
#define MIPS_ENTRYHI_ASID	(_ULCAST_(0xff) << 0)

