/*
 * arm64-codegen.h
 *
 * AArch64 instruction encoding macros for the Vincent GLES1 JIT.
 * Parallel to arm-codegen.h (ARM32), providing the same macro API
 * with ARM64_ prefix and ARM_* compatibility defines.
 *
 * Copyright (c) 2002 Wild West Software
 * Copyright (c) 2001, 2002 Sergey Chaban
 * Copyright (c) 2004, Hans-Martin Will. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ARM64_H
#define ARM64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "segment.h"


typedef unsigned int arm64instr_t;
typedef unsigned int arm64word_t;


/* ======================================================================== */
/*  Emit helper                                                             */
/* ======================================================================== */

#define ARM64_EMIT(p, i) cg_segment_emit_u32(p, (unsigned int)(i))


/* ======================================================================== */
/*  Register definitions                                                    */
/* ======================================================================== */

typedef enum {
	ARM64REG_X0 = 0,
	ARM64REG_X1,
	ARM64REG_X2,
	ARM64REG_X3,
	ARM64REG_X4,
	ARM64REG_X5,
	ARM64REG_X6,
	ARM64REG_X7,
	ARM64REG_X8,
	ARM64REG_X9,
	ARM64REG_X10,
	ARM64REG_X11,
	ARM64REG_X12,
	ARM64REG_X13,
	ARM64REG_X14,
	ARM64REG_X15,
	ARM64REG_X16,
	ARM64REG_X17,
	ARM64REG_X18,
	ARM64REG_X19,
	ARM64REG_X20,
	ARM64REG_X21,
	ARM64REG_X22,
	ARM64REG_X23,
	ARM64REG_X24,
	ARM64REG_X25,
	ARM64REG_X26,
	ARM64REG_X27,
	ARM64REG_X28,
	ARM64REG_X29,
	ARM64REG_X30,

	/* SP and XZR share encoding 31; context determines which */
	ARM64REG_SP  = 31,
	ARM64REG_XZR = 31,
	ARM64REG_WZR = 31,

	/* named aliases */
	ARM64REG_FP  = 29,
	ARM64REG_LR  = 30,
	ARM64REG_IP0 = 16,
	ARM64REG_IP1 = 17,

	/* ABI argument registers (compatible with ARM32 naming) */
	ARM64REG_A1 = 0,
	ARM64REG_A2 = 1,
	ARM64REG_A3 = 2,
	ARM64REG_A4 = 3,
	ARM64REG_A5 = 4,
	ARM64REG_A6 = 5,
	ARM64REG_A7 = 6,
	ARM64REG_A8 = 7,

	/* callee-saved variable registers (V1-V10 map to X19-X28) */
	ARM64REG_V1  = 19,
	ARM64REG_V2  = 20,
	ARM64REG_V3  = 21,
	ARM64REG_V4  = 22,
	ARM64REG_V5  = 23,
	ARM64REG_V6  = 24,
	ARM64REG_V7  = 25,
	ARM64REG_V8  = 26,
	ARM64REG_V9  = 27,
	ARM64REG_V10 = 28,

	ARM64REG_MAX = ARM64REG_X30
} ARM64Reg;

/* number of argument registers */
#define ARM64_NUM_ARG_REGS 8

/* number of callee-saved variable registers */
#define ARM64_NUM_VARIABLE_REGS 10

/* bitvector for all argument regs */
#define ARM64_ALL_ARG_REGS \
	((1 << ARM64REG_A1) | (1 << ARM64REG_A2) | (1 << ARM64REG_A3) | \
	 (1 << ARM64REG_A4) | (1 << ARM64REG_A5) | (1 << ARM64REG_A6) | \
	 (1 << ARM64REG_A7) | (1 << ARM64REG_A8))


/* ======================================================================== */
/*  Condition codes (same encoding as ARM32)                                */
/* ======================================================================== */

typedef enum {
	ARM64COND_EQ = 0x0,
	ARM64COND_NE = 0x1,
	ARM64COND_CS = 0x2,
	ARM64COND_HS = 0x2,
	ARM64COND_CC = 0x3,
	ARM64COND_LO = 0x3,
	ARM64COND_MI = 0x4,
	ARM64COND_PL = 0x5,
	ARM64COND_VS = 0x6,
	ARM64COND_VC = 0x7,
	ARM64COND_HI = 0x8,
	ARM64COND_LS = 0x9,
	ARM64COND_GE = 0xA,
	ARM64COND_LT = 0xB,
	ARM64COND_GT = 0xC,
	ARM64COND_LE = 0xD,
	ARM64COND_AL = 0xE,
	ARM64COND_NV = 0xF
} ARM64Cond;

#define ARM64COND_INVERT(c) ((c) ^ 1)


/* ======================================================================== */
/*  Shift types (same encoding as ARM32)                                    */
/* ======================================================================== */

typedef enum {
	ARM64SHIFT_LSL = 0,
	ARM64SHIFT_LSR = 1,
	ARM64SHIFT_ASR = 2,
	ARM64SHIFT_ROR = 3
} ARM64ShiftType;


/* ======================================================================== */
/*  ARM32 opcode enum kept for the generic DPIOP compat macros              */
/* ======================================================================== */

typedef enum {
	ARM64OP_AND = 0x0,
	ARM64OP_EOR = 0x1,
	ARM64OP_SUB = 0x2,
	ARM64OP_RSB = 0x3,
	ARM64OP_ADD = 0x4,
	ARM64OP_ADC = 0x5,
	ARM64OP_SBC = 0x6,
	ARM64OP_RSC = 0x7,
	ARM64OP_TST = 0x8,
	ARM64OP_TEQ = 0x9,
	ARM64OP_CMP = 0xa,
	ARM64OP_CMN = 0xb,
	ARM64OP_ORR = 0xc,
	ARM64OP_MOV = 0xd,
	ARM64OP_BIC = 0xe,
	ARM64OP_MVN = 0xf,

	ARM64OP_STR = 0x0,
	ARM64OP_LDR = 0x1,
	ARM64OP_MUL = 0x0,
	ARM64OP_SMULL = 0x6
} ARM64Opcode;


/* ======================================================================== */
/*  Data Processing — Register (W, 32-bit, sf=0)                           */
/* ======================================================================== */

/* ADD Wd, Wn, Wm */
#define ARM64_ADD_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x0B000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* ADDS Wd, Wn, Wm (sets flags) */
#define ARM64_ADDS_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x2B000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* SUB Wd, Wn, Wm */
#define ARM64_SUB_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x4B000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* SUBS Wd, Wn, Wm */
#define ARM64_SUBS_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x6B000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* AND Wd, Wn, Wm */
#define ARM64_AND_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x0A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* ANDS Wd, Wn, Wm (flag-setting) */
#define ARM64_ANDS_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x6A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* ORR Wd, Wn, Wm */
#define ARM64_ORR_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x2A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* EOR Wd, Wn, Wm */
#define ARM64_EOR_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x4A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* ORN Wd, Wn, Wm  (OR NOT) */
#define ARM64_ORN_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x2A200000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* BIC Wd, Wn, Wm = AND Wd, Wn, NOT(Wm) = 0x0A200000 */
#define ARM64_BIC_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x0A200000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* BICS Wd, Wn, Wm = ANDS with inverted Rm = 0x6A200000 */
#define ARM64_BICS_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x6A200000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* EON Wd, Wn, Wm = EOR with inverted Rm = 0x4A200000 */
#define ARM64_EON_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x4A200000 | ((rm) << 16) | ((rn) << 5) | (rd))


/* ======================================================================== */
/*  Data Processing — Register with shifted Rm (W, sf=0)                   */
/*  shift_type: 0=LSL, 1=LSR, 2=ASR                                        */
/* ======================================================================== */

/* ADD Wd, Wn, Wm, <shift> #amount */
#define ARM64_ADD_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x0B000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_ADDS_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x2B000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_SUB_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x4B000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_SUBS_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x6B000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_AND_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x0A000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_ANDS_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x6A000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_ORR_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x2A000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_EOR_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x4A000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_ORN_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x2A200000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))

#define ARM64_BIC_REG_REGSHIFT(p, rd, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x0A200000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5) | (rd))


/* ======================================================================== */
/*  MOV / MVN (register)                                                    */
/* ======================================================================== */

/* MOV Wd, Wm = ORR Wd, WZR, Wm */
#define ARM64_MOV_REG_REG(p, rd, rm) \
	ARM64_EMIT(p, 0x2A000000 | ((rm) << 16) | (31 << 5) | (rd))

/* MVN Wd, Wm = ORN Wd, WZR, Wm */
#define ARM64_MVN_REG_REG(p, rd, rm) \
	ARM64_EMIT(p, 0x2A200000 | ((rm) << 16) | (31 << 5) | (rd))

/* MOVS: not directly available on AArch64, use ORR + ANDS pattern.
   For flag-setting MOV: ANDS Wd, Wm, Wm (tests and moves) */
#define ARM64_MOVS_REG_REG(p, rd, rm) \
	ARM64_EMIT(p, 0x6A000000 | ((rm) << 16) | ((rm) << 5) | (rd))

/* MVNS: ORN with flags -> not available directly; use ORN then TST.
   Approximate: ORN Wd, WZR, Wm; then caller tests if needed.
   We provide MVN only (no flag-setting variant is exact). */
#define ARM64_MVNS_REG_REG(p, rd, rm) \
	do { \
		ARM64_MVN_REG_REG(p, rd, rm); \
		/* set flags by testing result against itself */ \
		ARM64_EMIT(p, 0x6A000000 | ((rd) << 16) | ((rd) << 5) | 31); \
	} while (0)


/* ======================================================================== */
/*  Data Processing — Immediate (W, sf=0)                                  */
/* ======================================================================== */

/* ADD Wd, Wn, #imm12 (shift=0) */
#define ARM64_ADD_REG_IMM(p, rd, rn, imm12, rot) \
	ARM64_EMIT(p, 0x11000000 | (((rot) ? 1 : 0) << 22) | \
		(((imm12) & 0xFFF) << 10) | ((rn) << 5) | (rd))

/* convenience: ADD Wd, Wn, #imm8 (no shift) */
#define ARM64_ADD_REG_IMM8(p, rd, rn, imm8) \
	ARM64_EMIT(p, 0x11000000 | (((imm8) & 0xFFF) << 10) | ((rn) << 5) | (rd))

#define ARM64_ADD_REG_IMM8_COND(p, rd, rn, imm8, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		ARM64_ADD_REG_IMM8(p, rd, rn, imm8); \
	} while (0)

/* ADDS Wd, Wn, #imm12 */
#define ARM64_ADDS_REG_IMM8(p, rd, rn, imm8) \
	ARM64_EMIT(p, 0x31000000 | (((imm8) & 0xFFF) << 10) | ((rn) << 5) | (rd))

/* SUB Wd, Wn, #imm12 */
#define ARM64_SUB_REG_IMM8(p, rd, rn, imm8) \
	ARM64_EMIT(p, 0x51000000 | (((imm8) & 0xFFF) << 10) | ((rn) << 5) | (rd))

#define ARM64_SUB_REG_IMM8_COND(p, rd, rn, imm8, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		ARM64_SUB_REG_IMM8(p, rd, rn, imm8); \
	} while (0)

/* SUBS Wd, Wn, #imm12 */
#define ARM64_SUBS_REG_IMM8(p, rd, rn, imm8) \
	ARM64_EMIT(p, 0x71000000 | (((imm8) & 0xFFF) << 10) | ((rn) << 5) | (rd))


/* ======================================================================== */
/*  CMP / CMN (immediate and register)                                     */
/* ======================================================================== */

/* CMP Wn, #imm12 = SUBS WZR, Wn, #imm12 */
#define ARM64_CMP_REG_IMM8(p, rn, imm8) \
	ARM64_EMIT(p, 0x7100001F | (((imm8) & 0xFFF) << 10) | ((rn) << 5))

/* CMP Wn, #imm8 ROR rot — on AArch64 the rot param from ARM32's
   imm8-ror scheme must be translated. For small values (rot=0),
   this works directly. For rotated, caller must pre-compute. */
#define ARM64_CMP_REG_IMM(p, rn, imm8, rot) \
	ARM64_CMP_REG_IMM8(p, rn, ARM64_DECODE_IMM8ROT(imm8, rot))

/* CMP Wn, Wm = SUBS WZR, Wn, Wm */
#define ARM64_CMP_REG_REG(p, rn, rm) \
	ARM64_EMIT(p, 0x6B00001F | ((rm) << 16) | ((rn) << 5))

/* CMP Wn, Wm, <shift> #amount */
#define ARM64_CMP_REG_IMMSHIFT(p, rn, rm, shift_type, amount) \
	ARM64_EMIT(p, 0x6B00001F | ((shift_type) << 22) | ((rm) << 16) | \
		(((amount) & 0x3F) << 10) | ((rn) << 5))

/* CMP with register-shifted register: use SUBS WZR with shifted Rm.
   AArch64 doesn't have register-by-register shifts in data processing,
   so we use LSLV/LSRV/ASRV into IP0 first, then CMP. */
#define ARM64_CMP_REG_REGSHIFT(p, rn, rm, shift_type, rs) \
	do { \
		ARM64_SHIFT_REG(p, ARM64REG_IP0, rm, shift_type, rs); \
		ARM64_CMP_REG_REG(p, rn, ARM64REG_IP0); \
	} while (0)

/* CMN Wn, #imm8 = ADDS WZR, Wn, #imm */
#define ARM64_CMN_REG_IMM8(p, rn, imm8) \
	ARM64_EMIT(p, 0x3100001F | (((imm8) & 0xFFF) << 10) | ((rn) << 5))

#define ARM64_CMN_REG_IMM(p, rn, imm8, rot) \
	ARM64_CMN_REG_IMM8(p, rn, ARM64_DECODE_IMM8ROT(imm8, rot))

/* CMN Wn, Wm = ADDS WZR, Wn, Wm */
#define ARM64_CMN_REG_REG(p, rn, rm) \
	ARM64_EMIT(p, 0x2B00001F | ((rm) << 16) | ((rn) << 5))


/* ======================================================================== */
/*  Move (Wide Immediate)                                                   */
/* ======================================================================== */

/* MOVZ Wd, #imm16 {, LSL #shift}  shift=0 or 16 (hw=0 or 1) */
#define ARM64_MOVZ(p, rd, imm16, hw) \
	ARM64_EMIT(p, 0x52800000 | ((hw) << 21) | (((imm16) & 0xFFFF) << 5) | (rd))

/* MOVK Wd, #imm16 {, LSL #shift} */
#define ARM64_MOVK(p, rd, imm16, hw) \
	ARM64_EMIT(p, 0x72800000 | ((hw) << 21) | (((imm16) & 0xFFFF) << 5) | (rd))

/* MOVN Wd, #imm16 */
#define ARM64_MOVN(p, rd, imm16, hw) \
	ARM64_EMIT(p, 0x12800000 | ((hw) << 21) | (((imm16) & 0xFFFF) << 5) | (rd))

/* MOV Wd, #imm8  (convenience: MOVZ with hw=0) */
#define ARM64_MOV_REG_IMM8(p, rd, imm8) \
	ARM64_MOVZ(p, rd, (imm8) & 0xFF, 0)

#define ARM64_MOV_REG_IMM8_COND(p, rd, imm8, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		ARM64_MOV_REG_IMM8(p, rd, imm8); \
	} while (0)

/* Load a full 32-bit immediate into Wd */
#define ARM64_MOV_REG_IMM32(p, rd, imm32) \
	do { \
		ARM64_MOVZ(p, rd, (unsigned int)(imm32) & 0xFFFF, 0); \
		if ((unsigned int)(imm32) > 0xFFFF) { \
			ARM64_MOVK(p, rd, ((unsigned int)(imm32) >> 16) & 0xFFFF, 1); \
		} \
	} while (0)


/* ======================================================================== */
/*  Shifts — Register (W, sf=0)                                            */
/* ======================================================================== */

/* LSLV Wd, Wn, Wm */
#define ARM64_LSLV(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x1AC02000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* LSRV Wd, Wn, Wm */
#define ARM64_LSRV(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x1AC02400 | ((rm) << 16) | ((rn) << 5) | (rd))

/* ASRV Wd, Wn, Wm */
#define ARM64_ASRV(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x1AC02800 | ((rm) << 16) | ((rn) << 5) | (rd))

/* RORV Wd, Wn, Wm */
#define ARM64_RORV(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x1AC02C00 | ((rm) << 16) | ((rn) << 5) | (rd))

/* Generic shift-by-register dispatch */
#define ARM64_SHIFT_REG(p, rd, rn, shift_type, rs) \
	do { \
		switch (shift_type) { \
		case ARM64SHIFT_LSL: ARM64_LSLV(p, rd, rn, rs); break; \
		case ARM64SHIFT_LSR: ARM64_LSRV(p, rd, rn, rs); break; \
		case ARM64SHIFT_ASR: ARM64_ASRV(p, rd, rn, rs); break; \
		case ARM64SHIFT_ROR: ARM64_RORV(p, rd, rn, rs); break; \
		} \
	} while (0)


/* ======================================================================== */
/*  Shifts — Immediate (W, sf=0) using UBFM/SBFM                          */
/* ======================================================================== */

/* LSL Wd, Wn, #sh = UBFM Wd, Wn, #(-sh MOD 32), #(31-sh) */
#define ARM64_LSL_IMM(p, rd, rn, sh) \
	ARM64_EMIT(p, 0x53000000 | (((-(sh)) & 31) << 16) | ((31 - (sh)) << 10) | ((rn) << 5) | (rd))

/* LSR Wd, Wn, #sh = UBFM Wd, Wn, #sh, #31 */
#define ARM64_LSR_IMM(p, rd, rn, sh) \
	ARM64_EMIT(p, 0x53000000 | ((sh) << 16) | (31 << 10) | ((rn) << 5) | (rd))

/* ASR Wd, Wn, #sh = SBFM Wd, Wn, #sh, #31 */
#define ARM64_ASR_IMM(p, rd, rn, sh) \
	ARM64_EMIT(p, 0x13000000 | ((sh) << 16) | (31 << 10) | ((rn) << 5) | (rd))

/* Aliases matching ARM32 naming */
#define ARM64_SHL_IMM(p, rd, rm, imm) ARM64_LSL_IMM(p, rd, rm, imm)
#define ARM64_SHR_IMM(p, rd, rm, imm) ARM64_LSR_IMM(p, rd, rm, imm)
#define ARM64_SAR_IMM(p, rd, rm, imm) ARM64_ASR_IMM(p, rd, rm, imm)

#define ARM64_SHL_REG(p, rd, rm, rs) ARM64_LSLV(p, rd, rm, rs)
#define ARM64_SHR_REG(p, rd, rm, rs) ARM64_LSRV(p, rd, rm, rs)
#define ARM64_SAR_REG(p, rd, rm, rs) ARM64_ASRV(p, rd, rm, rs)
#define ARM64_ROR_REG(p, rd, rm, rs) ARM64_RORV(p, rd, rm, rs)

/* Flag-setting shift aliases — do shift then TST result */
#define ARM64_SHLS_IMM(p, rd, rm, imm) \
	do { ARM64_LSL_IMM(p, rd, rm, imm); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while (0)
#define ARM64_SHRS_IMM(p, rd, rm, imm) \
	do { ARM64_LSR_IMM(p, rd, rm, imm); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while (0)
#define ARM64_SARS_IMM(p, rd, rm, imm) \
	do { ARM64_ASR_IMM(p, rd, rm, imm); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while (0)

#define ARM64_SHLS_REG(p, rd, rm, rs) \
	do { ARM64_LSLV(p, rd, rm, rs); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while (0)
#define ARM64_SHRS_REG(p, rd, rm, rs) \
	do { ARM64_LSRV(p, rd, rm, rs); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while (0)
#define ARM64_SARS_REG(p, rd, rm, rs) \
	do { ARM64_ASRV(p, rd, rm, rs); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while (0)

#define ARM64_SHLS_REG_REG(p, rd, rm, rs) ARM64_SHLS_REG(p, rd, rm, rs)
#define ARM64_SHRS_REG_REG(p, rd, rm, rs) ARM64_SHRS_REG(p, rd, rm, rs)
#define ARM64_SARS_REG_REG(p, rd, rm, rs) ARM64_SARS_REG(p, rd, rm, rs)


/* ======================================================================== */
/*  Multiply                                                                */
/* ======================================================================== */

/* MUL Wd, Wn, Wm = MADD Wd, Wn, Wm, WZR */
#define ARM64_MUL(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x1B007C00 | ((rm) << 16) | ((rn) << 5) | (rd))

#define ARM64_MUL_REG_REG(p, rd, rm, rs) ARM64_MUL(p, rd, rm, rs)

/* MULS — no direct encoding; MUL then set flags */
#define ARM64_MULS(p, rd, rn, rm) \
	do { \
		ARM64_MUL(p, rd, rn, rm); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

#define ARM64_MULS_REG_REG(p, rd, rm, rs) ARM64_MULS(p, rd, rm, rs)

/* SMULL Xd, Wn, Wm = SMADDL Xd, Wn, Wm, XZR */
#define ARM64_SMULL(p, rd_lo, rd_hi, rm, rs) \
	do { \
		ARM64_EMIT(p, 0x9B207C00 | ((rs) << 16) | ((rm) << 5) | (rd_lo)); \
		/* extract high 32 bits: ASR Xd_hi, Xd_lo, #32 (64-bit) */ \
		ARM64_EMIT(p, 0x9340FC00 | ((rd_lo) << 5) | (rd_hi)); \
	} while (0)

/* MLA Wd, Wn, Wm, Wa = MADD Wd, Wn, Wm, Wa */
#define ARM64_MLA(p, rd, rn, rm, ra) \
	ARM64_EMIT(p, 0x1B000000 | ((rm) << 16) | ((ra) << 10) | ((rn) << 5) | (rd))


/* ======================================================================== */
/*  Load/Store — Unsigned Offset (scaled)                                   */
/* ======================================================================== */

/* LDR Wt, [Xn, #imm]  imm is byte offset, must be multiple of 4 */
#define ARM64_LDR_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0xB9400000 | ((((unsigned int)(imm) / 4) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* STR Wt, [Xn, #imm] */
#define ARM64_STR_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0xB9000000 | ((((unsigned int)(imm) / 4) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* LDRH Wt, [Xn, #imm]  imm must be multiple of 2 */
#define ARM64_LDRH_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0x79400000 | ((((unsigned int)(imm) / 2) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* STRH Wt, [Xn, #imm] */
#define ARM64_STRH_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0x79000000 | ((((unsigned int)(imm) / 2) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* LDRB Wt, [Xn, #imm] */
#define ARM64_LDRB_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0x39400000 | (((unsigned int)(imm) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* STRB Wt, [Xn, #imm] */
#define ARM64_STRB_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0x39000000 | (((unsigned int)(imm) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* LDRSH Wt, [Xn, #imm]  (sign-extending halfword load into W) */
#define ARM64_LDRSH_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0x79C00000 | ((((unsigned int)(imm) / 2) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* LDRSB Wt, [Xn, #imm]  (sign-extending byte load into W) */
#define ARM64_LDRSB_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0x39C00000 | (((unsigned int)(imm) & 0xFFF) << 10) | ((rn) << 5) | (rt))


/* ======================================================================== */
/*  Load/Store — Unscaled (for negative/unaligned offsets, simm9)           */
/* ======================================================================== */

#define ARM64_LDUR(p, rt, rn, simm9) \
	ARM64_EMIT(p, 0xB8400000 | (((simm9) & 0x1FF) << 12) | ((rn) << 5) | (rt))

#define ARM64_STUR(p, rt, rn, simm9) \
	ARM64_EMIT(p, 0xB8000000 | (((simm9) & 0x1FF) << 12) | ((rn) << 5) | (rt))

#define ARM64_LDURH(p, rt, rn, simm9) \
	ARM64_EMIT(p, 0x78400000 | (((simm9) & 0x1FF) << 12) | ((rn) << 5) | (rt))

#define ARM64_STURH(p, rt, rn, simm9) \
	ARM64_EMIT(p, 0x78000000 | (((simm9) & 0x1FF) << 12) | ((rn) << 5) | (rt))

#define ARM64_LDURB(p, rt, rn, simm9) \
	ARM64_EMIT(p, 0x38400000 | (((simm9) & 0x1FF) << 12) | ((rn) << 5) | (rt))

#define ARM64_STURB(p, rt, rn, simm9) \
	ARM64_EMIT(p, 0x38000000 | (((simm9) & 0x1FF) << 12) | ((rn) << 5) | (rt))


/* ======================================================================== */
/*  Load/Store — Register Offset                                            */
/* ======================================================================== */

/* LDR Wt, [Xn, Xm] = LDR Wt, [Xn, Xm, LSL #0] */
#define ARM64_LDR_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0xB8606800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* STR Wt, [Xn, Xm] */
#define ARM64_STR_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0xB8206800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* LDRH Wt, [Xn, Xm] */
#define ARM64_LDRH_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0x78606800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* STRH Wt, [Xn, Xm] */
#define ARM64_STRH_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0x78206800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* LDRB Wt, [Xn, Xm] */
#define ARM64_LDRB_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0x38606800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* STRB Wt, [Xn, Xm] */
#define ARM64_STRB_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0x38206800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* LDRSH Wt, [Xn, Xm] */
#define ARM64_LDRSH_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0x78E06800 | ((rm) << 16) | ((rn) << 5) | (rt))

/* LDRSB Wt, [Xn, Xm] */
#define ARM64_LDRSB_REG_REG(p, rt, rn, rm) \
	ARM64_EMIT(p, 0x38E06800 | ((rm) << 16) | ((rn) << 5) | (rt))


/* ======================================================================== */
/*  Load/Store — with shifted register (for LDR Wt,[Xn,Rm,shift #amt])    */
/* ======================================================================== */

#define ARM64_LDR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
	ARM64_LDR_REG_REG(p, rd, rn, rm)

#define ARM64_STR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
	ARM64_STR_REG_REG(p, rd, rn, rm)

#define ARM64_LDRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
	ARM64_LDRB_REG_REG(p, rd, rn, rm)

#define ARM64_STRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) \
	ARM64_STRB_REG_REG(p, rd, rn, rm)


/* ======================================================================== */
/*  Load/Store Pair                                                         */
/* ======================================================================== */

/* STP Wt1, Wt2, [Xn, #imm]! (pre-index, 32-bit) */
#define ARM64_STP_W_PRE(p, rt1, rt2, rn, imm) \
	ARM64_EMIT(p, 0x29800000 | (((((int)(imm)) / 4) & 0x7F) << 15) | \
		((rt2) << 10) | ((rn) << 5) | (rt1))

/* LDP Wt1, Wt2, [Xn], #imm (post-index, 32-bit) */
#define ARM64_LDP_W_POST(p, rt1, rt2, rn, imm) \
	ARM64_EMIT(p, 0x28C00000 | (((((int)(imm)) / 4) & 0x7F) << 15) | \
		((rt2) << 10) | ((rn) << 5) | (rt1))

/* STP Xt1, Xt2, [Xn, #imm]! (pre-index, 64-bit) */
#define ARM64_STP_X_PRE(p, rt1, rt2, rn, imm) \
	ARM64_EMIT(p, 0xA9800000 | (((((int)(imm)) / 8) & 0x7F) << 15) | \
		((rt2) << 10) | ((rn) << 5) | (rt1))

/* LDP Xt1, Xt2, [Xn], #imm (post-index, 64-bit) */
#define ARM64_LDP_X_POST(p, rt1, rt2, rn, imm) \
	ARM64_EMIT(p, 0xA8C00000 | (((((int)(imm)) / 8) & 0x7F) << 15) | \
		((rt2) << 10) | ((rn) << 5) | (rt1))

/* STP Xt1, Xt2, [Xn, #imm] (signed offset, 64-bit) */
#define ARM64_STP_X(p, rt1, rt2, rn, imm) \
	ARM64_EMIT(p, 0xA9000000 | (((((int)(imm)) / 8) & 0x7F) << 15) | \
		((rt2) << 10) | ((rn) << 5) | (rt1))

/* LDP Xt1, Xt2, [Xn, #imm] (signed offset, 64-bit) */
#define ARM64_LDP_X(p, rt1, rt2, rn, imm) \
	ARM64_EMIT(p, 0xA9400000 | (((((int)(imm)) / 8) & 0x7F) << 15) | \
		((rt2) << 10) | ((rn) << 5) | (rt1))


/* ======================================================================== */
/*  Push/Pop emulation (AArch64 uses STP/LDP to SP)                        */
/* ======================================================================== */

/* PUSH two X registers: STP Xt1, Xt2, [SP, #-16]! */
#define ARM64_PUSH2(p, r1, r2) \
	ARM64_STP_X_PRE(p, r1, r2, ARM64REG_SP, -16)

/* POP two X registers: LDP Xt1, Xt2, [SP], #16 */
#define ARM64_POP2(p, r1, r2) \
	ARM64_LDP_X_POST(p, r1, r2, ARM64REG_SP, 16)

/* Single register push/pop via STR/LDR with pre/post index */
#define ARM64_PUSH1(p, r1) \
	ARM64_EMIT(p, 0xF81F0C00 | (((unsigned int)(-16) & 0x1FF) << 12) | \
		(ARM64REG_SP << 5) | (r1))  /* STR Xt, [SP, #-16]! */

#define ARM64_POP1(p, r1) \
	ARM64_EMIT(p, 0xF8410400 | ((16 & 0x1FF) << 12) | \
		(ARM64REG_SP << 5) | (r1))  /* LDR Xt, [SP], #16 */


/* ======================================================================== */
/*  Branches                                                                */
/* ======================================================================== */

/* B imm26 (PC-relative, offset in instructions) */
#define ARM64_B(p, offset) \
	ARM64_EMIT(p, 0x14000000 | ((offset) & 0x3FFFFFF))

/* BL imm26 */
#define ARM64_BL(p, offset) \
	ARM64_EMIT(p, 0x94000000 | ((offset) & 0x3FFFFFF))

/* B.cond imm19 (offset in instructions) */
#define ARM64_B_COND(p, cond, offset) \
	ARM64_EMIT(p, 0x54000000 | ((((unsigned int)(offset)) & 0x7FFFF) << 5) | (cond))

/* BR Xn */
#define ARM64_BR(p, rn) \
	ARM64_EMIT(p, 0xD61F0000 | ((rn) << 5))

/* BLR Xn */
#define ARM64_BLR(p, rn) \
	ARM64_EMIT(p, 0xD63F0000 | ((rn) << 5))

/* RET {Xn} (default X30) */
#define ARM64_RET(p) \
	ARM64_EMIT(p, 0xD65F03C0)

#define ARM64_RET_REG(p, rn) \
	ARM64_EMIT(p, 0xD65F0000 | ((rn) << 5))


/* ======================================================================== */
/*  Conditional Select                                                      */
/* ======================================================================== */

/* CSEL Wd, Wn, Wm, cond */
#define ARM64_CSEL(p, rd, rn, rm, cond) \
	ARM64_EMIT(p, 0x1A800000 | ((rm) << 16) | ((cond) << 12) | ((rn) << 5) | (rd))

/* CSINC Wd, Wn, Wm, cond */
#define ARM64_CSINC(p, rd, rn, rm, cond) \
	ARM64_EMIT(p, 0x1A800400 | ((rm) << 16) | ((cond) << 12) | ((rn) << 5) | (rd))

/* CSINV Wd, Wn, Wm, cond */
#define ARM64_CSINV(p, rd, rn, rm, cond) \
	ARM64_EMIT(p, 0x5A800000 | ((rm) << 16) | ((cond) << 12) | ((rn) << 5) | (rd))

/* CSNEG Wd, Wn, Wm, cond */
#define ARM64_CSNEG(p, rd, rn, rm, cond) \
	ARM64_EMIT(p, 0x5A800400 | ((rm) << 16) | ((cond) << 12) | ((rn) << 5) | (rd))


/* ======================================================================== */
/*  System / Miscellaneous                                                  */
/* ======================================================================== */

/* MRS Xd, NZCV */
#define ARM64_MRS_NZCV(p, rd) \
	ARM64_EMIT(p, 0xD53B4200 | (rd))

/* MSR NZCV, Xn */
#define ARM64_MSR_NZCV(p, rn) \
	ARM64_EMIT(p, 0xD51B4200 | (rn))

/* NOP */
#define ARM64_NOP(p) \
	ARM64_EMIT(p, 0xD503201F)

/* CLZ Wd, Wn */
#define ARM64_CLZ(p, rd, rn) \
	ARM64_EMIT(p, 0x5AC01000 | ((rn) << 5) | (rd))

/* BRK #imm16 (debug breakpoint) */
#define ARM64_DBRK(p) \
	ARM64_EMIT(p, 0xD4200000)

/* INC / DEC convenience */
#define ARM64_INC(p, reg) ARM64_ADD_REG_IMM8(p, reg, reg, 1)
#define ARM64_DEC(p, reg) ARM64_SUB_REG_IMM8(p, reg, reg, 1)


/* ======================================================================== */
/*  64-bit variants for pointer operations (sf=1)                           */
/* ======================================================================== */

/* ADD Xd, Xn, Xm */
#define ARM64_ADD_X_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x8B000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* SUB Xd, Xn, Xm */
#define ARM64_SUB_X_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0xCB000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* MOV Xd, Xm = ORR Xd, XZR, Xm */
#define ARM64_MOV_X_REG_REG(p, rd, rm) \
	ARM64_EMIT(p, 0xAA000000 | ((rm) << 16) | (31 << 5) | (rd))

/* ADD Xd, Xn, #imm12 */
#define ARM64_ADD_X_REG_IMM(p, rd, rn, imm12) \
	ARM64_EMIT(p, 0x91000000 | (((imm12) & 0xFFF) << 10) | ((rn) << 5) | (rd))

/* SUB Xd, Xn, #imm12 */
#define ARM64_SUB_X_REG_IMM(p, rd, rn, imm12) \
	ARM64_EMIT(p, 0xD1000000 | (((imm12) & 0xFFF) << 10) | ((rn) << 5) | (rd))

/* LDR Xt, [Xn, #imm] (scaled by 8) */
#define ARM64_LDR_X_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0xF9400000 | ((((unsigned int)(imm) / 8) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* STR Xt, [Xn, #imm] (scaled by 8) */
#define ARM64_STR_X_IMM(p, rt, rn, imm) \
	ARM64_EMIT(p, 0xF9000000 | ((((unsigned int)(imm) / 8) & 0xFFF) << 10) | ((rn) << 5) | (rt))

/* MOVZ Xd, #imm16 (64-bit) */
#define ARM64_MOVZ_X(p, rd, imm16, hw) \
	ARM64_EMIT(p, 0xD2800000 | ((hw) << 21) | (((imm16) & 0xFFFF) << 5) | (rd))

/* MOVK Xd, #imm16 (64-bit) */
#define ARM64_MOVK_X(p, rd, imm16, hw) \
	ARM64_EMIT(p, 0xF2800000 | ((hw) << 21) | (((imm16) & 0xFFFF) << 5) | (rd))


/* ======================================================================== */
/*  _COND variants (conditional execution)                                  */
/*  AArch64 has no predicated instructions; we use either CSEL or           */
/*  B.cond-skip patterns.                                                   */
/* ======================================================================== */

/* MOV_REG_REG_COND: CSEL rd, rm, rd, cond */
#define ARM64_MOV_REG_REG_COND(p, rd, rm, cond) \
	ARM64_CSEL(p, rd, rm, rd, cond)

/* MVN_REG_REG_COND: B.invcond skip; MVN rd, rm */
#define ARM64_MVN_REG_REG_COND(p, rd, rm, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		ARM64_MVN_REG_REG(p, rd, rm); \
	} while (0)

/* Generic _COND for data processing: B.invcond over 1 instruction */
#define ARM64_ADD_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_ADD_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_SUB_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SUB_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_AND_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_AND_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_ORR_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_ORR_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_EOR_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_EOR_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_BIC_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_BIC_REG_REG(p, rd, rn, rm); } while (0)

/* Shifted register _COND variants */
#define ARM64_ADD_REG_REGSHIFT_COND(p, rd, rn, rm, st, amt, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_ADD_REG_REGSHIFT(p, rd, rn, rm, st, amt); } while (0)

#define ARM64_SUB_REG_REGSHIFT_COND(p, rd, rn, rm, st, amt, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SUB_REG_REGSHIFT(p, rd, rn, rm, st, amt); } while (0)

#define ARM64_AND_REG_REGSHIFT_COND(p, rd, rn, rm, st, amt, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_AND_REG_REGSHIFT(p, rd, rn, rm, st, amt); } while (0)

#define ARM64_ORR_REG_REGSHIFT_COND(p, rd, rn, rm, st, amt, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_ORR_REG_REGSHIFT(p, rd, rn, rm, st, amt); } while (0)

#define ARM64_EOR_REG_REGSHIFT_COND(p, rd, rn, rm, st, amt, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_EOR_REG_REGSHIFT(p, rd, rn, rm, st, amt); } while (0)

/* Flag-setting _COND variants */
#define ARM64_ADDS_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_ADDS_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_SUBS_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SUBS_REG_REG(p, rd, rn, rm); } while (0)

#define ARM64_ANDS_REG_REG_COND(p, rd, rn, rm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_ANDS_REG_REG(p, rd, rn, rm); } while (0)

/* Immediate shift _COND: uses the shifted-register encoding with B.cond skip */
#define ARM64_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) \
	ARM64_EMIT(p, 0x2A000000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((imm_shift) & 0x3F) << 10) | (31 << 5) | (rd))

#define ARM64_MOVS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) \
	do { \
		ARM64_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

#define ARM64_MOV_REG_IMMSHIFT_COND(p, rd, rm, shift_type, imm_shift, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		ARM64_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); \
	} while (0)

#define ARM64_MOVS_REG_IMMSHIFT_COND(p, rd, rm, shift_type, imm_shift, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 3); \
		ARM64_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

/* MVN with immediate shift */
#define ARM64_MVN_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) \
	ARM64_EMIT(p, 0x2A200000 | ((shift_type) << 22) | ((rm) << 16) | \
		(((imm_shift) & 0x3F) << 10) | (31 << 5) | (rd))

#define ARM64_MVNS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) \
	do { \
		ARM64_MVN_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

/* MOV/MVN with register shift (use variable shift instructions) */
#define ARM64_MOV_REG_REGSHIFT(p, rd, rm, shift_type, rs) \
	ARM64_SHIFT_REG(p, rd, rm, shift_type, rs)

#define ARM64_MOVS_REG_REGSHIFT(p, rd, rm, shift_type, rs) \
	do { \
		ARM64_SHIFT_REG(p, rd, rm, shift_type, rs); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

#define ARM64_MVN_REG_REGSHIFT(p, rd, rm, shift_type, rs) \
	do { \
		ARM64_SHIFT_REG(p, ARM64REG_IP0, rm, shift_type, rs); \
		ARM64_MVN_REG_REG(p, rd, ARM64REG_IP0); \
	} while (0)

#define ARM64_MVNS_REG_REGSHIFT(p, rd, rm, shift_type, rs) \
	do { \
		ARM64_SHIFT_REG(p, ARM64REG_IP0, rm, shift_type, rs); \
		ARM64_MVN_REG_REG(p, rd, ARM64REG_IP0); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

/* MVN with immediate (imm8, rot): negate the decoded immediate */
#define ARM64_MVN_REG_IMM(p, rd, imm8, rot) \
	do { \
		ARM64_MOV_REG_IMM8(p, rd, ARM64_DECODE_IMM8ROT(imm8, rot)); \
		ARM64_MVN_REG_REG(p, rd, rd); \
	} while (0)

#define ARM64_MVNS_REG_IMM(p, rd, imm8, rot) \
	do { \
		ARM64_MVN_REG_IMM(p, rd, imm8, rot); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)


/* ======================================================================== */
/*  RSB (Reverse Subtract) — not native on AArch64                         */
/*  RSB Wd, Wn, #imm = SUB Wd, WZR, Wn then ADD Wd, Wd, #imm             */
/*  RSB Wd, Wn, #imm8 with rot=0: NEGS + ADD                              */
/*  Simplified: RSB Wd, Wn, #0 = NEG = SUB Wd, WZR, Wn                    */
/* ======================================================================== */

/* NEG Wd, Wn = SUB Wd, WZR, Wn */
#define ARM64_NEG(p, rd, rn) \
	ARM64_EMIT(p, 0x4B000000 | ((rn) << 16) | (31 << 5) | (rd))

/* NEGS Wd, Wn = SUBS Wd, WZR, Wn */
#define ARM64_NEGS(p, rd, rn) \
	ARM64_EMIT(p, 0x6B000000 | ((rn) << 16) | (31 << 5) | (rd))

/* RSB Wd, Wn, #imm8 (rot=0): NEG Wd, Wn; ADD Wd, Wd, #imm8 */
#define ARM64_RSB_REG_IMM(p, rd, rn, imm8, rot) \
	do { \
		ARM64_NEG(p, rd, rn); \
		if (ARM64_DECODE_IMM8ROT(imm8, rot) != 0) { \
			ARM64_ADD_REG_IMM8(p, rd, rd, ARM64_DECODE_IMM8ROT(imm8, rot)); \
		} \
	} while (0)

#define ARM64_RSB_REG_IMM8(p, rd, rn, imm8) \
	ARM64_RSB_REG_IMM(p, rd, rn, imm8, 0)

#define ARM64_RSBS_REG_IMM(p, rd, rn, imm8, rot) \
	do { \
		ARM64_NEGS(p, rd, rn); \
		if (ARM64_DECODE_IMM8ROT(imm8, rot) != 0) { \
			ARM64_ADDS_REG_IMM8(p, rd, rd, ARM64_DECODE_IMM8ROT(imm8, rot)); \
		} \
	} while (0)

#define ARM64_RSBS_REG_IMM8(p, rd, rn, imm8) \
	ARM64_RSBS_REG_IMM(p, rd, rn, imm8, 0)

/* RSB Wd, Wn, Wm = SUB Wd, Wm, Wn */
#define ARM64_RSB_REG_REG(p, rd, rn, rm) \
	ARM64_SUB_REG_REG(p, rd, rm, rn)

#define ARM64_RSBS_REG_REG(p, rd, rn, rm) \
	ARM64_SUBS_REG_REG(p, rd, rm, rn)


/* ======================================================================== */
/*  ADC / SBC — these exist on AArch64 with same semantics                  */
/* ======================================================================== */

/* ADC Wd, Wn, Wm */
#define ARM64_ADC_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x1A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* ADCS Wd, Wn, Wm */
#define ARM64_ADCS_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x3A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* SBC Wd, Wn, Wm */
#define ARM64_SBC_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x5A000000 | ((rm) << 16) | ((rn) << 5) | (rd))

/* SBCS Wd, Wn, Wm */
#define ARM64_SBCS_REG_REG(p, rd, rn, rm) \
	ARM64_EMIT(p, 0x7A000000 | ((rm) << 16) | ((rn) << 5) | (rd))


/* ======================================================================== */
/*  TST (= ANDS with WZR dest)                                             */
/* ======================================================================== */

/* TST Wn, Wm = ANDS WZR, Wn, Wm */
#define ARM64_TST_REG_REG(p, rn, rm) \
	ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rn, rm)


/* ======================================================================== */
/*  ORR/ORRS with immediate shift (for emit.c compatibility)               */
/* ======================================================================== */

#define ARM64_ORR_REG_IMMSHIFT(p, rd, rn, rm, shift_type, imm_shift) \
	ARM64_ORR_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift)

#define ARM64_ORRS_REG_IMMSHIFT(p, rd, rn, rm, shift_type, imm_shift) \
	do { \
		ARM64_ORR_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); \
		ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); \
	} while (0)

/* AND with immediate value (imm8 with rot=0) */
#define ARM64_ANDS_REG_IMM(p, rd, rn, imm8, rot) \
	do { \
		ARM64_MOV_REG_IMM8(p, ARM64REG_IP0, ARM64_DECODE_IMM8ROT(imm8, rot)); \
		ARM64_ANDS_REG_REG(p, rd, rn, ARM64REG_IP0); \
	} while (0)

#define ARM64_ANDS_REG_IMM8(p, rd, rn, imm8) \
	ARM64_ANDS_REG_IMM(p, rd, rn, imm8, 0)


/* ======================================================================== */
/*  MRS/MSR compatibility with ARM32 API                                    */
/* ======================================================================== */

#define ARM64_MRS_CPSR(p, rd) ARM64_MRS_NZCV(p, rd)
#define ARM64_MSR_REG(p, mask, rn, sel) ARM64_MSR_NZCV(p, rn)


/* ======================================================================== */
/*  ARM32 imm8-with-rotation decoding helper                                */
/*  ARM32 encodes immediates as imm8 ROR (rot*2).                          */
/*  This helper decodes to a plain value for use in AArch64 immediates.     */
/* ======================================================================== */

#define ARM64_DECODE_IMM8ROT(imm8, rot) \
	(((rot) == 0) ? ((imm8) & 0xFF) : \
	 ((((imm8) & 0xFF) >> ((rot) & 0x1E)) | (((imm8) & 0xFF) << (32 - ((rot) & 0x1E)))))


/* ======================================================================== */
/*  Generic DPIOP macros (emit.c uses these directly)                       */
/*  These dispatch based on the ARM32 opcode enum to the correct AArch64    */
/*  instruction. Only the register-register forms used by emit.c.           */
/* ======================================================================== */

/*
 * ARM64_DPIOP_REG_REG_COND dispatches based on ARM32 opcode value.
 * The opcodes used by emit.c via DPIOP are: AND, EOR, SUB, RSB,
 * ADD, ADC, SBC, ORR, BIC, MOV, MVN, TST, CMP, CMN, TEQ, RSC.
 */
#define ARM64_DPIOP_REG_REG_COND(p, op, rd, rn, rm, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		switch (op) { \
		case ARM64OP_AND: ARM64_AND_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_EOR: ARM64_EOR_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_SUB: ARM64_SUB_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_RSB: ARM64_SUB_REG_REG(p, rd, rm, rn); break; \
		case ARM64OP_ADD: ARM64_ADD_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_ADC: ARM64_ADC_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_SBC: ARM64_SBC_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_ORR: ARM64_ORR_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_MOV: ARM64_MOV_REG_REG(p, rd, rm); break; \
		case ARM64OP_BIC: ARM64_BIC_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_MVN: ARM64_MVN_REG_REG(p, rd, rm); break; \
		default: ARM64_NOP(p); break; \
		} \
	} while (0)

#define ARM64_DPIOP_S_REG_REG_COND(p, op, rd, rn, rm, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		switch (op) { \
		case ARM64OP_AND: ARM64_ANDS_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_SUB: ARM64_SUBS_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_RSB: ARM64_SUBS_REG_REG(p, rd, rm, rn); break; \
		case ARM64OP_ADD: ARM64_ADDS_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_ADC: ARM64_ADCS_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_SBC: ARM64_SBCS_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_TST: ARM64_TST_REG_REG(p, rn, rm); break; \
		case ARM64OP_CMP: ARM64_CMP_REG_REG(p, rn, rm); break; \
		case ARM64OP_CMN: ARM64_CMN_REG_REG(p, rn, rm); break; \
		case ARM64OP_ORR: do { ARM64_ORR_REG_REG(p, rd, rn, rm); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while(0); break; \
		case ARM64OP_EOR: do { ARM64_EOR_REG_REG(p, rd, rn, rm); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while(0); break; \
		case ARM64OP_MOV: ARM64_MOVS_REG_REG(p, rd, rm); break; \
		case ARM64OP_BIC: ARM64_BICS_REG_REG(p, rd, rn, rm); break; \
		case ARM64OP_MVN: ARM64_MVNS_REG_REG(p, rd, rm); break; \
		default: ARM64_NOP(p); break; \
		} \
	} while (0)

/* DPIOP with immediate-shifted register */
#define ARM64_DPIOP_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); \
		switch (op) { \
		case ARM64OP_AND: ARM64_AND_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_EOR: ARM64_EOR_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_SUB: ARM64_SUB_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_RSB: ARM64_SUB_REG_REGSHIFT(p, rd, rm, rn, shift_type, imm_shift); break; \
		case ARM64OP_ADD: ARM64_ADD_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_ORR: ARM64_ORR_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_BIC: ARM64_BIC_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_MOV: ARM64_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); break; \
		case ARM64OP_MVN: ARM64_MVN_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); break; \
		default: ARM64_NOP(p); break; \
		} \
	} while (0)

#define ARM64_DPIOP_S_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
	do { \
		ARM64_B_COND(p, ARM64COND_INVERT(cond), 3); \
		switch (op) { \
		case ARM64OP_AND: ARM64_AND_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); break; \
		case ARM64OP_SUB: ARM64_SUBS_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_ADD: ARM64_ADDS_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_CMP: ARM64_CMP_REG_IMMSHIFT(p, rn, rm, shift_type, imm_shift); break; \
		case ARM64OP_CMN: do { ARM64_ADD_REG_REGSHIFT(p, ARM64REG_IP0, rn, rm, shift_type, imm_shift); ARM64_CMN_REG_REG(p, ARM64REG_IP0, ARM64REG_WZR); } while(0); break; \
		case ARM64OP_ORR: do { ARM64_ORR_REG_REGSHIFT(p, rd, rn, rm, shift_type, imm_shift); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, rd, rd); } while(0); break; \
		case ARM64OP_MOV: ARM64_MOVS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); break; \
		case ARM64OP_MVN: ARM64_MVNS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift); break; \
		case ARM64OP_TST: do { ARM64_AND_REG_REGSHIFT(p, ARM64REG_IP0, rn, rm, shift_type, imm_shift); ARM64_ANDS_REG_REG(p, ARM64REG_WZR, ARM64REG_IP0, ARM64REG_IP0); } while(0); break; \
		default: ARM64_NOP(p); break; \
		} \
	} while (0)

/* DPIOP with register-shifted register (emit.c uses this) */
#define ARM64_DPIOP_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
	do { \
		ARM64_SHIFT_REG(p, ARM64REG_IP0, rm, shift_type, rs); \
		ARM64_DPIOP_REG_REG_COND(p, op, rd, rn, ARM64REG_IP0, cond); \
	} while (0)

#define ARM64_DPIOP_S_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
	do { \
		ARM64_SHIFT_REG(p, ARM64REG_IP0, rm, shift_type, rs); \
		ARM64_DPIOP_S_REG_REG_COND(p, op, rd, rn, ARM64REG_IP0, cond); \
	} while (0)


/* ======================================================================== */
/*  STMDB / LDMDB / LDMIA — multi-register transfer emulation              */
/*  On AArch64 we use sequences of STP/LDP.                                */
/*  For the specific patterns used by emit.c prologue/epilogue.             */
/* ======================================================================== */

/* These are provided as function-like macros for the specific register
   bitmask patterns used by the rasterizer. For general use, emit.c's
   prologue/epilogue code should be adapted for AArch64. */
#define ARM64_STMDB(p, rbase, regs) /* stub: needs per-use adaptation */
#define ARM64_LDMDB(p, rbase, regs) /* stub: needs per-use adaptation */
#define ARM64_LDMIA(p, rbase, regs) /* stub: needs per-use adaptation */
#define ARM64_LDMIA_WB(p, rbase, regs) /* stub: needs per-use adaptation */
#define ARM64_PUSH(p, regs) /* stub: needs per-use adaptation */
#define ARM64_POP(p, regs)  /* stub: needs per-use adaptation */


/* ======================================================================== */
/*  SHL/SHR/SAR _COND variants                                             */
/* ======================================================================== */

#define ARM64_SHL_IMM_COND(p, rd, rm, imm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SHL_IMM(p, rd, rm, imm); } while (0)

#define ARM64_SHR_IMM_COND(p, rd, rm, imm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SHR_IMM(p, rd, rm, imm); } while (0)

#define ARM64_SAR_IMM_COND(p, rd, rm, imm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SAR_IMM(p, rd, rm, imm); } while (0)

#define ARM64_SHL_REG_COND(p, rd, rm, rs, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SHL_REG(p, rd, rm, rs); } while (0)

#define ARM64_SHR_REG_COND(p, rd, rm, rs, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SHR_REG(p, rd, rm, rs); } while (0)

#define ARM64_SAR_REG_COND(p, rd, rm, rs, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_SAR_REG(p, rd, rm, rs); } while (0)

#define ARM64_SHLS_IMM_COND(p, rd, rm, imm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 3); ARM64_SHLS_IMM(p, rd, rm, imm); } while (0)

#define ARM64_SHRS_IMM_COND(p, rd, rm, imm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 3); ARM64_SHRS_IMM(p, rd, rm, imm); } while (0)

#define ARM64_SARS_IMM_COND(p, rd, rm, imm, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 3); ARM64_SARS_IMM(p, rd, rm, imm); } while (0)


/* ======================================================================== */
/*  PLD — prefetch (AArch64 uses PRFM)                                     */
/* ======================================================================== */

#define ARM64_PLD_IMM(p, rn, imm12) \
	ARM64_EMIT(p, 0xF9800000 | ((((unsigned int)(imm12) / 8) & 0xFFF) << 10) | ((rn) << 5) | 0)


/* ======================================================================== */
/*  ARM_* compatibility macros                                              */
/*  When building for AArch64, these redirect ARM32 names to ARM64.         */
/* ======================================================================== */

#ifdef __aarch64__

/* Types */
#define arminstr_t arm64instr_t
#define armword_t  arm64word_t

/* Emit */
#define ARM_EMIT(p, i) ARM64_EMIT(p, i)

/* Registers */
#define ARMREG_R0  ARM64REG_X0
#define ARMREG_R1  ARM64REG_X1
#define ARMREG_R2  ARM64REG_X2
#define ARMREG_R3  ARM64REG_X3
#define ARMREG_R4  ARM64REG_X4
#define ARMREG_R5  ARM64REG_X5
#define ARMREG_R6  ARM64REG_X6
#define ARMREG_R7  ARM64REG_X7
#define ARMREG_R8  ARM64REG_X8
#define ARMREG_R9  ARM64REG_X9
#define ARMREG_R10 ARM64REG_X10
#define ARMREG_R11 ARM64REG_X11
#define ARMREG_R12 ARM64REG_X12
#define ARMREG_R13 ARM64REG_X13
#define ARMREG_R14 ARM64REG_X14
#define ARMREG_R15 ARM64REG_X15

#define ARMREG_A1  ARM64REG_A1
#define ARMREG_A2  ARM64REG_A2
#define ARMREG_A3  ARM64REG_A3
#define ARMREG_A4  ARM64REG_A4

#define ARMREG_V1  ARM64REG_V1
#define ARMREG_V2  ARM64REG_V2
#define ARMREG_V3  ARM64REG_V3
#define ARMREG_V4  ARM64REG_V4
#define ARMREG_V5  ARM64REG_V5
#define ARMREG_V6  ARM64REG_V6
#define ARMREG_V7  ARM64REG_V7

#define ARMREG_FP  ARM64REG_FP
#define ARMREG_IP  ARM64REG_IP0
#define ARMREG_SP  ARM64REG_SP
#define ARMREG_LR  ARM64REG_LR
#define ARMREG_PC  ARM64REG_LR  /* no PC on AArch64; use LR as best approx */

#define ARMREG_MAX ARM64REG_MAX

/* Condition codes */
#define ARMCOND_EQ ARM64COND_EQ
#define ARMCOND_NE ARM64COND_NE
#define ARMCOND_CS ARM64COND_CS
#define ARMCOND_HS ARM64COND_HS
#define ARMCOND_CC ARM64COND_CC
#define ARMCOND_LO ARM64COND_LO
#define ARMCOND_MI ARM64COND_MI
#define ARMCOND_PL ARM64COND_PL
#define ARMCOND_VS ARM64COND_VS
#define ARMCOND_VC ARM64COND_VC
#define ARMCOND_HI ARM64COND_HI
#define ARMCOND_LS ARM64COND_LS
#define ARMCOND_GE ARM64COND_GE
#define ARMCOND_LT ARM64COND_LT
#define ARMCOND_GT ARM64COND_GT
#define ARMCOND_LE ARM64COND_LE
#define ARMCOND_AL ARM64COND_AL
#define ARMCOND_NV ARM64COND_NV

/* Shift types */
#define ARMSHIFT_LSL ARM64SHIFT_LSL
#define ARMSHIFT_LSR ARM64SHIFT_LSR
#define ARMSHIFT_ASR ARM64SHIFT_ASR
#define ARMSHIFT_ROR ARM64SHIFT_ROR
#define ARMSHIFT_ASL ARM64SHIFT_LSL

/* Opcodes */
#define ARMOP_AND ARM64OP_AND
#define ARMOP_EOR ARM64OP_EOR
#define ARMOP_SUB ARM64OP_SUB
#define ARMOP_RSB ARM64OP_RSB
#define ARMOP_ADD ARM64OP_ADD
#define ARMOP_ADC ARM64OP_ADC
#define ARMOP_SBC ARM64OP_SBC
#define ARMOP_RSC ARM64OP_RSC
#define ARMOP_TST ARM64OP_TST
#define ARMOP_TEQ ARM64OP_TEQ
#define ARMOP_CMP ARM64OP_CMP
#define ARMOP_CMN ARM64OP_CMN
#define ARMOP_ORR ARM64OP_ORR
#define ARMOP_MOV ARM64OP_MOV
#define ARMOP_BIC ARM64OP_BIC
#define ARMOP_MVN ARM64OP_MVN
#define ARMOP_STR ARM64OP_STR
#define ARMOP_LDR ARM64OP_LDR
#define ARMOP_MUL ARM64OP_MUL
#define ARMOP_SMULL ARM64OP_SMULL

/* Branch */
#define ARM_B_COND(p, cond, offset) ARM64_B_COND(p, cond, offset)
#define ARM_B(p, offset)            ARM64_B(p, offset)
#define ARM_BL_COND(p, cond, offset) do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_BL(p, offset); } while (0)
#define ARM_BL(p, offset)           ARM64_BL(p, offset)

/* MOV */
#define ARM_MOV_REG_REG(p, rd, rm) ARM64_MOV_REG_REG(p, rd, rm)
#define ARM_MOV_REG_REG_COND(p, rd, rm, cond) ARM64_MOV_REG_REG_COND(p, rd, rm, cond)
#define ARM_MOV_REG_IMM8(p, rd, imm8) ARM64_MOV_REG_IMM8(p, rd, imm8)
#define ARM_MOV_REG_IMM8_COND(p, rd, imm8, cond) ARM64_MOV_REG_IMM8_COND(p, rd, imm8, cond)
#define ARM_MOV_REG_IMM(p, rd, imm8, rot) ARM64_MOV_REG_IMM8(p, rd, ARM64_DECODE_IMM8ROT(imm8, rot))
#define ARM_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) ARM64_MOV_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift)
#define ARM_MOV_REG_IMMSHIFT_COND(p, rd, rm, shift_type, imm_shift, cond) ARM64_MOV_REG_IMMSHIFT_COND(p, rd, rm, shift_type, imm_shift, cond)
#define ARM_MOV_REG_REGSHIFT(p, rd, rm, shift_type, rs) ARM64_MOV_REG_REGSHIFT(p, rd, rm, shift_type, rs)
#define ARM_MOV_REG_REGSHIFT_COND(p, rd, rm, shift_type, rs, cond) \
	do { ARM64_B_COND(p, ARM64COND_INVERT(cond), 2); ARM64_MOV_REG_REGSHIFT(p, rd, rm, shift_type, rs); } while (0)
#define ARM_MOVS_REG_REG(p, rd, rm) ARM64_MOVS_REG_REG(p, rd, rm)
#define ARM_MOVS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) ARM64_MOVS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift)
#define ARM_MOVS_REG_IMMSHIFT_COND(p, rd, rm, shift_type, imm_shift, cond) ARM64_MOVS_REG_IMMSHIFT_COND(p, rd, rm, shift_type, imm_shift, cond)
#define ARM_MOVS_REG_REGSHIFT(p, rd, rm, shift_type, rs) ARM64_MOVS_REG_REGSHIFT(p, rd, rm, shift_type, rs)

/* MVN */
#define ARM_MVN_REG_REG(p, rd, rm) ARM64_MVN_REG_REG(p, rd, rm)
#define ARM_MVN_REG_REG_COND(p, rd, rm, cond) ARM64_MVN_REG_REG_COND(p, rd, rm, cond)
#define ARM_MVN_REG_IMM(p, rd, imm8, rot) ARM64_MVN_REG_IMM(p, rd, imm8, rot)
#define ARM_MVN_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) ARM64_MVN_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift)
#define ARM_MVN_REG_REGSHIFT(p, rd, rm, shift_type, rs) ARM64_MVN_REG_REGSHIFT(p, rd, rm, shift_type, rs)
#define ARM_MVNS_REG_REG(p, rd, rm) ARM64_MVNS_REG_REG(p, rd, rm)
#define ARM_MVNS_REG_IMM(p, rd, imm8, rot) ARM64_MVNS_REG_IMM(p, rd, imm8, rot)
#define ARM_MVNS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift) ARM64_MVNS_REG_IMMSHIFT(p, rd, rm, shift_type, imm_shift)
#define ARM_MVNS_REG_REGSHIFT(p, rd, rm, shift_type, rs) ARM64_MVNS_REG_REGSHIFT(p, rd, rm, shift_type, rs)

/* ADD */
#define ARM_ADD_REG_REG(p, rd, rn, rm) ARM64_ADD_REG_REG(p, rd, rn, rm)
#define ARM_ADD_REG_IMM8(p, rd, rn, imm8) ARM64_ADD_REG_IMM8(p, rd, rn, imm8)
#define ARM_ADD_REG_IMM8_COND(p, rd, rn, imm8, cond) ARM64_ADD_REG_IMM8_COND(p, rd, rn, imm8, cond)
#define ARM_ADD_REG_IMM(p, rd, rn, imm8, rot) ARM64_ADD_REG_IMM(p, rd, rn, ARM64_DECODE_IMM8ROT(imm8, rot), 0)
#define ARM_ADDS_REG_REG(p, rd, rn, rm) ARM64_ADDS_REG_REG(p, rd, rn, rm)

/* SUB */
#define ARM_SUB_REG_REG(p, rd, rn, rm) ARM64_SUB_REG_REG(p, rd, rn, rm)
#define ARM_SUB_REG_IMM8(p, rd, rn, imm8) ARM64_SUB_REG_IMM8(p, rd, rn, imm8)
#define ARM_SUB_REG_IMM8_COND(p, rd, rn, imm8, cond) ARM64_SUB_REG_IMM8_COND(p, rd, rn, imm8, cond)
#define ARM_SUBS_REG_REG(p, rd, rn, rm) ARM64_SUBS_REG_REG(p, rd, rn, rm)

/* RSB */
#define ARM_RSB_REG_IMM(p, rd, rn, imm8, rot) ARM64_RSB_REG_IMM(p, rd, rn, imm8, rot)
#define ARM_RSB_REG_IMM8(p, rd, rn, imm8) ARM64_RSB_REG_IMM8(p, rd, rn, imm8)
#define ARM_RSBS_REG_IMM(p, rd, rn, imm8, rot) ARM64_RSBS_REG_IMM(p, rd, rn, imm8, rot)
#define ARM_RSBS_REG_IMM8(p, rd, rn, imm8) ARM64_RSBS_REG_IMM8(p, rd, rn, imm8)
#define ARM_RSB_REG_REG(p, rd, rn, rm) ARM64_RSB_REG_REG(p, rd, rn, rm)
#define ARM_RSBS_REG_REG(p, rd, rn, rm) ARM64_RSBS_REG_REG(p, rd, rn, rm)

/* AND */
#define ARM_AND_REG_REG(p, rd, rn, rm) ARM64_AND_REG_REG(p, rd, rn, rm)
#define ARM_ANDS_REG_REG(p, rd, rn, rm) ARM64_ANDS_REG_REG(p, rd, rn, rm)
#define ARM_ANDS_REG_IMM(p, rd, rn, imm8, rot) ARM64_ANDS_REG_IMM(p, rd, rn, imm8, rot)
#define ARM_ANDS_REG_IMM8(p, rd, rn, imm8) ARM64_ANDS_REG_IMM8(p, rd, rn, imm8)

/* ORR */
#define ARM_ORR_REG_REG(p, rd, rn, rm) ARM64_ORR_REG_REG(p, rd, rn, rm)
#define ARM_ORR_REG_IMMSHIFT(p, rd, rn, rm, shift_type, imm_shift) ARM64_ORR_REG_IMMSHIFT(p, rd, rn, rm, shift_type, imm_shift)
#define ARM_ORRS_REG_IMMSHIFT(p, rd, rn, rm, shift_type, imm_shift) ARM64_ORRS_REG_IMMSHIFT(p, rd, rn, rm, shift_type, imm_shift)

/* EOR */
#define ARM_EOR_REG_REG(p, rd, rn, rm) ARM64_EOR_REG_REG(p, rd, rn, rm)

/* BIC */
#define ARM_BIC_REG_REG(p, rd, rn, rm) ARM64_BIC_REG_REG(p, rd, rn, rm)

/* ADC / SBC */
#define ARM_ADC_REG_REG(p, rd, rn, rm) ARM64_ADC_REG_REG(p, rd, rn, rm)
#define ARM_SBC_REG_REG(p, rd, rn, rm) ARM64_SBC_REG_REG(p, rd, rn, rm)

/* CMP */
#define ARM_CMP_REG_REG(p, rn, rm) ARM64_CMP_REG_REG(p, rn, rm)
#define ARM_CMP_REG_IMM(p, rn, imm8, rot) ARM64_CMP_REG_IMM(p, rn, imm8, rot)
#define ARM_CMP_REG_IMM8(p, rn, imm8) ARM64_CMP_REG_IMM8(p, rn, imm8)
#define ARM_CMP_REG_IMMSHIFT(p, rn, rm, shift_type, imm_shift) ARM64_CMP_REG_IMMSHIFT(p, rn, rm, shift_type, imm_shift)
#define ARM_CMP_REG_REGSHIFT(p, rn, rm, shift_type, rs) ARM64_CMP_REG_REGSHIFT(p, rn, rm, shift_type, rs)

/* CMN */
#define ARM_CMN_REG_IMM(p, rn, imm8, rot) ARM64_CMN_REG_IMM(p, rn, imm8, rot)
#define ARM_CMN_REG_IMM8(p, rn, imm8) ARM64_CMN_REG_IMM8(p, rn, imm8)
#define ARM_CMN_REG_REG(p, rn, rm) ARM64_CMN_REG_REG(p, rn, rm)

/* TST */
#define ARM_TST_REG_REG(p, rn, rm) ARM64_TST_REG_REG(p, rn, rm)

/* Multiply */
#define ARM_MUL(p, rd, rm, rs) ARM64_MUL(p, rd, rm, rs)
#define ARM_MUL_REG_REG(p, rd, rm, rs) ARM64_MUL_REG_REG(p, rd, rm, rs)
#define ARM_MULS(p, rd, rm, rs) ARM64_MULS(p, rd, rm, rs)
#define ARM_MULS_REG_REG(p, rd, rm, rs) ARM64_MULS_REG_REG(p, rd, rm, rs)
#define ARM_SMULL(p, rd_lo, rd_hi, rm, rs) ARM64_SMULL(p, rd_lo, rd_hi, rm, rs)
#define ARM_MLA(p, rd, rm, rs, rn) ARM64_MLA(p, rd, rm, rs, rn)

/* Load/Store */
#define ARM_LDR_IMM(p, rd, rn, imm) ARM64_LDR_IMM(p, rd, rn, imm)
#define ARM_STR_IMM(p, rd, rn, imm) ARM64_STR_IMM(p, rd, rn, imm)
#define ARM_LDRH_IMM(p, rd, rn, imm) ARM64_LDRH_IMM(p, rd, rn, imm)
#define ARM_STRH_IMM(p, rd, rn, imm) ARM64_STRH_IMM(p, rd, rn, imm)
#define ARM_LDRB_IMM(p, rd, rn, imm) ARM64_LDRB_IMM(p, rd, rn, imm)
#define ARM_STRB_IMM(p, rd, rn, imm) ARM64_STRB_IMM(p, rd, rn, imm)
#define ARM_LDRSH_IMM(p, rd, rn, imm) ARM64_LDRSH_IMM(p, rd, rn, imm)
#define ARM_LDRSB_IMM(p, rd, rn, imm) ARM64_LDRSB_IMM(p, rd, rn, imm)

#define ARM_LDR_REG_REG(p, rd, rn, rm) ARM64_LDR_REG_REG(p, rd, rn, rm)
#define ARM_STR_REG_REG(p, rd, rn, rm) ARM64_STR_REG_REG(p, rd, rn, rm)
#define ARM_LDRH_REG_REG(p, rd, rn, rm) ARM64_LDRH_REG_REG(p, rd, rn, rm)
#define ARM_STRH_REG_REG(p, rd, rn, rm) ARM64_STRH_REG_REG(p, rd, rn, rm)
#define ARM_LDRB_REG_REG(p, rd, rn, rm) ARM64_LDRB_REG_REG(p, rd, rn, rm)
#define ARM_STRB_REG_REG(p, rd, rn, rm) ARM64_STRB_REG_REG(p, rd, rn, rm)
#define ARM_LDRSH_REG_REG(p, rd, rn, rm) ARM64_LDRSH_REG_REG(p, rd, rn, rm)
#define ARM_LDRSB_REG_REG(p, rd, rn, rm) ARM64_LDRSB_REG_REG(p, rd, rn, rm)

#define ARM_LDR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) ARM64_LDR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift)
#define ARM_STR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) ARM64_STR_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift)
#define ARM_LDRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) ARM64_LDRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift)
#define ARM_STRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift) ARM64_STRB_REG_REG_SHIFT(p, rd, rn, rm, shift_type, shift)

/* Shift aliases */
#define ARM_SHL_IMM(p, rd, rm, imm) ARM64_SHL_IMM(p, rd, rm, imm)
#define ARM_SHR_IMM(p, rd, rm, imm) ARM64_SHR_IMM(p, rd, rm, imm)
#define ARM_SAR_IMM(p, rd, rm, imm) ARM64_SAR_IMM(p, rd, rm, imm)
#define ARM_SHL_IMM_COND(p, rd, rm, imm, cond) ARM64_SHL_IMM_COND(p, rd, rm, imm, cond)
#define ARM_SHR_IMM_COND(p, rd, rm, imm, cond) ARM64_SHR_IMM_COND(p, rd, rm, imm, cond)
#define ARM_SAR_IMM_COND(p, rd, rm, imm, cond) ARM64_SAR_IMM_COND(p, rd, rm, imm, cond)
#define ARM_SHL_REG(p, rd, rm, rs) ARM64_SHL_REG(p, rd, rm, rs)
#define ARM_SHR_REG(p, rd, rm, rs) ARM64_SHR_REG(p, rd, rm, rs)
#define ARM_SAR_REG(p, rd, rm, rs) ARM64_SAR_REG(p, rd, rm, rs)
#define ARM_SHLS_IMM(p, rd, rm, imm) ARM64_SHLS_IMM(p, rd, rm, imm)
#define ARM_SHRS_IMM(p, rd, rm, imm) ARM64_SHRS_IMM(p, rd, rm, imm)
#define ARM_SARS_IMM(p, rd, rm, imm) ARM64_SARS_IMM(p, rd, rm, imm)
#define ARM_SHLS_REG(p, rd, rm, rs) ARM64_SHLS_REG(p, rd, rm, rs)
#define ARM_SHRS_REG(p, rd, rm, rs) ARM64_SHRS_REG(p, rd, rm, rs)
#define ARM_SARS_REG(p, rd, rm, rs) ARM64_SARS_REG(p, rd, rm, rs)
#define ARM_SHLS_REG_REG(p, rd, rm, rs) ARM64_SHLS_REG_REG(p, rd, rm, rs)
#define ARM_SHRS_REG_REG(p, rd, rm, rs) ARM64_SHRS_REG_REG(p, rd, rm, rs)
#define ARM_SARS_REG_REG(p, rd, rm, rs) ARM64_SARS_REG_REG(p, rd, rm, rs)

/* Multi-register transfer stubs */
#define ARM_STMDB(p, rbase, regs) ARM64_STMDB(p, rbase, regs)
#define ARM_LDMDB(p, rbase, regs) ARM64_LDMDB(p, rbase, regs)
#define ARM_LDMIA(p, rbase, regs) ARM64_LDMIA(p, rbase, regs)
#define ARM_LDMIA_WB(p, rbase, regs) ARM64_LDMIA_WB(p, rbase, regs)
#define ARM_PUSH(p, regs) ARM64_PUSH(p, regs)
#define ARM_POP(p, regs) ARM64_POP(p, regs)

/* Misc */
#define ARM_NOP(p) ARM64_NOP(p)
#define ARM_CLZ(p, rd, rm) ARM64_CLZ(p, rd, rm)
#define ARM_DBRK(p) ARM64_DBRK(p)
#define ARM_INC(p, reg) ARM64_INC(p, reg)
#define ARM_DEC(p, reg) ARM64_DEC(p, reg)

/* DPIOP generic dispatchers */
#define ARM_DPIOP_REG_REG_COND(p, op, rd, rn, rm, cond) \
	ARM64_DPIOP_REG_REG_COND(p, op, rd, rn, rm, cond)
#define ARM_DPIOP_S_REG_REG_COND(p, op, rd, rn, rm, cond) \
	ARM64_DPIOP_S_REG_REG_COND(p, op, rd, rn, rm, cond)
#define ARM_DPIOP_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
	ARM64_DPIOP_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond)
#define ARM_DPIOP_S_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond) \
	ARM64_DPIOP_S_REG_IMMSHIFT_COND(p, op, rd, rn, rm, shift_type, imm_shift, cond)
#define ARM_DPIOP_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
	ARM64_DPIOP_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond)
#define ARM_DPIOP_S_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond) \
	ARM64_DPIOP_S_REG_REGSHIFT_COND(p, op, rd, rn, rm, shift_type, rs, cond)

/* MRS/MSR */
#define ARM_MRS_CPSR(p, rd) ARM64_MRS_CPSR(p, rd)
#define ARM_MRS_CPSR_COND(p, rd, cond) ARM64_MRS_CPSR(p, rd)
#define ARM_MSR_REG(p, mask, rn, sel) ARM64_MSR_REG(p, mask, rn, sel)

#define ARM_PSR_C 1
#define ARM_PSR_X 2
#define ARM_PSR_S 4
#define ARM_PSR_F 8
#define ARM_CPSR 0
#define ARM_SPSR 1

/* PLD */
#define ARM_PLD_IMM(p, rn, imm) ARM64_PLD_IMM(p, rn, imm)

/* Constants used by emit.c */
#define ARM_NUM_ARG_REGS ARM64_NUM_ARG_REGS
#define ARM_NUM_VARIABLE_REGS ARM64_NUM_VARIABLE_REGS
#define ARM_NUM_GLOBAL_REGS ARM64_NUM_VARIABLE_REGS

/* ======================================================================== */
/*  Type compatibility aliases for emit.c                                   */
/* ======================================================================== */

typedef int ARMReg;
typedef int ARMCond;
typedef int ARMShiftType;
typedef int ARMOpcode;

#define ARMREG_CPSR 0  /* flags register slot for emit.c */

/* DPI macros used by emit.c for immediate-operand instructions.
 * On AArch64, we translate rotated-imm8 to straight imm12. */
#define ARM_DPIOP_REG_IMM8ROT_COND(p, op, rd, rn, imm, rot, cond) \
	do { \
		unsigned int _val = (unsigned int)(imm) >> (((32 - (rot)) & 31)); \
		if ((op) == ARMOP_ADD) ARM64_ADD_REG_IMM8(p, rd, rn, _val); \
		else if ((op) == ARMOP_SUB) ARM64_SUB_REG_IMM8(p, rd, rn, _val); \
		else if ((op) == ARMOP_MOV) arm64_mov_reg_imm32(p, rd, _val); \
		else if ((op) == ARMOP_CMP) ARM64_CMP_REG_IMM8(p, rn, _val); \
		else if ((op) == ARMOP_AND) { arm64_mov_reg_imm32(p, ARM64REG_IP0, _val); ARM64_AND_REG_REG(p, rd, rn, ARM64REG_IP0); } \
		else if ((op) == ARMOP_ORR) { arm64_mov_reg_imm32(p, ARM64REG_IP0, _val); ARM64_ORR_REG_REG(p, rd, rn, ARM64REG_IP0); } \
		else if ((op) == ARMOP_EOR) { arm64_mov_reg_imm32(p, ARM64REG_IP0, _val); ARM64_EOR_REG_REG(p, rd, rn, ARM64REG_IP0); } \
		else if ((op) == ARMOP_RSB) { ARM64_NEG(p, rd, rn); if (_val) ARM64_ADD_REG_IMM8(p, rd, rd, _val); } \
		else arm64_mov_reg_imm32(p, rd, _val); \
	} while(0)

#define ARM_DPIOP_S_REG_IMM8ROT_COND(p, op, rd, rn, imm, rot, cond) \
	do { \
		unsigned int _val = (unsigned int)(imm) >> (((32 - (rot)) & 31)); \
		if ((op) == ARMOP_ADD) ARM64_ADDS_REG_IMM8(p, rd, rn, _val); \
		else if ((op) == ARMOP_SUB) ARM64_SUBS_REG_IMM8(p, rd, rn, _val); \
		else if ((op) == ARMOP_CMP) ARM64_CMP_REG_IMM8(p, rn, _val); \
		else if ((op) == ARMOP_AND) { arm64_mov_reg_imm32(p, ARM64REG_IP0, _val); ARM64_ANDS_REG_REG(p, rd, rn, ARM64REG_IP0); } \
		else ARM_DPIOP_REG_IMM8ROT_COND(p, op, rd, rn, imm, rot, cond); \
	} while(0)

/* ======================================================================== */
/*  Helper function declarations                                            */
/* ======================================================================== */

void arm64_mov_reg_imm32(cg_segment_t * segment, int reg, arm64word_t imm32);
void arm64_mov_reg_imm32_cond(cg_segment_t * segment, int reg, arm64word_t imm32, int cond);
int arm64_is_imm12(arm64word_t val);
void arm64_emit_std_prologue(cg_segment_t * segment, unsigned int local_size);
void arm64_emit_std_epilogue(cg_segment_t * segment, unsigned int local_size, int pop_regs);
void arm64_emit_lean_prologue(cg_segment_t * segment, unsigned int local_size, int push_regs);
int arm64_bsf(arm64word_t val);

/* ARM32 function name compatibility */
#define arm_mov_reg_imm32(s, r, i)           arm64_mov_reg_imm32(s, r, i)
#define arm_mov_reg_imm32_cond(s, r, i, c)   arm64_mov_reg_imm32_cond(s, r, i, c)
#define is_arm_const(val)                     arm64_is_imm12(val)
#define calc_arm_mov_const_shift(val)         0
#define arm_is_power_of_2(val)                (((val) != 0) && (((val) & ((val)-1)) == 0))
#define arm_bsf(val)                          arm64_bsf(val)
#define arm_emit_std_prologue(s, sz)          arm64_emit_std_prologue(s, sz)
#define arm_emit_std_epilogue(s, sz, r)       arm64_emit_std_epilogue(s, sz, r)
#define arm_emit_lean_prologue(s, sz, r)      arm64_emit_lean_prologue(s, sz, r)

#endif /* __aarch64__ */


#ifdef __cplusplus
}
#endif

#endif /* ARM64_H */
