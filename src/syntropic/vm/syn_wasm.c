/**
 * @file syn_wasm.c
 * @brief Implementation of 32-bit WebAssembly (Wasm MVP) cooperative interpreter.
 */

#include "syn_wasm.h"

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
#include <math.h>
#endif
#include <string.h>

#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
#include "syntropic/util/syn_qmath.h"
#endif

/** @cond INTERNAL */

/* Wasm Magic Header & Version */
#define WASM_MAGIC 0x6D736100U /* "\0asm" */
#define WASM_VERSION 0x00000001U

/* Value Types */
#define WASM_TYPE_I32 0x7FU
#define WASM_TYPE_I64 0x7EU
#define WASM_TYPE_F32 0x7DU
#define WASM_TYPE_F64 0x7CU

/* Section IDs */
#define WASM_SEC_TYPE 1
#define WASM_SEC_IMPORT 2
#define WASM_SEC_FUNCTION 3
#define WASM_SEC_GLOBAL 6
#define WASM_SEC_EXPORT 7
#define WASM_SEC_START 8
#define WASM_SEC_ELEMENT 9
#define WASM_SEC_CODE 10
#define WASM_SEC_DATA 11

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

#define OP_I32_REINTERPRET_F32 0xBC
#define OP_I64_REINTERPRET_F64 0xBD
#define OP_F32_REINTERPRET_I32 0xBE
#define OP_F64_REINTERPRET_I64 0xBF

#define OP_PREFIX_MISC 0xFC

/* ── LEB128 Decoding Helpers ───────────────────────────────────────────── */

static uint32_t read_u32_leb128(const uint8_t *bytes, uint32_t max_size, uint32_t *offset)
{
    uint32_t result = 0;
    uint32_t shift = 0;
    uint32_t cur = *offset;

    while (cur < max_size) {
        uint8_t byte = bytes[cur++];
        result |= (uint32_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            break;
        }
        shift += 7;
        /* LCOV_EXCL_START: LEB128 decoding maximum length safety check */
        if (shift >= 35) {
            break;
        }
        /* LCOV_EXCL_STOP */
    }

    *offset = cur;
    return result;
}

static int32_t read_i32_leb128(const uint8_t *bytes, uint32_t max_size, uint32_t *offset)
{
    int32_t result = 0;
    uint32_t shift = 0;
    uint32_t cur = *offset;
    uint8_t byte = 0;

    while (cur < max_size) {
        byte = bytes[cur++];
        result |= (int32_t)((uint32_t)(byte & 0x7FU) << shift);
        shift += 7;
        if ((byte & 0x80) == 0) {
            break;
        }
        /* LCOV_EXCL_START: Signed LEB128 decoding maximum length safety check */
        if (shift >= 35) {
            break;
        }
        /* LCOV_EXCL_STOP */
    }

    if ((shift < 32) && (byte & 0x40)) {
        result |= (int32_t)(~0U << shift);
    }

    *offset = cur;
    return result;
}

static int64_t read_i64_leb128(const uint8_t *bytes, uint32_t max_size, uint32_t *offset)
{
    int64_t result = 0;
    uint32_t shift = 0;
    uint32_t cur = *offset;
    uint8_t byte = 0;

    while (cur < max_size) {
        byte = bytes[cur++];
        result |= (int64_t)(((uint64_t)(byte & 0x7FU)) << shift);
        shift += 7;
        if ((byte & 0x80) == 0) {
            break;
        }
        /* LCOV_EXCL_START: 64-bit LEB128 decoding maximum length safety check */
        if (shift >= 70) {
            break;
        }
        /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START: Negative LEB128 sign extension */
    if ((shift < 64) && (byte & 0x40)) {
        result |= (int64_t)(~0ULL << shift);
    }
    /* LCOV_EXCL_STOP */

    *offset = cur;
    return result;
}

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
        if (shift == 28U && (byte & 0x80U) != 0U) {
            ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
            return false;
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
        if (shift == 63U && (byte & 0x80U) != 0U) {
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

/* ── Module Parser ──────────────────────────────────────────────────────── */

#define WASM_MAX_FUNC_PARAMS ((uint32_t)SYN_WASM_MAX_STACK)
#define WASM_MAX_FUNC_RESULTS ((uint32_t)SYN_WASM_MAX_STACK)
#define WASM_MAX_TABLE_ELEMS 64U

/** @brief Parsed Wasm function signature arity entry */
typedef struct {
    uint8_t param_count;  /**< Number of function parameters */
    uint8_t result_count; /**< Number of function results */
} WASM_ParsedType;

/** @brief Transient cross-section parser state during syn_wasm_module_load */
typedef struct {
    WASM_ParsedType parsed_types[SYN_WASM_MAX_FUNCTIONS]; /**< Parsed function signatures */
    uint32_t type_func_indices[SYN_WASM_MAX_FUNCTIONS];   /**< Declared function type indices */
    uint16_t type_count;                                  /**< Count of parsed type entries */
    uint16_t func_decl_count;                             /**< Count of declared internal funcs */
} WASM_LoadState;

/**
 * @brief Read and strictly validate an unsigned 32-bit LEB128 integer within section bounds.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of current section.
 * @param offset In/out cursor offset.
 * @param out Decoded uint32_t output.
 * @return true if a complete, non-overflowing u32 LEB128 was decoded, false otherwise.
 */
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

/**
 * @brief Read and strictly validate a signed 32-bit LEB128 integer within section bounds.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of current section.
 * @param offset In/out cursor offset.
 * @param out Decoded int32_t output.
 * @return true if a complete, canonical-range s32 LEB128 was decoded, false otherwise.
 */
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

/**
 * @brief Parse and bounds-check Wasm Type section (ID 1).
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state to populate parsed function signatures.
 * @return true if valid, false on malformed or out-of-bounds type entries.
 */
static bool wasm_parse_type_section(const uint8_t *bytes, uint32_t section_end, uint32_t *offset,
                                    WASM_LoadState *st)
{
    uint32_t num_types = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_types) ||
        (num_types > SYN_WASM_MAX_FUNCTIONS)) {
        return false;
    }
    for (uint32_t i = 0U; i < num_types; i++) {
        if ((*offset >= section_end) || (bytes[(*offset)++] != 0x60U)) {
            return false;
        }
        uint32_t num_params = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &num_params) ||
            (num_params > WASM_MAX_FUNC_PARAMS) ||
            (((uint64_t)(*offset) + (uint64_t)num_params) > (uint64_t)section_end)) {
            return false;
        }
        *offset += num_params;

        uint32_t num_results = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &num_results) ||
            (num_results > WASM_MAX_FUNC_RESULTS) ||
            (((uint64_t)(*offset) + (uint64_t)num_results) > (uint64_t)section_end)) {
            return false;
        }
        *offset += num_results;

        st->parsed_types[st->type_count].param_count = (uint8_t)num_params;
        st->parsed_types[st->type_count].result_count = (uint8_t)num_results;
        st->type_count++;
    }
    return true;
}

/**
 * @brief Parse Wasm resizable limits (flags in {0, 1}, min, optional max >= min).
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @return true if limits are valid and within section bounds, false otherwise.
 */
static bool wasm_parse_limits(const uint8_t *bytes, uint32_t section_end, uint32_t *offset)
{
    if (*offset >= section_end) {
        return false;
    }
    uint8_t flags = bytes[(*offset)++];
    uint32_t min_val = 0U;
    if ((flags > 1U) || !wasm_sec_read_u32(bytes, section_end, offset, &min_val)) {
        return false;
    }
    if ((flags & 1U) != 0U) {
        uint32_t max_val = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &max_val) || (max_val < min_val)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Parse and validate a single Wasm import descriptor after module/field names.
 * @param mod Module structure to populate for function imports.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param kind Import descriptor kind (0=func, 1=table, 2=memory, 3=global).
 * @param st Loader state holding parsed types.
 * @return true if descriptor is within bounds and valid, false otherwise.
 */
static bool wasm_parse_import_desc(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t section_end,
                                   uint32_t *offset, uint8_t kind, const WASM_LoadState *st)
{
    if (kind == 0U) {
        uint32_t type_idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &type_idx) ||
            (type_idx >= (uint32_t)st->type_count) || (mod->func_count >= SYN_WASM_MAX_FUNCTIONS)) {
            return false;
        }
        mod->funcs[mod->func_count].type_idx = type_idx;
        mod->funcs[mod->func_count].code_offset = 0U;
        mod->funcs[mod->func_count].code_size = 0U;
        mod->funcs[mod->func_count].param_count = st->parsed_types[type_idx].param_count;
        mod->funcs[mod->func_count].result_count = st->parsed_types[type_idx].result_count;
        mod->func_count++;
        mod->import_func_count++;
        return true;
    }
    if (kind == 1U) {
        if ((*offset >= section_end) || (bytes[(*offset)++] != 0x70U)) {
            return false;
        }
        return wasm_parse_limits(bytes, section_end, offset);
    }
    if (kind == 2U) {
        return wasm_parse_limits(bytes, section_end, offset);
    }
    if (kind == 3U) {
        if (((uint64_t)(*offset) + 2ULL) > (uint64_t)section_end) {
            return false;
        }
        *offset += 2U;
        return true;
    }
    return false;
}

/**
 * @brief Parse and bounds-check Wasm Import section (ID 2).
 * @param mod Module structure to populate.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state holding parsed types.
 * @return true if valid, false on truncated or invalid import entries.
 */
static bool wasm_parse_import_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                      uint32_t section_end, uint32_t *offset,
                                      const WASM_LoadState *st)
{
    uint32_t num_imports = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_imports)) {
        return false;
    }
    for (uint32_t i = 0U; i < num_imports; i++) {
        uint32_t mod_len = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &mod_len) ||
            (uint64_t)(*offset) + (uint64_t)mod_len > (uint64_t)section_end) {
            return false;
        }
        *offset += mod_len;

        uint32_t field_len = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &field_len) ||
            (uint64_t)(*offset) + (uint64_t)field_len >= (uint64_t)section_end) {
            return false;
        }
        *offset += field_len;

        uint8_t kind = bytes[(*offset)++];
        if (!wasm_parse_import_desc(mod, bytes, section_end, offset, kind, st)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Parse and bounds-check Wasm Function section (ID 3).
 * @param mod Module structure holding imported function count.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state to record declared function type indices.
 * @return true if all declared functions reference valid types and fit capacity, false otherwise.
 */
static bool wasm_parse_func_section(const SYN_WASM_Module *mod, const uint8_t *bytes,
                                    uint32_t section_end, uint32_t *offset, WASM_LoadState *st)
{
    uint32_t num_funcs = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_funcs) ||
        (((uint64_t)mod->import_func_count + (uint64_t)num_funcs) >
         (uint64_t)SYN_WASM_MAX_FUNCTIONS)) {
        return false;
    }
    for (uint32_t i = 0U; i < num_funcs; i++) {
        uint32_t type_idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &type_idx) ||
            (type_idx >= (uint32_t)st->type_count)) {
            return false;
        }
        st->type_func_indices[st->func_decl_count++] = type_idx;
    }
    return true;
}

/**
 * @brief Parse and bounds-check Wasm Export section (ID 7).
 * @param mod Module structure to populate function exports.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state holding declared function count.
 * @return true if valid, false on truncated or invalid export entries.
 */
static bool wasm_parse_export_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                      uint32_t section_end, uint32_t *offset,
                                      const WASM_LoadState *st)
{
    uint32_t num_exports = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_exports)) {
        return false;
    }
    uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;
    for (uint32_t i = 0U; i < num_exports; i++) {
        uint32_t name_len = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &name_len) ||
            (uint64_t)(*offset) + (uint64_t)name_len >= (uint64_t)section_end) {
            return false;
        }
        uint32_t name_offset = *offset;
        *offset += name_len;
        uint8_t kind = bytes[(*offset)++];
        uint32_t idx = 0U;
        if (kind > 3U || !wasm_sec_read_u32(bytes, section_end, offset, &idx)) {
            return false;
        }
        if (kind == 0U) {
            if (idx >= total_funcs) {
                return false;
            }
            if (mod->export_count < SYN_WASM_MAX_FUNCTIONS) {
                mod->exports[mod->export_count].name_offset = name_offset;
                mod->exports[mod->export_count].name_len = (uint16_t)name_len;
                mod->exports[mod->export_count].func_idx = (uint16_t)idx;
                mod->export_count++;
            }
        }
    }
    return true;
}

/**
 * @brief Parse active segment header (flags == 0) and constant i32 offset expression.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param out_val Decoded signed 32-bit constant offset.
 * @return true if flags == 0 and OP_I32_CONST + s32 + OP_END parsed cleanly, false otherwise.
 */
static bool wasm_read_active_i32_expr(const uint8_t *bytes, uint32_t section_end, uint32_t *offset,
                                      int32_t *out_val)
{
    uint32_t flags = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &flags) || (flags != 0U)) {
        return false;
    }
    if ((*offset >= section_end) || (bytes[(*offset)++] != OP_I32_CONST)) {
        return false;
    }
    if (!wasm_sec_read_i32(bytes, section_end, offset, out_val)) {
        return false;
    }
    if ((*offset >= section_end) || (bytes[(*offset)++] != OP_END)) {
        return false;
    }
    return true;
}

/**
 * @brief Parse active Wasm Element segment header and constant non-negative table offset.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param out_offset Decoded non-negative element table base offset.
 * @return true if flags == 0 and OP_I32_CONST >= 0 + OP_END parsed, false otherwise.
 */
static bool wasm_read_elem_offset(const uint8_t *bytes, uint32_t section_end, uint32_t *offset,
                                  uint32_t *out_offset)
{
    int32_t elem_offset = 0;
    if (!wasm_read_active_i32_expr(bytes, section_end, offset, &elem_offset) || (elem_offset < 0)) {
        return false;
    }
    *out_offset = (uint32_t)elem_offset;
    return true;
}

/**
 * @brief Parse and bounds-check a single active Wasm Element segment.
 * @param mod Module structure to populate indirect function table elements.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param max_func_idx Exclusive upper bound on valid function indices.
 * @return true if segment is valid and within table/function bounds, false otherwise.
 */
static bool wasm_parse_elem_segment(SYN_WASM_Module *mod, const uint8_t *bytes,
                                    uint32_t section_end, uint32_t *offset, uint32_t max_func_idx)
{
    uint32_t base_offset = 0U;
    if (!wasm_read_elem_offset(bytes, section_end, offset, &base_offset)) {
        return false;
    }
    uint32_t num_funcs = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_funcs) ||
        (((uint64_t)base_offset + (uint64_t)num_funcs) > (uint64_t)WASM_MAX_TABLE_ELEMS)) {
        return false;
    }
    for (uint32_t f = 0U; f < num_funcs; f++) {
        uint32_t f_idx = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &f_idx) || (f_idx >= max_func_idx)) {
            return false;
        }
        uint32_t dest = base_offset + f;
        mod->table_elements[dest] = (uint16_t)f_idx;
        if ((dest + 1U) > (uint32_t)mod->table_element_count) {
            mod->table_element_count = (uint16_t)(dest + 1U);
        }
    }
    return true;
}

/**
 * @brief Parse and bounds-check Wasm Element section (ID 9).
 * @param mod Module structure to populate indirect function table elements.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state holding declared function count.
 * @return true if valid, false on invalid offset, opcode, table overflow, or function index.
 */
static bool wasm_parse_elem_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                    uint32_t section_end, uint32_t *offset,
                                    const WASM_LoadState *st)
{
    uint32_t num_segments = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_segments)) {
        return false;
    }
    uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;
    for (uint32_t s = 0U; s < num_segments; s++) {
        if (!wasm_parse_elem_segment(mod, bytes, section_end, offset, total_funcs)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Parse and bounds-check Wasm Start section (ID 8).
 * @param mod Module structure to populate start function index.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state holding declared function count.
 * @return true if single valid start function index with zero params/results, false otherwise.
 */
static bool wasm_parse_start_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                     uint32_t section_end, uint32_t *offset,
                                     const WASM_LoadState *st)
{
    uint32_t total_funcs = (uint32_t)mod->import_func_count + (uint32_t)st->func_decl_count;
    if (mod->has_start_func ||
        !wasm_sec_read_u32(bytes, section_end, offset, &mod->start_func_idx) ||
        (mod->start_func_idx >= total_funcs) || (*offset != section_end)) {
        return false;
    }
    mod->has_start_func = true;
    return true;
}

/**
 * @brief Parse and bounds-check Wasm Code section (ID 10).
 * @param mod Module structure to populate function code offsets and sizes.
 * @param bytes Module byte buffer.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset.
 * @param st Loader state holding declared function type indices.
 * @return true if all function bodies are within bounds, false otherwise.
 */
static bool wasm_parse_code_section(SYN_WASM_Module *mod, const uint8_t *bytes,
                                    uint32_t section_end, uint32_t *offset,
                                    const WASM_LoadState *st)
{
    uint32_t num_bodies = 0U;
    if (!wasm_sec_read_u32(bytes, section_end, offset, &num_bodies) ||
        (num_bodies != (uint32_t)st->func_decl_count)) {
        return false;
    }
    for (uint32_t i = 0U; i < num_bodies; i++) {
        uint32_t body_size = 0U;
        if (!wasm_sec_read_u32(bytes, section_end, offset, &body_size) || (body_size == 0U)) {
            return false;
        }
        uint32_t body_start = *offset;
        if (((uint64_t)body_start + (uint64_t)body_size) > (uint64_t)section_end) {
            return false;
        }
        uint32_t t_idx = st->type_func_indices[i];
        mod->funcs[mod->func_count].type_idx = t_idx;
        mod->funcs[mod->func_count].code_offset = body_start;
        mod->funcs[mod->func_count].code_size = body_size;
        mod->funcs[mod->func_count].param_count = st->parsed_types[t_idx].param_count;
        mod->funcs[mod->func_count].result_count = st->parsed_types[t_idx].result_count;
        mod->func_count++;
        *offset = body_start + body_size;
    }
    return true;
}

/**
 * @brief Dispatch parsing for a single bounded Wasm binary section.
 * @param mod Module structure to populate.
 * @param section_id Wasm binary section identifier.
 * @param section_end Exclusive end offset of section.
 * @param offset In/out cursor offset positioned at section payload start.
 * @param st Loader state across sections.
 * @return true if section parsed cleanly or skipped as unknown/optional, false on malformed data.
 */
static bool wasm_dispatch_load_section(SYN_WASM_Module *mod, uint8_t section_id,
                                       uint32_t section_end, uint32_t *offset, WASM_LoadState *st)
{
    switch (section_id) {
    case WASM_SEC_TYPE:
        return wasm_parse_type_section(mod->bytes, section_end, offset, st);
    case WASM_SEC_IMPORT:
        return wasm_parse_import_section(mod, mod->bytes, section_end, offset, st);
    case WASM_SEC_FUNCTION:
        return wasm_parse_func_section(mod, mod->bytes, section_end, offset, st);
    case WASM_SEC_EXPORT:
        return wasm_parse_export_section(mod, mod->bytes, section_end, offset, st);
    case WASM_SEC_START:
        return wasm_parse_start_section(mod, mod->bytes, section_end, offset, st);
    case WASM_SEC_ELEMENT:
        return wasm_parse_elem_section(mod, mod->bytes, section_end, offset, st);
    case WASM_SEC_CODE:
        return wasm_parse_code_section(mod, mod->bytes, section_end, offset, st);
    default:
        return true;
    }
}

bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size)
{
    if (mod == NULL || bytes == NULL || size < 8U) {
        return false;
    }

    memset(mod, 0, sizeof(*mod));
    mod->bytes = bytes;
    mod->size = size;

    /* Verify Magic Header & Version */
    uint32_t magic = ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8U) |
                     ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
    uint32_t version = ((uint32_t)bytes[4]) | ((uint32_t)bytes[5] << 8U) |
                       ((uint32_t)bytes[6] << 16U) | ((uint32_t)bytes[7] << 24U);

    if (magic != WASM_MAGIC || version != WASM_VERSION) {
        return false;
    }

    WASM_LoadState st;
    memset(&st, 0, sizeof(st));

    uint32_t offset = 8U;
    while (offset < size) {
        uint8_t section_id = bytes[offset++];
        uint32_t section_len = 0U;
        if (!wasm_sec_read_u32(bytes, size, &offset, &section_len) ||
            (uint64_t)offset + (uint64_t)section_len > (uint64_t)size) {
            return false;
        }
        uint32_t section_end = offset + section_len;
        if (!wasm_dispatch_load_section(mod, section_id, section_end, &offset, &st)) {
            return false;
        }
        offset = section_end;
    }

    return true;
}

/* ── Context & Execution ─────────────────────────────────────────────────── */

static bool wasm_load_data_section(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod,
                                   uint32_t section_end, uint32_t *offset)
{
    uint32_t num_segments = 0U;
    if (!wasm_sec_read_u32(mod->bytes, section_end, offset, &num_segments)) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return false;
    }
    for (uint32_t s = 0U; s < num_segments; s++) {
        int32_t data_offset = 0;
        uint32_t data_size = 0U;
        if (!wasm_read_active_i32_expr(mod->bytes, section_end, offset, &data_offset) ||
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
        *offset += data_size;
    }
    return true;
}

bool syn_wasm_init(SYN_WASM_Context *ctx, const SYN_WASM_Module *mod, uint8_t *linear_mem,
                   uint32_t mem_size)
{
    if (!ctx || !mod) {
        return false;
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->module = mod;
    ctx->linear_mem = linear_mem;
    ctx->linear_mem_size = mem_size;
    ctx->local_count = SYN_WASM_MAX_LOCALS;
    ctx->status = SYN_WASM_OK;

    /* Parse and populate Global (Section 6) and Data (Section 11) */
    static const uint8_t wasm_hdr[8] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U};
    if (mod->bytes != NULL && mod->size > 8U &&
        memcmp(mod->bytes, wasm_hdr, sizeof(wasm_hdr)) == 0) {
        uint32_t offset = 8U;
        while (offset < mod->size) {
            uint8_t section_id = mod->bytes[offset++];
            uint32_t section_size = read_u32_leb128(mod->bytes, mod->size, &offset);
            if ((uint64_t)offset + (uint64_t)section_size > (uint64_t)mod->size) {
                ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                return false;
            }
            uint32_t section_end = offset + section_size;

            if (section_id == WASM_SEC_GLOBAL) {
                uint32_t num_globals = read_u32_leb128(mod->bytes, section_end, &offset);
                for (uint32_t g = 0; g < num_globals && offset < section_end; g++) {
                    if ((uint64_t)offset + 2ULL > (uint64_t)section_end) {
                        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
                        return false;
                    }
                    offset += 2U; /* Type + Mutability */
                    int64_t init_val = 0;
                    if (offset < section_end) {
                        uint8_t op = mod->bytes[offset++];
                        if (op == OP_I32_CONST) {
                            init_val = read_i32_leb128(mod->bytes, section_end, &offset);
                            /* LCOV_EXCL_START: Wasm i64 global initializer */
                        } else if (op == OP_I64_CONST) {
                            init_val = read_i64_leb128(mod->bytes, section_end, &offset);
                        }
                        /* LCOV_EXCL_STOP */
                        if (offset < section_end && mod->bytes[offset] == OP_END) {
                            offset++;
                        }
                    }
                    if (g < SYN_WASM_MAX_GLOBALS) {
                        ctx->globals[g] = (uint64_t)init_val;
                        ctx->global_count = (uint16_t)(g + 1U);
                    }
                }
            } else if (section_id == WASM_SEC_DATA) {
                if (!wasm_load_data_section(ctx, mod, section_end, &offset)) {
                    return false;
                }
            }

            offset = section_end;
        }
    }

    return true;
}

bool syn_wasm_register_host(SYN_WASM_Context *ctx, uint16_t import_index, SYN_WASM_HostFunc func)
{
    if (!ctx || import_index >= SYN_WASM_MAX_HOST_FUNCS) {
        return false;
    }

    ctx->host_funcs[import_index] = func;
    if (import_index >= ctx->host_func_count) {
        ctx->host_func_count = import_index + 1;
    }
    return true;
}

int32_t syn_wasm_find_export(const SYN_WASM_Module *mod, const char *name)
{
    if (!mod || !name) {
        return -1;
    }

    size_t len = strlen(name);
    for (uint16_t i = 0; i < mod->export_count; i++) {
        if (mod->exports[i].name_len == len &&
            memcmp(mod->bytes + mod->exports[i].name_offset, name, len) == 0) {
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

static void wasm_invoke_func(SYN_WASM_Context *ctx, uint32_t target_idx)
{
    const SYN_WASM_Module *mod = ctx->module;
    if (target_idx >= mod->func_count) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    if (target_idx < mod->import_func_count) {
        if (target_idx >= ctx->host_func_count || ctx->host_funcs[target_idx] == NULL) {
            ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
            return;
        }
        uint8_t argc = mod->funcs[target_idx].param_count;
        if (ctx->sp < (uint32_t)argc) {
            ctx->status = SYN_WASM_TRAP_STACK_UNDERFLOW;
            return;
        }
        ctx->sp -= (uint32_t)argc;
        const uint64_t *args_ptr = (argc > 0U) ? &ctx->stack[ctx->sp] : NULL;
        uint64_t ret = ctx->host_funcs[target_idx](ctx, args_ptr, argc);
        if (ctx->status == SYN_WASM_OK && mod->funcs[target_idx].result_count > 0U) {
            (void)push_stack64(ctx, ret);
        }
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
    if (ctx == NULL || ctx->module == NULL || func_index >= ctx->module->func_count) {
        return false;
    }

    ctx->call_depth = 0U;
    ctx->label_depth = 0U;
    ctx->local_count = 0U;
    ctx->status = SYN_WASM_OK;

    if (func_index < ctx->module->import_func_count) {
        wasm_invoke_func(ctx, (uint32_t)func_index);
        if (ctx->status != SYN_WASM_OK) {
            return false;
        }
        ctx->status = SYN_WASM_HALTED;
        return true;
    }

    return wasm_push_frame(ctx, func_index, true);
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
    uint8_t label_base = ctx->call_stack[ctx->call_depth - 1U].label_base;
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
    uint8_t label_base = ctx->call_stack[ctx->call_depth - 1U].label_base;
    uint32_t active_labels = (uint32_t)(ctx->label_depth - label_base);
    if (label_idx == active_labels) {
        wasm_do_return(ctx);
        return;
    }
    if (label_idx > active_labels) {
        ctx->status = SYN_WASM_TRAP_INVALID_MODULE;
        return;
    }
    uint8_t target_depth = (uint8_t)(ctx->label_depth - 1U - label_idx);
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
        ctx->label_depth = (uint8_t)(target_depth + 1U);
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

static void wasm_exec_bulk_mem(SYN_WASM_Context *ctx, uint32_t subop, uint32_t code_end)
{
    uint32_t imm0 = 0U;
    uint32_t imm1 = 0U;
    if (subop == 8U) {
        if (!wasm_read_u32_leb(ctx, code_end, &imm0) || !wasm_read_u32_leb(ctx, code_end, &imm1)) {
            return;
        }
        uint32_t n = 0U;
        uint32_t src = 0U;
        uint32_t dst = 0U;
        (void)(pop_stack(ctx, &n) && pop_stack(ctx, &src) && pop_stack(ctx, &dst));
    } else if (subop == 9U) {
        (void)wasm_read_u32_leb(ctx, code_end, &imm0);
    } else if (subop == 10U) {
        if (!wasm_read_u32_leb(ctx, code_end, &imm0) || !wasm_read_u32_leb(ctx, code_end, &imm1)) {
            return;
        }
        uint32_t len = 0U;
        uint32_t src = 0U;
        uint32_t dst = 0U;
        if (pop_stack(ctx, &len) && pop_stack(ctx, &src) && pop_stack(ctx, &dst)) {
            if (ctx->linear_mem != NULL &&
                (uint64_t)dst + (uint64_t)len <= (uint64_t)ctx->linear_mem_size &&
                (uint64_t)src + (uint64_t)len <= (uint64_t)ctx->linear_mem_size) {
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
        uint32_t len = 0U;
        uint32_t val = 0U;
        uint32_t dst = 0U;
        if (pop_stack(ctx, &len) && pop_stack(ctx, &val) && pop_stack(ctx, &dst)) {
            if (ctx->linear_mem != NULL &&
                (uint64_t)dst + (uint64_t)len <= (uint64_t)ctx->linear_mem_size) {
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

/* ── Interpreter Step Loop ───────────────────────────────────────────────── */

#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
/**
 * @brief Truncate Q16.16 fixed-point value toward zero without signed shift UB.
 * @param qa Input Q16.16 value.
 * @return Truncated Q16.16 integer value.
 */
static q16_t wasm_q16_trunc(q16_t qa)
{
    uint32_t u = (uint32_t)qa;
    uint32_t base = u & 0xFFFF0000U;
    if (qa < 0 && (u & 0xFFFFU) != 0U) {
        base += 0x00010000U;
    }
    return (q16_t)base;
}

/**
 * @brief Execute unary Q16.16 fixed-point F32 opcode.
 * @param ctx Active Wasm execution context.
 * @param opcode Wasm unary F32 opcode.
 */
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

/**
 * @brief Execute binary Q16.16 fixed-point F32 opcode.
 * @param ctx Active Wasm execution context.
 * @param opcode Wasm binary F32 opcode.
 */
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

/**
 * @brief Dispatch Q16.16 fixed-point F32 unary or binary opcode.
 * @param ctx Active Wasm execution context.
 * @param opcode Wasm F32 opcode.
 */
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

    const SYN_WASM_Module *mod = ctx->module;
    uint16_t executed = 0;

    while (executed < max_instructions && ctx->status == SYN_WASM_OK) {
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
                (void)mem_idx;
                uint32_t pages = ctx->linear_mem_size / 65536U;
                (void)push_stack(ctx, pages);
            }
            break;
        }

        case OP_MEMORY_GROW: {
            uint8_t mem_idx = 0U;
            uint32_t delta = 0U;
            if (wasm_read_byte(ctx, code_end, &mem_idx) && pop_stack(ctx, &delta)) {
                (void)mem_idx;
                uint32_t current_pages = ctx->linear_mem_size / 65536U;
                if (delta == 0U) {
                    (void)push_stack(ctx, current_pages);
                } else {
                    (void)push_stack(ctx, 0xFFFFFFFFU);
                }
            }
            break;
        }

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

        case OP_CALL_INDIRECT: {
            uint32_t type_idx = 0U;
            uint32_t table_idx = 0U;
            uint32_t elem_idx = 0U;
            if (!wasm_read_u32_leb(ctx, code_end, &type_idx) ||
                !wasm_read_u32_leb(ctx, code_end, &table_idx) || !pop_stack(ctx, &elem_idx)) {
                break;
            }
            (void)type_idx;
            (void)table_idx;
            if (elem_idx >= mod->table_element_count) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }
            wasm_invoke_func(ctx, mod->table_elements[elem_idx]);
            break;
        }

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
            if (opcode == OP_F32_ABS || opcode == OP_F32_NEG || opcode == OP_F32_CEIL ||
                opcode == OP_F32_FLOOR || opcode == OP_F32_TRUNC || opcode == OP_F32_NEAREST ||
                opcode == OP_F32_SQRT) {
                float fa = 0.0f;
                if (!pop_f32(ctx, &fa))
                    break;
                float fres = 0.0f;
                if (opcode == OP_F32_ABS)
                    fres = fabsf(fa);
                else if (opcode == OP_F32_NEG)
                    fres = -fa;
                else if (opcode == OP_F32_CEIL)
                    fres = ceilf(fa);
                else if (opcode == OP_F32_FLOOR)
                    fres = floorf(fa);
                else if (opcode == OP_F32_TRUNC)
                    fres = truncf(fa);
                else if (opcode == OP_F32_NEAREST)
                    fres = roundf(fa);
                else if (opcode == OP_F32_SQRT)
                    fres = sqrtf(fa);
                push_f32(ctx, fres);
            } else {
                float fb = 0.0f, fa = 0.0f;
                if (!pop_f32(ctx, &fb) || !pop_f32(ctx, &fa))
                    break;
                float fres = 0.0f;
                if (opcode == OP_F32_ADD)
                    fres = fa + fb;
                else if (opcode == OP_F32_SUB)
                    fres = fa - fb;
                else if (opcode == OP_F32_MUL)
                    fres = fa * fb;
                else if (opcode == OP_F32_DIV)
                    fres = fa / fb;
                else if (opcode == OP_F32_MIN)
                    fres = (fa < fb) ? fa : fb;
                else if (opcode == OP_F32_MAX)
                    fres = (fa > fb) ? fa : fb;
                else if (opcode == OP_F32_COPYSIGN)
                    fres = copysignf(fa, fb);
                push_f32(ctx, fres);
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
            if (opcode == OP_F64_ABS || opcode == OP_F64_NEG || opcode == OP_F64_CEIL ||
                opcode == OP_F64_FLOOR || opcode == OP_F64_TRUNC || opcode == OP_F64_NEAREST ||
                opcode == OP_F64_SQRT) {
                double da = 0.0;
                if (!pop_f64(ctx, &da))
                    break;
                double dres = 0.0;
                if (opcode == OP_F64_ABS)
                    dres = fabs(da);
                else if (opcode == OP_F64_NEG)
                    dres = -da;
                else if (opcode == OP_F64_CEIL)
                    dres = ceil(da);
                else if (opcode == OP_F64_FLOOR)
                    dres = floor(da);
                else if (opcode == OP_F64_TRUNC)
                    dres = trunc(da);
                else if (opcode == OP_F64_NEAREST)
                    dres = round(da);
                else if (opcode == OP_F64_SQRT)
                    dres = sqrt(da);
                push_f64(ctx, dres);
            } else {
                double db = 0.0, da = 0.0;
                if (!pop_f64(ctx, &db) || !pop_f64(ctx, &da))
                    break;
                double dres = 0.0;
                if (opcode == OP_F64_ADD)
                    dres = da + db;
                else if (opcode == OP_F64_SUB)
                    dres = da - db;
                else if (opcode == OP_F64_MUL)
                    dres = da * db;
                else if (opcode == OP_F64_DIV)
                    dres = da / db;
                else if (opcode == OP_F64_MIN)
                    dres = (da < db) ? da : db;
                else if (opcode == OP_F64_MAX)
                    dres = (da > db) ? da : db;
                else if (opcode == OP_F64_COPYSIGN)
                    dres = copysign(da, db);
                push_f64(ctx, dres);
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

    if (executed >= max_instructions && ctx->status == SYN_WASM_OK) {
        return SYN_WASM_YIELDED;
    }

    return ctx->status;
}

SYN_PT_Status syn_wasm_exec_pt(SYN_PT *pt, SYN_WASM_Context *ctx, uint16_t quantum_instructions)
{
    if (pt == NULL || ctx == NULL || ctx->module == NULL) {
        return PT_EXITED;
    }

    uint16_t step_limit =
        (quantum_instructions == 0U) ? SYN_WASM_DEFAULT_QUANTUM : quantum_instructions;

    PT_BEGIN(pt);

    while (ctx->status == SYN_WASM_OK && ctx->call_depth > 0U) {
        SYN_WASM_Status st = syn_wasm_step(ctx, step_limit);
        if (st == SYN_WASM_YIELDED) {
            PT_CONTINUE(pt);
        }
    }

    if (ctx->status != SYN_WASM_HALTED) {
        PT_EXIT(pt);
    }

    PT_END(pt);
}

SYN_PT_Status syn_wasm_task(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_WASM_Context *ctx = (SYN_WASM_Context *)task->user_data;

    PT_BEGIN(pt);

    while (ctx->status <= SYN_WASM_HALTED) {
        if (ctx->status == SYN_WASM_OK && ctx->call_depth > 0U) {
            if (syn_wasm_step(ctx, SYN_WASM_DEFAULT_QUANTUM) == SYN_WASM_YIELDED) {
                PT_CONTINUE_IF_NEEDED(pt, task);
            }
        } else {
            PT_PASS(pt);
        }
    }

    PT_END(pt);
}

/** @endcond */
