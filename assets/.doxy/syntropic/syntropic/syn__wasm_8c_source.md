

# File syn\_wasm.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**vm**](dir_10a4ed2785bdaf7cfd36ec9032e0af72.md) **>** [**syn\_wasm.c**](syn__wasm_8c.md)

[Go to the documentation of this file](syn__wasm_8c.md)


```C++


#include "syn_wasm.h"

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
#include <math.h>
#endif
#include <string.h>

#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
#include "syntropic/util/syn_qmath.h"
#endif


/* Wasm Magic Header & Version */
#define WASM_MAGIC 0x6D736100U /* "\0asm" */
#define WASM_VERSION 0x00000001U

/* Value Types */
#define WASM_TYPE_I32 0x7FU
#define WASM_TYPE_I64 0x7EU
#define WASM_TYPE_F32 0x7DU
#define WASM_TYPE_F64 0x7CU

/* Section IDs */
#define WASM_SEC_CUSTOM 0U
#define WASM_SEC_TYPE 1U
#define WASM_SEC_IMPORT 2U
#define WASM_SEC_FUNCTION 3U
#define WASM_SEC_TABLE 4U
#define WASM_SEC_MEMORY 5U
#define WASM_SEC_GLOBAL 6U
#define WASM_SEC_EXPORT 7U
#define WASM_SEC_START 8U
#define WASM_SEC_ELEMENT 9U
#define WASM_SEC_CODE 10U
#define WASM_SEC_DATA 11U
#define WASM_SEC_DATACOUNT 12U

/* Opcodes */
#define OP_UNREACHABLE 0x00
#define OP_NOP 0x01
#define OP_BLOCK 0x02
#define OP_LOOP 0x03
#define OP_IF 0x04
#define OP_ELSE 0x05
#define OP_END 0x0B
#define OP_BR 0x0C
#define OP_BR_IF 0x0D
#define OP_BR_TABLE 0x0E
#define OP_RETURN 0x0F
#define OP_CALL 0x10
#define OP_CALL_INDIRECT 0x11

#define OP_DROP 0x1A
#define OP_SELECT 0x1B

#define OP_LOCAL_GET 0x20
#define OP_LOCAL_SET 0x21
#define OP_LOCAL_TEE 0x22
#define OP_GLOBAL_GET 0x23
#define OP_GLOBAL_SET 0x24

#define OP_I32_LOAD 0x28
#define OP_I32_LOAD8_S 0x2C
#define OP_I32_LOAD8_U 0x2D
#define OP_I32_LOAD16_S 0x2E
#define OP_I32_LOAD16_U 0x2F
#define OP_I32_STORE 0x36
#define OP_I32_STORE8 0x3A
#define OP_I32_STORE16 0x3B
#define OP_MEMORY_SIZE 0x3F
#define OP_MEMORY_GROW 0x40

#define OP_I32_CONST 0x41
#define OP_I32_EQZ 0x45
#define OP_I32_EQ 0x46
#define OP_I32_NE 0x47
#define OP_I32_LT_S 0x48
#define OP_I32_LT_U 0x49
#define OP_I32_GT_S 0x4A
#define OP_I32_GT_U 0x4B
#define OP_I32_LE_S 0x4C
#define OP_I32_LE_U 0x4D
#define OP_I32_GE_S 0x4E
#define OP_I32_GE_U 0x4F

#define OP_I32_CLZ 0x67
#define OP_I32_CTZ 0x68
#define OP_I32_POPCNT 0x69
#define OP_I32_ADD 0x6A
#define OP_I32_SUB 0x6B
#define OP_I32_MUL 0x6C
#define OP_I32_DIV_S 0x6D
#define OP_I32_DIV_U 0x6E
#define OP_I32_REM_S 0x6F
#define OP_I32_REM_U 0x70
#define OP_I32_AND 0x71
#define OP_I32_OR 0x72
#define OP_I32_XOR 0x73
#define OP_I32_SHL 0x74
#define OP_I32_SHR_S 0x75
#define OP_I32_SHR_U 0x76
#define OP_I32_ROTL 0x77
#define OP_I32_ROTR 0x78

#define OP_I64_LOAD 0x29
#define OP_I64_LOAD8_S 0x30
#define OP_I64_LOAD8_U 0x31
#define OP_I64_LOAD16_S 0x32
#define OP_I64_LOAD16_U 0x33
#define OP_I64_LOAD32_S 0x34
#define OP_I64_LOAD32_U 0x35
#define OP_I64_STORE 0x37
#define OP_I64_STORE8 0x3C
#define OP_I64_STORE16 0x3D
#define OP_I64_STORE32 0x3E

#define OP_I64_CONST 0x42
#define OP_I64_EQZ 0x50
#define OP_I64_EQ 0x51
#define OP_I64_NE 0x52
#define OP_I64_LT_S 0x53
#define OP_I64_LT_U 0x54
#define OP_I64_GT_S 0x55
#define OP_I64_GT_U 0x56
#define OP_I64_LE_S 0x57
#define OP_I64_LE_U 0x58
#define OP_I64_GE_S 0x59
#define OP_I64_GE_U 0x5A

#define OP_I64_CLZ 0x79
#define OP_I64_CTZ 0x7A
#define OP_I64_POPCNT 0x7B
#define OP_I64_ADD 0x7C
#define OP_I64_SUB 0x7D
#define OP_I64_MUL 0x7E
#define OP_I64_DIV_S 0x7F
#define OP_I64_DIV_U 0x80
#define OP_I64_REM_S 0x81
#define OP_I64_REM_U 0x82
#define OP_I64_AND 0x83
#define OP_I64_OR 0x84
#define OP_I64_XOR 0x85
#define OP_I64_SHL 0x86
#define OP_I64_SHR_S 0x87
#define OP_I64_SHR_U 0x88
#define OP_I64_ROTL 0x89
#define OP_I64_ROTR 0x8A

#define OP_I32_WRAP_I64 0xA7
#define OP_I64_EXTEND_I32_S 0xAC
#define OP_I64_EXTEND_I32_U 0xAD

#define OP_F32_LOAD 0x2A
#define OP_F64_LOAD 0x2B
#define OP_F32_STORE 0x38
#define OP_F64_STORE 0x39

#define OP_F32_CONST 0x43
#define OP_F64_CONST 0x44

#define OP_F32_EQ 0x5B
#define OP_F32_NE 0x5C
#define OP_F32_LT 0x5D
#define OP_F32_GT 0x5E
#define OP_F32_LE 0x5F
#define OP_F32_GE 0x60

#define OP_F64_EQ 0x61
#define OP_F64_NE 0x62
#define OP_F64_LT 0x63
#define OP_F64_GT 0x64
#define OP_F64_LE 0x65
#define OP_F64_GE 0x66

#define OP_F32_ABS 0x8B
#define OP_F32_NEG 0x8C
#define OP_F32_CEIL 0x8D
#define OP_F32_FLOOR 0x8E
#define OP_F32_TRUNC 0x8F
#define OP_F32_NEAREST 0x90
#define OP_F32_SQRT 0x91
#define OP_F32_ADD 0x92
#define OP_F32_SUB 0x93
#define OP_F32_MUL 0x94
#define OP_F32_DIV 0x95
#define OP_F32_MIN 0x96
#define OP_F32_MAX 0x97
#define OP_F32_COPYSIGN 0x98

#define OP_F64_ABS 0x99
#define OP_F64_NEG 0x9A
#define OP_F64_CEIL 0x9B
#define OP_F64_FLOOR 0x9C
#define OP_F64_TRUNC 0x9D
#define OP_F64_NEAREST 0x9E
#define OP_F64_SQRT 0x9F
#define OP_F64_ADD 0xA0
#define OP_F64_SUB 0xA1
#define OP_F64_MUL 0xA2
#define OP_F64_DIV 0xA3
#define OP_F64_MIN 0xA4
#define OP_F64_MAX 0xA5
#define OP_F64_COPYSIGN 0xA6

#define OP_I32_TRUNC_F32_S 0xA8
#define OP_I32_TRUNC_F32_U 0xA9
#define OP_I32_TRUNC_F64_S 0xAA
#define OP_I32_TRUNC_F64_U 0xAB

#define OP_I64_TRUNC_F32_S 0xAE
#define OP_I64_TRUNC_F32_U 0xAF
#define OP_I64_TRUNC_F64_S 0xB0
#define OP_I64_TRUNC_F64_U 0xB1

#define OP_F32_CONVERT_I32_S 0xB2
#define OP_F32_CONVERT_I32_U 0xB3
#define OP_F32_CONVERT_I64_S 0xB4
#define OP_F32_CONVERT_I64_U 0xB5
#define OP_F32_DEMOTE_F64 0xB6

#define OP_F64_CONVERT_I32_S 0xB7
#define OP_F64_CONVERT_I32_U 0xB8
#define OP_F64_CONVERT_I64_S 0xB9
#define OP_F64_CONVERT_I64_U 0xBA
#define OP_F64_PROMOTE_F32 0xBB

#define OP_I32_REINTERPRET_F32 0xBCU
#define OP_I64_REINTERPRET_F64 0xBDU
#define OP_F32_REINTERPRET_I32 0xBEU
#define OP_F64_REINTERPRET_I64 0xBFU

#define OP_I32_EXTEND8_S 0xC0U
#define OP_I32_EXTEND16_S 0xC1U
#define OP_I64_EXTEND8_S 0xC2U
#define OP_I64_EXTEND16_S 0xC3U
#define OP_I64_EXTEND32_S 0xC4U

#define OP_PREFIX_GC 0xFBU
#define OP_PREFIX_MISC 0xFCU
#define OP_PREFIX_SIMD 0xFDU
#define OP_PREFIX_ATOMIC 0xFEU

/* ── Stack Helpers ──────────────────────────────────────────────────────── */

static bool push_stack64(SYN_WASM_Context *ctx, uint64_t val)
{
    /* LCOV_EXCL_START: Wasm operand stack overflow guard */
    if (ctx->sp >= SYN_WASM_MAX_STACK) {
        ctx->status = SYN_WASM_TRAP_STACK_OVERFLOW;
        return false;
    }
    /* LCOV_EXCL_STOP */
    ctx->stack[ctx->sp++] = val;
    return true;
}

static bool pop_stack64(SYN_WASM_Context *ctx, uint64_t *val)
{
    if (ctx->sp == 0) {
        /* LCOV_EXCL_START: Pop 64-bit stack underflow return */
        ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
        return false;
        /* LCOV_EXCL_STOP */
    }
    *val = ctx->stack[--ctx->sp];
    return true;
}

static bool push_stack(SYN_WASM_Context *ctx, uint32_t val)
{
    return push_stack64(ctx, (uint64_t)val);
}

static bool pop_stack(SYN_WASM_Context *ctx, uint32_t *val)
{
    uint64_t v64 = 0;
    if (!pop_stack64(ctx, &v64)) {
        /* LCOV_EXCL_START: Pop 32-bit stack underflow return */
        return false;
        /* LCOV_EXCL_STOP */
    }
    *val = (uint32_t)v64;
    return true;
}

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
static bool push_f32(SYN_WASM_Context *ctx, float val)
{
    uint32_t u32;
    memcpy(&u32, &val, sizeof(u32));
    return push_stack64(ctx, (uint64_t)u32);
}

static bool pop_f32(SYN_WASM_Context *ctx, float *val)
{
    uint64_t v64 = 0;
    if (!pop_stack64(ctx, &v64)) {
        /* LCOV_EXCL_START: Pop float stack underflow return */
        return false;
        /* LCOV_EXCL_STOP */
    }
    uint32_t u32 = (uint32_t)v64;
    memcpy(val, &u32, sizeof(u32));
    return true;
}

static bool push_f64(SYN_WASM_Context *ctx, double val)
{
    uint64_t u64;
    memcpy(&u64, &val, sizeof(u64));
    return push_stack64(ctx, u64);
}

static bool pop_f64(SYN_WASM_Context *ctx, double *val)
{
    uint64_t v64 = 0;
    if (!pop_stack64(ctx, &v64)) {
        /* LCOV_EXCL_START: Pop double float stack underflow return */
        return false;
        /* LCOV_EXCL_STOP */
    }
    memcpy(val, &v64, sizeof(v64));
    return true;
}
#endif

static bool wasm_read_byte(SYN_WASM_Context *ctx, uint32_t limit, uint8_t *out)
{
    if (ctx->pc >= limit) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    *out = ctx->module->bytes[ctx->pc++];
    return true;
}

static bool wasm_advance_pc(SYN_WASM_Context *ctx, uint32_t limit, uint32_t count)
{
    if ((uint64_t)ctx->pc + (uint64_t)count > (uint64_t)limit) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    ctx->pc += count;
    return true;
}

static bool wasm_read_u32_leb(SYN_WASM_Context *ctx, uint32_t limit, uint32_t *out)
{
    uint32_t result = 0U;
    uint32_t shift = 0U;

    while (ctx->pc < limit) {
        uint8_t byte = ctx->module->bytes[ctx->pc++];
        if (shift == 28U && (byte & 0xF0U) != 0U) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
        }
        result |= (uint32_t)(byte & 0x7FU) << shift;
        if ((byte & 0x80U) == 0U) {
            *out = result;
            return true;
        }
        shift += 7U;
    }

    ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
    return false;
}

static bool wasm_read_i32_leb(SYN_WASM_Context *ctx, uint32_t limit, int32_t *out)
{
    uint32_t result = 0U;
    uint32_t shift = 0U;

    while (ctx->pc < limit) {
        uint8_t byte = ctx->module->bytes[ctx->pc++];
        if (shift == 28U) {
            uint8_t top = (uint8_t)(byte & 0xF8U);
            if (top != 0x00U && top != 0x78U) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                return false;
            }
        }
        result |= (uint32_t)(byte & 0x7FU) << shift;
        shift += 7U;
        if ((byte & 0x80U) == 0U) {
            if (shift < 32U && (byte & 0x40U) != 0U) {
                result |= (~0U << shift);
            }
            *out = (int32_t)result;
            return true;
        }
    }

    ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
    return false;
}

static bool wasm_read_i64_leb(SYN_WASM_Context *ctx, uint32_t limit, int64_t *out)
{
    uint64_t result = 0ULL;
    uint32_t shift = 0U;

    while (ctx->pc < limit) {
        uint8_t byte = ctx->module->bytes[ctx->pc++];
        if (shift == 63U && byte != 0x00U && byte != 0x7FU) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
        }
        uint64_t chunk = (shift == 63U) ? (uint64_t)(byte & 0x01U) : (uint64_t)(byte & 0x7FU);
        result |= chunk << shift;
        shift += 7U;
        if ((byte & 0x80U) == 0U) {
            if (shift < 64U && (byte & 0x40U) != 0U) {
                result |= (~0ULL << shift);
            }
            *out = (int64_t)result;
            return true;
        }
    }

    ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
    return false;
}

static uint8_t wasm_op_u32_leb_count(uint8_t op)
{
    if ((op >= OP_LOCAL_GET && op <= OP_GLOBAL_SET) || op == OP_BR || op == OP_BR_IF ||
        op == OP_CALL) {
        return 1U;
    }
    if ((op >= OP_I32_LOAD && op <= OP_I64_STORE32) || op == OP_CALL_INDIRECT) {
        return 2U;
    }
    return 0U;
}

static bool wasm_skip_br_table(SYN_WASM_Context *ctx, uint32_t limit)
{
    uint32_t count = 0U;
    if (!wasm_read_u32_leb(ctx, limit, &count)) {
        return false;
    }
    for (uint32_t i = 0U; i <= count; i++) {
        uint32_t target = 0U;
        if (!wasm_read_u32_leb(ctx, limit, &target)) {
            return false;
        }
    }
    return true;
}

static bool wasm_skip_misc_op(SYN_WASM_Context *ctx, uint32_t limit)
{
    uint32_t subop = 0U;
    if (!wasm_read_u32_leb(ctx, limit, &subop)) {
        return false;
    }
    uint8_t extra = 0U;
    if (subop == 8U || subop == 10U) {
        extra = 2U;
    } else if (subop == 9U || subop == 11U) {
        extra = 1U;
    }
    for (uint8_t i = 0U; i < extra; i++) {
        uint32_t imm = 0U;
        if (!wasm_read_u32_leb(ctx, limit, &imm)) {
            return false;
        }
    }
    return true;
}

static bool wasm_skip_immediates_checked(SYN_WASM_Context *ctx, uint8_t op, uint32_t limit)
{
    if (op == OP_MEMORY_SIZE || op == OP_MEMORY_GROW) {
        uint8_t b = 0U;
        return wasm_read_byte(ctx, limit, &b);
    }
    if (op == OP_I32_CONST) {
        int32_t v = 0;
        return wasm_read_i32_leb(ctx, limit, &v);
    }
    if (op == OP_I64_CONST) {
        int64_t v = 0;
        return wasm_read_i64_leb(ctx, limit, &v);
    }
    if (op == OP_F32_CONST) {
        return wasm_advance_pc(ctx, limit, 4U);
    }
    if (op == OP_F64_CONST) {
        return wasm_advance_pc(ctx, limit, 8U);
    }
    if (op == OP_BR_TABLE) {
        return wasm_skip_br_table(ctx, limit);
    }
    if (op == OP_PREFIX_MISC) {
        return wasm_skip_misc_op(ctx, limit);
    }

    uint8_t leb_cnt = wasm_op_u32_leb_count(op);
    for (uint8_t i = 0U; i < leb_cnt; i++) {
        uint32_t v = 0U;
        if (!wasm_read_u32_leb(ctx, limit, &v)) {
            return false;
        }
    }
    return true;
}

static bool wasm_skip_block(SYN_WASM_Context *ctx, uint32_t limit, bool stop_at_else,
                            bool *out_found_else)
{
    *out_found_else = false;
    uint32_t depth = 1U;
    while (depth > 0U) {
        uint8_t op = 0U;
        if (!wasm_read_byte(ctx, limit, &op)) {
            return false;
        }
        if (op == OP_BLOCK || op == OP_LOOP || op == OP_IF) {
            uint8_t bt = 0U;
            if (!wasm_read_byte(ctx, limit, &bt)) {
                return false;
            }
            depth++;
        } else if (op == OP_END) {
            depth--;
        } else if (stop_at_else && op == OP_ELSE && depth == 1U) {
            *out_found_else = true;
            return true;
        } else if (!wasm_skip_immediates_checked(ctx, op, limit)) {
            return false;
        }
    }
    return true;
}

/* ── Module Parser & Load-Time Validator ─────────────────────────────────── */

#define WASM_MAX_FUNC_PARAMS ((SYN_WASM_MAX_STACK > 255) ? 255U : (uint32_t)SYN_WASM_MAX_STACK)
#define WASM_MAX_TABLE_ELEMS ((uint32_t)SYN_WASM_MAX_TABLE_ELEMS)
#define WASM_MAX_EXPORTS ((uint32_t)SYN_WASM_MAX_EXPORTS)
#define WASM_MAX_MEM_PAGES 65536U
#define WASM_VAL_UNKNOWN 0x00U
#define WASM_BLOCK_VOID 0x40U

typedef struct {
    uint32_t type_func_indices[SYN_WASM_MAX_FUNCTIONS]; 
    uint8_t global_types[SYN_WASM_MAX_GLOBALS];         
    uint8_t global_mut[SYN_WASM_MAX_GLOBALS];           
    uint32_t table_min;                                 
    uint32_t datacount;                                 
    uint16_t func_decl_count;                           
    uint16_t code_body_count;                           
    uint16_t global_count;                              
    uint8_t table_count;                                
    uint8_t memory_count;                               
    uint8_t last_sec_rank;                              
    bool has_datacount;                                 
} WASM_LoadState;

typedef struct {
    uint16_t height;     
    uint8_t opcode;      
    uint8_t result_type; 
    uint8_t unreachable; 
} WASM_CtrlFrame;

typedef struct {
    uint8_t local_types[SYN_WASM_MAX_LOCALS];     
    uint8_t val_stack[SYN_WASM_MAX_STACK];        
    WASM_CtrlFrame ctrl[SYN_WASM_MAX_LABELS + 1]; 
    uint16_t local_count;                         
    uint16_t val_sp;                              
    uint16_t ctrl_depth;                          
} WASM_CodeVal;

typedef struct {
    uint8_t val_type;       
    uint8_t is_store;       
    uint8_t max_align_log2; 
} WASM_MemOpSig;

typedef struct {
    uint8_t in1;     
    uint8_t in2;     
    uint8_t out;     
    uint8_t fp_mode; 
} WASM_NumOpSig;

static const WASM_MemOpSig g_wasm_mem_sigs[23] = {
    {WASM_TYPE_I32, 0U, 2U}, {WASM_TYPE_I64, 0U, 3U}, {WASM_TYPE_F32, 0U, 2U},
    {WASM_TYPE_F64, 0U, 3U}, {WASM_TYPE_I32, 0U, 0U}, {WASM_TYPE_I32, 0U, 0U},
    {WASM_TYPE_I32, 0U, 1U}, {WASM_TYPE_I32, 0U, 1U}, {WASM_TYPE_I64, 0U, 0U},
    {WASM_TYPE_I64, 0U, 0U}, {WASM_TYPE_I64, 0U, 1U}, {WASM_TYPE_I64, 0U, 1U},
    {WASM_TYPE_I64, 0U, 2U}, {WASM_TYPE_I64, 0U, 2U}, {WASM_TYPE_I32, 1U, 2U},
    {WASM_TYPE_I64, 1U, 3U}, {WASM_TYPE_F32, 1U, 2U}, {WASM_TYPE_F64, 1U, 3U},
    {WASM_TYPE_I32, 1U, 0U}, {WASM_TYPE_I32, 1U, 1U}, {WASM_TYPE_I64, 1U, 0U},
    {WASM_TYPE_I64, 1U, 1U}, {WASM_TYPE_I64, 1U, 2U},
};

static const WASM_NumOpSig g_wasm_num_sigs[128] = {
    /* 0x45 i32.eqz */
    {WASM_TYPE_I32, 0U, WASM_TYPE_I32, 0U},
    /* 0x46..0x4F i32.relop (10) */
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    /* 0x50 i64.eqz */
    {WASM_TYPE_I64, 0U, WASM_TYPE_I32, 0U},
    /* 0x51..0x5A i64.relop (10) */
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I32, 0U},
    /* 0x5B..0x60 f32.relop (6) */
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_I32, 2U},
    /* 0x61..0x66 f64.relop (6) */
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_I32, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_I32, 2U},
    /* 0x67..0x69 i32.unop (3) */
    {WASM_TYPE_I32, 0U, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, 0U, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, 0U, WASM_TYPE_I32, 0U},
    /* 0x6A..0x78 i32.binop (15) */
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    {WASM_TYPE_I32, WASM_TYPE_I32, WASM_TYPE_I32, 0U},
    /* 0x79..0x7B i64.unop (3) */
    {WASM_TYPE_I64, 0U, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, 0U, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, 0U, WASM_TYPE_I64, 0U},
    /* 0x7C..0x8A i64.binop (15) */
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    {WASM_TYPE_I64, WASM_TYPE_I64, WASM_TYPE_I64, 0U},
    /* 0x8B..0x91 f32.unop (7) */
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, 0U, WASM_TYPE_F32, 1U},
    /* 0x92..0x98 f32.binop (7) */
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    {WASM_TYPE_F32, WASM_TYPE_F32, WASM_TYPE_F32, 1U},
    /* 0x99..0x9F f64.unop (7) */
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, 0U, WASM_TYPE_F64, 2U},
    /* 0xA0..0xA6 f64.binop (7) */
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    {WASM_TYPE_F64, WASM_TYPE_F64, WASM_TYPE_F64, 2U},
    /* 0xA7..0xBF conversions & reinterpretations (25) */
    {WASM_TYPE_I64, 0U, WASM_TYPE_I32, 0U}, /* 0xA7 i32.wrap_i64 */
    {WASM_TYPE_F32, 0U, WASM_TYPE_I32, 2U}, /* 0xA8 i32.trunc_f32_s */
    {WASM_TYPE_F32, 0U, WASM_TYPE_I32, 2U}, /* 0xA9 i32.trunc_f32_u */
    {WASM_TYPE_F64, 0U, WASM_TYPE_I32, 2U}, /* 0xAA i32.trunc_f64_s */
    {WASM_TYPE_F64, 0U, WASM_TYPE_I32, 2U}, /* 0xAB i32.trunc_f64_u */
    {WASM_TYPE_I32, 0U, WASM_TYPE_I64, 0U}, /* 0xAC i64.extend_i32_s */
    {WASM_TYPE_I32, 0U, WASM_TYPE_I64, 0U}, /* 0xAD i64.extend_i32_u */
    {WASM_TYPE_F32, 0U, WASM_TYPE_I64, 2U}, /* 0xAE i64.trunc_f32_s */
    {WASM_TYPE_F32, 0U, WASM_TYPE_I64, 2U}, /* 0xAF i64.trunc_f32_u */
    {WASM_TYPE_F64, 0U, WASM_TYPE_I64, 2U}, /* 0xB0 i64.trunc_f64_s */
    {WASM_TYPE_F64, 0U, WASM_TYPE_I64, 2U}, /* 0xB1 i64.trunc_f64_u */
    {WASM_TYPE_I32, 0U, WASM_TYPE_F32, 2U}, /* 0xB2 f32.convert_i32_s */
    {WASM_TYPE_I32, 0U, WASM_TYPE_F32, 2U}, /* 0xB3 f32.convert_i32_u */
    {WASM_TYPE_I64, 0U, WASM_TYPE_F32, 2U}, /* 0xB4 f32.convert_i64_s */
    {WASM_TYPE_I64, 0U, WASM_TYPE_F32, 2U}, /* 0xB5 f32.convert_i64_u */
    {WASM_TYPE_F64, 0U, WASM_TYPE_F32, 2U}, /* 0xB6 f32.demote_f64 */
    {WASM_TYPE_I32, 0U, WASM_TYPE_F64, 2U}, /* 0xB7 f64.convert_i32_s */
    {WASM_TYPE_I32, 0U, WASM_TYPE_F64, 2U}, /* 0xB8 f64.convert_i32_u */
    {WASM_TYPE_I64, 0U, WASM_TYPE_F64, 2U}, /* 0xB9 f64.convert_i64_s */
    {WASM_TYPE_I64, 0U, WASM_TYPE_F64, 2U}, /* 0xBA f64.convert_i64_u */
    {WASM_TYPE_F32, 0U, WASM_TYPE_F64, 2U}, /* 0xBB f64.promote_f32 */
    {WASM_TYPE_F32, 0U, WASM_TYPE_I32, 2U}, /* 0xBC i32.reinterpret_f32 */
    {WASM_TYPE_F64, 0U, WASM_TYPE_I64, 2U}, /* 0xBD i64.reinterpret_f64 */
    {WASM_TYPE_I32, 0U, WASM_TYPE_F32, 2U}, /* 0xBE f32.reinterpret_i32 */
    {WASM_TYPE_I64, 0U, WASM_TYPE_F64, 2U}, /* 0xBF f64.reinterpret_i64 */
    /* 0xC0..0xC4 sign-ext (5) */
    {WASM_TYPE_I32, 0U, WASM_TYPE_I32, 0U}, /* 0xC0 i32.extend8_s */
    {WASM_TYPE_I32, 0U, WASM_TYPE_I32, 0U}, /* 0xC1 i32.extend16_s */
    {WASM_TYPE_I64, 0U, WASM_TYPE_I64, 0U}, /* 0xC2 i64.extend8_s */
    {WASM_TYPE_I64, 0U, WASM_TYPE_I64, 0U}, /* 0xC3 i64.extend16_s */
    {WASM_TYPE_I64, 0U, WASM_TYPE_I64, 0U}, /* 0xC4 i64.extend32_s */
};

const char *syn_wasm_load_error_str(SYN_WASM_LoadError err)
{
    switch (err) {
    case SYN_WASM_LOAD_OK:
        return "OK";
    case SYN_WASM_LOAD_ERR_INVALID_ARG:
        return "invalid argument or truncated buffer";
    case SYN_WASM_LOAD_ERR_BAD_HEADER:
        return "invalid WebAssembly magic or version";
    case SYN_WASM_LOAD_ERR_MALFORMED:
        return "malformed or type-invalid WebAssembly module";
    case SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED:
        return "module exceeds static VM resource limits";
    case SYN_WASM_LOAD_ERR_UNSUPPORTED:
        return "module uses unsupported WebAssembly extension";
    default:
        return "unknown WebAssembly load error";
    }
}

static bool wasm_is_valid_utf8(const uint8_t *buf, uint32_t len)
{
    /* RFC 3629 branchless DFA (Hoehrmann): 256 byte-to-class + 108 state transitions */
    static const uint8_t utf8_dfa[364] = {
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,  0U,
        0U,  0U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,  1U,
        9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  9U,  7U,  7U,
        7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,
        7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  7U,  8U,  8U,  2U,  2U,  2U,  2U,
        2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,
        2U,  2U,  2U,  2U,  2U,  2U,  2U,  2U,  10U, 3U,  3U,  3U,  3U,  3U,  3U,  3U,  3U,  3U,
        3U,  3U,  3U,  4U,  3U,  3U,  11U, 6U,  6U,  6U,  5U,  8U,  8U,  8U,  8U,  8U,  8U,  8U,
        8U,  8U,  8U,  8U,  0U,  12U, 24U, 36U, 60U, 96U, 84U, 12U, 12U, 12U, 48U, 72U, 12U, 12U,
        12U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 0U,  12U, 12U, 12U, 12U, 12U, 0U,
        12U, 0U,  12U, 12U, 12U, 24U, 12U, 12U, 12U, 12U, 12U, 24U, 12U, 24U, 12U, 12U, 12U, 12U,
        12U, 12U, 12U, 12U, 12U, 24U, 12U, 12U, 12U, 12U, 12U, 24U, 12U, 12U, 12U, 12U, 12U, 12U,
        12U, 24U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 12U, 36U, 12U, 36U, 12U, 12U, 12U, 36U,
        12U, 12U, 12U, 12U, 12U, 36U, 12U, 36U, 12U, 12U, 12U, 36U, 12U, 12U, 12U, 12U, 12U, 12U,
        12U, 12U, 12U, 12U,
    };

    uint32_t state = 0U;
    for (uint32_t i = 0U; i < len; i++) {
        uint8_t cls = utf8_dfa[buf[i]];
        state = utf8_dfa[256U + state + cls];
    }
    return state == 0U;
}

static bool wasm_sec_read_u32(const uint8_t *bytes, uint32_t section_end, uint32_t *offset,
                              uint32_t *out)
{
    uint32_t result = 0U;
    uint32_t shift = 0U;
    uint32_t cur = *offset;

    for (;;) {
        if (cur >= section_end) {
            return false;
        }
        uint8_t byte = bytes[cur++];
        if ((shift == 28U) && ((byte & 0xF0U) != 0U)) {
            return false;
        }
        result |= (uint32_t)(byte & 0x7FU) << shift;
        if ((byte & 0x80U) == 0U) {
            *offset = cur;
            *out = result;
            return true;
        }
        shift += 7U;
    }
}

static bool wasm_sec_read_i32(const uint8_t *bytes, uint32_t section_end, uint32_t *offset,
                              int32_t *out)
{
    uint32_t result = 0U;
    uint32_t shift = 0U;
    uint32_t cur = *offset;

    for (;;) {
        if (cur >= section_end) {
            return false;
        }
        uint8_t byte = bytes[cur++];
        if (shift == 28U) {
            uint8_t top = (uint8_t)(byte & 0xF8U);
            if ((top != 0x00U) && (top != 0x78U)) {
                return false;
            }
        }
        result |= (uint32_t)(byte & 0x7FU) << shift;
        shift += 7U;
        if ((byte & 0x80U) == 0U) {
            if ((shift < 32U) && ((byte & 0x40U) != 0U)) {
                result |= (~0U << shift);
            }
            *offset = cur;
            *out = (int32_t)result;
            return true;
        }
    }
}

static bool wasm_sec_read_i64(const uint8_t *bytes, uint32_t section_end, uint32_t *offset,
                              int64_t *out)
{
    uint64_t result = 0ULL;
    uint32_t shift = 0U;
    uint32_t cur = *offset;

    for (;;) {
        if (cur >= section_end) {
            return false;
        }
        uint8_t byte = bytes[cur++];
        if ((shift == 63U) && (byte != 0x00U) && (byte != 0x7FU)) {
            return false;
        }
        result |= (uint64_t)(byte & 0x7FU) << shift;
        shift += 7U;
        if ((byte & 0x80U) == 0U) {
            if ((shift < 64U) && ((byte & 0x40U) != 0U)) {
                result |= (~0ULL << shift);
            }
            *offset = cur;
            *out = (int64_t)result;
            return true;
        }
    }
}

static SYN_WASM_LoadError wasm_check_valtype(uint8_t vt)
{
    if ((vt == WASM_TYPE_I32) || (vt == WASM_TYPE_I64) || (vt == WASM_TYPE_F32)) {
        return SYN_WASM_LOAD_OK;
    }
    if (vt == WASM_TYPE_F64) {
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
#else
        return SYN_WASM_LOAD_OK;
#endif
    }
    if ((vt == 0x7BU) || (vt == 0x70U) || (vt == 0x6FU)) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    return SYN_WASM_LOAD_ERR_MALFORMED;
}

static SYN_WASM_LoadError wasm_read_utf8_name(const uint8_t *bytes, uint32_t section_end,
                                              uint32_t *offset, uint32_t *out_start,
                                              uint32_t *out_len)
{
    uint32_t len = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &len) ||
        (((uint64_t)(*offset) + (uint64_t)len) > (uint64_t)section_end) ||
        !wasm_is_valid_utf8(&bytes[*offset], len)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    *out_start = *offset;
    *out_len = len;
    *offset += len;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_check_valtype_vec(const uint8_t *bytes, uint32_t pos, uint32_t count)
{
    for (uint32_t i = 0U; i < count; i++) {
        SYN_WASM_LoadError err = wasm_check_valtype(bytes[pos + i]);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_type_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                  uint32_t section_end, uint32_t *offset)
{
    uint32_t num_types = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_types)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (num_types > SYN_WASM_MAX_TYPES) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    for (uint32_t i = 0U; i < num_types; i++) {
        if ((*offset >= section_end) || (bytes[*offset] != 0x60U)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        (*offset)++;
        uint32_t num_params = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &num_params) ||
            (((uint64_t)(*offset) + (uint64_t)num_params) > (uint64_t)section_end)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if (num_params > WASM_MAX_FUNC_PARAMS) {
            return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
        }
        uint32_t params_off = *offset;
        SYN_WASM_LoadError perr = wasm_check_valtype_vec(bytes, params_off, num_params);
        if (perr != SYN_WASM_LOAD_OK) {
            return perr;
        }
        *offset += num_params;

        uint32_t num_results = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &num_results) ||
            (((uint64_t)(*offset) + (uint64_t)num_results) > (uint64_t)section_end)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if (num_results > 1U) {
            return SYN_WASM_LOAD_ERR_UNSUPPORTED;
        }
        uint32_t results_off = *offset;
        SYN_WASM_LoadError rerr = wasm_check_valtype_vec(bytes, results_off, num_results);
        if (rerr != SYN_WASM_LOAD_OK) {
            return rerr;
        }
        *offset += num_results;

        mod->types[mod->type_count].params_offset = params_off;
        mod->types[mod->type_count].results_offset = results_off;
        mod->types[mod->type_count].param_count = (uint8_t)num_params;
        mod->types[mod->type_count].result_count = (uint8_t)num_results;
        mod->type_count++;
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_limits_ex(const uint8_t *bytes, uint32_t section_end,
                                               uint32_t *offset, bool is_memory, uint32_t *out_min,
                                               bool *out_has_max, uint32_t *out_max)
{
    if (*offset >= section_end) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint8_t flags = bytes[(*offset)++];
    if ((flags >= 2U) && (flags <= 7U)) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    uint32_t min_val = 0U;
    if ((flags > 1U) || !wasm_sec_read_u32(bytes, section_end, offset, &min_val)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (is_memory && (min_val > WASM_MAX_MEM_PAGES)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint32_t max_val = 0U;
    bool has_max = ((flags & 1U) != 0U);
    if (has_max) {
        if (!wasm_sec_read_u32(bytes, section_end, offset, &max_val) || (max_val < min_val) ||
            (is_memory && (max_val > WASM_MAX_MEM_PAGES))) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
    }
    *out_min = min_val;
    *out_has_max = has_max;
    *out_max = max_val;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_table_type(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                uint32_t section_end, uint32_t *offset,
                                                WASM_LoadState *st)
{
    if (*offset >= section_end) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint8_t elemtype = bytes[(*offset)++];
    if (elemtype == 0x6FU) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    if (elemtype != 0x70U) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (st->table_count >= 1U) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    SYN_WASM_LoadError err =
        wasm_parse_limits_ex(bytes, section_end, offset, false, &mod->table_min_elems,
                             &mod->table_has_max, &mod->table_max_elems);
    if (err == SYN_WASM_LOAD_OK) {
        st->table_count++;
        st->table_min = mod->table_min_elems;
        mod->has_table = true;
    }
    return err;
}

static SYN_WASM_LoadError wasm_parse_memory_type(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                 uint32_t section_end, uint32_t *offset,
                                                 WASM_LoadState *st)
{
    if (st->memory_count >= 1U) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    SYN_WASM_LoadError err =
        wasm_parse_limits_ex(bytes, section_end, offset, true, &mod->mem_min_pages,
                             &mod->mem_has_max, &mod->mem_max_pages);
    if (err == SYN_WASM_LOAD_OK) {
        st->memory_count++;
        mod->has_memory = true;
    }
    return err;
}

static SYN_WASM_LoadError wasm_record_global(WASM_LoadState *st, uint8_t gtype, uint8_t mut_flag)
{
    SYN_WASM_LoadError err = wasm_check_valtype(gtype);
    if (err != SYN_WASM_LOAD_OK) {
        return err;
    }
    if (mut_flag > 1U) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (st->global_count >= SYN_WASM_MAX_GLOBALS) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    st->global_types[st->global_count] = gtype;
    st->global_mut[st->global_count] = mut_flag;
    st->global_count++;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_import_desc(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                 uint32_t section_end, uint32_t *offset,
                                                 uint8_t kind, WASM_LoadState *st)
{
    if (kind == 0U) {
        uint32_t type_idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &type_idx) ||
            (type_idx >= (uint32_t)mod->type_count)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if ((mod->func_count >= SYN_WASM_MAX_FUNCTIONS) ||
            (mod->import_func_count >= SYN_WASM_MAX_HOST_FUNCS) ||
            (mod->types[type_idx].param_count > SYN_WASM_MAX_HOST_ARGS)) {
            return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
        }
        mod->funcs[mod->func_count].type_idx = type_idx;
        mod->funcs[mod->func_count].code_offset = 0U;
        mod->funcs[mod->func_count].code_size = 0U;
        mod->funcs[mod->func_count].param_count = mod->types[type_idx].param_count;
        mod->funcs[mod->func_count].result_count = mod->types[type_idx].result_count;
        mod->func_count++;
        mod->import_func_count++;
        return SYN_WASM_LOAD_OK;
    }
    if (kind == 1U) {
        return wasm_parse_table_type(mod, bytes, section_end, offset, st);
    }
    if (kind == 2U) {
        return wasm_parse_memory_type(mod, bytes, section_end, offset, st);
    }
    if (kind == 3U) {
        if (((uint64_t)(*offset) + 2ULL) > (uint64_t)section_end) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t gtype = bytes[(*offset)++];
        uint8_t gmut = bytes[(*offset)++];
        SYN_WASM_LoadError err = wasm_record_global(st, gtype, gmut);
        if (err == SYN_WASM_LOAD_OK) {
            mod->import_global_count++;
        }
        return err;
    }
    return SYN_WASM_LOAD_ERR_MALFORMED;
}

static SYN_WASM_LoadError wasm_parse_import_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                    uint32_t section_end, uint32_t *offset,
                                                    WASM_LoadState *st)
{
    uint32_t num_imports = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_imports)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    for (uint32_t i = 0U; i < num_imports; i++) {
        uint32_t dummy_pos = 0U;
        uint32_t dummy_len = 0U;
        if ((wasm_read_utf8_name(bytes, section_end, offset, &dummy_pos, &dummy_len) !=
             SYN_WASM_LOAD_OK) ||
            (wasm_read_utf8_name(bytes, section_end, offset, &dummy_pos, &dummy_len) !=
             SYN_WASM_LOAD_OK) ||
            (*offset >= section_end)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t kind = bytes[(*offset)++];
        SYN_WASM_LoadError err = wasm_parse_import_desc(mod, bytes, section_end, offset, kind, st);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_func_section(const SYN_WASM_Module *mod, const uint8_t *bytes,
                                                  uint32_t section_end, uint32_t *offset,
                                                  WASM_LoadState *st)
{
    uint32_t num_funcs = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_funcs)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (((uint64_t)mod->import_func_count + (uint64_t)num_funcs) >
        (uint64_t)SYN_WASM_MAX_FUNCTIONS) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    for (uint32_t i = 0U; i < num_funcs; i++) {
        uint32_t type_idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &type_idx) ||
            (type_idx >= (uint32_t)mod->type_count)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        st->type_func_indices[st->func_decl_count++] = type_idx;
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_table_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                   uint32_t section_end, uint32_t *offset,
                                                   WASM_LoadState *st)
{
    uint32_t num_tables = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_tables)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    for (uint32_t i = 0U; i < num_tables; i++) {
        SYN_WASM_LoadError err = wasm_parse_table_type(mod, bytes, section_end, offset, st);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_memory_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                    uint32_t section_end, uint32_t *offset,
                                                    WASM_LoadState *st)
{
    uint32_t num_mems = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_mems)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    for (uint32_t i = 0U; i < num_mems; i++) {
        SYN_WASM_LoadError err = wasm_parse_memory_type(mod, bytes, section_end, offset, st);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_validate_const_expr(const SYN_WASM_Module *mod,
                                                   const WASM_LoadState *st, uint32_t section_end,
                                                   uint32_t *offset, uint8_t expected_type,
                                                   int32_t *out_i32)
{
    if (*offset >= section_end) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint8_t op = mod->bytes[(*offset)++];
    int32_t v32 = 0;
    bool ok = false;

    if ((op == OP_I32_CONST) && (expected_type == WASM_TYPE_I32)) {
        ok = wasm_sec_read_i32(mod->bytes, section_end, offset, &v32);
    } else if ((op == OP_I64_CONST) && (expected_type == WASM_TYPE_I64)) {
        int64_t v64 = 0;
        ok = wasm_sec_read_i64(mod->bytes, section_end, offset, &v64);
    } else if ((op == OP_F32_CONST) && (expected_type == WASM_TYPE_F32)) {
        ok = (((uint64_t)(*offset) + 4ULL) <= (uint64_t)section_end);
        if (ok) {
            *offset += 4U;
        }
    } else if ((op == OP_F64_CONST) && (expected_type == WASM_TYPE_F64)) {
        ok = (((uint64_t)(*offset) + 8ULL) <= (uint64_t)section_end);
        if (ok) {
            *offset += 8U;
        }
    } else if (op == OP_GLOBAL_GET) {
        uint32_t g_idx = 0U;
        ok = wasm_sec_read_u32(mod->bytes, section_end, offset, &g_idx) &&
             (g_idx < (uint32_t)mod->import_global_count) &&
             (st->global_types[g_idx] == expected_type) && (st->global_mut[g_idx] == 0U);
    } else if ((op == 0xD0U) || (op == 0xD2U)) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }

    if (!ok || (*offset >= section_end) || (mod->bytes[*offset] != OP_END)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    (*offset)++;
    if (out_i32 != NULL) {
        *out_i32 = v32;
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_global_section(const SYN_WASM_Module *mod,
                                                    const uint8_t *bytes, uint32_t section_end,
                                                    uint32_t *offset, WASM_LoadState *st)
{
    uint32_t num_globals = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_globals)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    for (uint32_t i = 0U; i < num_globals; i++) {
        if (((uint64_t)(*offset) + 2ULL) > (uint64_t)section_end) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t gtype = bytes[(*offset)++];
        uint8_t gmut = bytes[(*offset)++];
        SYN_WASM_LoadError err =
            wasm_validate_const_expr(mod, st, section_end, offset, gtype, NULL);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
        err = wasm_record_global(st, gtype, gmut);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static bool wasm_export_name_seen(const uint8_t *bytes, uint32_t section_end,
                                  uint32_t entries_start, uint32_t count_before, uint32_t name_off,
                                  uint32_t name_len)
{
    uint32_t scan = entries_start;
    for (uint32_t j = 0U; j < count_before; j++) {
        uint32_t prev_len = 0U;
        uint32_t prev_idx = 0U;
        (void)wasm_sec_read_u32(bytes, section_end, &scan, &prev_len);
        uint32_t prev_off = scan;
        scan += prev_len + 1U;
        (void)wasm_sec_read_u32(bytes, section_end, &scan, &prev_idx);
        if ((prev_len == name_len) && (memcmp(&bytes[prev_off], &bytes[name_off], name_len) == 0)) {
            return true;
        }
    }
    return false;
}

static bool wasm_check_export_target(uint8_t kind, uint32_t idx, uint32_t total_funcs,
                                     const WASM_LoadState *st)
{
    if (kind == 0U) {
        return idx < total_funcs;
    }
    if (kind == 1U) {
        return idx < (uint32_t)st->table_count;
    }
    if (kind == 2U) {
        return idx < (uint32_t)st->memory_count;
    }
    return (kind == 3U) && (idx < (uint32_t)st->global_count);
}

static SYN_WASM_LoadError wasm_parse_export_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                    uint32_t section_end, uint32_t *offset,
                                                    const WASM_LoadState *st)
{
    uint32_t num_exports = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_exports)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (num_exports > WASM_MAX_EXPORTS) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    uint32_t entries_start = *offset;
    uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;

    for (uint32_t i = 0U; i < num_exports; i++) {
        uint32_t name_off = 0U;
        uint32_t name_len = 0U;
        if ((wasm_read_utf8_name(bytes, section_end, offset, &name_off, &name_len) !=
             SYN_WASM_LOAD_OK) ||
            (*offset >= section_end) ||
            wasm_export_name_seen(bytes, section_end, entries_start, i, name_off, name_len)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t kind = bytes[(*offset)++];
        uint32_t idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &idx) ||
            !wasm_check_export_target(kind, idx, total_funcs, st)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        mod->exports[mod->export_count].name_offset = name_off;
        mod->exports[mod->export_count].name_len = (uint16_t)name_len;
        mod->exports[mod->export_count].func_idx = (uint16_t)idx;
        mod->exports[mod->export_count].kind = kind;
        mod->export_count++;
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_start_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                   uint32_t section_end, uint32_t *offset,
                                                   const WASM_LoadState *st)
{
    uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;
    if (mod->has_start_func ||
        !wasm_sec_read_u32(bytes, section_end, offset, &mod->start_func_idx) ||
        (mod->start_func_idx >= total_funcs)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint32_t t_idx = (mod->start_func_idx < (uint32_t)mod->import_func_count)
                         ? mod->funcs[mod->start_func_idx].type_idx
                         : st->type_func_indices[mod->start_func_idx - mod->import_func_count];
    if ((mod->types[t_idx].param_count != 0U) || (mod->types[t_idx].result_count != 0U)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    mod->has_start_func = true;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_elem_segment(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                  uint32_t section_end, uint32_t *offset,
                                                  const WASM_LoadState *st, uint32_t max_func_idx)
{
    uint32_t flags = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &flags)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (flags != 0U) {
        return (flags <= 7U) ? SYN_WASM_LOAD_ERR_UNSUPPORTED : SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (st->table_count == 0U) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    bool is_const = (*offset < section_end) && (bytes[*offset] == OP_I32_CONST);
    int32_t elem_off = 0;
    SYN_WASM_LoadError cerr =
        wasm_validate_const_expr(mod, st, section_end, offset, WASM_TYPE_I32, &elem_off);
    if (cerr != SYN_WASM_LOAD_OK) {
        return cerr;
    }
    uint32_t num_funcs = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_funcs)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    bool seg_oob =
        is_const && ((elem_off < 0) || (((uint64_t)(uint32_t)elem_off + (uint64_t)num_funcs) >
                                        (uint64_t)st->table_min));
    if (is_const && !seg_oob &&
        (((uint64_t)(uint32_t)elem_off + (uint64_t)num_funcs) > (uint64_t)WASM_MAX_TABLE_ELEMS)) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    for (uint32_t f = 0U; f < num_funcs; f++) {
        uint32_t f_idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &f_idx) || (f_idx >= max_func_idx)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if (is_const && !seg_oob) {
            uint32_t dest = (uint32_t)elem_off + f;
            mod->table_elements[dest] = (uint16_t)f_idx;
            if ((dest + 1U) > (uint32_t)mod->table_element_count) {
                mod->table_element_count = (uint16_t)(dest + 1U);
            }
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_elem_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                  uint32_t section_end, uint32_t *offset,
                                                  const WASM_LoadState *st)
{
    uint32_t num_segments = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_segments)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;
    for (uint32_t s = 0U; s < num_segments; s++) {
        SYN_WASM_LoadError err =
            wasm_parse_elem_segment(mod, bytes, section_end, offset, st, total_funcs);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_datacount_section(const uint8_t *bytes, uint32_t section_end,
                                                       uint32_t *offset, WASM_LoadState *st)
{
    if (!wasm_sec_read_u32(bytes, section_end, offset, &st->datacount)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    st->has_datacount = true;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_data_seg(SYN_WASM_Module *mod, const uint8_t *bytes,
                                              uint32_t section_end, uint32_t *offset,
                                              const WASM_LoadState *st, uint32_t seg_idx)
{
    uint32_t flags = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &flags)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (flags > 1U) {
        return (flags == 2U) ? SYN_WASM_LOAD_ERR_UNSUPPORTED : SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (flags == 0U) {
        if (st->memory_count == 0U) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        SYN_WASM_LoadError cerr =
            wasm_validate_const_expr(mod, st, section_end, offset, WASM_TYPE_I32, NULL);
        if (cerr != SYN_WASM_LOAD_OK) {
            return cerr;
        }
    } else if (seg_idx >= SYN_WASM_MAX_DATA_SEGS) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    uint32_t data_len = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &data_len) ||
        (((uint64_t)(*offset) + (uint64_t)data_len) > (uint64_t)section_end)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (seg_idx < SYN_WASM_MAX_DATA_SEGS) {
        mod->data_segs[seg_idx].offset = *offset;
        mod->data_segs[seg_idx].size = data_len;
        mod->data_segs[seg_idx].passive = (flags == 1U);
        mod->data_seg_count = (uint16_t)(seg_idx + 1U);
    }
    *offset += data_len;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_parse_data_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                  uint32_t section_end, uint32_t *offset,
                                                  const WASM_LoadState *st)
{
    uint32_t num_segments = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_segments) ||
        (st->has_datacount && (num_segments != st->datacount))) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (st->has_datacount && (num_segments > SYN_WASM_MAX_DATA_SEGS)) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    for (uint32_t s = 0U; s < num_segments; s++) {
        SYN_WASM_LoadError err = wasm_parse_data_seg(mod, bytes, section_end, offset, st, s);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

/* ── Bytecode & Operand Stack Validator (Wasm 1.0 Appendix A.2) ─────────── */

static SYN_WASM_LoadError wasm_val_push(WASM_CodeVal *v, uint8_t type)
{
    if (v->val_sp >= SYN_WASM_MAX_STACK) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    v->val_stack[v->val_sp++] = type;
    return SYN_WASM_LOAD_OK;
}

static bool wasm_val_pop(WASM_CodeVal *v, uint8_t *out_type)
{
    const WASM_CtrlFrame *top = &v->ctrl[v->ctrl_depth];
    if (v->val_sp == top->height) {
        if (top->unreachable != 0U) {
            *out_type = WASM_VAL_UNKNOWN;
            return true;
        }
        return false;
    }
    *out_type = v->val_stack[--v->val_sp];
    return true;
}

static bool wasm_val_pop_expect(WASM_CodeVal *v, uint8_t expected)
{
    uint8_t actual = 0U;
    if (!wasm_val_pop(v, &actual)) {
        return false;
    }
    return (actual == expected) || (actual == WASM_VAL_UNKNOWN) || (expected == WASM_VAL_UNKNOWN);
}

static void wasm_val_mark_unreachable(WASM_CodeVal *v)
{
    v->val_sp = v->ctrl[v->ctrl_depth].height;
    v->ctrl[v->ctrl_depth].unreachable = 1U;
}

static SYN_WASM_LoadError wasm_val_apply_ctrl_types(WASM_CodeVal *v, uint16_t ctrl_idx,
                                                    bool is_branch, bool push_back)
{
    const WASM_CtrlFrame *frm = &v->ctrl[ctrl_idx];
    if (is_branch && (frm->opcode == OP_LOOP)) {
        return SYN_WASM_LOAD_OK;
    }
    if (frm->result_type != WASM_BLOCK_VOID) {
        if (!wasm_val_pop_expect(v, frm->result_type)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if (push_back) {
            return wasm_val_push(v, frm->result_type);
        }
    }
    return SYN_WASM_LOAD_OK;
}

static uint8_t wasm_val_label_type(const WASM_CodeVal *v, uint16_t idx)
{
    const WASM_CtrlFrame *f = &v->ctrl[idx];
    return (f->opcode == OP_LOOP) ? WASM_BLOCK_VOID : f->result_type;
}

static SYN_WASM_LoadError wasm_val_push_block(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                              uint8_t op, uint32_t body_end, uint32_t *pc)
{
    if (*pc >= body_end) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint8_t bt = mod->bytes[(*pc)++];
    if (bt != WASM_BLOCK_VOID) {
        if ((bt < 0x40U) || (bt >= 0x80U)) {
            (*pc)--;
            int64_t s33 = 0;
            if (!wasm_sec_read_i64(mod->bytes, body_end, pc, &s33)) {
                return SYN_WASM_LOAD_ERR_MALFORMED;
            }
            return ((s33 >= 0) && (s33 < (int64_t)mod->type_count)) ? SYN_WASM_LOAD_ERR_UNSUPPORTED
                                                                    : SYN_WASM_LOAD_ERR_MALFORMED;
        }
        SYN_WASM_LoadError bterr = wasm_check_valtype(bt);
        if (bterr != SYN_WASM_LOAD_OK) {
            return bterr;
        }
    }
    if ((op == OP_IF) && !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (v->ctrl_depth >= SYN_WASM_MAX_LABELS) {
        return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
    }
    v->ctrl_depth++;
    v->ctrl[v->ctrl_depth].opcode = op;
    v->ctrl[v->ctrl_depth].result_type = bt;
    v->ctrl[v->ctrl_depth].height = v->val_sp;
    v->ctrl[v->ctrl_depth].unreachable = 0U;
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_val_else_or_end(WASM_CodeVal *v, uint8_t op, bool *out_done)
{
    WASM_CtrlFrame *frm = &v->ctrl[v->ctrl_depth];
    if (op == OP_ELSE) {
        if ((v->ctrl_depth == 0U) || (frm->opcode != OP_IF)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if ((wasm_val_apply_ctrl_types(v, v->ctrl_depth, false, false) != SYN_WASM_LOAD_OK) ||
            (v->val_sp != frm->height)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        frm->opcode = OP_ELSE;
        frm->unreachable = 0U;
        return SYN_WASM_LOAD_OK;
    }

    if ((frm->opcode == OP_IF) && (frm->result_type != WASM_BLOCK_VOID)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if ((wasm_val_apply_ctrl_types(v, v->ctrl_depth, false, false) != SYN_WASM_LOAD_OK) ||
        (v->val_sp != frm->height)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (v->ctrl_depth == 0U) {
        *out_done = true;
        return SYN_WASM_LOAD_OK;
    }
    uint8_t res_type = frm->result_type;
    v->ctrl_depth--;
    return (res_type != WASM_BLOCK_VOID) ? wasm_val_push(v, res_type) : SYN_WASM_LOAD_OK;
}

static bool wasm_val_br_target_ok(const WASM_CodeVal *v, uint32_t lbl, bool want_val,
                                  uint8_t opd_type)
{
    if (lbl > (uint32_t)v->ctrl_depth) {
        return false;
    }
    uint8_t lt = wasm_val_label_type(v, (uint16_t)(v->ctrl_depth - (uint16_t)lbl));
    if (!want_val) {
        return lt == WASM_BLOCK_VOID;
    }
    return (lt != WASM_BLOCK_VOID) && ((opd_type == WASM_VAL_UNKNOWN) || (opd_type == lt));
}

static SYN_WASM_LoadError wasm_val_br_table(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                            uint32_t body_end, uint32_t *pc)
{
    uint32_t count = 0U;
    if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &count) ||
        !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint32_t list_start = *pc;
    for (uint32_t i = 0U; i < count; i++) {
        uint32_t dummy = 0U;
        if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &dummy)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
    }
    uint32_t def_lbl = 0U;
    if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &def_lbl) ||
        (def_lbl > (uint32_t)v->ctrl_depth)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint8_t def_t = wasm_val_label_type(v, (uint16_t)(v->ctrl_depth - (uint16_t)def_lbl));
    bool want_val = (def_t != WASM_BLOCK_VOID);
    uint8_t opd_t = WASM_VAL_UNKNOWN;
    if (want_val && (!wasm_val_pop(v, &opd_t) || !wasm_val_br_target_ok(v, def_lbl, true, opd_t))) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint32_t scan = list_start;
    for (uint32_t i = 0U; i < count; i++) {
        uint32_t lbl = 0U;
        (void)wasm_sec_read_u32(mod->bytes, body_end, &scan, &lbl);
        if (!wasm_val_br_target_ok(v, lbl, want_val, opd_t)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
    }
    wasm_val_mark_unreachable(v);
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_val_apply_sig(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                             uint32_t type_idx)
{
    const SYN_WASM_FuncType *ft = &mod->types[type_idx];
    for (uint8_t p = ft->param_count; p > 0U; p--) {
        if (!wasm_val_pop_expect(v, mod->bytes[ft->params_offset + p - 1U])) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
    }
    for (uint8_t r = 0U; r < ft->result_count; r++) {
        SYN_WASM_LoadError err = wasm_val_push(v, mod->bytes[ft->results_offset + r]);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_val_control_op(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                              const WASM_LoadState *st, uint8_t op,
                                              uint32_t body_end, uint32_t *pc, bool *out_done)
{
    if (op == OP_UNREACHABLE) {
        wasm_val_mark_unreachable(v);
        return SYN_WASM_LOAD_OK;
    }
    if (op == OP_NOP) {
        return SYN_WASM_LOAD_OK;
    }
    if ((op == OP_BLOCK) || (op == OP_LOOP) || (op == OP_IF)) {
        return wasm_val_push_block(v, mod, op, body_end, pc);
    }
    if ((op == OP_ELSE) || (op == OP_END)) {
        return wasm_val_else_or_end(v, op, out_done);
    }
    if ((op == OP_BR) || (op == OP_BR_IF)) {
        uint32_t lbl = 0U;
        if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &lbl) || (lbl > (uint32_t)v->ctrl_depth)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if ((op == OP_BR_IF) && !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint16_t target = (uint16_t)(v->ctrl_depth - (uint16_t)lbl);
        SYN_WASM_LoadError err = wasm_val_apply_ctrl_types(v, target, true, (op == OP_BR_IF));
        if ((err == SYN_WASM_LOAD_OK) && (op == OP_BR)) {
            wasm_val_mark_unreachable(v);
        }
        return err;
    }
    if (op == OP_BR_TABLE) {
        return wasm_val_br_table(v, mod, body_end, pc);
    }
    if (op == OP_RETURN) {
        SYN_WASM_LoadError err = wasm_val_apply_ctrl_types(v, 0U, false, false);
        if (err == SYN_WASM_LOAD_OK) {
            wasm_val_mark_unreachable(v);
        }
        return err;
    }
    if (op == OP_CALL) {
        uint32_t fn_idx = 0U;
        uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;
        if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &fn_idx) || (fn_idx >= total_funcs)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint32_t t_idx = (fn_idx < (uint32_t)mod->import_func_count)
                             ? mod->funcs[fn_idx].type_idx
                             : st->type_func_indices[fn_idx - mod->import_func_count];
        return wasm_val_apply_sig(v, mod, t_idx);
    }
    if (op == OP_CALL_INDIRECT) {
        uint32_t t_idx = 0U;
        if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &t_idx) ||
            (t_idx >= (uint32_t)mod->type_count) || (*pc >= body_end)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t tbl_zero = mod->bytes[(*pc)++];
        if (tbl_zero != 0x00U) {
            return ((tbl_zero < 0x80U) && (st->table_count > 0U)) ? SYN_WASM_LOAD_ERR_UNSUPPORTED
                                                                  : SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if ((st->table_count == 0U) || !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return wasm_val_apply_sig(v, mod, t_idx);
    }
    return ((op >= 0x06U && op <= 0x09U) || (op == 0x12U) || (op == 0x13U) || (op == 0x18U) ||
            (op == 0x19U))
               ? SYN_WASM_LOAD_ERR_UNSUPPORTED
               : SYN_WASM_LOAD_ERR_MALFORMED;
}

static SYN_WASM_LoadError wasm_val_var_param_op(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                                const WASM_LoadState *st, uint8_t op,
                                                uint32_t body_end, uint32_t *pc)
{
    if (op == OP_DROP) {
        uint8_t dummy = 0U;
        return wasm_val_pop(v, &dummy) ? SYN_WASM_LOAD_OK : SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (op == OP_SELECT) {
        uint8_t t2 = 0U;
        uint8_t t1 = 0U;
        if (!wasm_val_pop_expect(v, WASM_TYPE_I32) || !wasm_val_pop(v, &t2) ||
            !wasm_val_pop(v, &t1) ||
            ((t1 != WASM_VAL_UNKNOWN) && (t2 != WASM_VAL_UNKNOWN) && (t1 != t2))) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return wasm_val_push(v, (t1 == WASM_VAL_UNKNOWN) ? t2 : t1);
    }
    if ((op == 0x1CU) || (op == 0x25U) || (op == 0x26U)) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    if ((op < OP_LOCAL_GET) || (op > OP_GLOBAL_SET)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    uint32_t idx = 0U;
    if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &idx)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (op <= OP_LOCAL_TEE) {
        if (idx >= (uint32_t)v->local_count) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t lt = v->local_types[idx];
        if ((op != OP_LOCAL_GET) && !wasm_val_pop_expect(v, lt)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return (op != OP_LOCAL_SET) ? wasm_val_push(v, lt) : SYN_WASM_LOAD_OK;
    }
    if (idx >= (uint32_t)st->global_count) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (op == OP_GLOBAL_GET) {
        return wasm_val_push(v, st->global_types[idx]);
    }
    if ((st->global_mut[idx] == 0U) || !wasm_val_pop_expect(v, st->global_types[idx])) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_val_memory_op(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                             const WASM_LoadState *st, uint8_t op,
                                             uint32_t body_end, uint32_t *pc)
{
    if (st->memory_count == 0U) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if ((op == OP_MEMORY_SIZE) || (op == OP_MEMORY_GROW)) {
        if ((*pc >= body_end) || (mod->bytes[*pc] != 0x00U)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        (*pc)++;
        if ((op == OP_MEMORY_GROW) && !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return wasm_val_push(v, WASM_TYPE_I32);
    }

    const WASM_MemOpSig *sig = &g_wasm_mem_sigs[op - OP_I32_LOAD];
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
    if (sig->val_type == WASM_TYPE_F64) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
#endif
    uint32_t align = 0U;
    uint32_t mem_off = 0U;
    if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &align) ||
        !wasm_sec_read_u32(mod->bytes, body_end, pc, &mem_off) ||
        (align > (uint32_t)sig->max_align_log2)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (sig->is_store != 0U) {
        if (!wasm_val_pop_expect(v, sig->val_type) || !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return SYN_WASM_LOAD_OK;
    }
    if (!wasm_val_pop_expect(v, WASM_TYPE_I32)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    return wasm_val_push(v, sig->val_type);
}

static SYN_WASM_LoadError wasm_val_const_or_num_op(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                                   uint8_t op, uint32_t body_end, uint32_t *pc)
{
    if (op == OP_I32_CONST) {
        int32_t c32 = 0;
        return wasm_sec_read_i32(mod->bytes, body_end, pc, &c32) ? wasm_val_push(v, WASM_TYPE_I32)
                                                                 : SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (op == OP_I64_CONST) {
        int64_t c64 = 0;
        return wasm_sec_read_i64(mod->bytes, body_end, pc, &c64) ? wasm_val_push(v, WASM_TYPE_I64)
                                                                 : SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (op == OP_F32_CONST) {
        if (((uint64_t)(*pc) + 4ULL) > (uint64_t)body_end) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        *pc += 4U;
        return wasm_val_push(v, WASM_TYPE_F32);
    }
    if (op == OP_F64_CONST) {
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
#else
        if (((uint64_t)(*pc) + 8ULL) > (uint64_t)body_end) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        *pc += 8U;
        return wasm_val_push(v, WASM_TYPE_F64);
#endif
    }

    const WASM_NumOpSig *sig = &g_wasm_num_sigs[op - OP_I32_EQZ];
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
    if (sig->fp_mode == 2U) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
#endif
    if ((sig->in2 != 0U) && !wasm_val_pop_expect(v, sig->in2)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (!wasm_val_pop_expect(v, sig->in1)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    return wasm_val_push(v, sig->out);
}

static SYN_WASM_LoadError wasm_val_misc_op(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                           const WASM_LoadState *st, uint32_t body_end,
                                           uint32_t *pc)
{
    uint32_t subop = 0U;
    if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &subop)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    if (subop <= 7U) {
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
#else
        uint8_t in_t = ((subop & 2U) == 0U) ? WASM_TYPE_F32 : WASM_TYPE_F64;
        uint8_t out_t = (subop < 4U) ? WASM_TYPE_I32 : WASM_TYPE_I64;
        return wasm_val_pop_expect(v, in_t) ? wasm_val_push(v, out_t) : SYN_WASM_LOAD_ERR_MALFORMED;
#endif
    }
    if (subop == 9U) {
        uint32_t seg = 0U;
        if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &seg) || !st->has_datacount ||
            (seg >= st->datacount)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return SYN_WASM_LOAD_OK;
    }
    if ((subop >= 8U) && (subop <= 11U)) {
        if (subop == 8U) {
            uint32_t seg = 0U;
            if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &seg) || !st->has_datacount ||
                (seg >= st->datacount)) {
                return SYN_WASM_LOAD_ERR_MALFORMED;
            }
        }
        uint32_t mem_bytes = (subop == 10U) ? 2U : 1U;
        if ((st->memory_count == 0U) || (((uint64_t)(*pc) + mem_bytes) > (uint64_t)body_end)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        for (uint32_t m = 0U; m < mem_bytes; m++) {
            uint8_t mb = mod->bytes[(*pc)++];
            if (mb != 0x00U) {
                return (mb < 0x80U) ? SYN_WASM_LOAD_ERR_UNSUPPORTED : SYN_WASM_LOAD_ERR_MALFORMED;
            }
        }
        if (!wasm_val_pop_expect(v, WASM_TYPE_I32) || !wasm_val_pop_expect(v, WASM_TYPE_I32) ||
            !wasm_val_pop_expect(v, WASM_TYPE_I32)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        return SYN_WASM_LOAD_OK;
    }
    return (subop <= 17U) ? SYN_WASM_LOAD_ERR_UNSUPPORTED : SYN_WASM_LOAD_ERR_MALFORMED;
}

static SYN_WASM_LoadError wasm_val_init_locals(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                               uint16_t func_idx, uint32_t body_end, uint32_t *pc)
{
    memset(v, 0, sizeof(*v));

    const SYN_WASM_FuncType *ft = &mod->types[mod->funcs[func_idx].type_idx];
    for (uint8_t p = 0U; p < ft->param_count; p++) {
        v->local_types[p] = mod->bytes[ft->params_offset + p];
    }
    uint64_t total_locals = (uint64_t)ft->param_count;

    uint32_t num_decls = 0U;
    if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &num_decls)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    for (uint32_t d = 0U; d < num_decls; d++) {
        uint32_t cnt = 0U;
        if (!wasm_sec_read_u32(mod->bytes, body_end, pc, &cnt) || (*pc >= body_end)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t ltype = mod->bytes[(*pc)++];
        SYN_WASM_LoadError lerr = wasm_check_valtype(ltype);
        if (lerr != SYN_WASM_LOAD_OK) {
            return lerr;
        }
        total_locals += (uint64_t)cnt;
        if (total_locals > 0xFFFFFFFFULL) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        if (total_locals > (uint64_t)SYN_WASM_MAX_LOCALS) {
            return SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED;
        }
        for (uint32_t k = 0U; k < cnt; k++) {
            v->local_types[v->local_count + ft->param_count + k] = ltype;
        }
        v->local_count = (uint16_t)(v->local_count + (uint16_t)cnt);
    }
    v->local_count = (uint16_t)total_locals;

    v->ctrl[0].opcode = OP_BLOCK;
    v->ctrl[0].result_type =
        (ft->result_count == 0U) ? WASM_BLOCK_VOID : mod->bytes[ft->results_offset];
    return SYN_WASM_LOAD_OK;
}

static SYN_WASM_LoadError wasm_validate_step(WASM_CodeVal *v, const SYN_WASM_Module *mod,
                                             const WASM_LoadState *st, uint8_t op,
                                             uint32_t body_end, uint32_t *pc, bool *out_done)
{
    if (op <= 0x19U) {
        return wasm_val_control_op(v, mod, st, op, body_end, pc, out_done);
    }
    if (op <= 0x26U) {
        return wasm_val_var_param_op(v, mod, st, op, body_end, pc);
    }
    if ((op >= OP_I32_LOAD) && (op <= OP_MEMORY_GROW)) {
        return wasm_val_memory_op(v, mod, st, op, body_end, pc);
    }
    if ((op >= OP_I32_CONST) && (op <= OP_I64_EXTEND32_S)) {
        return wasm_val_const_or_num_op(v, mod, op, body_end, pc);
    }
    if (op == OP_PREFIX_MISC) {
        return wasm_val_misc_op(v, mod, st, body_end, pc);
    }
    if ((op == OP_PREFIX_GC) || (op == OP_PREFIX_SIMD) || (op == OP_PREFIX_ATOMIC) ||
        ((op >= 0xD0U) && (op <= 0xD2U))) {
        return SYN_WASM_LOAD_ERR_UNSUPPORTED;
    }
    return SYN_WASM_LOAD_ERR_MALFORMED;
}

static SYN_WASM_LoadError wasm_validate_code_body(const SYN_WASM_Module *mod,
                                                  const WASM_LoadState *st, uint16_t func_idx,
                                                  uint32_t body_start, uint32_t body_end)
{
    WASM_CodeVal v;
    uint32_t pc = body_start;
    SYN_WASM_LoadError err = wasm_val_init_locals(&v, mod, func_idx, body_end, &pc);
    if (err != SYN_WASM_LOAD_OK) {
        return err;
    }

    bool done = false;
    while (!done) {
        if (pc >= body_end) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint8_t op = mod->bytes[pc++];
        err = wasm_validate_step(&v, mod, st, op, body_end, &pc, &done);
        if (err != SYN_WASM_LOAD_OK) {
            return err;
        }
    }
    return (pc == body_end) ? SYN_WASM_LOAD_OK : SYN_WASM_LOAD_ERR_MALFORMED;
}

static SYN_WASM_LoadError wasm_parse_code_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                                  uint32_t section_end, uint32_t *offset,
                                                  WASM_LoadState *st)
{
    uint32_t num_bodies = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_bodies) ||
        (num_bodies != (uint32_t)st->func_decl_count)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    for (uint32_t i = 0U; i < num_bodies; i++) {
        uint32_t body_size = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &body_size) || (body_size == 0U)) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint32_t body_start = *offset;
        if (((uint64_t)body_start + (uint64_t)body_size) > (uint64_t)section_end) {
            return SYN_WASM_LOAD_ERR_MALFORMED;
        }
        uint16_t fn_idx = mod->func_count;
        uint32_t t_idx = st->type_func_indices[i];
        mod->funcs[fn_idx].type_idx = t_idx;
        mod->funcs[fn_idx].code_offset = body_start;
        mod->funcs[fn_idx].code_size = body_size;
        mod->funcs[fn_idx].param_count = mod->types[t_idx].param_count;
        mod->funcs[fn_idx].result_count = mod->types[t_idx].result_count;
        mod->func_count++;
        st->code_body_count++;

        SYN_WASM_LoadError berr =
            wasm_validate_code_body(mod, st, fn_idx, body_start, body_start + body_size);
        if (berr != SYN_WASM_LOAD_OK) {
            return berr;
        }
        *offset = body_start + body_size;
    }
    return SYN_WASM_LOAD_OK;
}

static uint8_t wasm_section_order_rank(uint8_t section_id)
{
    if ((section_id >= WASM_SEC_TYPE) && (section_id <= WASM_SEC_ELEMENT)) {
        return section_id;
    }
    if (section_id == WASM_SEC_DATACOUNT) {
        return 10U;
    }
    if (section_id == WASM_SEC_CODE) {
        return 11U;
    }
    if (section_id == WASM_SEC_DATA) {
        return 12U;
    }
    return 0U;
}

static SYN_WASM_LoadError wasm_dispatch_load_section(SYN_WASM_Module *mod, uint8_t section_id,
                                                     uint32_t section_end, uint32_t *offset,
                                                     WASM_LoadState *st)
{
    if (section_id == WASM_SEC_CUSTOM) {
        uint32_t dummy_off = 0U;
        uint32_t dummy_len = 0U;
        SYN_WASM_LoadError nerr =
            wasm_read_utf8_name(mod->bytes, section_end, offset, &dummy_off, &dummy_len);
        *offset = section_end;
        return nerr;
    }

    uint8_t rank = wasm_section_order_rank(section_id);
    if ((rank == 0U) || (rank <= st->last_sec_rank)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    st->last_sec_rank = rank;

    SYN_WASM_LoadError err = SYN_WASM_LOAD_OK;
    switch (section_id) {
    case WASM_SEC_TYPE:
        err = wasm_parse_type_section(mod, mod->bytes, section_end, offset);
        break;
    case WASM_SEC_IMPORT:
        err = wasm_parse_import_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_FUNCTION:
        err = wasm_parse_func_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_TABLE:
        err = wasm_parse_table_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_MEMORY:
        err = wasm_parse_memory_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_GLOBAL:
        err = wasm_parse_global_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_EXPORT:
        err = wasm_parse_export_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_START:
        err = wasm_parse_start_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_ELEMENT:
        err = wasm_parse_elem_section(mod, mod->bytes, section_end, offset, st);
        break;
    case WASM_SEC_DATACOUNT:
        err = wasm_parse_datacount_section(mod->bytes, section_end, offset, st);
        break;
    default:
        err = (section_id == WASM_SEC_CODE)
                  ? wasm_parse_code_section(mod, mod->bytes, section_end, offset, st)
                  : wasm_parse_data_section(mod, mod->bytes, section_end, offset, st);
        break;
    }

    if ((err == SYN_WASM_LOAD_OK) && (*offset != section_end)) {
        return SYN_WASM_LOAD_ERR_MALFORMED;
    }
    return err;
}

static bool wasm_finish_load(SYN_WASM_Module *mod, SYN_WASM_LoadError err,
                             SYN_WASM_LoadError *out_error)
{
    if (mod != NULL) {
        mod->load_error = err;
    }
    if (out_error != NULL) {
        *out_error = err;
    }
    return err == SYN_WASM_LOAD_OK;
}

bool syn_wasm_module_load_ex(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size,
                             SYN_WASM_LoadError *out_error)
{
    if ((mod == NULL) || (bytes == NULL) || (size < 8U)) {
        return wasm_finish_load(mod, SYN_WASM_LOAD_ERR_INVALID_ARG, out_error);
    }

    memset(mod, 0, sizeof(*mod));
    for (uint32_t t = 0U; t < WASM_MAX_TABLE_ELEMS; t++) {
        mod->table_elements[t] = SYN_WASM_TABLE_NULL;
    }
    mod->bytes = bytes;
    mod->size = size;

    uint32_t magic = ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8U) |
                     ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
    uint32_t version = ((uint32_t)bytes[4]) | ((uint32_t)bytes[5] << 8U) |
                       ((uint32_t)bytes[6] << 16U) | ((uint32_t)bytes[7] << 24U);
    if ((magic != WASM_MAGIC) || (version != WASM_VERSION)) {
        return wasm_finish_load(mod, SYN_WASM_LOAD_ERR_BAD_HEADER, out_error);
    }

    WASM_LoadState st;
    memset(&st, 0, sizeof(st));

    uint32_t offset = 8U;
    while (offset < size) {
        uint8_t section_id = bytes[offset++];
        uint32_t section_len = 0U;
        if (!wasm_sec_read_u32(bytes, size, &offset, &section_len) ||
            (((uint64_t)offset + (uint64_t)section_len) > (uint64_t)size)) {
            return wasm_finish_load(mod, SYN_WASM_LOAD_ERR_MALFORMED, out_error);
        }
        uint32_t section_end = offset + section_len;
        SYN_WASM_LoadError err =
            wasm_dispatch_load_section(mod, section_id, section_end, &offset, &st);
        if (err != SYN_WASM_LOAD_OK) {
            return wasm_finish_load(mod, err, out_error);
        }
    }

    if ((st.code_body_count != st.func_decl_count) ||
        (st.has_datacount && (st.datacount > 0U) && (st.last_sec_rank < 12U))) {
        return wasm_finish_load(mod, SYN_WASM_LOAD_ERR_MALFORMED, out_error);
    }
    return wasm_finish_load(mod, SYN_WASM_LOAD_OK, out_error);
}

bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size)
{
    return syn_wasm_module_load_ex(mod, bytes, size, NULL);
}

/* ── Context & Execution ─────────────────────────────────────────────────── */

static bool wasm_eval_global_init(const SYN_WASM_Context *ctx, const uint8_t *bytes,
                                  uint32_t section_end, uint32_t *offset, uint64_t *out_val)
{
    if (*offset >= section_end) {
        return false;
    }
    uint8_t op = bytes[(*offset)++];
    bool ok = false;
    uint64_t val = 0ULL;

    if (op == OP_I32_CONST) {
        int32_t v32 = 0;
        ok = wasm_sec_read_i32(bytes, section_end, offset, &v32);
        val = (uint64_t)(int64_t)v32;
    } else if (op == OP_I64_CONST) {
        int64_t v64 = 0;
        ok = wasm_sec_read_i64(bytes, section_end, offset, &v64);
        val = (uint64_t)v64;
    } else if (op == OP_F32_CONST) {
        ok = (((uint64_t)(*offset) + 4ULL) <= (uint64_t)section_end);
        if (ok) {
            uint32_t u32 = ((uint32_t)bytes[*offset]) | ((uint32_t)bytes[*offset + 1U] << 8U) |
                           ((uint32_t)bytes[*offset + 2U] << 16U) |
                           ((uint32_t)bytes[*offset + 3U] << 24U);
            *offset += 4U;
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
            float fv = 0.0F;
            memcpy(&fv, &u32, sizeof(fv));
            val = (uint64_t)(uint32_t)q16_from_float(fv);
#else
            val = (uint64_t)u32;
#endif
        }
    } else if (op == OP_F64_CONST) {
        ok = (((uint64_t)(*offset) + 8ULL) <= (uint64_t)section_end);
        if (ok) {
            for (uint32_t b = 0U; b < 8U; b++) {
                val |= ((uint64_t)bytes[*offset + b]) << (b * 8U);
            }
            *offset += 8U;
        }
    } else if (op == OP_GLOBAL_GET) {
        uint32_t g_idx = 0U;
        ok = wasm_sec_read_u32(bytes, section_end, offset, &g_idx) &&
             (g_idx < (uint32_t)ctx->module->import_global_count);
        if (ok) {
            val = ctx->globals[g_idx];
        }
    }

    if (!ok || (*offset >= section_end) || (bytes[*offset] != OP_END)) {
        return false;
    }
    (*offset)++;
    *out_val = val;
    return true;
}

static bool wasm_eval_const_i32(const SYN_WASM_Context *ctx, const uint8_t *bytes,
                                uint32_t section_end, uint32_t *offset, int32_t *out_val)
{
    uint8_t peek = (*offset < section_end) ? bytes[*offset] : 0U;
    uint64_t val = 0ULL;
    if (((peek != OP_I32_CONST) && (peek != OP_GLOBAL_GET)) ||
        !wasm_eval_global_init(ctx, bytes, section_end, offset, &val)) {
        return false;
    }
    *out_val = (int32_t)(uint32_t)val;
    return true;
}

static bool wasm_load_global_section(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                                     uint32_t section_end, uint32_t *offset)
{
    uint32_t num_globals = 0U;
    if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &num_globals)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    for (uint32_t g = 0U; g < num_globals; g++) {
        if (((uint64_t)(*offset) + 2ULL) > (uint64_t)section_end) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
        }
        *offset += 2U;
        uint64_t init_val = 0ULL;
        uint32_t g_slot = (uint32_t)mod->import_global_count + g;
        if (!wasm_eval_global_init(ctx, mod->bytes, section_end, offset, &init_val) ||
            (g_slot >= SYN_WASM_MAX_GLOBALS)) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
        }
        ctx->globals[g_slot] = init_val;
        ctx->global_count = (uint16_t)(g_slot + 1U);
    }
    return true;
}

static bool wasm_load_elem_seg(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                               uint32_t section_end, uint32_t *offset)
{
    uint32_t flags = 0U;
    int32_t elem_off = 0;
    uint32_t num_funcs = 0U;
    if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &flags) || (flags != 0U) ||
        !wasm_eval_const_i32(ctx, mod->bytes, section_end, offset, &elem_off) ||
        !wasm_sec_read_u32(mod->bytes, section_end, offset, &num_funcs)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    uint64_t end_slot = (uint64_t)(uint32_t)elem_off + (uint64_t)num_funcs;
    if ((elem_off < 0) || (end_slot > (uint64_t)mod->table_min_elems) ||
        (end_slot > (uint64_t)WASM_MAX_TABLE_ELEMS)) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return false;
    }
    for (uint32_t f = 0U; f < num_funcs; f++) {
        uint32_t f_idx = 0U;
        if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &f_idx) ||
            (f_idx >= (uint32_t)mod->func_count)) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
        }
        uint32_t dest = (uint32_t)elem_off + f;
        ctx->table_elements[dest] = (uint16_t)f_idx;
        if ((dest + 1U) > (uint32_t)ctx->table_element_count) {
            ctx->table_element_count = (uint16_t)(dest + 1U);
        }
    }
    return true;
}

static bool wasm_load_elem_section(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                                   uint32_t section_end, uint32_t *offset)
{
    uint32_t num_segments = 0U;
    if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &num_segments)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    for (uint32_t s = 0U; s < num_segments; s++) {
        if (!wasm_load_elem_seg(ctx, mod, section_end, offset)) {
            return false;
        }
    }
    return true;
}

static bool wasm_load_data_seg(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                               uint32_t section_end, uint32_t *offset, uint32_t seg_idx)
{
    uint32_t flags = 0U;
    if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &flags) || (flags > 1U)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    if (flags == 1U) {
        uint32_t pas_size = 0U;
        if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &pas_size) ||
            (((uint64_t)(*offset) + (uint64_t)pas_size) > (uint64_t)section_end)) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
        }
        *offset += pas_size;
        return true;
    }
    int32_t data_offset = 0;
    uint32_t data_size = 0U;
    if (!wasm_eval_const_i32(ctx, mod->bytes, section_end, offset, &data_offset) ||
        !wasm_sec_read_u32(mod->bytes, section_end, offset, &data_size) ||
        (((uint64_t)(*offset) + (uint64_t)data_size) > (uint64_t)section_end)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    if ((data_offset < 0) || ((data_size > 0U) && (ctx->linear_mem == NULL)) ||
        (((uint64_t)(uint32_t)data_offset + (uint64_t)data_size) >
         (uint64_t)ctx->linear_mem_size)) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return false;
    }
    if (data_size > 0U) {
        memcpy(&ctx->linear_mem[(uint32_t)data_offset], &mod->bytes[*offset], data_size);
    }
    if (seg_idx < SYN_WASM_MAX_DATA_SEGS) {
        ctx->data_dropped[seg_idx / 64U] |= (1ULL << (seg_idx % 64U));
    }
    *offset += data_size;
    return true;
}

static bool wasm_load_data_section(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                                   uint32_t section_end, uint32_t *offset)
{
    uint32_t num_segments = 0U;
    if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &num_segments)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    for (uint32_t s = 0U; s < num_segments; s++) {
        if (!wasm_load_data_seg(ctx, mod, section_end, offset, s)) {
            return false;
        }
    }
    return true;
}

static void wasm_init_memory_and_imports(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                                         uint8_t *linear_mem, uint32_t mem_size,
                                         const uint64_t *import_globals,
                                         uint16_t import_global_count)
{
    ctx->linear_mem = linear_mem;
    ctx->linear_mem_cap = mem_size;
    uint64_t min_bytes = (uint64_t)mod->mem_min_pages * 65536ULL;
    if (mod->has_memory && ((uint64_t)mem_size >= min_bytes)) {
        ctx->linear_mem_size = (uint32_t)min_bytes;
    } else {
        ctx->linear_mem_size = mem_size;
    }
    if (mod->has_memory && (linear_mem != NULL) && (ctx->linear_mem_size > 0U)) {
        memset(linear_mem, 0, ctx->linear_mem_size);
    }

    memcpy(ctx->table_elements, mod->table_elements, sizeof(ctx->table_elements));
    ctx->table_element_count = mod->table_element_count;

    uint16_t copy_globals = (import_global_count < mod->import_global_count)
                                ? import_global_count
                                : mod->import_global_count;
    if ((import_globals != NULL) && (copy_globals > 0U)) {
        memcpy(ctx->globals, import_globals, (size_t)copy_globals * sizeof(uint64_t));
    }
}

bool syn_wasm_init_ex(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                      uint32_t mem_size, const uint64_t *import_globals,
                      uint16_t import_global_count)
{
    if ((ctx == NULL) || (mod == NULL)) {
        return false;
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->module = mod;
    ctx->local_count = SYN_WASM_MAX_LOCALS;
    ctx->global_count = mod->import_global_count;
    ctx->status = SYN_WASM_OK;
    wasm_init_memory_and_imports(ctx, mod, linear_mem, mem_size, import_globals,
                                 import_global_count);

    static const uint8_t wasm_hdr[8] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U};
    if ((mod->bytes != NULL) && (mod->size > 8U) &&
        (memcmp(mod->bytes, wasm_hdr, sizeof(wasm_hdr)) == 0)) {
        uint32_t offset = 8U;
        while (offset < mod->size) {
            uint8_t section_id = mod->bytes[offset++];
            uint32_t section_size = 0U;
            if (!wasm_sec_read_u32(mod->bytes, mod->size, &offset, &section_size) ||
                (((uint64_t)offset + (uint64_t)section_size) > (uint64_t)mod->size)) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                return false;
            }
            uint32_t section_end = offset + section_size;
            if ((section_id == WASM_SEC_GLOBAL) &&
                !wasm_load_global_section(ctx, mod, section_end, &offset)) {
                return false;
            }
            if ((section_id == WASM_SEC_ELEMENT) &&
                !wasm_load_elem_section(ctx, mod, section_end, &offset)) {
                return false;
            }
            if ((section_id == WASM_SEC_DATA) &&
                !wasm_load_data_section(ctx, mod, section_end, &offset)) {
                return false;
            }
            offset = section_end;
        }
    }

    return true;
}

bool syn_wasm_init(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                   uint32_t mem_size)
{
    return syn_wasm_init_ex(ctx, mod, linear_mem, mem_size, NULL, 0U);
}

bool syn_wasm_register_host_ex(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func,
                               SYN_WASM_HostCancelFunc cancel, void *user)
{
    if ((ctx == NULL) || (import_index >= SYN_WASM_MAX_HOST_FUNCS) ||
        (ctx->host_pending && (ctx->host_import_idx == import_index))) {
        return false;
    }

    ctx->host_funcs[import_index] = func;
    ctx->host_cancels[import_index] = cancel;
    ctx->host_users[import_index] = user;
    if (import_index >= ctx->host_func_count) {
        ctx->host_func_count = (uint16_t)(import_index + 1U);
    }
    return true;
}

bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func)
{
    return syn_wasm_register_host_ex(ctx, import_index, func, NULL, NULL);
}

void *syn_wasm_host_user(const SYN_WASM_Context *ctx)
{
    if ((ctx == NULL) || !ctx->host_pending || (ctx->host_import_idx >= SYN_WASM_MAX_HOST_FUNCS)) {
        return NULL;
    }
    return ctx->host_users[ctx->host_import_idx];
}

void *syn_wasm_mem_ptr(SYN_WASM_Context *ctx, uint32_t offset, uint32_t size)
{
    if ((ctx == NULL) || (ctx->linear_mem == NULL) ||
        (((uint64_t)offset + (uint64_t)size) > (uint64_t)ctx->linear_mem_size)) {
        return NULL;
    }
    return (void *)(ctx->linear_mem + offset);
}

const void *syn_wasm_mem_cptr(const SYN_WASM_Context *ctx, uint32_t offset, uint32_t size)
{
    if ((ctx == NULL) || (ctx->linear_mem == NULL) ||
        (((uint64_t)offset + (uint64_t)size) > (uint64_t)ctx->linear_mem_size)) {
        return NULL;
    }
    return (const void *)(ctx->linear_mem + offset);
}

int32_t syn_wasm_find_export(const SYN_WASM_Module *mod, const char *name)
{
    if (!mod || !name) {
        return -1;
    }

    size_t len = strlen(name);
    for (uint16_t i = 0U; i < mod->export_count; i++) {
        if ((mod->exports[i].kind == 0U) && (mod->exports[i].name_len == len) &&
            (memcmp(mod->bytes + mod->exports[i].name_offset, name, len) == 0)) {
            return (int32_t)mod->exports[i].func_idx;
        }
    }

    return -1;
}

static uint32_t wasm_frame_code_end(const SYN_WASM_Context *ctx)
{
    const SYN_WASM_Module *mod = ctx->module;
    uint16_t fn_idx = ctx->call_stack[ctx->call_depth - 1U].func_idx;
    if (fn_idx >= mod->func_count || mod->funcs[fn_idx].code_size == 0U) {
        return mod->size;
    }
    uint64_t end =
        (uint64_t)mod->funcs[fn_idx].code_offset + (uint64_t)mod->funcs[fn_idx].code_size;
    return (end < (uint64_t)mod->size) ? (uint32_t)end : mod->size;
}

static bool wasm_parse_func_locals(SYN_WASM_Context *ctx, uint16_t func_index, uint32_t *out_pc,
                                   uint32_t *out_locals)
{
    const SYN_WASM_FuncDef *fn = &ctx->module->funcs[func_index];
    uint64_t end64 = (uint64_t)fn->code_offset + (uint64_t)fn->code_size;
    if (end64 > (uint64_t)ctx->module->size) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    uint32_t saved_pc = ctx->pc;
    uint32_t end = (uint32_t)end64;
    ctx->pc = fn->code_offset;

    uint32_t num_vecs = 0U;
    if (!wasm_read_u32_leb(ctx, end, &num_vecs)) {
        ctx->pc = saved_pc;
        return false;
    }
    uint64_t total = 0ULL;
    for (uint32_t i = 0U; i < num_vecs; i++) {
        uint32_t count = 0U;
        uint8_t ltype = 0U;
        if (!wasm_read_u32_leb(ctx, end, &count) || !wasm_read_byte(ctx, end, &ltype)) {
            ctx->pc = saved_pc;
            return false;
        }
        (void)ltype;
        total += (uint64_t)count;
        if (total > (uint64_t)SYN_WASM_MAX_LOCALS) {
            ctx->pc = saved_pc;
            ctx->status = SYN_WASM_TRAP_STACK_OVERFLOW;
            return false;
        }
    }
    *out_pc = ctx->pc;
    *out_locals = (uint32_t)total;
    ctx->pc = saved_pc;
    return true;
}

static bool wasm_push_frame(SYN_WASM_Context *ctx, uint16_t target_idx, bool allow_empty_stack)
{
    if (ctx->call_depth >= SYN_WASM_MAX_CALL_DEPTH) {
        ctx->status = SYN_WASM_TRAP_CALL_STACK_OVERFLOW;
        return false;
    }
    uint8_t argc = ctx->module->funcs[target_idx].param_count;
    if (!allow_empty_stack && ctx->sp < (uint32_t)argc) {
        ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
        return false;
    }
    uint32_t body_pc = 0U;
    uint32_t declared_locals = 0U;
    if (!wasm_parse_func_locals(ctx, target_idx, &body_pc, &declared_locals)) {
        return false;
    }
    uint16_t next_base = (ctx->call_depth == 0U) ? 0U : ctx->local_count;
    uint32_t total_locals = (uint32_t)argc + declared_locals;
    if ((uint32_t)next_base + total_locals > (uint32_t)SYN_WASM_MAX_LOCALS) {
        ctx->status = SYN_WASM_TRAP_STACK_OVERFLOW;
        return false;
    }
    if (total_locals > 0U) {
        memset(&ctx->locals[next_base], 0, (size_t)total_locals * sizeof(uint64_t));
    }
    if (ctx->sp >= (uint32_t)argc) {
        for (uint8_t i = argc; i > 0U; i--) {
            ctx->locals[next_base + (uint16_t)(i - 1U)] = ctx->stack[--ctx->sp];
        }
    } else {
        ctx->sp = 0U;
    }

    SYN_WASM_CallFrame *frame = &ctx->call_stack[ctx->call_depth++];
    frame->func_idx = target_idx;
    frame->return_pc = allow_empty_stack ? 0U : ctx->pc;
    frame->frame_sp = allow_empty_stack ? 0U : ctx->sp;
    frame->local_base = next_base;
    frame->label_base = ctx->label_depth;
    if (allow_empty_stack) {
        ctx->sp = 0U;
    }
    ctx->local_count = (uint16_t)(next_base + (uint16_t)total_locals);
    ctx->pc = body_pc;
    return true;
}

static void wasm_poll_host(SYN_WASM_Context *ctx)
{
    uint16_t imp_idx = ctx->host_import_idx;
    const SYN_WASM_Module *mod = ctx->module;
    uint64_t ret_val = 0ULL;
    const uint64_t *args_ptr = (ctx->host_argc > 0U) ? ctx->host_args : NULL;

    SYN_PT_Status pst =
        ctx->host_funcs[imp_idx](&ctx->host_pt, ctx, args_ptr, ctx->host_argc, &ret_val);
    if (!ctx->host_pending) {
        return;
    }
    if ((ctx->status == SYN_WASM_OK) && (pst < PT_EXITED)) {
        ctx->host_waiting = (pst == PT_IDLE);
        return;
    }

    ctx->host_pending = false;
    ctx->host_waiting = false;
    PT_INIT(&ctx->host_pt);

    if (pst == PT_EXITED) {
        if (ctx->status == SYN_WASM_OK) {
            ctx->status = SYN_WASM_TRAP_HOST;
        }
        return;
    }
    if (ctx->status == SYN_WASM_OK) {
        if (mod->funcs[imp_idx].result_count > 0U) {
            (void)push_stack64(ctx, ret_val);
        }
        if ((ctx->status == SYN_WASM_OK) && (ctx->call_depth == 0U)) {
            ctx->status = SYN_WASM_HALTED;
        }
    }
}

static void wasm_invoke_func(SYN_WASM_Context *ctx, uint32_t target_idx)
{
    const SYN_WASM_Module *mod = ctx->module;
    if (target_idx >= mod->func_count) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    if (target_idx < mod->import_func_count) {
        if ((target_idx >= ctx->host_func_count) || (ctx->host_funcs[target_idx] == NULL)) {
            ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
            return;
        }
        uint8_t argc = mod->funcs[target_idx].param_count;
        if (argc > SYN_WASM_MAX_HOST_ARGS) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return;
        }
        if (ctx->sp < (uint32_t)argc) {
            ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
            return;
        }
        ctx->sp -= (uint32_t)argc;
        if (argc > 0U) {
            memcpy(ctx->host_args, &ctx->stack[ctx->sp], (size_t)argc * sizeof(uint64_t));
        }
        PT_INIT(&ctx->host_pt);
        memset(&ctx->host_scratch, 0, sizeof(ctx->host_scratch));
        ctx->host_import_idx = (uint16_t)target_idx;
        ctx->host_argc = argc;
        ctx->host_waiting = false;
        ctx->host_pending = true;
        wasm_poll_host(ctx);
        return;
    }
    (void)wasm_push_frame(ctx, (uint16_t)target_idx, false);
}

static void wasm_do_return(SYN_WASM_Context *ctx)
{
    const SYN_WASM_Module *mod = ctx->module;
    SYN_WASM_CallFrame *frame = &ctx->call_stack[ctx->call_depth - 1U];
    uint16_t fn_idx = frame->func_idx;
    uint8_t res_count = (fn_idx < mod->func_count) ? mod->funcs[fn_idx].result_count : 0U;

    if (res_count > 0U) {
        if (ctx->sp < frame->frame_sp + (uint32_t)res_count) {
            ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
            return;
        }
        if (ctx->sp != frame->frame_sp + (uint32_t)res_count) {
            uint32_t src_base = ctx->sp - (uint32_t)res_count;
            for (uint8_t i = 0U; i < res_count; i++) {
                ctx->stack[frame->frame_sp + (uint32_t)i] = ctx->stack[src_base + (uint32_t)i];
            }
            ctx->sp = frame->frame_sp + (uint32_t)res_count;
        }
    } else {
        ctx->sp = frame->frame_sp;
    }

    ctx->label_depth = frame->label_base;
    ctx->local_count = frame->local_base;
    ctx->call_depth--;
    if (ctx->call_depth == 0U) {
        ctx->status = SYN_WASM_HALTED;
    } else {
        ctx->pc = frame->return_pc;
    }
}

bool syn_wasm_call(SYN_WASM_Context *ctx, uint16_t func_index)
{
    if ((ctx == NULL) || (ctx->module == NULL) || ctx->host_pending ||
        (func_index >= ctx->module->func_count)) {
        return false;
    }

    ctx->call_depth = 0U;
    ctx->label_depth = 0U;
    ctx->local_count = 0U;
    ctx->status = SYN_WASM_OK;

    if (func_index < ctx->module->import_func_count) {
        wasm_invoke_func(ctx, (uint32_t)func_index);
        return (ctx->status == SYN_WASM_OK) || (ctx->status == SYN_WASM_HALTED);
    }

    return wasm_push_frame(ctx, func_index, true);
}

void syn_wasm_abort(SYN_WASM_Context *ctx)
{
    if (ctx == NULL) {
        return;
    }

    bool was_pending = ctx->host_pending;
    SYN_WASM_HostCancelFunc cancel_fn = NULL;
    void *cancel_user = NULL;
    if (was_pending && (ctx->host_import_idx < SYN_WASM_MAX_HOST_FUNCS)) {
        cancel_fn = ctx->host_cancels[ctx->host_import_idx];
        cancel_user = ctx->host_users[ctx->host_import_idx];
    }

    ctx->host_pending = false;
    ctx->host_waiting = false;
    ctx->host_argc = 0U;
    PT_INIT(&ctx->host_pt);
    memset(&ctx->host_scratch, 0, sizeof(ctx->host_scratch));

    ctx->call_depth = 0U;
    ctx->label_depth = 0U;
    ctx->local_count = 0U;
    ctx->sp = 0U;
    if (was_pending && (ctx->active_task != NULL)) {
        ctx->active_task->delay_until = 0U;
    }
    ctx->status = SYN_WASM_HALTED;

    if (cancel_fn != NULL) {
        cancel_fn(ctx, cancel_user);
    }
    if (!ctx->host_pending && (ctx->call_depth == 0U)) {
        ctx->active_task = NULL;
    }
}

uint64_t syn_wasm_result(const SYN_WASM_Context *ctx)
{
    if (!ctx || ctx->sp == 0) {
        return 0;
    }
    return ctx->stack[ctx->sp - 1];
}

static bool wasm_push_label(SYN_WASM_Context *ctx, uint8_t opcode, uint8_t arity)
{
    if (ctx->label_depth >= SYN_WASM_MAX_LABELS) {
        ctx->status = SYN_WASM_TRAP_CALL_STACK_OVERFLOW;
        return false;
    }
    SYN_WASM_Label *lbl = &ctx->label_stack[ctx->label_depth++];
    lbl->opcode = opcode;
    lbl->arity = arity;
    lbl->target_pc = ctx->pc;
    lbl->stack_sp = ctx->sp;
    return true;
}

static void wasm_exec_block_if(SYN_WASM_Context *ctx, uint8_t opcode, uint32_t code_end)
{
    uint8_t bt = 0U;
    if (!wasm_read_byte(ctx, code_end, &bt)) {
        return;
    }
    uint8_t arity = (opcode != OP_LOOP && bt != 0x40U) ? 1U : 0U;
    if (opcode == OP_IF) {
        uint32_t cond = 0U;
        if (!pop_stack(ctx, &cond)) {
            return;
        }
        if (cond == 0U) {
            bool found_else = false;
            if (!wasm_skip_block(ctx, code_end, true, &found_else)) {
                return;
            }
            if (found_else) {
                (void)wasm_push_label(ctx, OP_IF, arity);
            }
            return;
        }
    }
    (void)wasm_push_label(ctx, opcode, arity);
}

static void wasm_exec_else(SYN_WASM_Context *ctx, uint32_t code_end)
{
    uint16_t label_base = ctx->call_stack[ctx->call_depth - 1U].label_base;
    if (ctx->label_depth <= label_base) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    bool dummy_else = false;
    if (wasm_skip_block(ctx, code_end, false, &dummy_else)) {
        ctx->label_depth--;
    }
}

static void wasm_branch_to_label(SYN_WASM_Context *ctx, uint32_t label_idx, uint32_t code_end)
{
    uint16_t label_base = ctx->call_stack[ctx->call_depth - 1U].label_base;
    uint32_t active_labels = (uint32_t)(ctx->label_depth - label_base);
    if (label_idx == active_labels) {
        wasm_do_return(ctx);
        return;
    }
    if (label_idx > active_labels) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    uint16_t target_depth = (uint16_t)((uint32_t)ctx->label_depth - 1U - label_idx);
    SYN_WASM_Label *lbl = &ctx->label_stack[target_depth];

    if (ctx->sp < lbl->stack_sp + (uint32_t)lbl->arity) {
        ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
        return;
    }
    if (lbl->arity > 0U) {
        ctx->stack[lbl->stack_sp] = ctx->stack[ctx->sp - 1U];
        ctx->sp = lbl->stack_sp + 1U;
    } else {
        ctx->sp = lbl->stack_sp;
    }

    if (lbl->opcode == OP_LOOP) {
        ctx->label_depth = (uint16_t)(target_depth + 1U);
        ctx->pc = lbl->target_pc;
    } else {
        ctx->label_depth = target_depth;
        ctx->pc = lbl->target_pc;
        bool dummy_else = false;
        (void)wasm_skip_block(ctx, code_end, false, &dummy_else);
    }
}

static void wasm_exec_br_table(SYN_WASM_Context *ctx, uint32_t code_end)
{
    uint32_t count = 0U;
    if (!wasm_read_u32_leb(ctx, code_end, &count)) {
        return;
    }
    uint32_t index = 0U;
    bool has_idx = pop_stack(ctx, &index);
    uint32_t target_idx = 0U;

    for (uint32_t i = 0U; i < count; i++) {
        uint32_t tbl_target = 0U;
        if (!wasm_read_u32_leb(ctx, code_end, &tbl_target)) {
            return;
        }
        if (has_idx && index == i) {
            target_idx = tbl_target;
        }
    }
    uint32_t default_target = 0U;
    if (!wasm_read_u32_leb(ctx, code_end, &default_target) || !has_idx) {
        return;
    }
    if (index >= count) {
        target_idx = default_target;
    }
    wasm_branch_to_label(ctx, target_idx, code_end);
}

static bool wasm_read_memarg(SYN_WASM_Context *ctx, uint32_t code_end, uint32_t *out_offset)
{
    uint32_t align = 0U;
    return wasm_read_u32_leb(ctx, code_end, &align) && wasm_read_u32_leb(ctx, code_end, out_offset);
}

static bool wasm_check_mem(SYN_WASM_Context *ctx, uint32_t base_addr, uint32_t offset,
                           uint32_t req_bytes, uint64_t *out_addr)
{
    uint64_t addr = (uint64_t)base_addr + (uint64_t)offset;
    if (ctx->linear_mem == NULL || addr + (uint64_t)req_bytes > (uint64_t)ctx->linear_mem_size) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return false;
    }
    *out_addr = addr;
    return true;
}

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
static void wasm_exec_trunc_i32(SYN_WASM_Context *ctx, double d, bool is_signed)
{
    if (isnan(d) || isinf(d)) {
        ctx->status = SYN_WASM_TRAP_DIV_ZERO;
        return;
    }
    double t = trunc(d);
    if (is_signed) {
        if (t < -2147483648.0 || t > 2147483647.0) {
            ctx->status = SYN_WASM_TRAP_DIV_ZERO;
            return;
        }
        (void)push_stack(ctx, (uint32_t)(int32_t)t);
    } else {
        if (t < 0.0 || t > 4294967295.0) {
            ctx->status = SYN_WASM_TRAP_DIV_ZERO;
            return;
        }
        (void)push_stack(ctx, (uint32_t)t);
    }
}

static void wasm_exec_trunc_i64(SYN_WASM_Context *ctx, double d, bool is_signed)
{
    if (isnan(d) || isinf(d)) {
        ctx->status = SYN_WASM_TRAP_DIV_ZERO;
        return;
    }
    double t = trunc(d);
    if (is_signed) {
        if (t < -9223372036854775808.0 || t >= 9223372036854775808.0) {
            ctx->status = SYN_WASM_TRAP_DIV_ZERO;
            return;
        }
        int64_t iv = (t == -9223372036854775808.0) ? INT64_MIN : (int64_t)t;
        (void)push_stack64(ctx, (uint64_t)iv);
    } else {
        if (t < 0.0 || t >= 18446744073709551616.0) {
            ctx->status = SYN_WASM_TRAP_DIV_ZERO;
            return;
        }
        (void)push_stack64(ctx, (uint64_t)t);
    }
}

static void wasm_exec_trunc(SYN_WASM_Context *ctx, uint8_t opcode)
{
    double d = 0.0;
    if (opcode == OP_I32_TRUNC_F32_S || opcode == OP_I32_TRUNC_F32_U ||
        opcode == OP_I64_TRUNC_F32_S || opcode == OP_I64_TRUNC_F32_U) {
        float fa = 0.0F;
        if (!pop_f32(ctx, &fa)) {
            return;
        }
        d = (double)fa;
    } else if (!pop_f64(ctx, &d)) {
        return;
    }

    bool is_signed = (opcode == OP_I32_TRUNC_F32_S || opcode == OP_I32_TRUNC_F64_S ||
                      opcode == OP_I64_TRUNC_F32_S || opcode == OP_I64_TRUNC_F64_S);
    if (opcode <= OP_I32_TRUNC_F64_U) {
        wasm_exec_trunc_i32(ctx, d, is_signed);
    } else {
        wasm_exec_trunc_i64(ctx, d, is_signed);
    }
}

static uint32_t wasm_sat_to_i32(double d, bool is_unsigned)
{
    if (isnan(d)) {
        return 0U;
    }
    if (is_unsigned) {
        if (d <= 0.0) {
            return 0U;
        }
        if (d >= 4294967296.0) {
            return 0xFFFFFFFFU;
        }
        return (uint32_t)trunc(d);
    }
    if (d <= -2147483648.0) {
        return 0x80000000U;
    }
    if (d >= 2147483647.0) {
        return 0x7FFFFFFFU;
    }
    return (uint32_t)(int32_t)trunc(d);
}

static uint64_t wasm_sat_to_i64(double d, bool is_unsigned)
{
    if (isnan(d)) {
        return 0ULL;
    }
    if (is_unsigned) {
        if (d <= 0.0) {
            return 0ULL;
        }
        if (d >= 18446744073709551616.0) {
            return 0xFFFFFFFFFFFFFFFFULL;
        }
        return (uint64_t)trunc(d);
    }
    if (d <= -9223372036854775808.0) {
        return 0x8000000000000000ULL;
    }
    if (d >= 9223372036854775808.0) {
        return 0x7FFFFFFFFFFFFFFFULL;
    }
    return (uint64_t)(int64_t)trunc(d);
}

static void wasm_exec_trunc_sat(SYN_WASM_Context *ctx, uint32_t subop)
{
    double d = 0.0;
    if ((subop & 2U) == 0U) {
        float f = 0.0F;
        if (!pop_f32(ctx, &f)) {
            return;
        }
        d = (double)f;
    } else if (!pop_f64(ctx, &d)) {
        return;
    }

    bool is_unsigned = ((subop & 1U) != 0U);
    if (subop < 4U) {
        (void)push_stack(ctx, wasm_sat_to_i32(d, is_unsigned));
    } else {
        (void)push_stack64(ctx, wasm_sat_to_i64(d, is_unsigned));
    }
}
#endif

static void wasm_exec_mem_grow(SYN_WASM_Context *ctx, uint32_t code_end)
{
    uint8_t mem_idx = 0U;
    uint32_t delta = 0U;
    if (!wasm_read_byte(ctx, code_end, &mem_idx) || !pop_stack(ctx, &delta)) {
        return;
    }
    if (mem_idx != 0U) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    uint32_t cur_pages = ctx->linear_mem_size / 65536U;
    if (delta == 0U) {
        (void)push_stack(ctx, cur_pages);
        return;
    }
    uint64_t new_pages = (uint64_t)cur_pages + (uint64_t)delta;
    uint32_t max_pages = ctx->module->mem_has_max ? ctx->module->mem_max_pages : WASM_MAX_MEM_PAGES;
    uint64_t new_bytes = new_pages * 65536ULL;
    if ((ctx->linear_mem != NULL) && (new_pages <= (uint64_t)max_pages) &&
        (new_bytes <= (uint64_t)ctx->linear_mem_cap)) {
        memset(&ctx->linear_mem[ctx->linear_mem_size], 0,
               (size_t)(new_bytes - (uint64_t)ctx->linear_mem_size));
        ctx->linear_mem_size = (uint32_t)new_bytes;
        (void)push_stack(ctx, cur_pages);
    } else {
        (void)push_stack(ctx, 0xFFFFFFFFU);
    }
}

static void wasm_exec_mem_init(SYN_WASM_Context *ctx, uint32_t code_end)
{
    const SYN_WASM_Module *mod = ctx->module;
    uint32_t seg_idx = 0U;
    uint32_t mem_idx = 0U;
    if (!wasm_read_u32_leb(ctx, code_end, &seg_idx) ||
        !wasm_read_u32_leb(ctx, code_end, &mem_idx)) {
        return;
    }
    if ((mem_idx != 0U) || (seg_idx >= SYN_WASM_MAX_DATA_SEGS)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    uint32_t n = 0U;
    uint32_t src = 0U;
    uint32_t dst = 0U;
    if (!pop_stack(ctx, &n) || !pop_stack(ctx, &src) || !pop_stack(ctx, &dst)) {
        return;
    }
    bool dropped = (seg_idx >= (uint32_t)mod->data_seg_count) || !mod->data_segs[seg_idx].passive ||
                   ((ctx->data_dropped[seg_idx / 64U] & (1ULL << (seg_idx % 64U))) != 0ULL);
    uint32_t seg_size = dropped ? 0U : mod->data_segs[seg_idx].size;
    if ((((uint64_t)src + (uint64_t)n) > (uint64_t)seg_size) ||
        (((uint64_t)dst + (uint64_t)n) > (uint64_t)ctx->linear_mem_size) ||
        ((n > 0U) && (ctx->linear_mem == NULL))) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return;
    }
    if (n > 0U) {
        memcpy(&ctx->linear_mem[dst], &mod->bytes[mod->data_segs[seg_idx].offset + src], n);
    }
}

static void wasm_exec_bulk_mem(SYN_WASM_Context *ctx, uint32_t subop, uint32_t code_end)
{
    uint32_t imm0 = 0U;
    uint32_t imm1 = 0U;
    if (subop == 8U) {
        wasm_exec_mem_init(ctx, code_end);
    } else if (subop == 9U) {
        if (wasm_read_u32_leb(ctx, code_end, &imm0)) {
            if (imm0 >= SYN_WASM_MAX_DATA_SEGS) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            } else {
                ctx->data_dropped[imm0 / 64U] |= (1ULL << (imm0 % 64U));
            }
        }
    } else if (subop == 10U) {
        if (!wasm_read_u32_leb(ctx, code_end, &imm0) || !wasm_read_u32_leb(ctx, code_end, &imm1)) {
            return;
        }
        if ((imm0 | imm1) != 0U) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return;
        }
        uint32_t len = 0U;
        uint32_t src = 0U;
        uint32_t dst = 0U;
        if (pop_stack(ctx, &len) && pop_stack(ctx, &src) && pop_stack(ctx, &dst)) {
            if (((len == 0U) || (ctx->linear_mem != NULL)) &&
                (((uint64_t)dst + (uint64_t)len) <= (uint64_t)ctx->linear_mem_size) &&
                (((uint64_t)src + (uint64_t)len) <= (uint64_t)ctx->linear_mem_size)) {
                if (len > 0U) {
                    memmove(&ctx->linear_mem[dst], &ctx->linear_mem[src], len);
                }
            } else {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
            }
        }
    } else if (subop == 11U) {
        if (!wasm_read_u32_leb(ctx, code_end, &imm0)) {
            return;
        }
        if (imm0 != 0U) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return;
        }
        uint32_t len = 0U;
        uint32_t val = 0U;
        uint32_t dst = 0U;
        if (pop_stack(ctx, &len) && pop_stack(ctx, &val) && pop_stack(ctx, &dst)) {
            if (((len == 0U) || (ctx->linear_mem != NULL)) &&
                (((uint64_t)dst + (uint64_t)len) <= (uint64_t)ctx->linear_mem_size)) {
                if (len > 0U) {
                    memset(&ctx->linear_mem[dst], (uint8_t)val, len);
                }
            } else {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
            }
        }
    } else {
        ctx->status = SYN_WASM_TRAP_BAD_OPCODE;
    }
}

static void wasm_exec_misc(SYN_WASM_Context *ctx, uint32_t code_end)
{
    uint32_t subop = 0U;
    if (!wasm_read_u32_leb(ctx, code_end, &subop)) {
        return;
    }
#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
    if (subop <= 7U) {
        wasm_exec_trunc_sat(ctx, subop);
        return;
    }
#endif
    wasm_exec_bulk_mem(ctx, subop, code_end);
}

static bool wasm_types_equal(const SYN_WASM_Module *mod, uint32_t idx_a, uint32_t idx_b)
{
    if (idx_a == idx_b) {
        return true;
    }
    const SYN_WASM_FuncType *a = &mod->types[idx_a];
    const SYN_WASM_FuncType *b = &mod->types[idx_b];
    if ((a->param_count != b->param_count) || (a->result_count != b->result_count)) {
        return false;
    }
    return (memcmp(&mod->bytes[a->params_offset], &mod->bytes[b->params_offset], a->param_count) ==
            0) &&
           (memcmp(&mod->bytes[a->results_offset], &mod->bytes[b->results_offset],
                   a->result_count) == 0);
}

static void wasm_exec_call_indirect(SYN_WASM_Context *ctx, uint32_t code_end)
{
    const SYN_WASM_Module *mod = ctx->module;
    uint32_t type_idx = 0U;
    uint32_t table_idx = 0U;
    uint32_t elem_idx = 0U;
    if (!wasm_read_u32_leb(ctx, code_end, &type_idx) ||
        !wasm_read_u32_leb(ctx, code_end, &table_idx) || !pop_stack(ctx, &elem_idx)) {
        return;
    }
    if (table_idx != 0U) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    if ((elem_idx >= ctx->table_element_count) ||
        (ctx->table_elements[elem_idx] == SYN_WASM_TABLE_NULL)) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return;
    }
    uint16_t target_fn = ctx->table_elements[elem_idx];
    if ((target_fn >= mod->func_count) || (type_idx >= mod->type_count) ||
        (mod->funcs[target_fn].type_idx >= mod->type_count)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    if (!wasm_types_equal(mod, type_idx, mod->funcs[target_fn].type_idx)) {
        ctx->status = SYN_WASM_TRAP_TYPE_MISMATCH;
        return;
    }
    wasm_invoke_func(ctx, target_fn);
}

static void wasm_exec_sign_ext(SYN_WASM_Context *ctx, uint8_t opcode)
{
    if (opcode <= OP_I32_EXTEND16_S) {
        uint32_t a = 0U;
        if (!pop_stack(ctx, &a)) {
            return;
        }
        uint32_t mask = (opcode == OP_I32_EXTEND8_S) ? 0xFFU : 0xFFFFU;
        uint32_t sign_bit = (opcode == OP_I32_EXTEND8_S) ? 0x80U : 0x8000U;
        uint32_t res = ((a & mask) ^ sign_bit) - sign_bit;
        (void)push_stack(ctx, res);
        return;
    }

    uint64_t a64 = 0ULL;
    if (!pop_stack64(ctx, &a64)) {
        return;
    }
    uint64_t mask64 = 0xFFFFFFFFULL;
    uint64_t sign64 = 0x80000000ULL;
    if (opcode == OP_I64_EXTEND8_S) {
        mask64 = 0xFFULL;
        sign64 = 0x80ULL;
    } else if (opcode == OP_I64_EXTEND16_S) {
        mask64 = 0xFFFFULL;
        sign64 = 0x8000ULL;
    }
    uint64_t res64 = ((a64 & mask64) ^ sign64) - sign64;
    (void)push_stack64(ctx, res64);
}

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
static float wasm_f32_nearest(float x)
{
    if (isnan(x)) {
        return x + x;
    }
    float ax = fabsf(x);
    if (isinf(ax) || (ax == 0.0F) || (ax >= 8388608.0F)) {
        return x;
    }
    float fl = floorf(ax);
    float diff = ax - fl;
    float r = fl;
    if ((diff > 0.5F) || ((diff == 0.5F) && (((uint32_t)fl & 1U) != 0U))) {
        r = fl + 1.0F;
    }
    return copysignf(r, x);
}

static double wasm_f64_nearest(double x)
{
    if (isnan(x)) {
        return x + x;
    }
    double ax = fabs(x);
    if (isinf(ax) || (ax == 0.0) || (ax >= 4503599627370496.0)) {
        return x;
    }
    double fl = floor(ax);
    double diff = ax - fl;
    double r = fl;
    if ((diff > 0.5) || ((diff == 0.5) && (((uint64_t)fl & 1ULL) != 0ULL))) {
        r = fl + 1.0;
    }
    return copysign(r, x);
}

static float wasm_f32_min_max(float a, float b, bool is_max)
{
    if (isnan(a) || isnan(b)) {
        return a + b;
    }
    if ((a == 0.0F) && (b == 0.0F)) {
        uint32_t ua = 0U;
        uint32_t ub = 0U;
        memcpy(&ua, &a, sizeof(ua));
        memcpy(&ub, &b, sizeof(ub));
        uint32_t ures = is_max ? (ua & ub) : (ua | ub);
        float res = 0.0F;
        memcpy(&res, &ures, sizeof(res));
        return res;
    }
    if (is_max) {
        return (a > b) ? a : b;
    }
    return (a < b) ? a : b;
}

static double wasm_f64_min_max(double a, double b, bool is_max)
{
    if (isnan(a) || isnan(b)) {
        return a + b;
    }
    if ((a == 0.0) && (b == 0.0)) {
        uint64_t ua = 0ULL;
        uint64_t ub = 0ULL;
        memcpy(&ua, &a, sizeof(ua));
        memcpy(&ub, &b, sizeof(ub));
        uint64_t ures = is_max ? (ua & ub) : (ua | ub);
        double res = 0.0;
        memcpy(&res, &ures, sizeof(res));
        return res;
    }
    if (is_max) {
        return (a > b) ? a : b;
    }
    return (a < b) ? a : b;
}
#endif

/* ── Interpreter Step Loop ───────────────────────────────────────────────── */

#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
static q16_t wasm_q16_trunc(q16_t qa)
{
    uint32_t u = (uint32_t)qa;
    uint32_t base = u & 0xFFFF0000U;
    if (qa < 0 && (u & 0xFFFFU) != 0U) {
        base += 0x00010000U;
    }
    return (q16_t)base;
}

static void wasm_exec_f32_fixed_unary(SYN_WASM_Context *ctx, uint8_t opcode)
{
    uint32_t a = 0U;
    if (!pop_stack(ctx, &a)) {
        return;
    }
    q16_t qa = (q16_t)a;
    q16_t qres = 0;
    switch (opcode) {
    case OP_F32_ABS:
        qres = q16_abs(qa);
        break;
    case OP_F32_NEG:
        qres = (qa == INT32_MIN) ? INT32_MAX : -qa;
        break;
    case OP_F32_CEIL:
        qres = q16_ceil(qa);
        break;
    case OP_F32_FLOOR:
        qres = q16_floor(qa);
        break;
    case OP_F32_TRUNC:
        qres = wasm_q16_trunc(qa);
        break;
    case OP_F32_NEAREST:
        qres = q16_round(qa);
        break;
    default:
        qres = q16_sqrt(qa);
        break;
    }
    (void)push_stack(ctx, (uint32_t)qres);
}

static void wasm_exec_f32_fixed_binary(SYN_WASM_Context *ctx, uint8_t opcode)
{
    uint32_t b = 0U;
    uint32_t a = 0U;
    if (!pop_stack(ctx, &b) || !pop_stack(ctx, &a)) {
        return;
    }
    q16_t qa = (q16_t)a;
    q16_t qb = (q16_t)b;
    q16_t qres = 0;
    switch (opcode) {
    case OP_F32_ADD:
        qres = q16_add(qa, qb);
        break;
    case OP_F32_SUB:
        qres = q16_sub(qa, qb);
        break;
    case OP_F32_MUL:
        qres = q16_mul(qa, qb);
        break;
    case OP_F32_DIV:
        if (qb == 0) {
            ctx->status = SYN_WASM_TRAP_DIV_ZERO;
            return;
        }
        qres = q16_div(qa, qb);
        break;
    case OP_F32_MIN:
        qres = (qa < qb) ? qa : qb;
        break;
    case OP_F32_MAX:
        qres = (qa > qb) ? qa : qb;
        break;
    default:
        qres = (qb < 0) ? -q16_abs(qa) : q16_abs(qa);
        break;
    }
    (void)push_stack(ctx, (uint32_t)qres);
}

static void wasm_exec_f32_fixed(SYN_WASM_Context *ctx, uint8_t opcode)
{
    switch (opcode) {
    case OP_F32_ABS:
    case OP_F32_NEG:
    case OP_F32_CEIL:
    case OP_F32_FLOOR:
    case OP_F32_TRUNC:
    case OP_F32_NEAREST:
    case OP_F32_SQRT:
        wasm_exec_f32_fixed_unary(ctx, opcode);
        break;
    default:
        wasm_exec_f32_fixed_binary(ctx, opcode);
        break;
    }
}
#endif

static SYN_WASM_Status wasm_pending_status(const SYN_WASM_Context *ctx)
{
    return ctx->host_waiting ? SYN_WASM_WAITING : SYN_WASM_YIELDED;
}

SYN_WASM_Status syn_wasm_step(SYN_WASM_Context *ctx, uint16_t max_instructions)
{
    if (!ctx || !ctx->module) {
        return SYN_WASM_TRAP_INVALID_MODULE;
    }

    /* LCOV_EXCL_START: Wasm status error guard */
    if (ctx->status != SYN_WASM_OK) {
        return ctx->status;
    }
    /* LCOV_EXCL_STOP */

    if (ctx->host_pending) {
        wasm_poll_host(ctx);
        if (ctx->host_pending) {
            return wasm_pending_status(ctx);
        }
        if (ctx->status != SYN_WASM_OK) {
            return ctx->status;
        }
    }

    const SYN_WASM_Module *mod = ctx->module;
    uint16_t executed = 0;

    while ((executed < max_instructions) && (ctx->status == SYN_WASM_OK) && !ctx->host_pending) {
        /* LCOV_EXCL_START: Wasm call depth zero halt check */
        if (ctx->call_depth == 0) {
            ctx->status = SYN_WASM_HALTED;
            break;
        }
        /* LCOV_EXCL_STOP */

        uint32_t code_end = wasm_frame_code_end(ctx);
        uint8_t opcode = 0U;
        if (!wasm_read_byte(ctx, code_end, &opcode)) {
            break;
        }
        executed++;

        switch (opcode) {
        /* LCOV_EXCL_START: Wasm unreachable opcode */
        case OP_UNREACHABLE:
            ctx->status = SYN_WASM_TRAP_UNREACHABLE;
            break;

        case OP_NOP:
            break;
            /* LCOV_EXCL_STOP */

        case OP_BLOCK:
        case OP_LOOP:
        case OP_IF:
            wasm_exec_block_if(ctx, opcode, code_end);
            break;

        case OP_ELSE:
            wasm_exec_else(ctx, code_end);
            break;

        case OP_END:
            if (ctx->label_depth > ctx->call_stack[ctx->call_depth - 1U].label_base) {
                ctx->label_depth--;
            } else {
                wasm_do_return(ctx);
            }
            break;

        case OP_BR: {
            uint32_t label_idx = 0U;
            if (wasm_read_u32_leb(ctx, code_end, &label_idx)) {
                wasm_branch_to_label(ctx, label_idx, code_end);
            }
            break;
        }

        case OP_BR_IF: {
            uint32_t label_idx = 0U;
            uint32_t cond = 0U;
            if (wasm_read_u32_leb(ctx, code_end, &label_idx) && pop_stack(ctx, &cond) &&
                cond != 0U) {
                wasm_branch_to_label(ctx, label_idx, code_end);
            }
            break;
        }

        case OP_BR_TABLE:
            wasm_exec_br_table(ctx, code_end);
            break;

        case OP_MEMORY_SIZE: {
            uint8_t mem_idx = 0U;
            if (wasm_read_byte(ctx, code_end, &mem_idx)) {
                if (mem_idx != 0U) {
                    ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                    break;
                }
                uint32_t pages = ctx->linear_mem_size / 65536U;
                (void)push_stack(ctx, pages);
            }
            break;
        }

        case OP_MEMORY_GROW:
            wasm_exec_mem_grow(ctx, code_end);
            break;

        case OP_RETURN:
            wasm_do_return(ctx);
            break;

        case OP_CALL: {
            uint32_t target_idx = 0U;
            if (wasm_read_u32_leb(ctx, code_end, &target_idx)) {
                wasm_invoke_func(ctx, target_idx);
            }
            break;
        }

        case OP_CALL_INDIRECT:
            wasm_exec_call_indirect(ctx, code_end);
            break;

        /* LCOV_EXCL_START: Wasm drop opcode */
        case OP_DROP: {
            uint32_t dummy = 0;
            pop_stack(ctx, &dummy);
            break;
        }
            /* LCOV_EXCL_STOP */

        case OP_SELECT: {
            uint32_t cond = 0U;
            uint64_t val2 = 0ULL;
            uint64_t val1 = 0ULL;
            if (pop_stack(ctx, &cond) && pop_stack64(ctx, &val2) && pop_stack64(ctx, &val1)) {
                (void)push_stack64(ctx, (cond != 0U) ? val1 : val2);
            }
            break;
        }

        case OP_LOCAL_GET: {
            uint32_t idx = 0U;
            if (!wasm_read_u32_leb(ctx, code_end, &idx)) {
                break;
            }
            uint16_t base = ctx->call_stack[ctx->call_depth - 1U].local_base;
            if ((uint64_t)base + (uint64_t)idx >= (uint64_t)ctx->local_count) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            (void)push_stack64(ctx, ctx->locals[base + (uint16_t)idx]);
            break;
        }

        case OP_LOCAL_SET: {
            uint32_t idx = 0U;
            uint64_t val = 0ULL;
            if (!wasm_read_u32_leb(ctx, code_end, &idx) || !pop_stack64(ctx, &val)) {
                break;
            }
            uint16_t base = ctx->call_stack[ctx->call_depth - 1U].local_base;
            if ((uint64_t)base + (uint64_t)idx >= (uint64_t)ctx->local_count) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            ctx->locals[base + (uint16_t)idx] = val;
            break;
        }

        case OP_LOCAL_TEE: {
            uint32_t idx = 0U;
            if (!wasm_read_u32_leb(ctx, code_end, &idx)) {
                break;
            }
            if (ctx->sp == 0U) {
                ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
                break;
            }
            uint16_t base = ctx->call_stack[ctx->call_depth - 1U].local_base;
            if ((uint64_t)base + (uint64_t)idx >= (uint64_t)ctx->local_count) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            ctx->locals[base + (uint16_t)idx] = ctx->stack[ctx->sp - 1U];
            break;
        }

        case OP_GLOBAL_GET: {
            uint32_t idx = 0U;
            if (!wasm_read_u32_leb(ctx, code_end, &idx)) {
                break;
            }
            if (idx >= SYN_WASM_MAX_GLOBALS) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            (void)push_stack64(ctx, ctx->globals[idx]);
            break;
        }

        case OP_GLOBAL_SET: {
            uint32_t idx = 0U;
            uint64_t val = 0ULL;
            if (!wasm_read_u32_leb(ctx, code_end, &idx) || !pop_stack64(ctx, &val)) {
                break;
            }
            if (idx >= SYN_WASM_MAX_GLOBALS) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            ctx->globals[idx] = val;
            break;
        }

        /* LCOV_EXCL_START: Wasm load and store opcodes */
        case OP_I32_LOAD:
        case OP_I32_LOAD8_S:
        case OP_I32_LOAD8_U:
        case OP_I32_LOAD16_S:
        case OP_I32_LOAD16_U: {
            uint32_t offset = 0U;
            uint32_t base_addr = 0U;
            if (!wasm_read_memarg(ctx, code_end, &offset) || !pop_stack(ctx, &base_addr)) {
                break;
            }
            uint32_t req_bytes = 4U;
            if (opcode == OP_I32_LOAD8_S || opcode == OP_I32_LOAD8_U) {
                req_bytes = 1U;
            } else if (opcode == OP_I32_LOAD16_S || opcode == OP_I32_LOAD16_U) {
                req_bytes = 2U;
            }
            uint64_t addr = 0ULL;
            if (!wasm_check_mem(ctx, base_addr, offset, req_bytes, &addr)) {
                break;
            }

            uint32_t val = 0U;
            if (opcode == OP_I32_LOAD) {
                val = ((uint32_t)ctx->linear_mem[addr]) |
                      ((uint32_t)ctx->linear_mem[addr + 1U] << 8U) |
                      ((uint32_t)ctx->linear_mem[addr + 2U] << 16U) |
                      ((uint32_t)ctx->linear_mem[addr + 3U] << 24U);
            } else if (opcode == OP_I32_LOAD8_U) {
                val = ctx->linear_mem[addr];
            } else if (opcode == OP_I32_LOAD8_S) {
                val = (uint32_t)(int32_t)(int8_t)ctx->linear_mem[addr];
            } else if (opcode == OP_I32_LOAD16_U) {
                val = ((uint32_t)ctx->linear_mem[addr]) |
                      ((uint32_t)ctx->linear_mem[addr + 1U] << 8U);
            } else if (opcode == OP_I32_LOAD16_S) {
                val = (uint32_t)(int32_t)(int16_t)(((uint32_t)ctx->linear_mem[addr]) |
                                                   ((uint32_t)ctx->linear_mem[addr + 1U] << 8U));
            }
            push_stack(ctx, val);
            break;
        }

        case OP_I64_LOAD:
        case OP_I64_LOAD8_S:
        case OP_I64_LOAD8_U:
        case OP_I64_LOAD16_S:
        case OP_I64_LOAD16_U:
        case OP_I64_LOAD32_S:
        case OP_I64_LOAD32_U: {
            uint32_t offset = 0U;
            uint32_t base_addr = 0U;
            if (!wasm_read_memarg(ctx, code_end, &offset) || !pop_stack(ctx, &base_addr)) {
                break;
            }
            uint32_t req_bytes = 8U;
            if (opcode == OP_I64_LOAD8_S || opcode == OP_I64_LOAD8_U) {
                req_bytes = 1U;
            } else if (opcode == OP_I64_LOAD16_S || opcode == OP_I64_LOAD16_U) {
                req_bytes = 2U;
            } else if (opcode == OP_I64_LOAD32_S || opcode == OP_I64_LOAD32_U) {
                req_bytes = 4U;
            }
            uint64_t addr = 0ULL;
            if (!wasm_check_mem(ctx, base_addr, offset, req_bytes, &addr)) {
                break;
            }

            uint64_t val = 0ULL;
            if (opcode == OP_I64_LOAD) {
                val = ((uint64_t)ctx->linear_mem[addr]) |
                      ((uint64_t)ctx->linear_mem[addr + 1U] << 8U) |
                      ((uint64_t)ctx->linear_mem[addr + 2U] << 16U) |
                      ((uint64_t)ctx->linear_mem[addr + 3U] << 24U) |
                      ((uint64_t)ctx->linear_mem[addr + 4U] << 32U) |
                      ((uint64_t)ctx->linear_mem[addr + 5U] << 40U) |
                      ((uint64_t)ctx->linear_mem[addr + 6U] << 48U) |
                      ((uint64_t)ctx->linear_mem[addr + 7U] << 56U);
            } else if (opcode == OP_I64_LOAD8_U) {
                val = ctx->linear_mem[addr];
            } else if (opcode == OP_I64_LOAD8_S) {
                val = (uint64_t)(int64_t)(int8_t)ctx->linear_mem[addr];
            } else if (opcode == OP_I64_LOAD16_U) {
                val = ((uint64_t)ctx->linear_mem[addr]) |
                      ((uint64_t)ctx->linear_mem[addr + 1U] << 8U);
            } else if (opcode == OP_I64_LOAD16_S) {
                val = (uint64_t)(int64_t)(int16_t)(((uint32_t)ctx->linear_mem[addr]) |
                                                   ((uint32_t)ctx->linear_mem[addr + 1U] << 8U));
            } else if (opcode == OP_I64_LOAD32_U) {
                val = ((uint64_t)ctx->linear_mem[addr]) |
                      ((uint64_t)ctx->linear_mem[addr + 1U] << 8U) |
                      ((uint64_t)ctx->linear_mem[addr + 2U] << 16U) |
                      ((uint64_t)ctx->linear_mem[addr + 3U] << 24U);
            } else if (opcode == OP_I64_LOAD32_S) {
                val = (uint64_t)(int64_t)(int32_t)(((uint32_t)ctx->linear_mem[addr]) |
                                                   ((uint32_t)ctx->linear_mem[addr + 1U] << 8U) |
                                                   ((uint32_t)ctx->linear_mem[addr + 2U] << 16U) |
                                                   ((uint32_t)ctx->linear_mem[addr + 3U] << 24U));
            }
            push_stack64(ctx, val);
            break;
        }

        case OP_I32_STORE:
        case OP_I32_STORE8:
        case OP_I32_STORE16: {
            uint32_t offset = 0U;
            uint32_t val = 0U;
            uint32_t base_addr = 0U;
            if (!wasm_read_memarg(ctx, code_end, &offset) || !pop_stack(ctx, &val) ||
                !pop_stack(ctx, &base_addr)) {
                break;
            }
            uint32_t req_bytes = 4U;
            if (opcode == OP_I32_STORE8) {
                req_bytes = 1U;
            } else if (opcode == OP_I32_STORE16) {
                req_bytes = 2U;
            }
            uint64_t addr = 0ULL;
            if (!wasm_check_mem(ctx, base_addr, offset, req_bytes, &addr)) {
                break;
            }

            if (opcode == OP_I32_STORE) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFFU);
                ctx->linear_mem[addr + 1U] = (uint8_t)((val >> 8U) & 0xFFU);
                ctx->linear_mem[addr + 2U] = (uint8_t)((val >> 16U) & 0xFFU);
                ctx->linear_mem[addr + 3U] = (uint8_t)((val >> 24U) & 0xFFU);
            } else if (opcode == OP_I32_STORE8) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFFU);
            } else if (opcode == OP_I32_STORE16) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFFU);
                ctx->linear_mem[addr + 1U] = (uint8_t)((val >> 8U) & 0xFFU);
            }
            break;
        }

        case OP_I64_STORE:
        case OP_I64_STORE8:
        case OP_I64_STORE16:
        case OP_I64_STORE32: {
            uint32_t offset = 0U;
            uint64_t val = 0ULL;
            uint32_t base_addr = 0U;
            if (!wasm_read_memarg(ctx, code_end, &offset) || !pop_stack64(ctx, &val) ||
                !pop_stack(ctx, &base_addr)) {
                break;
            }
            uint32_t req_bytes = 8U;
            if (opcode == OP_I64_STORE8) {
                req_bytes = 1U;
            } else if (opcode == OP_I64_STORE16) {
                req_bytes = 2U;
            } else if (opcode == OP_I64_STORE32) {
                req_bytes = 4U;
            }
            uint64_t addr = 0ULL;
            if (!wasm_check_mem(ctx, base_addr, offset, req_bytes, &addr)) {
                break;
            }

            if (opcode == OP_I64_STORE) {
                for (uint32_t i = 0U; i < 8U; i++) {
                    ctx->linear_mem[addr + i] = (uint8_t)((val >> (i * 8U)) & 0xFFU);
                }
            } else if (opcode == OP_I64_STORE8) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFFU);
            } else if (opcode == OP_I64_STORE16) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFFU);
                ctx->linear_mem[addr + 1U] = (uint8_t)((val >> 8U) & 0xFFU);
            } else if (opcode == OP_I64_STORE32) {
                for (uint32_t i = 0U; i < 4U; i++) {
                    ctx->linear_mem[addr + i] = (uint8_t)((val >> (i * 8U)) & 0xFFU);
                }
            }
            break;
        }
            /* LCOV_EXCL_STOP */

        case OP_I32_CONST: {
            int32_t val = 0;
            if (wasm_read_i32_leb(ctx, code_end, &val)) {
                push_stack(ctx, (uint32_t)val);
            }
            break;
        }

        case OP_I64_CONST: {
            int64_t val = 0;
            if (wasm_read_i64_leb(ctx, code_end, &val)) {
                push_stack64(ctx, (uint64_t)val);
            }
            break;
        }

        case OP_I32_EQZ: {
            uint32_t a = 0;
            if (pop_stack(ctx, &a)) {
                push_stack(ctx, (a == 0) ? 1 : 0);
            }
            break;
        }

        /* LCOV_EXCL_START: Wasm 64-bit eqz opcode */
        case OP_I64_EQZ: {
            uint64_t a = 0;
            if (pop_stack64(ctx, &a)) {
                push_stack(ctx, (a == 0) ? 1 : 0);
            }
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm 32-bit comparison opcodes */
        case OP_I32_EQ:
        case OP_I32_NE:
        case OP_I32_LT_S:
        case OP_I32_LT_U:
        case OP_I32_GT_S:
        case OP_I32_GT_U:
        case OP_I32_LE_S:
        case OP_I32_LE_U:
        case OP_I32_GE_S:
        case OP_I32_GE_U: {
            uint32_t b = 0, a = 0;
            if (!pop_stack(ctx, &b) || !pop_stack(ctx, &a))
                break;
            bool res = false;
            switch (opcode) {
            case OP_I32_EQ:
                res = (a == b);
                break;
            case OP_I32_NE:
                res = (a != b);
                break;
            case OP_I32_LT_S:
                res = ((int32_t)a < (int32_t)b);
                break;
            case OP_I32_LT_U:
                res = (a < b);
                break;
            case OP_I32_GT_S:
                res = ((int32_t)a > (int32_t)b);
                break;
            case OP_I32_GT_U:
                res = (a > b);
                break;
            case OP_I32_LE_S:
                res = ((int32_t)a <= (int32_t)b);
                break;
            case OP_I32_LE_U:
                res = (a <= b);
                break;
            case OP_I32_GE_S:
                res = ((int32_t)a >= (int32_t)b);
                break;
            case OP_I32_GE_U:
                res = (a >= b);
                break;
            }
            push_stack(ctx, res ? 1 : 0);
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm 64-bit comparison opcodes */
        case OP_I64_EQ:
        case OP_I64_NE:
        case OP_I64_LT_S:
        case OP_I64_LT_U:
        case OP_I64_GT_S:
        case OP_I64_GT_U:
        case OP_I64_LE_S:
        case OP_I64_LE_U:
        case OP_I64_GE_S:
        case OP_I64_GE_U: {
            uint64_t b = 0, a = 0;
            if (!pop_stack64(ctx, &b) || !pop_stack64(ctx, &a))
                break;
            bool res = false;
            switch (opcode) {
            case OP_I64_EQ:
                res = (a == b);
                break;
            case OP_I64_NE:
                res = (a != b);
                break;
            case OP_I64_LT_S:
                res = ((int64_t)a < (int64_t)b);
                break;
            case OP_I64_LT_U:
                res = (a < b);
                break;
            case OP_I64_GT_S:
                res = ((int64_t)a > (int64_t)b);
                break;
            case OP_I64_GT_U:
                res = (a > b);
                break;
            case OP_I64_LE_S:
                res = ((int64_t)a <= (int64_t)b);
                break;
            case OP_I64_LE_U:
                res = (a <= b);
                break;
            case OP_I64_GE_S:
                res = ((int64_t)a >= (int64_t)b);
                break;
            case OP_I64_GE_U:
                res = (a >= b);
                break;
            }
            push_stack(ctx, res ? 1 : 0);
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm bitwise opcodes */
        case OP_I32_CLZ:
        case OP_I32_CTZ:
        case OP_I32_POPCNT: {
            uint32_t a = 0;
            if (!pop_stack(ctx, &a))
                break;
            uint32_t res = 0;
            if (opcode == OP_I32_CLZ) {
                res = a ? (uint32_t)__builtin_clz(a) : 32;
            } else if (opcode == OP_I32_CTZ) {
                res = a ? (uint32_t)__builtin_ctz(a) : 32;
            } else if (opcode == OP_I32_POPCNT) {
                res = (uint32_t)__builtin_popcount(a);
            }
            push_stack(ctx, res);
            break;
        }

        case OP_I64_CLZ:
        case OP_I64_CTZ:
        case OP_I64_POPCNT: {
            uint64_t a = 0;
            if (!pop_stack64(ctx, &a))
                break;
            uint64_t res = 0;
            if (opcode == OP_I64_CLZ) {
                res = a ? (uint64_t)__builtin_clzll(a) : 64;
            } else if (opcode == OP_I64_CTZ) {
                res = a ? (uint64_t)__builtin_ctzll(a) : 64;
            } else if (opcode == OP_I64_POPCNT) {
                res = (uint64_t)__builtin_popcountll(a);
            }
            push_stack64(ctx, res);
            break;
        }

        case OP_I32_ROTL:
        case OP_I32_ROTR: {
            uint32_t b = 0, a = 0;
            if (!pop_stack(ctx, &b) || !pop_stack(ctx, &a))
                break;
            uint32_t r = b % 32;
            uint32_t res = 0;
            if (opcode == OP_I32_ROTL) {
                res = (a << r) | (a >> ((32 - r) % 32));
            } else {
                res = (a >> r) | (a << ((32 - r) % 32));
            }
            push_stack(ctx, res);
            break;
        }

        case OP_I64_ROTL:
        case OP_I64_ROTR: {
            uint64_t b = 0, a = 0;
            if (!pop_stack64(ctx, &b) || !pop_stack64(ctx, &a))
                break;
            uint64_t r = b % 64;
            uint64_t res = 0;
            if (opcode == OP_I64_ROTL) {
                res = (a << r) | (a >> ((64 - r) % 64));
            } else {
                res = (a >> r) | (a << ((64 - r) % 64));
            }
            push_stack64(ctx, res);
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm 64-bit arithmetic opcodes */
        case OP_I64_ADD:
        case OP_I64_SUB:
        case OP_I64_MUL:
        case OP_I64_DIV_S:
        case OP_I64_DIV_U:
        case OP_I64_REM_S:
        case OP_I64_REM_U:
        case OP_I64_AND:
        case OP_I64_OR:
        case OP_I64_XOR:
        case OP_I64_SHL:
        case OP_I64_SHR_S:
        case OP_I64_SHR_U: {
            uint64_t b = 0, a = 0;
            if (!pop_stack64(ctx, &b) || !pop_stack64(ctx, &a))
                break;
            uint64_t res = 0;
            switch (opcode) {
            case OP_I64_ADD:
                res = a + b;
                break;
            case OP_I64_SUB:
                res = a - b;
                break;
            case OP_I64_MUL:
                res = a * b;
                break;
            case OP_I64_DIV_S:
                if (b == 0ULL || ((int64_t)a == INT64_MIN && (int64_t)b == -1LL)) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = (uint64_t)((int64_t)a / (int64_t)b);
                break;
            case OP_I64_DIV_U:
                if (b == 0ULL) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = a / b;
                break;
            case OP_I64_REM_S:
                if (b == 0ULL) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                if ((int64_t)a == INT64_MIN && (int64_t)b == -1LL) {
                    res = 0ULL;
                    break;
                }
                res = (uint64_t)((int64_t)a % (int64_t)b);
                break;
            case OP_I64_REM_U:
                if (b == 0ULL) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = a % b;
                break;
            case OP_I64_AND:
                res = a & b;
                break;
            case OP_I64_OR:
                res = a | b;
                break;
            case OP_I64_XOR:
                res = a ^ b;
                break;
            case OP_I64_SHL:
                res = a << (b % 64ULL);
                break;
            case OP_I64_SHR_S:
                res = (uint64_t)((int64_t)a >> (b % 64ULL));
                break;
            case OP_I64_SHR_U:
                res = a >> (b % 64ULL);
                break;
            }
            if (ctx->status == SYN_WASM_OK) {
                push_stack64(ctx, res);
            }
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm 64-bit integer conversion opcodes */
        case OP_I32_WRAP_I64: {
            uint64_t a = 0;
            if (pop_stack64(ctx, &a)) {
                push_stack(ctx, (uint32_t)a);
            }
            break;
        }

        case OP_I64_EXTEND_I32_S: {
            uint32_t a = 0;
            if (pop_stack(ctx, &a)) {
                push_stack64(ctx, (uint64_t)(int64_t)(int32_t)a);
            }
            break;
        }

        case OP_I64_EXTEND_I32_U: {
            uint32_t a = 0;
            if (pop_stack(ctx, &a)) {
                push_stack64(ctx, (uint64_t)a);
            }
            break;
        }
            /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm 32-bit arithmetic opcodes */
        case OP_I32_ADD:
        case OP_I32_SUB:
        case OP_I32_MUL:
        case OP_I32_DIV_S:
        case OP_I32_DIV_U:
        case OP_I32_REM_S:
        case OP_I32_REM_U:
        case OP_I32_AND:
        case OP_I32_OR:
        case OP_I32_XOR:
        case OP_I32_SHL:
        case OP_I32_SHR_S:
        case OP_I32_SHR_U: {
            uint32_t b = 0, a = 0;
            if (!pop_stack(ctx, &b) || !pop_stack(ctx, &a))
                break;
            uint32_t res = 0;
            switch (opcode) {
            case OP_I32_ADD:
                res = a + b;
                break;
            case OP_I32_SUB:
                res = a - b;
                break;
            case OP_I32_MUL:
                res = a * b;
                break;
            case OP_I32_DIV_S:
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                if ((int32_t)a == (int32_t)0x80000000U && (int32_t)b == -1) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = (uint32_t)((int32_t)a / (int32_t)b);
                break;
            case OP_I32_DIV_U:
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = a / b;
                break;
            case OP_I32_REM_S:
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                if ((int32_t)a == (int32_t)0x80000000U && (int32_t)b == -1) {
                    res = 0;
                    break;
                }
                res = (uint32_t)((int32_t)a % (int32_t)b);
                break;
            case OP_I32_REM_U:
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = a % b;
                break;
            case OP_I32_AND:
                res = a & b;
                break;
            case OP_I32_OR:
                res = a | b;
                break;
            case OP_I32_XOR:
                res = a ^ b;
                break;
            case OP_I32_SHL:
                res = a << (b & 31);
                break;
            case OP_I32_SHR_S:
                res = (uint32_t)((int32_t)a >> (b & 31));
                break;
            case OP_I32_SHR_U:
                res = a >> (b & 31);
                break;
            }
            if (ctx->status == SYN_WASM_OK) {
                push_stack(ctx, res);
            }
            break;
        }
        /* LCOV_EXCL_STOP */

        /* ── Floating-Point & Bulk Memory Opcodes ───────────────────────── */
        case OP_F32_CONST: {
            if ((uint64_t)ctx->pc + 4ULL > (uint64_t)code_end) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            uint32_t val = (uint32_t)mod->bytes[ctx->pc] |
                           ((uint32_t)mod->bytes[ctx->pc + 1U] << 8U) |
                           ((uint32_t)mod->bytes[ctx->pc + 2U] << 16U) |
                           ((uint32_t)mod->bytes[ctx->pc + 3U] << 24U);
            ctx->pc += 4U;
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
            float fval;
            memcpy(&fval, &val, sizeof(float));
            q16_t qval = q16_from_float(fval);
            push_stack(ctx, (uint32_t)qval);
#else
            push_stack64(ctx, (uint64_t)val);
#endif
            break;
        }

        case OP_F64_CONST: {
            if ((uint64_t)ctx->pc + 8ULL > (uint64_t)code_end) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                break;
            }
            uint64_t val = 0ULL;
            for (uint32_t i = 0U; i < 8U; i++) {
                val |= ((uint64_t)mod->bytes[ctx->pc + i]) << (i * 8U);
            }
            ctx->pc += 8U;
            push_stack64(ctx, val);
            break;
        }

        /* LCOV_EXCL_START: Wasm floating-point & bulk memory opcode interpreter fallbacks */
        case OP_F32_LOAD:
        case OP_F64_LOAD: {
            uint32_t offset = 0U;
            uint32_t base_addr = 0U;
            if (!wasm_read_memarg(ctx, code_end, &offset) || !pop_stack(ctx, &base_addr)) {
                break;
            }
            uint32_t req = (opcode == OP_F32_LOAD) ? 4U : 8U;
            uint64_t addr = 0ULL;
            if (!wasm_check_mem(ctx, base_addr, offset, req, &addr)) {
                break;
            }
            uint64_t val = 0ULL;
            for (uint32_t i = 0U; i < req; i++) {
                val |= ((uint64_t)ctx->linear_mem[addr + i]) << (i * 8U);
            }
            push_stack64(ctx, val);
            break;
        }

        case OP_F32_STORE:
        case OP_F64_STORE: {
            uint32_t offset = 0U;
            uint64_t val = 0ULL;
            uint32_t base_addr = 0U;
            if (!wasm_read_memarg(ctx, code_end, &offset) || !pop_stack64(ctx, &val) ||
                !pop_stack(ctx, &base_addr)) {
                break;
            }
            uint32_t req = (opcode == OP_F32_STORE) ? 4U : 8U;
            uint64_t addr = 0ULL;
            if (!wasm_check_mem(ctx, base_addr, offset, req, &addr)) {
                break;
            }
            for (uint32_t i = 0U; i < req; i++) {
                ctx->linear_mem[addr + i] = (uint8_t)((val >> (i * 8U)) & 0xFFU);
            }
            break;
        }

        case OP_F32_ADD:
        case OP_F32_SUB:
        case OP_F32_MUL:
        case OP_F32_DIV:
        case OP_F32_ABS:
        case OP_F32_NEG:
        case OP_F32_CEIL:
        case OP_F32_FLOOR:
        case OP_F32_TRUNC:
        case OP_F32_NEAREST:
        case OP_F32_SQRT:
        case OP_F32_MIN:
        case OP_F32_MAX:
        case OP_F32_COPYSIGN: {
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
            wasm_exec_f32_fixed(ctx, opcode);
#else
            /* Standard Native F32 Floating-Point Mode */
            if ((opcode == OP_F32_ABS) || (opcode == OP_F32_NEG)) {
                uint32_t ua = 0U;
                if (pop_stack(ctx, &ua)) {
                    uint32_t ures =
                        (opcode == OP_F32_ABS) ? (ua & 0x7FFFFFFFU) : (ua ^ 0x80000000U);
                    (void)push_stack(ctx, ures);
                }
            } else if (opcode == OP_F32_COPYSIGN) {
                uint32_t ub = 0U;
                uint32_t ua = 0U;
                if (pop_stack(ctx, &ub) && pop_stack(ctx, &ua)) {
                    (void)push_stack(ctx, (ua & 0x7FFFFFFFU) | (ub & 0x80000000U));
                }
            } else if (opcode == OP_F32_CEIL || opcode == OP_F32_FLOOR || opcode == OP_F32_TRUNC ||
                       opcode == OP_F32_NEAREST || opcode == OP_F32_SQRT) {
                float fa = 0.0F;
                if (!pop_f32(ctx, &fa)) {
                    break;
                }
                float fres = 0.0F;
                if (opcode == OP_F32_NEAREST) {
                    fres = wasm_f32_nearest(fa);
                } else if (opcode == OP_F32_SQRT) {
                    fres = sqrtf(fa);
                } else if (isnan(fa)) {
                    fres = fa + fa;
                } else if (opcode == OP_F32_CEIL) {
                    fres = ceilf(fa);
                } else if (opcode == OP_F32_FLOOR) {
                    fres = floorf(fa);
                } else {
                    fres = truncf(fa);
                }
                (void)push_f32(ctx, fres);
            } else {
                float fb = 0.0F;
                float fa = 0.0F;
                if (!pop_f32(ctx, &fb) || !pop_f32(ctx, &fa)) {
                    break;
                }
                float fres = 0.0F;
                if (opcode == OP_F32_ADD) {
                    fres = fa + fb;
                } else if (opcode == OP_F32_SUB) {
                    fres = fa - fb;
                } else if (opcode == OP_F32_MUL) {
                    fres = fa * fb;
                } else if (opcode == OP_F32_DIV) {
                    fres = fa / fb;
                } else {
                    fres = wasm_f32_min_max(fa, fb, opcode == OP_F32_MAX);
                }
                (void)push_f32(ctx, fres);
            }
#endif
            break;
        }

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
        case OP_F64_ADD:
        case OP_F64_SUB:
        case OP_F64_MUL:
        case OP_F64_DIV:
        case OP_F64_ABS:
        case OP_F64_NEG:
        case OP_F64_CEIL:
        case OP_F64_FLOOR:
        case OP_F64_TRUNC:
        case OP_F64_NEAREST:
        case OP_F64_SQRT:
        case OP_F64_MIN:
        case OP_F64_MAX:
        case OP_F64_COPYSIGN: {
            if ((opcode == OP_F64_ABS) || (opcode == OP_F64_NEG)) {
                uint64_t ua = 0ULL;
                if (pop_stack64(ctx, &ua)) {
                    uint64_t ures = (opcode == OP_F64_ABS) ? (ua & 0x7FFFFFFFFFFFFFFFULL)
                                                           : (ua ^ 0x8000000000000000ULL);
                    (void)push_stack64(ctx, ures);
                }
            } else if (opcode == OP_F64_COPYSIGN) {
                uint64_t ub = 0ULL;
                uint64_t ua = 0ULL;
                if (pop_stack64(ctx, &ub) && pop_stack64(ctx, &ua)) {
                    (void)push_stack64(ctx,
                                       (ua & 0x7FFFFFFFFFFFFFFFULL) | (ub & 0x8000000000000000ULL));
                }
            } else if (opcode == OP_F64_CEIL || opcode == OP_F64_FLOOR || opcode == OP_F64_TRUNC ||
                       opcode == OP_F64_NEAREST || opcode == OP_F64_SQRT) {
                double da = 0.0;
                if (!pop_f64(ctx, &da)) {
                    break;
                }
                double dres = 0.0;
                if (opcode == OP_F64_NEAREST) {
                    dres = wasm_f64_nearest(da);
                } else if (opcode == OP_F64_SQRT) {
                    dres = sqrt(da);
                } else if (isnan(da)) {
                    dres = da + da;
                } else if (opcode == OP_F64_CEIL) {
                    dres = ceil(da);
                } else if (opcode == OP_F64_FLOOR) {
                    dres = floor(da);
                } else {
                    dres = trunc(da);
                }
                (void)push_f64(ctx, dres);
            } else {
                double db = 0.0;
                double da = 0.0;
                if (!pop_f64(ctx, &db) || !pop_f64(ctx, &da)) {
                    break;
                }
                double dres = 0.0;
                if (opcode == OP_F64_ADD) {
                    dres = da + db;
                } else if (opcode == OP_F64_SUB) {
                    dres = da - db;
                } else if (opcode == OP_F64_MUL) {
                    dres = da * db;
                } else if (opcode == OP_F64_DIV) {
                    dres = da / db;
                } else {
                    dres = wasm_f64_min_max(da, db, opcode == OP_F64_MAX);
                }
                (void)push_f64(ctx, dres);
            }
            break;
        }

        case OP_F32_EQ:
        case OP_F32_NE:
        case OP_F32_LT:
        case OP_F32_GT:
        case OP_F32_LE:
        case OP_F32_GE: {
            float fb = 0.0f, fa = 0.0f;
            if (!pop_f32(ctx, &fb) || !pop_f32(ctx, &fa))
                break;
            bool res = false;
            if (opcode == OP_F32_EQ)
                res = (fa == fb);
            else if (opcode == OP_F32_NE)
                res = (fa != fb);
            else if (opcode == OP_F32_LT)
                res = (fa < fb);
            else if (opcode == OP_F32_GT)
                res = (fa > fb);
            else if (opcode == OP_F32_LE)
                res = (fa <= fb);
            else if (opcode == OP_F32_GE)
                res = (fa >= fb);
            push_stack(ctx, res ? 1 : 0);
            break;
        }

        case OP_F64_EQ:
        case OP_F64_NE:
        case OP_F64_LT:
        case OP_F64_GT:
        case OP_F64_LE:
        case OP_F64_GE: {
            double db = 0.0, da = 0.0;
            if (!pop_f64(ctx, &db) || !pop_f64(ctx, &da))
                break;
            bool res = false;
            if (opcode == OP_F64_EQ)
                res = (da == db);
            else if (opcode == OP_F64_NE)
                res = (da != db);
            else if (opcode == OP_F64_LT)
                res = (da < db);
            else if (opcode == OP_F64_GT)
                res = (da > db);
            else if (opcode == OP_F64_LE)
                res = (da <= db);
            else if (opcode == OP_F64_GE)
                res = (da >= db);
            push_stack(ctx, res ? 1 : 0);
            break;
        }

        case OP_F32_CONVERT_I32_S:
        case OP_F32_CONVERT_I32_U: {
            uint32_t ia = 0;
            if (!pop_stack(ctx, &ia))
                break;
            float fres = (opcode == OP_F32_CONVERT_I32_S) ? (float)(int32_t)ia : (float)ia;
            push_f32(ctx, fres);
            break;
        }

        case OP_F32_CONVERT_I64_S:
        case OP_F32_CONVERT_I64_U: {
            uint64_t ia = 0;
            if (!pop_stack64(ctx, &ia))
                break;
            float fres = (opcode == OP_F32_CONVERT_I64_S) ? (float)(int64_t)ia : (float)ia;
            push_f32(ctx, fres);
            break;
        }

        case OP_F32_DEMOTE_F64: {
            double da = 0.0;
            if (pop_f64(ctx, &da))
                push_f32(ctx, (float)da);
            break;
        }

        case OP_F64_CONVERT_I32_S:
        case OP_F64_CONVERT_I32_U: {
            uint32_t ia = 0;
            if (!pop_stack(ctx, &ia))
                break;
            double dres = (opcode == OP_F64_CONVERT_I32_S) ? (double)(int32_t)ia : (double)ia;
            push_f64(ctx, dres);
            break;
        }

        case OP_F64_CONVERT_I64_S:
        case OP_F64_CONVERT_I64_U: {
            uint64_t ia = 0;
            if (!pop_stack64(ctx, &ia))
                break;
            double dres = (opcode == OP_F64_CONVERT_I64_S) ? (double)(int64_t)ia : (double)ia;
            push_f64(ctx, dres);
            break;
        }

        case OP_F64_PROMOTE_F32: {
            float fa = 0.0f;
            if (pop_f32(ctx, &fa))
                push_f64(ctx, (double)fa);
            break;
        }

        case OP_I32_REINTERPRET_F32: {
            float fa = 0.0f;
            if (pop_f32(ctx, &fa)) {
                uint32_t u32;
                memcpy(&u32, &fa, sizeof(u32));
                push_stack(ctx, u32);
            }
            break;
        }

        case OP_I64_REINTERPRET_F64: {
            double da = 0.0;
            if (pop_f64(ctx, &da)) {
                uint64_t u64;
                memcpy(&u64, &da, sizeof(u64));
                push_stack64(ctx, u64);
            }
            break;
        }

        case OP_F32_REINTERPRET_I32: {
            uint32_t u32 = 0;
            if (pop_stack(ctx, &u32)) {
                float fa;
                memcpy(&fa, &u32, sizeof(u32));
                push_f32(ctx, fa);
            }
            break;
        }

        case OP_F64_REINTERPRET_I64: {
            uint64_t u64 = 0;
            if (pop_stack64(ctx, &u64)) {
                double da;
                memcpy(&da, &u64, sizeof(da));
                push_f64(ctx, da);
            }
            break;
        }
#endif
        /* LCOV_EXCL_STOP */

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
        case OP_I32_TRUNC_F32_S:
        case OP_I32_TRUNC_F32_U:
        case OP_I32_TRUNC_F64_S:
        case OP_I32_TRUNC_F64_U:
        case OP_I64_TRUNC_F32_S:
        case OP_I64_TRUNC_F32_U:
        case OP_I64_TRUNC_F64_S:
        case OP_I64_TRUNC_F64_U:
            wasm_exec_trunc(ctx, opcode);
            break;
#endif

        case OP_I32_EXTEND8_S:
        case OP_I32_EXTEND16_S:
        case OP_I64_EXTEND8_S:
        case OP_I64_EXTEND16_S:
        case OP_I64_EXTEND32_S:
            wasm_exec_sign_ext(ctx, opcode);
            break;

        case OP_PREFIX_MISC:
            wasm_exec_misc(ctx, code_end);
            break;

        /* LCOV_EXCL_START: Default bad opcode trap */
        default:
            ctx->status = SYN_WASM_TRAP_BAD_OPCODE;
            break;
            /* LCOV_EXCL_STOP */
        }
    }

    if (ctx->host_pending) {
        return wasm_pending_status(ctx);
    }

    if (executed >= max_instructions && ctx->status == SYN_WASM_OK) {
        return SYN_WASM_YIELDED;
    }

    return ctx->status;
}

SYN_PT_Status syn_wasm_exec_pt(SYN_PT *pt, SYN_Task *task, SYN_WASM_Context *ctx,
                               uint16_t quantum_instructions)
{
    if ((pt == NULL) || (ctx == NULL) || (ctx->module == NULL)) {
        return PT_EXITED;
    }

    ctx->active_task = task;
    uint16_t step_limit =
        (quantum_instructions == 0U) ? SYN_WASM_DEFAULT_QUANTUM : quantum_instructions;

    PT_BEGIN(pt);

    while ((ctx->status == SYN_WASM_OK) && ((ctx->call_depth > 0U) || ctx->host_pending)) {
        SYN_WASM_Status st = syn_wasm_step(ctx, step_limit);
        if (st == SYN_WASM_WAITING) {
            PT_PASS(pt);
        } else if (st == SYN_WASM_YIELDED) {
            PT_CONTINUE(pt);
        }
    }

    ctx->active_task = NULL;
    if (ctx->status != SYN_WASM_HALTED) {
        PT_EXIT(pt);
    }

    PT_END(pt);
}

SYN_PT_Status syn_wasm_task(SYN_PT *pt, SYN_Task *task)
{
    if ((pt == NULL) || (task == NULL) || (task->user_data == NULL)) {
        return PT_EXITED;
    }

    SYN_WASM_Context *ctx = (SYN_WASM_Context *)task->user_data;
    ctx->active_task = task;

    PT_BEGIN(pt);

    while (ctx->status <= SYN_WASM_HALTED) {
        if ((ctx->status == SYN_WASM_OK) && ((ctx->call_depth > 0U) || ctx->host_pending)) {
            SYN_WASM_Status st = syn_wasm_step(ctx, SYN_WASM_DEFAULT_QUANTUM);
            if (st == SYN_WASM_WAITING) {
                PT_PASS(pt);
            } else if (st == SYN_WASM_YIELDED) {
                PT_CONTINUE_IF_NEEDED(pt, task);
            }
        } else {
            ctx->active_task = NULL;
            PT_PASS(pt);
        }
    }

    ctx->active_task = NULL;
    PT_END(pt);
}

SYN_PT_Status syn_wasm_host_millis(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                   uint8_t argc, uint64_t *ret)
{
    (void)ctx;
    (void)args;
    (void)argc;
    if ((pt == NULL) || (ret == NULL)) {
        return PT_EXITED;
    }
    PT_INIT(pt);
    *ret = (uint64_t)syn_port_get_tick_ms();
    return PT_ENDED;
}

SYN_PT_Status syn_wasm_host_micros(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                   uint8_t argc, uint64_t *ret)
{
    (void)ctx;
    (void)args;
    (void)argc;
    if ((pt == NULL) || (ret == NULL)) {
        return PT_EXITED;
    }
    PT_INIT(pt);
    *ret = (uint64_t)syn_port_get_tick_us();
    return PT_ENDED;
}

SYN_PT_Status syn_wasm_host_sleep_ms(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                     uint8_t argc, uint64_t *ret)
{
    if ((pt == NULL) || (ctx == NULL) || (argc < 1U) || (args == NULL) || (ret == NULL)) {
        return PT_EXITED;
    }

    uint32_t raw_ms = (uint32_t)args[0];
    if ((raw_ms == 0U) || (raw_ms > 0x7FFFFFFFU)) {
        PT_INIT(pt);
        *ret = 0ULL;
        return PT_ENDED;
    }

    PT_BEGIN(pt);

    if (ctx->active_task != NULL) {
        PT_TASK_DELAY_MS(pt, ctx->active_task, raw_ms);
    } else {
        PT_DELAY_MS(pt, &ctx->host_scratch.u32[0], raw_ms);
    }

    *ret = 0ULL;
    PT_END(pt);
}

SYN_PT_Status syn_wasm_host_yield(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                  uint8_t argc, uint64_t *ret)
{
    (void)ctx;
    (void)args;
    (void)argc;
    if ((pt == NULL) || (ret == NULL)) {
        return PT_EXITED;
    }

    PT_BEGIN(pt);
    PT_PASS(pt);
    *ret = 0ULL;
    PT_END(pt);
}

```


