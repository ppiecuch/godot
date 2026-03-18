/*
 * arm64-codegen.c
 *
 * AArch64 code generation helpers for the Vincent GLES1 JIT.
 * Provides constant loading (MOVZ/MOVK), prologue/epilogue generation,
 * and utility functions parallel to arm-codegen.c.
 *
 * Copyright (c) 2004, Hans-Martin Will (original ARM32 version)
 * AArch64 port for Godot Engine
 */

#include "arm64-codegen.h"
#include <string.h>

/* ====================================================================== */
/* Constant loading via MOVZ/MOVK                                          */
/* ====================================================================== */

int arm64_is_imm12(arm64word_t val) {
	return val < 4096;
}

int arm64_bsf(arm64word_t val) {
	int pos = 0;
	if (val == 0) return -1;
	while (!(val & 1)) {
		val >>= 1;
		pos++;
	}
	return pos;
}

void arm64_mov_reg_imm32(cg_segment_t * segment, int reg, arm64word_t imm32) {
	unsigned short lo = (unsigned short)(imm32 & 0xFFFF);
	unsigned short hi = (unsigned short)((imm32 >> 16) & 0xFFFF);

	if (hi == 0) {
		/* Single MOVZ */
		ARM64_EMIT(segment, 0x52800000u | ((unsigned int)lo << 5) | (unsigned int)reg);
	} else if (lo == 0) {
		/* MOVZ with LSL #16 */
		ARM64_EMIT(segment, 0x52A00000u | ((unsigned int)hi << 5) | (unsigned int)reg);
	} else if (imm32 == 0xFFFFFFFF) {
		/* MOVN Wd, #0 */
		ARM64_EMIT(segment, 0x12800000u | (unsigned int)reg);
	} else if ((imm32 & 0xFFFF0000) == 0xFFFF0000) {
		/* MOVN Wd, #~lo */
		ARM64_EMIT(segment, 0x12800000u | ((unsigned int)(~lo & 0xFFFF) << 5) | (unsigned int)reg);
	} else {
		/* MOVZ lo, then MOVK hi */
		ARM64_EMIT(segment, 0x52800000u | ((unsigned int)lo << 5) | (unsigned int)reg);
		ARM64_EMIT(segment, 0x72A00000u | ((unsigned int)hi << 5) | (unsigned int)reg);
	}
}

void arm64_mov_reg_imm32_cond(cg_segment_t * segment, int reg, arm64word_t imm32, int cond) {
	if (cond == ARM64COND_AL) {
		arm64_mov_reg_imm32(segment, reg, imm32);
		return;
	}

	/* Count instructions needed for the constant load */
	unsigned short lo = (unsigned short)(imm32 & 0xFFFF);
	unsigned short hi = (unsigned short)((imm32 >> 16) & 0xFFFF);
	int skip_count;

	if (hi == 0 || lo == 0 || imm32 == 0xFFFFFFFF || (imm32 & 0xFFFF0000) == 0xFFFF0000) {
		skip_count = 1;
	} else {
		skip_count = 2;
	}

	/* B.invcond skip */
	ARM64_B_COND(segment, ARM64COND_INVERT(cond), skip_count + 1);
	arm64_mov_reg_imm32(segment, reg, imm32);
}

/* ====================================================================== */
/* Function prologue/epilogue                                              */
/* ====================================================================== */

/*
 * AArch64 prologue:
 *   STP X29, X30, [SP, #-frame_size]!    ; save FP, LR
 *   MOV X29, SP                           ; set frame pointer
 *   STP X19, X20, [SP, #16]              ; save callee-saved regs
 *   STP X21, X22, [SP, #32]
 *   STP X23, X24, [SP, #48]
 *   STP X25, X26, [SP, #64]
 *   STP X27, X28, [SP, #80]
 *   ; locals start at [SP + 96]
 *
 * Frame layout:
 *   [FP+0]   = saved X29 (FP)
 *   [FP+8]   = saved X30 (LR)
 *   [FP+16]  = saved X19
 *   [FP+24]  = saved X20
 *   ...
 *   [FP+80]  = saved X27
 *   [FP+88]  = saved X28
 *   [FP+96]  = first local variable
 */

#define ARM64_SAVE_AREA_SIZE 96  /* 12 registers * 8 bytes */

void arm64_emit_std_prologue(cg_segment_t * segment, unsigned int local_size) {
	unsigned int frame_size = ARM64_SAVE_AREA_SIZE + local_size;

	/* Round up to 16-byte alignment */
	frame_size = (frame_size + 15) & ~15u;

	/* STP X29, X30, [SP, #-frame_size]! */
	ARM64_STP_X_PRE(segment, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -(int)frame_size);

	/* MOV X29, SP — must use ADD Xd, SP, #0, not ORR (reg 31 = XZR in ORR) */
	ARM64_ADD_X_REG_IMM(segment, ARM64REG_FP, ARM64REG_SP, 0);

	/* Save callee-saved registers */
	ARM64_STP_X(segment, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
	ARM64_STP_X(segment, ARM64REG_X21, ARM64REG_X22, ARM64REG_SP, 32);
	ARM64_STP_X(segment, ARM64REG_X23, ARM64REG_X24, ARM64REG_SP, 48);
	ARM64_STP_X(segment, ARM64REG_X25, ARM64REG_X26, ARM64REG_SP, 64);
	ARM64_STP_X(segment, ARM64REG_X27, ARM64REG_X28, ARM64REG_SP, 80);
}

void arm64_emit_std_epilogue(cg_segment_t * segment, unsigned int local_size, int pop_regs) {
	unsigned int frame_size = ARM64_SAVE_AREA_SIZE + local_size;
	frame_size = (frame_size + 15) & ~15u;

	/* Restore callee-saved registers */
	ARM64_LDP_X(segment, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
	ARM64_LDP_X(segment, ARM64REG_X21, ARM64REG_X22, ARM64REG_SP, 32);
	ARM64_LDP_X(segment, ARM64REG_X23, ARM64REG_X24, ARM64REG_SP, 48);
	ARM64_LDP_X(segment, ARM64REG_X25, ARM64REG_X26, ARM64REG_SP, 64);
	ARM64_LDP_X(segment, ARM64REG_X27, ARM64REG_X28, ARM64REG_SP, 80);

	/* LDP X29, X30, [SP], #frame_size */
	ARM64_LDP_X_POST(segment, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, frame_size);

	/* RET */
	ARM64_RET(segment);
}

void arm64_emit_lean_prologue(cg_segment_t * segment, unsigned int local_size, int push_regs) {
	/* Same as std_prologue for AArch64 */
	arm64_emit_std_prologue(segment, local_size);
}
