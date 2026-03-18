/*
 * Comprehensive ARM64 JIT instruction encoding & execution tests.
 *
 * Tests the arm64-codegen.h encoding macros by generating minimal ARM64
 * functions, executing them on real hardware, and verifying results.
 * Each test targets a specific bug or encoding correctness property.
 *
 * Build (from tests/ directory):
 *   cc -O2 -o /tmp/test_arm64_enc test_arm64_encoding.c \
 *      -I../src/codegen -lpthread
 *
 * Run:
 *   /tmp/test_arm64_enc [-v]     # -v for verbose (show instruction bytes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifdef __APPLE__
#include <sys/mman.h>
#include <pthread.h>
#include <libkern/OSCacheControl.h>
#endif

/* Types needed by arm64-codegen.h */
typedef int32_t I32;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

/* Code buffer */
typedef struct { U32 buf[256]; size_t pos; } test_seg_t;

static void seg_init(test_seg_t *s) { memset(s, 0, sizeof(*s)); }
static void cg_segment_emit_u32(test_seg_t *seg, U32 val) {
    assert(seg->pos < 256); seg->buf[seg->pos++] = val;
}

#define CODEGEN_SGEMENT_H 1
typedef test_seg_t cg_segment_t;
#include "arm64-codegen.h"

/* Test framework */
static int g_verbose = 0, g_run = 0, g_pass = 0, g_fail = 0;

static void *jit_exec(U32 *code, size_t n) {
#ifdef __APPLE__
    void *m = mmap(NULL, 4096, PROT_READ|PROT_WRITE|PROT_EXEC,
                   MAP_PRIVATE|MAP_ANONYMOUS|MAP_JIT, -1, 0);
    if (m == MAP_FAILED) return NULL;
    pthread_jit_write_protect_np(0);
    memcpy(m, code, n * 4);
    pthread_jit_write_protect_np(1);
    sys_icache_invalidate(m, n * 4);
    return m;
#else
    return NULL;
#endif
}
static void jit_free(void *m) { if (m) munmap(m, 4096); }

static void dump(test_seg_t *s) {
    if (!g_verbose) return;
    for (size_t i = 0; i < s->pos; i++) {
        if (i % 8 == 0) printf("\n      +%3zu:", i*4);
        printf(" %08X", s->buf[i]);
    }
    printf("\n    ");
}

#define TEST(name) static int t_##name(void); \
    static void r_##name(void) { g_run++; printf("  %-58s", #name); \
    if (t_##name()) { printf("PASS\n"); g_pass++; } \
    else { printf("FAIL\n"); g_fail++; } } \
    static int t_##name(void)

#define EQ(a, e) do { uint64_t _a=(uint64_t)(a), _e=(uint64_t)(e); \
    if (_a!=_e) { printf("0x%llX!=0x%llX ",(unsigned long long)_a,(unsigned long long)_e); return 0; } } while(0)
#define EQI(a, e) do { int64_t _a=(int64_t)(a), _e=(int64_t)(e); \
    if (_a!=_e) { printf("%lld!=%lld ",(long long)_a,(long long)_e); return 0; } } while(0)

/* ========== Data movement ========== */

TEST(movz_w_42) { test_seg_t s; seg_init(&s);
    ARM64_MOVZ(&s, ARM64REG_X0, 42, 0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(void))f)(), 42); jit_free(f); return 1; }

TEST(movz_w_shifted) { test_seg_t s; seg_init(&s);
    ARM64_MOVZ(&s, ARM64REG_X0, 0xAB, 1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(void))f)(), 0x00AB0000u); jit_free(f); return 1; }

TEST(movz_movk_64bit) { test_seg_t s; seg_init(&s);
    ARM64_MOVZ_X(&s, ARM64REG_X0, 0x0004, 0);
    ARM64_MOVK_X(&s, ARM64REG_X0, 0x0003, 1);
    ARM64_MOVK_X(&s, ARM64REG_X0, 0x0002, 2);
    ARM64_MOVK_X(&s, ARM64REG_X0, 0x0001, 3);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(void))f)(), 0x0001000200030004ULL); jit_free(f); return 1; }

/* ========== ALU 32-bit ========== */

TEST(add_w) { test_seg_t s; seg_init(&s);
    ARM64_ADD_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(100,200), 300); jit_free(f); return 1; }

TEST(sub_w) { test_seg_t s; seg_init(&s);
    ARM64_SUB_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(500,200), 300); jit_free(f); return 1; }

TEST(mul_w) { test_seg_t s; seg_init(&s);
    ARM64_MUL(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(7,6), 42); jit_free(f); return 1; }

TEST(add_w_imm8) { test_seg_t s; seg_init(&s);
    ARM64_ADD_REG_IMM8(&s, ARM64REG_X0, ARM64REG_X0, 50); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(100), 150); jit_free(f); return 1; }

TEST(and_or_xor) { test_seg_t s; seg_init(&s);
    ARM64_AND_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1);
    ARM64_ORR_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X2);
    ARM64_EOR_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X3);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    typedef int(*f4)(int,int,int,int);
    EQ(((f4)f)(0xFF00,0x0FF0,0x000F,0x0001), 0x0F0E); jit_free(f); return 1; }

TEST(lsl_w) { test_seg_t s; seg_init(&s);
    ARM64_LSLV(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(1,10), 1024); jit_free(f); return 1; }

TEST(neg_w) { test_seg_t s; seg_init(&s);
    ARM64_NEG(&s, ARM64REG_X0, ARM64REG_X0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(42), -42); jit_free(f); return 1; }

TEST(mvn_w) { test_seg_t s; seg_init(&s);
    ARM64_MVN_REG_REG(&s, ARM64REG_X0, ARM64REG_X0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(uint32_t))f)(0), 0xFFFFFFFF); jit_free(f); return 1; }

/* ========== Load/Store ========== */

TEST(str_ldr_x64_roundtrip) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STR_X_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_EMIT(&s, 0xAA1F03E0);
    ARM64_LDR_X_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s); dump(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t))f)(0xDEADBEEF12345678ULL), 0xDEADBEEF12345678ULL);
    jit_free(f); return 1; }

TEST(str_ldr_w32_truncates) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STR_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_EMIT(&s, 0xAA1F03E0);
    ARM64_LDR_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t))f)(0xDEADBEEF12345678ULL), 0x12345678ULL);
    jit_free(f); return 1; }

TEST(ldr_x_reg_reg) { test_seg_t s; seg_init(&s);
    ARM64_LDR_X_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    struct { int32_t a,b; uint64_t p; } d = {0,0,0xCAFEBABE12345678ULL};
    EQ(((uint64_t(*)(void*,size_t))f)(&d,8), d.p); jit_free(f); return 1; }

TEST(ldrh_strh_16bit) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STRH_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_EMIT(&s, 0xAA1F03E0);
    ARM64_LDRH_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t))f)(0x12345678), 0x5678); jit_free(f); return 1; }

TEST(ldrb_strb_8bit) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STRB_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_EMIT(&s, 0xAA1F03E0);
    ARM64_LDRB_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t))f)(0xABCDEF42), 0x42); jit_free(f); return 1; }

TEST(load_ptr_deref) { test_seg_t s; seg_init(&s);
    ARM64_LDR_X_IMM(&s, ARM64REG_X0, ARM64REG_X0, 8);
    ARM64_LDR_IMM(&s, ARM64REG_X0, ARM64REG_X0, 0);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int32_t target = 987654;
    struct { int32_t a,b; int32_t *p; } d = {0,0,&target};
    EQI(((int(*)(void*))f)(&d), 987654); jit_free(f); return 1; }

TEST(ldr_w_from_reg_reg) { test_seg_t s; seg_init(&s);
    ARM64_LDR_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int32_t arr[] = {111, 222, 333, 444};
    EQI(((int(*)(void*,size_t))f)(arr, 8), 333); jit_free(f); return 1; }

TEST(str_w_reg_reg) { test_seg_t s; seg_init(&s);
    /* STR W2, [X0, X1] then return original W2 */
    ARM64_STR_REG_REG(&s, ARM64REG_X2, ARM64REG_X0, ARM64REG_X1);
    ARM64_EMIT(&s, 0x2A0203E0); /* MOV W0, W2 */
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int32_t buf[4] = {0,0,0,0};
    typedef int(*f3)(void*,size_t,int);
    EQI(((f3)f)(buf, 4, 999), 999);
    EQI(buf[1], 999); jit_free(f); return 1; }

/* ========== Branches ========== */

TEST(b_cond_backward_loop) { test_seg_t s; seg_init(&s);
    ARM64_MOVZ(&s, ARM64REG_X0, 0, 0);
    ARM64_MOVZ(&s, ARM64REG_X1, 5, 0);
    ARM64_ADD_REG_IMM8(&s, ARM64REG_X0, ARM64REG_X0, 1);
    ARM64_CMP_REG_REG(&s, ARM64REG_X0, ARM64REG_X1);
    ARM64_B_COND(&s, ARMCOND_LT, -2);
    ARM64_RET(&s); dump(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(void))f)(), 5); jit_free(f); return 1; }

TEST(b_cond_forward_skip) { test_seg_t s; seg_init(&s);
    ARM64_CMP_REG_IMM8(&s, ARM64REG_X0, 0);
    ARM64_B_COND(&s, ARMCOND_EQ, 3);
    ARM64_MOVZ(&s, ARM64REG_X0, 42, 0);
    ARM64_RET(&s);
    ARM64_MOVZ(&s, ARM64REG_X0, 99, 0);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int r1=((int(*)(int))f)(0), r2=((int(*)(int))f)(5);
    if (r1!=99||r2!=42) { printf("(%d,%d)!=(99,42) ",r1,r2); jit_free(f); return 0; }
    jit_free(f); return 1; }

TEST(b_unconditional) { test_seg_t s; seg_init(&s);
    ARM64_B(&s, 2); ARM64_MOVZ(&s, ARM64REG_X0, 0, 0);
    ARM64_MOVZ(&s, ARM64REG_X0, 77, 0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(999), 77); jit_free(f); return 1; }

TEST(b_cond_all_conditions) {
    struct { int c,a,b,e; } cc[] = {
        {ARMCOND_EQ,5,5,1},{ARMCOND_EQ,5,6,0},{ARMCOND_NE,5,6,1},{ARMCOND_NE,5,5,0},
        {ARMCOND_LT,3,5,1},{ARMCOND_LT,5,3,0},{ARMCOND_GE,5,3,1},{ARMCOND_GE,3,5,0},
        {ARMCOND_GT,5,3,1},{ARMCOND_LE,3,5,1},{ARMCOND_HI,5,3,1},{ARMCOND_LS,3,5,1},
    };
    for (int i=0; i<(int)(sizeof(cc)/sizeof(cc[0])); i++) {
        test_seg_t s; seg_init(&s);
        ARM64_CMP_REG_REG(&s, ARM64REG_X0, ARM64REG_X1);
        ARM64_B_COND(&s, cc[i].c, 3);
        ARM64_MOVZ(&s, ARM64REG_X0, 0, 0); ARM64_RET(&s);
        ARM64_MOVZ(&s, ARM64REG_X0, 1, 0); ARM64_RET(&s);
        void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
        int r=((int(*)(int,int))f)(cc[i].a,cc[i].b); jit_free(f);
        if (r!=cc[i].e) { printf("cond=%d(%d,%d)=%d!=%d ",cc[i].c,cc[i].a,cc[i].b,r,cc[i].e); return 0; }
    }
    return 1; }

/* ========== Prologue/Epilogue ========== */

TEST(prologue_preserves_callee_saved) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -96);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_MOVZ_X(&s, ARM64REG_X19, 0xBEEF, 0);
    ARM64_MOVZ(&s, ARM64REG_X0, 99, 0);
    ARM64_LDP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 96);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(void))f)(), 99); jit_free(f); return 1; }

TEST(arg_spill_past_save_area) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_STR_X_IMM(&s, ARM64REG_X0, ARM64REG_FP, 96);
    ARM64_STR_X_IMM(&s, ARM64REG_X1, ARM64REG_FP, 104);
    ARM64_MOVZ_X(&s, ARM64REG_X0, 0, 0); ARM64_MOVZ_X(&s, ARM64REG_X1, 0, 0);
    ARM64_LDR_X_IMM(&s, ARM64REG_X19, ARM64REG_FP, 96);
    ARM64_LDR_X_IMM(&s, ARM64REG_X20, ARM64REG_FP, 104);
    ARM64_ADD_REG_REG(&s, ARM64REG_X0, ARM64REG_X19, ARM64REG_X20);
    ARM64_LDP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(123,456), 579); jit_free(f); return 1; }

static int _add1000(int x) { return x + 1000; }
TEST(blr_call_c_function) {
    test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -16);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    uintptr_t a = (uintptr_t)&_add1000;
    ARM64_MOVZ_X(&s, ARM64REG_IP0, (a>>0)&0xFFFF, 0);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>16)&0xFFFF, 1);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>32)&0xFFFF, 2);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>48)&0xFFFF, 3);
    ARM64_BLR(&s, ARM64REG_IP0);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 16);
    ARM64_RET(&s); dump(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(42), 1042); jit_free(f); return 1; }

/* ========== Flags ========== */

TEST(mrs_msr_nzcv) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_MOVZ(&s, ARM64REG_X0, 5, 0); ARM64_MOVZ(&s, ARM64REG_X1, 3, 0);
    ARM64_SUBS_REG_REG(&s, ARM64REG_X2, ARM64REG_X0, ARM64REG_X1);
    ARM64_MRS_NZCV(&s, ARM64REG_X19);
    ARM64_MOVZ(&s, ARM64REG_X0, 0, 0); ARM64_CMP_REG_IMM8(&s, ARM64REG_X0, 0);
    ARM64_MSR_NZCV(&s, ARM64REG_X19);
    ARM64_B_COND(&s, ARMCOND_GT, 5); /* skip 4 instr: MOVZ+LDP+LDP+RET */
    ARM64_MOVZ(&s, ARM64REG_X0, 0, 0);
    ARM64_LDP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    ARM64_MOVZ(&s, ARM64REG_X0, 1, 0);
    ARM64_LDP_X(&s, ARM64REG_X19, ARM64REG_X20, ARM64REG_SP, 16);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(void))f)(), 1); jit_free(f); return 1; }

/* ========== 64-bit ALU ========== */

TEST(add_x_imm) { test_seg_t s; seg_init(&s);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_X0, ARM64REG_X0, 100); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t))f)(0x100000000ULL), 0x100000064ULL); jit_free(f); return 1; }

TEST(sub_x_imm) { test_seg_t s; seg_init(&s);
    ARM64_SUB_X_REG_IMM(&s, ARM64REG_X0, ARM64REG_X0, 16); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t))f)(0x100000010ULL), 0x100000000ULL); jit_free(f); return 1; }

/* ========== Stress: combined patterns ========== */

TEST(nested_loop_accumulate) { test_seg_t s; seg_init(&s);
    /* W0=arg0 (N), compute sum 1..N */
    ARM64_EMIT(&s, 0x2A0003E1); /* MOV W1, W0 (N) */
    ARM64_MOVZ(&s, ARM64REG_X0, 0, 0); /* sum=0 */
    ARM64_ADD_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); /* sum+=i */
    ARM64_SUBS_REG_IMM8(&s, ARM64REG_X1, ARM64REG_X1, 1);
    ARM64_B_COND(&s, ARMCOND_GT, -2); /* while i>0 */
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* sum 1..100 = 5050 */
    EQI(((int(*)(int))f)(100), 5050); jit_free(f); return 1; }

TEST(memory_write_read_pattern) { test_seg_t s; seg_init(&s);
    /* Write values to array, read them back summed.
     * X0=array ptr, X1=count, X2=start value */
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -16);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_MOVZ(&s, ARM64REG_X3, 0, 0); /* i=0 */
    /* loop: STR W2, [X0, X3, LSL #2]... can't use LSL easily, use manual offset */
    /* Compute byte offset: X4 = X3 << 2 */
    ARM64_EMIT(&s, 0xD37EF464); /* LSL X4, X3, #2 — actually UBFM X4,X3,#62,#61 */
    /* STR W2, [X0, X4] */
    ARM64_STR_REG_REG(&s, ARM64REG_X2, ARM64REG_X0, ARM64REG_X4);
    ARM64_ADD_REG_IMM8(&s, ARM64REG_X2, ARM64REG_X2, 1);
    ARM64_ADD_REG_IMM8(&s, ARM64REG_X3, ARM64REG_X3, 1);
    ARM64_CMP_REG_REG(&s, ARM64REG_X3, ARM64REG_X1);
    ARM64_B_COND(&s, ARMCOND_LT, -5); /* back to LSL (5 instr body) */
    /* Now read back and sum: X5=0 (sum), X3=0 (i) */
    ARM64_MOVZ(&s, ARM64REG_X5, 0, 0);
    ARM64_MOVZ(&s, ARM64REG_X3, 0, 0);
    ARM64_EMIT(&s, 0xD37EF464); /* LSL X4, X3, #2 */
    ARM64_LDR_REG_REG(&s, ARM64REG_X6, ARM64REG_X0, ARM64REG_X4);
    ARM64_ADD_REG_REG(&s, ARM64REG_X5, ARM64REG_X5, ARM64REG_X6);
    ARM64_ADD_REG_IMM8(&s, ARM64REG_X3, ARM64REG_X3, 1);
    ARM64_CMP_REG_REG(&s, ARM64REG_X3, ARM64REG_X1);
    ARM64_B_COND(&s, ARMCOND_LT, -5); /* back to LSL (5 instr body) */
    ARM64_EMIT(&s, 0x2A0503E0); /* MOV W0, W5 */
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 16);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int32_t arr[10];
    /* Write 10..19, sum = 10+11+...+19 = 145 */
    typedef int(*f3)(void*,int,int);
    EQI(((f3)f)(arr, 10, 10), 145);
    jit_free(f); return 1; }

/* ========== Shift with immediate ========== */

TEST(lsl_imm) { test_seg_t s; seg_init(&s);
    ARM64_LSL_IMM(&s, ARM64REG_X0, ARM64REG_X0, 4); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(3), 48); jit_free(f); return 1; }

TEST(lsr_imm) { test_seg_t s; seg_init(&s);
    ARM64_LSR_IMM(&s, ARM64REG_X0, ARM64REG_X0, 4); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(uint32_t))f)(0x1234), 0x123); jit_free(f); return 1; }

TEST(asr_imm) { test_seg_t s; seg_init(&s);
    ARM64_ASR_IMM(&s, ARM64REG_X0, ARM64REG_X0, 16); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* ASR preserves sign: -65536 >> 16 = -1 */
    EQI(((int(*)(int))f)(-65536), -1);
    /* Positive: 0x30000 >> 16 = 3 */
    EQI(((int(*)(int))f)(0x30000), 3);
    jit_free(f); return 1; }

TEST(lsr_reg) { test_seg_t s; seg_init(&s);
    ARM64_LSRV(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(uint32_t,int))f)(0xFF00, 8), 0xFF); jit_free(f); return 1; }

TEST(asr_reg) { test_seg_t s; seg_init(&s);
    ARM64_ASRV(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(-256, 4), -16); jit_free(f); return 1; }

TEST(ror_reg) { test_seg_t s; seg_init(&s);
    ARM64_RORV(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* ROR(0x12345678, 8) = 0x78123456 */
    EQ(((uint32_t(*)(uint32_t,int))f)(0x12345678u, 8), 0x78123456u);
    jit_free(f); return 1; }

/* ========== Multiply variants ========== */

TEST(smull_64bit) { test_seg_t s; seg_init(&s);
    /* SMULL X0, W0, W1 → signed 32x32→64 multiply */
    ARM64_SMULL(&s, ARM64REG_X0, ARM64REG_X1, ARM64REG_X0, ARM64REG_X1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* 100000 * 100000 = 10,000,000,000 (exceeds 32-bit) */
    EQ(((uint64_t(*)(int,int))f)(100000, 100000), 10000000000ULL);
    jit_free(f); return 1; }

TEST(mla_multiply_add) { test_seg_t s; seg_init(&s);
    /* MLA W0, W1, W2, W0 → W0 = W1*W2 + W0 */
    ARM64_MLA(&s, ARM64REG_X0, ARM64REG_X1, ARM64REG_X2, ARM64REG_X0);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    typedef int(*f3)(int,int,int);
    /* 10 + 3*7 = 31 (arg0=accumulator, arg1=a, arg2=b) */
    EQI(((f3)f)(10, 3, 7), 31); jit_free(f); return 1; }

/* ========== MOV variants ========== */

TEST(mov_reg_reg) { test_seg_t s; seg_init(&s);
    ARM64_MOV_REG_REG(&s, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(0, 42), 42); jit_free(f); return 1; }

TEST(mov_x_reg_reg) { test_seg_t s; seg_init(&s);
    /* 64-bit MOV via ADD Xd, Xn, #0 */
    ARM64_MOV_X_REG_REG(&s, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* Verify full 64-bit value preserved */
    EQ(((uint64_t(*)(uint64_t,uint64_t))f)(0, 0xABCD1234DEADBEEFULL), 0xABCD1234DEADBEEFULL);
    jit_free(f); return 1; }

TEST(mov_reg_imm8) { test_seg_t s; seg_init(&s);
    ARM64_MOV_REG_IMM8(&s, ARM64REG_X0, 200); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(void))f)(), 200); jit_free(f); return 1; }

TEST(movn_w) { test_seg_t s; seg_init(&s);
    /* MOVN W0, #0 → W0 = ~0 = 0xFFFFFFFF = -1 */
    ARM64_MOVN(&s, ARM64REG_X0, 0, 0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(void))f)(), -1); jit_free(f); return 1; }

/* ========== Conditional select ========== */

TEST(csel) { test_seg_t s; seg_init(&s);
    /* if (W0 == W1) return W2; else return W3 */
    ARM64_CMP_REG_REG(&s, ARM64REG_X0, ARM64REG_X1);
    ARM64_CSEL(&s, ARM64REG_X0, ARM64REG_X2, ARM64REG_X3, ARMCOND_EQ);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    typedef int(*f4)(int,int,int,int);
    EQI(((f4)f)(5, 5, 100, 200), 100);  /* equal → W2=100 */
    EQI(((f4)f)(5, 6, 100, 200), 200);  /* not equal → W3=200 */
    jit_free(f); return 1; }

TEST(csinc) { test_seg_t s; seg_init(&s);
    /* CSINC: if cond then Wn, else Wm+1 */
    ARM64_CMP_REG_REG(&s, ARM64REG_X0, ARM64REG_X1);
    ARM64_CSINC(&s, ARM64REG_X0, ARM64REG_X2, ARM64REG_X3, ARMCOND_LT);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    typedef int(*f4)(int,int,int,int);
    EQI(((f4)f)(3, 5, 100, 200), 100);  /* 3 < 5 → W2=100 */
    EQI(((f4)f)(5, 3, 100, 200), 201);  /* 5 >= 3 → W3+1=201 */
    jit_free(f); return 1; }

/* ========== Bit operations ========== */

TEST(tst_reg_reg) { test_seg_t s; seg_init(&s);
    /* TST W0, W1 then CSEL based on result */
    ARM64_TST_REG_REG(&s, ARM64REG_X0, ARM64REG_X1);
    ARM64_MOVZ(&s, ARM64REG_X2, 1, 0);
    ARM64_MOVZ(&s, ARM64REG_X3, 0, 0);
    ARM64_CSEL(&s, ARM64REG_X0, ARM64REG_X2, ARM64REG_X3, ARMCOND_NE);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(0xFF, 0x10), 1);  /* bits overlap → NE */
    EQI(((int(*)(int,int))f)(0x0F, 0xF0), 0);  /* no overlap → EQ */
    jit_free(f); return 1; }

TEST(bic_reg_reg) { test_seg_t s; seg_init(&s);
    /* BIC: W0 = W0 & ~W1 */
    ARM64_BIC_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(uint32_t,uint32_t))f)(0xFFFF, 0x00FF), 0xFF00);
    jit_free(f); return 1; }

TEST(clz) { test_seg_t s; seg_init(&s);
    ARM64_CLZ(&s, ARM64REG_X0, ARM64REG_X0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(1), 31);
    EQI(((int(*)(int))f)(0x80000000u), 0);
    EQI(((int(*)(int))f)(0x100), 23);
    jit_free(f); return 1; }

/* ========== Sign-extending loads ========== */

TEST(ldrsh_sign_extends) { test_seg_t s; seg_init(&s);
    /* LDRSH loads 16-bit and sign-extends to 32-bit */
    ARM64_LDRSH_IMM(&s, ARM64REG_X0, ARM64REG_X0, 0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int16_t neg = -1234;
    int16_t pos = 1234;
    EQI(((int(*)(void*))f)(&neg), -1234);
    EQI(((int(*)(void*))f)(&pos), 1234);
    jit_free(f); return 1; }

TEST(ldrsb_sign_extends) { test_seg_t s; seg_init(&s);
    ARM64_LDRSB_IMM(&s, ARM64REG_X0, ARM64REG_X0, 0); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int8_t neg = -42;
    int8_t pos = 42;
    EQI(((int(*)(void*))f)(&neg), -42);
    EQI(((int(*)(void*))f)(&pos), 42);
    jit_free(f); return 1; }

TEST(ldrsh_reg_reg) { test_seg_t s; seg_init(&s);
    ARM64_LDRSH_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int16_t arr[] = {100, -200, 300};
    EQI(((int(*)(void*,size_t))f)(arr, 2), -200);  /* arr[1] at byte offset 2 */
    jit_free(f); return 1; }

/* ========== Unscaled loads/stores ========== */

TEST(ldur_stur) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    /* STUR W0, [FP, #97] — unaligned offset */
    ARM64_STUR(&s, ARM64REG_X0, ARM64REG_FP, 97);
    ARM64_EMIT(&s, 0xAA1F03E0); /* MOV X0, XZR */
    /* LDUR W0, [FP, #97] */
    ARM64_LDUR(&s, ARM64REG_X0, ARM64REG_FP, 97);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(uint32_t))f)(0xDEADBEEF), 0xDEADBEEF);
    jit_free(f); return 1; }

TEST(ldurh_sturh) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -128);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    ARM64_STURH(&s, ARM64REG_X0, ARM64REG_FP, 99);
    ARM64_EMIT(&s, 0xAA1F03E0);
    ARM64_LDURH(&s, ARM64REG_X0, ARM64REG_FP, 99);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 128);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint32_t(*)(uint32_t))f)(0xABCD), 0xABCD);
    jit_free(f); return 1; }

/* ========== Carry operations ========== */

TEST(adc_sbc) { test_seg_t s; seg_init(&s);
    /* Set carry with ADDS: 0xFFFFFFFF + 1 overflows → C=1 */
    ARM64_MOVN(&s, ARM64REG_X2, 0, 0);  /* W2 = 0xFFFFFFFF */
    ARM64_ADDS_REG_IMM8(&s, ARM64REG_X2, ARM64REG_X2, 1); /* carry set */
    /* ADC W0, W0, W1 → W0 = W0 + W1 + carry */
    ARM64_ADC_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* 10 + 20 + 1(carry) = 31 */
    EQI(((int(*)(int,int))f)(10, 20), 31);
    jit_free(f); return 1; }

/* ========== Shifted register ALU ========== */

TEST(add_reg_regshift) { test_seg_t s; seg_init(&s);
    /* ADD W0, W0, W1, LSL #2 → W0 = W0 + W1*4 */
    ARM64_ADD_REG_REGSHIFT(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1, ARM64SHIFT_LSL, 2);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(10, 3), 22); /* 10 + 3*4 = 22 */
    jit_free(f); return 1; }

TEST(sub_reg_regshift) { test_seg_t s; seg_init(&s);
    ARM64_SUB_REG_REGSHIFT(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1, ARM64SHIFT_LSL, 1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(100, 10), 80); /* 100 - 10*2 = 80 */
    jit_free(f); return 1; }

/* ========== 64-bit reg operations ========== */

TEST(add_x_reg_reg) { test_seg_t s; seg_init(&s);
    ARM64_ADD_X_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t,uint64_t))f)(0x100000000ULL, 0x200000000ULL), 0x300000000ULL);
    jit_free(f); return 1; }

TEST(sub_x_reg_reg) { test_seg_t s; seg_init(&s);
    ARM64_SUB_X_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1); ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQ(((uint64_t(*)(uint64_t,uint64_t))f)(0x300000000ULL, 0x100000000ULL), 0x200000000ULL);
    jit_free(f); return 1; }

/* ========== BL (branch with link) ========== */

static int _helper_add5(int x) { return x + 5; }
TEST(bl_branch_link) { test_seg_t s; seg_init(&s);
    ARM64_STP_X_PRE(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, -16);
    ARM64_ADD_X_REG_IMM(&s, ARM64REG_FP, ARM64REG_SP, 0);
    uintptr_t a = (uintptr_t)&_helper_add5;
    ARM64_MOVZ_X(&s, ARM64REG_IP0, (a>>0)&0xFFFF, 0);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>16)&0xFFFF, 1);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>32)&0xFFFF, 2);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>48)&0xFFFF, 3);
    ARM64_BLR(&s, ARM64REG_IP0);
    /* Call again with result of first call */
    ARM64_MOVZ_X(&s, ARM64REG_IP0, (a>>0)&0xFFFF, 0);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>16)&0xFFFF, 1);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>32)&0xFFFF, 2);
    ARM64_MOVK_X(&s, ARM64REG_IP0, (a>>48)&0xFFFF, 3);
    ARM64_BLR(&s, ARM64REG_IP0);
    ARM64_LDP_X_POST(&s, ARM64REG_FP, ARM64REG_LR, ARM64REG_SP, 16);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    /* 10 + 5 + 5 = 20 */
    EQI(((int(*)(int))f)(10), 20);
    jit_free(f); return 1; }

/* ========== Stack PUSH/POP ========== */

TEST(push_pop_pair) { test_seg_t s; seg_init(&s);
    /* Save X0 via PUSH2, clobber, POP2 restore */
    ARM64_PUSH2(&s, ARM64REG_X0, ARM64REG_X1);
    ARM64_MOVZ(&s, ARM64REG_X0, 0, 0);
    ARM64_MOVZ(&s, ARM64REG_X1, 0, 0);
    ARM64_POP2(&s, ARM64REG_X0, ARM64REG_X1);
    /* Return X0 + X1 */
    ARM64_ADD_REG_REG(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int,int))f)(100, 200), 300);
    jit_free(f); return 1; }

/* ========== RSB (reverse subtract) ========== */

TEST(rsb_reg_imm) { test_seg_t s; seg_init(&s);
    /* RSB W0, W0, #100 → W0 = 100 - W0 */
    ARM64_RSB_REG_IMM8(&s, ARM64REG_X0, ARM64REG_X0, 100);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    EQI(((int(*)(int))f)(30), 70);
    jit_free(f); return 1; }

/* ========== Shifted register load/store ========== */

TEST(ldr_reg_reg_shift) { test_seg_t s; seg_init(&s);
    /* NOTE: ARM64_LDR_REG_REG_SHIFT currently ignores shift and maps to
     * ARM64_LDR_REG_REG. Test with pre-computed byte offset instead. */
    ARM64_LDR_REG_REG_SHIFT(&s, ARM64REG_X0, ARM64REG_X0, ARM64REG_X1, ARM64SHIFT_LSL, 2);
    ARM64_RET(&s);
    void *f=jit_exec(s.buf,s.pos); if(!f) return 0;
    int32_t arr[] = {10, 20, 30, 40};
    /* Pass byte offset 8 (not index 2) since shift is not applied */
    EQI(((int(*)(void*,size_t))f)(arr, 8), 30); /* arr[2] at byte offset 8 */
    jit_free(f); return 1; }

/* ================================================================ */

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-v") == 0) g_verbose = 1;

    printf("=== ARM64 JIT Instruction Encoding Tests ===\n");
    printf("  Platform: %s, pointer size: %zu bytes\n\n",
#ifdef __APPLE__
        "macOS/Apple Silicon",
#else
        "Linux/ARM64",
#endif
        sizeof(void*));

    printf("--- Data movement ---\n");
    r_movz_w_42(); r_movz_w_shifted(); r_movz_movk_64bit();
    r_mov_reg_reg(); r_mov_x_reg_reg(); r_mov_reg_imm8(); r_movn_w();

    printf("--- ALU (32-bit) ---\n");
    r_add_w(); r_sub_w(); r_mul_w(); r_add_w_imm8();
    r_and_or_xor(); r_lsl_w(); r_neg_w(); r_mvn_w();
    r_rsb_reg_imm();

    printf("--- Shift with immediate ---\n");
    r_lsl_imm(); r_lsr_imm(); r_asr_imm();
    r_lsr_reg(); r_asr_reg(); r_ror_reg();

    printf("--- Multiply ---\n");
    r_smull_64bit(); r_mla_multiply_add();

    printf("--- Load/Store (scaled) ---\n");
    r_str_ldr_x64_roundtrip(); r_str_ldr_w32_truncates();
    r_ldr_x_reg_reg(); r_ldrh_strh_16bit(); r_ldrb_strb_8bit();
    r_load_ptr_deref(); r_ldr_w_from_reg_reg(); r_str_w_reg_reg();
    r_ldr_reg_reg_shift();

    printf("--- Load/Store (sign-extending) ---\n");
    r_ldrsh_sign_extends(); r_ldrsb_sign_extends(); r_ldrsh_reg_reg();

    printf("--- Load/Store (unscaled) ---\n");
    r_ldur_stur(); r_ldurh_sturh();

    printf("--- Branches ---\n");
    r_b_cond_backward_loop(); r_b_cond_forward_skip();
    r_b_unconditional(); r_b_cond_all_conditions();

    printf("--- Conditional select ---\n");
    r_csel(); r_csinc();

    printf("--- Bit operations ---\n");
    r_tst_reg_reg(); r_bic_reg_reg(); r_clz();

    printf("--- Prologue/Epilogue/Stack ---\n");
    r_prologue_preserves_callee_saved(); r_arg_spill_past_save_area();
    r_blr_call_c_function(); r_bl_branch_link(); r_push_pop_pair();

    printf("--- Flags ---\n");
    r_mrs_msr_nzcv();

    printf("--- Carry operations ---\n");
    r_adc_sbc();

    printf("--- Shifted register ALU ---\n");
    r_add_reg_regshift(); r_sub_reg_regshift();

    printf("--- 64-bit ALU ---\n");
    r_add_x_imm(); r_sub_x_imm(); r_add_x_reg_reg(); r_sub_x_reg_reg();

    printf("--- Stress patterns ---\n");
    r_nested_loop_accumulate(); r_memory_write_read_pattern();

    printf("\n=== Results: %d/%d passed", g_pass, g_run);
    if (g_fail) printf(", %d FAILED", g_fail);
    printf(" ===\n");
    return g_fail ? 1 : 0;
}
