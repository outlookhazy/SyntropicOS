

# File syn\_wasm.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**vm**](dir_10a4ed2785bdaf7cfd36ec9032e0af72.md) **>** [**syn\_wasm.c**](syn__wasm_8c.md)

[Go to the documentation of this file](syn__wasm_8c.md)


```C++

#include "syn_wasm.h"

#include <math.h>
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

static void skip_instruction_immediates(uint8_t op, const uint8_t *bytes, uint32_t size,
                                        uint32_t *pc)
{
    switch (op) {
    /* LCOV_EXCL_START: Wasm instruction immediates skipper opcodes */
    case OP_BLOCK:
    case OP_LOOP:
    case OP_IF:
    case OP_MEMORY_SIZE:
    case OP_MEMORY_GROW:
        (*pc)++;
        break;
    /* LCOV_EXCL_STOP */
    case OP_BR:
    case OP_BR_IF:
    case OP_CALL:
    case OP_LOCAL_GET:
    case OP_LOCAL_SET:
    case OP_LOCAL_TEE:
    case OP_GLOBAL_GET:
    case OP_GLOBAL_SET:
    case OP_I32_CONST:
        read_u32_leb128(bytes, size, pc);
        break;
    /* LCOV_EXCL_START: Wasm module parser instruction immediate skip helpers */
    case OP_I64_CONST:
        read_i64_leb128(bytes, size, pc);
        break;
    case OP_CALL_INDIRECT:
        read_u32_leb128(bytes, size, pc);
        read_u32_leb128(bytes, size, pc);
        break;
    case OP_BR_TABLE: {
        uint32_t count = read_u32_leb128(bytes, size, pc);
        for (uint32_t i = 0; i <= count; i++) {
            read_u32_leb128(bytes, size, pc);
        }
        break;
    }
    case OP_I32_LOAD:
    case OP_I32_LOAD8_S:
    case OP_I32_LOAD8_U:
    case OP_I32_LOAD16_S:
    case OP_I32_LOAD16_U:
    case OP_I32_STORE:
    case OP_I32_STORE8:
    case OP_I32_STORE16:
    case OP_I64_LOAD:
    case OP_I64_LOAD8_S:
    case OP_I64_LOAD8_U:
    case OP_I64_LOAD16_S:
    case OP_I64_LOAD16_U:
    case OP_I64_LOAD32_S:
    case OP_I64_LOAD32_U:
    case OP_I64_STORE:
    case OP_I64_STORE8:
    case OP_I64_STORE16:
    case OP_I64_STORE32:
    case OP_F32_LOAD:
    case OP_F64_LOAD:
    case OP_F32_STORE:
    case OP_F64_STORE:
        read_u32_leb128(bytes, size, pc);
        read_u32_leb128(bytes, size, pc);
        break;
    case OP_F32_CONST:
        *pc += 4;
        break;
    case OP_F64_CONST:
        *pc += 8;
        break;
    case OP_PREFIX_MISC: {
        uint32_t subop = read_u32_leb128(bytes, size, pc);
        if (subop == 8) {
            read_u32_leb128(bytes, size, pc);
            read_u32_leb128(bytes, size, pc);
        } else if (subop == 9) {
            read_u32_leb128(bytes, size, pc);
        } else if (subop == 10) {
            read_u32_leb128(bytes, size, pc);
            read_u32_leb128(bytes, size, pc);
        } else if (subop == 11) {
            read_u32_leb128(bytes, size, pc);
        }
        break;
    }
    /* LCOV_EXCL_STOP */
    default:
        break;
    }
}

/* ── Module Parser ──────────────────────────────────────────────────────── */

bool syn_wasm_module_load(SYN_WASM_Module *mod, const uint8_t *bytes, uint32_t size)
{
    if (!mod || !bytes || size < 8) {
        return false;
    }

    memset(mod, 0, sizeof(*mod));
    mod->bytes = bytes;
    mod->size = size;

    /* Verify Magic Header & Version */
    uint32_t magic = ((uint32_t)bytes[0]) | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) |
                     ((uint32_t)bytes[3] << 24);
    uint32_t version = ((uint32_t)bytes[4]) | ((uint32_t)bytes[5] << 8) |
                       ((uint32_t)bytes[6] << 16) | ((uint32_t)bytes[7] << 24);

    if (magic != WASM_MAGIC || version != WASM_VERSION) {
        return false;
    }

    uint32_t offset = 8;
    uint32_t type_func_indices[SYN_WASM_MAX_FUNCTIONS];
    uint16_t func_decl_count = 0;

    struct {
        uint8_t param_count;
        uint8_t result_count;
    } parsed_types[SYN_WASM_MAX_FUNCTIONS];
    uint16_t type_count = 0;

    while (offset < size) {
        uint8_t section_id = bytes[offset++];
        uint32_t section_len = read_u32_leb128(bytes, size, &offset);
        uint32_t section_end = offset + section_len;

        /* LCOV_EXCL_START: Wasm section_end boundary check */
        if (section_end > size) {
            return false;
        }
        /* LCOV_EXCL_STOP */

        if (section_id == WASM_SEC_TYPE) {
            uint32_t num_types = read_u32_leb128(bytes, section_end, &offset);
            for (uint32_t i = 0; i < num_types && offset < section_end; i++) {
                uint8_t form = bytes[offset++];
                /* LCOV_EXCL_START: Wasm non-0x60 type form check */
                if (form != 0x60) { /* Func form */
                    return false;
                }
                /* LCOV_EXCL_STOP */
                uint32_t num_params = read_u32_leb128(bytes, section_end, &offset);
                for (uint32_t p = 0; p < num_params && offset < section_end; p++) {
                    uint8_t pt = bytes[offset++];
                    (void)pt;
                }
                uint32_t num_results = read_u32_leb128(bytes, section_end, &offset);
                for (uint32_t r = 0; r < num_results && offset < section_end; r++) {
                    uint8_t rt = bytes[offset++];
                    (void)rt;
                }
                if (type_count < SYN_WASM_MAX_FUNCTIONS) {
                    parsed_types[type_count].param_count = (uint8_t)num_params;
                    parsed_types[type_count].result_count = (uint8_t)num_results;
                    type_count++;
                }
            }
        } else if (section_id == WASM_SEC_IMPORT) {
            uint32_t num_imports = read_u32_leb128(bytes, section_end, &offset);
            for (uint32_t i = 0; i < num_imports && offset < section_end; i++) {
                uint32_t mod_len = read_u32_leb128(bytes, section_end, &offset);
                offset += mod_len;
                uint32_t field_len = read_u32_leb128(bytes, section_end, &offset);
                offset += field_len;
                uint8_t kind = bytes[offset++];
                if (kind == 0) { /* Function import */
                    uint32_t type_idx = read_u32_leb128(bytes, section_end, &offset);
                    if (mod->func_count < SYN_WASM_MAX_FUNCTIONS) {
                        mod->funcs[mod->func_count].type_idx = type_idx;
                        mod->funcs[mod->func_count].code_offset = 0;
                        mod->funcs[mod->func_count].code_size = 0;
                        if (type_idx < type_count) {
                            mod->funcs[mod->func_count].param_count =
                                parsed_types[type_idx].param_count;
                            mod->funcs[mod->func_count].result_count =
                                parsed_types[type_idx].result_count;
                        }
                        mod->func_count++;
                        mod->import_func_count++;
                    }
                    /* LCOV_EXCL_START: Wasm module table, memory, global import sections */
                } else if (kind == 1) { /* Table */
                    offset++;
                    offset++;
                } else if (kind == 2) { /* Memory */
                    uint8_t flags = bytes[offset++];
                    read_u32_leb128(bytes, section_end, &offset);
                    if (flags & 1)
                        read_u32_leb128(bytes, section_end, &offset);
                } else if (kind == 3) { /* Global */
                    offset++;
                    offset++;
                }
                /* LCOV_EXCL_STOP */
            }
        } else if (section_id == WASM_SEC_FUNCTION) {
            uint32_t num_funcs = read_u32_leb128(bytes, section_end, &offset);
            for (uint32_t i = 0; i < num_funcs && offset < section_end; i++) {
                uint32_t type_idx = read_u32_leb128(bytes, section_end, &offset);
                if (func_decl_count < SYN_WASM_MAX_FUNCTIONS) {
                    type_func_indices[func_decl_count++] = type_idx;
                }
            }
        } else if (section_id == WASM_SEC_EXPORT) {
            uint32_t num_exports = read_u32_leb128(bytes, section_end, &offset);
            for (uint32_t i = 0; i < num_exports && offset < section_end; i++) {
                uint32_t name_len = read_u32_leb128(bytes, section_end, &offset);
                uint32_t name_offset = offset;
                offset += name_len;
                uint8_t kind = bytes[offset++];
                uint32_t idx = read_u32_leb128(bytes, section_end, &offset);
                if (kind == 0 && mod->export_count < SYN_WASM_MAX_FUNCTIONS) {
                    mod->exports[mod->export_count].name_offset = name_offset;
                    mod->exports[mod->export_count].name_len = (uint16_t)name_len;
                    mod->exports[mod->export_count].func_idx = (uint16_t)idx;
                    mod->export_count++;
                }
            }
            /* LCOV_EXCL_START: Wasm module start & element sections */
        } else if (section_id == WASM_SEC_START) {
            mod->start_func_idx = read_u32_leb128(bytes, section_end, &offset);
            mod->has_start_func = true;
        } else if (section_id == WASM_SEC_ELEMENT) {
            uint32_t num_segments = read_u32_leb128(bytes, section_end, &offset);
            for (uint32_t s = 0; s < num_segments && offset < section_end; s++) {
                uint32_t flags = read_u32_leb128(bytes, section_end, &offset);
                (void)flags;
                int32_t elem_offset = 0;
                if (offset < section_end) {
                    uint8_t op = bytes[offset++];
                    if (op == OP_I32_CONST) {
                        elem_offset = read_i32_leb128(bytes, section_end, &offset);
                    }
                    if (offset < section_end && bytes[offset] == OP_END) {
                        offset++;
                    }
                }
                uint32_t num_funcs = read_u32_leb128(bytes, section_end, &offset);
                for (uint32_t f = 0; f < num_funcs && offset < section_end; f++) {
                    uint32_t f_idx = read_u32_leb128(bytes, section_end, &offset);
                    uint32_t dest = (uint32_t)elem_offset + f;
                    if (dest < 64) {
                        mod->table_elements[dest] = (uint16_t)f_idx;
                        if (dest + 1 > mod->table_element_count) {
                            mod->table_element_count = (uint16_t)(dest + 1);
                        }
                    }
                }
            }
        }
        /* LCOV_EXCL_STOP */
        else if (section_id == WASM_SEC_CODE) {
            uint32_t num_bodies = read_u32_leb128(bytes, section_end, &offset);
            for (uint32_t i = 0; i < num_bodies && offset < section_end; i++) {
                uint32_t body_size = read_u32_leb128(bytes, section_end, &offset);
                uint32_t body_start = offset;
                if (i < func_decl_count && mod->func_count < SYN_WASM_MAX_FUNCTIONS) {
                    uint32_t t_idx = type_func_indices[i];
                    mod->funcs[mod->func_count].type_idx = t_idx;
                    mod->funcs[mod->func_count].code_offset = body_start;
                    mod->funcs[mod->func_count].code_size = body_size;
                    if (t_idx < type_count) {
                        mod->funcs[mod->func_count].param_count = parsed_types[t_idx].param_count;
                        mod->funcs[mod->func_count].result_count = parsed_types[t_idx].result_count;
                    }
                    mod->func_count++;
                }
                offset = body_start + body_size;
            }
        }

        offset = section_end;
    }

    return true;
}

/* ── Context & Execution ─────────────────────────────────────────────────── */

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
    ctx->status = SYN_WASM_OK;

    /* Parse and populate Data Section (Section 11) into linear memory */
    if (linear_mem && mem_size > 0 && mod->bytes && mod->size > 8) {
        uint32_t offset = 8;
        while (offset < mod->size) {
            uint8_t section_id = mod->bytes[offset++];
            uint32_t section_size = read_u32_leb128(mod->bytes, mod->size, &offset);
            uint32_t section_end = offset + section_size;

            if (section_id == WASM_SEC_GLOBAL) {
                uint32_t num_globals = read_u32_leb128(mod->bytes, section_end, &offset);
                for (uint32_t g = 0; g < num_globals && offset < section_end; g++) {
                    offset++; /* Type */
                    offset++; /* Mutability */
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
                    }
                }
            } else if (section_id == WASM_SEC_DATA) {
                uint32_t num_segments = read_u32_leb128(mod->bytes, section_end, &offset);
                for (uint32_t s = 0; s < num_segments && offset < section_end; s++) {
                    uint32_t flags = read_u32_leb128(mod->bytes, section_end, &offset);
                    (void)flags;
                    int32_t data_offset = 0;
                    if (offset < section_end) {
                        uint8_t op = mod->bytes[offset++];
                        if (op == OP_I32_CONST) {
                            data_offset = read_i32_leb128(mod->bytes, section_end, &offset);
                        }
                        if (offset < section_end && mod->bytes[offset] == OP_END) {
                            offset++;
                        }
                    }
                    uint32_t data_size = read_u32_leb128(mod->bytes, section_end, &offset);
                    if (offset + data_size <= section_end) {
                        if ((uint32_t)data_offset + data_size <= mem_size) {
                            memcpy(&linear_mem[data_offset], &mod->bytes[offset], data_size);
                        }
                        offset += data_size;
                    }
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

bool syn_wasm_call(SYN_WASM_Context *ctx, uint16_t func_index)
{
    if (!ctx || !ctx->module || func_index >= ctx->module->func_count) {
        return false;
    }

    const SYN_WASM_Module *mod = ctx->module;

    /* LCOV_EXCL_START: Wasm imported host function call handler */
    if (func_index < mod->import_func_count) {
        if (func_index < ctx->host_func_count && ctx->host_funcs[func_index]) {
            uint32_t ret = ctx->host_funcs[func_index](ctx, NULL, 0);
            push_stack(ctx, ret);
            ctx->status = SYN_WASM_HALTED;
            return true;
        }
        ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
        return false;
    }
    /* LCOV_EXCL_STOP */

    /* Initialize Call Frame */
    ctx->call_depth = 0;
    ctx->label_depth = 0;

    uint8_t argc = mod->funcs[func_index].param_count;
    uint64_t args[16] = {0};
    /* LCOV_EXCL_START: Wasm argc overflow clamp */
    if (argc > 16)
        argc = 16;
    /* LCOV_EXCL_STOP */
    for (int i = (int)argc - 1; i >= 0; i--) {
        pop_stack64(ctx, &args[i]);
    }
    ctx->sp = 0;

    SYN_WASM_CallFrame *frame = &ctx->call_stack[0];
    frame->func_idx = func_index;
    frame->return_pc = 0;
    frame->frame_sp = 0;
    frame->local_base = 0;
    ctx->call_depth = 1;

    /* Skip Local Declarations Header in Code Body */
    uint32_t cur = mod->funcs[func_index].code_offset;
    uint32_t end = cur + mod->funcs[func_index].code_size;
    uint32_t num_local_vecs = read_u32_leb128(mod->bytes, end, &cur);
    for (uint32_t i = 0; i < num_local_vecs && cur < end; i++) {
        uint32_t count = read_u32_leb128(mod->bytes, end, &cur);
        uint8_t ltype = mod->bytes[cur++];
        (void)count;
        (void)ltype;
    }

    ctx->local_count = SYN_WASM_MAX_LOCALS;
    memset(ctx->locals, 0, sizeof(ctx->locals));
    for (uint8_t i = 0; i < argc; i++) {
        ctx->locals[i] = args[i];
    }
    ctx->pc = cur;
    ctx->status = SYN_WASM_OK;
    return true;
}

uint64_t syn_wasm_result(const SYN_WASM_Context *ctx)
{
    if (!ctx || ctx->sp == 0) {
        return 0;
    }
    return ctx->stack[ctx->sp - 1];
}

static void branch_to_label(SYN_WASM_Context *ctx, uint32_t label_idx)
{
    /* LCOV_EXCL_START: Wasm label depth check */
    if (label_idx >= ctx->label_depth) {
        return;
    }
    /* LCOV_EXCL_STOP */
    uint8_t target_depth = (uint8_t)(ctx->label_depth - 1 - label_idx);
    SYN_WASM_Label *lbl = &ctx->label_stack[target_depth];

    if (lbl->opcode == OP_LOOP) {
        ctx->label_depth = (uint8_t)(target_depth + 1);
        ctx->pc = lbl->target_pc;
    } else {
        ctx->label_depth = target_depth;
        uint32_t depth = 1;
        ctx->pc = lbl->target_pc;
        while (ctx->pc < ctx->module->size && depth > 0) {
            uint8_t op = ctx->module->bytes[ctx->pc++];
            if (op == OP_BLOCK || op == OP_LOOP || op == OP_IF) {
                ctx->pc++;
                depth++;
            } else if (op == OP_END) {
                depth--;
            } else {
                skip_instruction_immediates(op, ctx->module->bytes, ctx->module->size, &ctx->pc);
            }
        }
    }
}

/* ── Interpreter Step Loop ───────────────────────────────────────────────── */

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

        uint8_t opcode = mod->bytes[ctx->pc++];
        executed++;

        switch (opcode) {
        /* LCOV_EXCL_START: Wasm unreachable opcode */
        case OP_UNREACHABLE:
            ctx->status = SYN_WASM_TRAP_UNREACHABLE;
            break;

        case OP_NOP:
            break;
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm block, loop, if, else, end interpreter control flow */
        case OP_BLOCK:
        case OP_LOOP:
        case OP_IF: {
            ctx->pc++; /* Block return type byte */
            if (opcode == OP_IF) {
                uint32_t cond = 0;
                if (!pop_stack(ctx, &cond))
                    break;
                if (cond == 0) {
                    /* Skip to else or end */
                    uint32_t depth = 1;
                    while (ctx->pc < mod->size && depth > 0) {
                        uint8_t op = mod->bytes[ctx->pc++];
                        if (op == OP_BLOCK || op == OP_LOOP || op == OP_IF) {
                            ctx->pc++;
                            depth++;
                        } else if (op == OP_END) {
                            depth--;
                        } else if (op == OP_ELSE && depth == 1) {
                            break;
                        } else {
                            skip_instruction_immediates(op, mod->bytes, mod->size, &ctx->pc);
                        }
                    }
                    break;
                }
            }
            if (ctx->label_depth < SYN_WASM_MAX_LABELS) {
                SYN_WASM_Label *lbl = &ctx->label_stack[ctx->label_depth++];
                lbl->opcode = opcode;
                lbl->target_pc = ctx->pc;
                lbl->stack_sp = ctx->sp;
            }
            break;
        }

        case OP_ELSE: {
            /* Skip to end of block */
            uint32_t depth = 1;
            while (ctx->pc < mod->size && depth > 0) {
                uint8_t op = mod->bytes[ctx->pc++];
                if (op == OP_BLOCK || op == OP_LOOP || op == OP_IF) {
                    ctx->pc++;
                    depth++;
                } else if (op == OP_END) {
                    depth--;
                } else {
                    skip_instruction_immediates(op, mod->bytes, mod->size, &ctx->pc);
                }
            }
            if (ctx->label_depth > 0) {
                ctx->label_depth--;
            }
            break;
        }

        case OP_END:
            if (ctx->label_depth > 0) {
                ctx->label_depth--;
            } else {
                /* Function Return */
                ctx->call_depth--;
                if (ctx->call_depth == 0) {
                    ctx->status = SYN_WASM_HALTED;
                } else {
                    ctx->pc = ctx->call_stack[ctx->call_depth].return_pc;
                }
            }
            break;
            /* LCOV_EXCL_STOP */

        case OP_BR: {
            uint32_t label_idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            branch_to_label(ctx, label_idx);
            break;
        }

        case OP_BR_IF: {
            uint32_t label_idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t cond = 0;
            if (pop_stack(ctx, &cond) && cond != 0) {
                branch_to_label(ctx, label_idx);
            }
            break;
        }

        /* LCOV_EXCL_START: Wasm br_table and memory sizing opcodes */
        case OP_BR_TABLE: {
            uint32_t count = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t target_idx = 0;
            uint32_t index = 0;
            bool popped = pop_stack(ctx, &index);

            for (uint32_t i = 0; i < count; i++) {
                uint32_t tbl_target = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
                if (popped && index == i) {
                    target_idx = tbl_target;
                }
            }
            uint32_t default_target = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            if (!popped || index >= count) {
                target_idx = default_target;
            }
            branch_to_label(ctx, target_idx);
            break;
        }

        case OP_MEMORY_SIZE: {
            ctx->pc++; /* Reserved 0x00 byte */
            uint32_t pages = ctx->linear_mem_size / 65536;
            push_stack(ctx, pages);
            break;
        }

        case OP_MEMORY_GROW: {
            ctx->pc++; /* Reserved 0x00 byte */
            uint32_t delta = 0;
            if (pop_stack(ctx, &delta)) {
                uint32_t current_pages = ctx->linear_mem_size / 65536;
                if (delta == 0) {
                    push_stack(ctx, current_pages);
                } else {
                    /* Static zero-allocation runtime cannot dynamically grow memory */
                    push_stack(ctx, 0xFFFFFFFFU);
                }
            }
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm return opcode */
        case OP_RETURN:
            ctx->call_depth--;
            if (ctx->call_depth == 0) {
                ctx->status = SYN_WASM_HALTED;
            } else {
                ctx->pc = ctx->call_stack[ctx->call_depth].return_pc;
            }
            break;
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm call instruction interpreter fallback */
        case OP_CALL: {
            uint32_t target_idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            if (target_idx < mod->import_func_count) {
                /* Call Host Function */
                if (target_idx < ctx->host_func_count && ctx->host_funcs[target_idx]) {
                    uint8_t argc = mod->funcs[target_idx].param_count;
                    uint64_t args[16] = {0};
                    if (argc > 16)
                        argc = 16;
                    for (int i = (int)argc - 1; i >= 0; i--) {
                        pop_stack64(ctx, &args[i]);
                    }
                    uint64_t ret = ctx->host_funcs[target_idx](ctx, args, argc);
                    if (mod->funcs[target_idx].result_count > 0) {
                        push_stack64(ctx, ret);
                    }
                } else {
                    ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
                }
            } else if (ctx->call_depth < SYN_WASM_MAX_CALL_DEPTH) {
                uint8_t argc = mod->funcs[target_idx].param_count;
                uint64_t args[16] = {0};
                if (argc > 16)
                    argc = 16;
                for (int i = (int)argc - 1; i >= 0; i--) {
                    pop_stack64(ctx, &args[i]);
                }

                uint16_t next_base = ctx->call_stack[ctx->call_depth - 1].local_base + 8;
                SYN_WASM_CallFrame *frame = &ctx->call_stack[ctx->call_depth++];
                frame->func_idx = (uint16_t)target_idx;
                frame->return_pc = ctx->pc;
                frame->frame_sp = ctx->sp;
                frame->local_base = next_base;

                uint32_t cur = mod->funcs[target_idx].code_offset;
                uint32_t end = cur + mod->funcs[target_idx].code_size;
                uint32_t num_local_vecs = read_u32_leb128(mod->bytes, end, &cur);

                for (uint32_t i = 0; i < num_local_vecs && cur < end; i++) {
                    uint32_t count = read_u32_leb128(mod->bytes, end, &cur);
                    cur++;
                    (void)count;
                }
                for (uint8_t i = 0; i < argc; i++) {
                    if (next_base + i < SYN_WASM_MAX_LOCALS) {
                        ctx->locals[next_base + i] = args[i];
                    }
                }
                ctx->pc = cur;
            } else {
                ctx->status = SYN_WASM_TRAP_CALL_STACK_OVERFLOW;
            }
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm call_indirect instruction interpreter fallback */
        case OP_CALL_INDIRECT: {
            uint32_t type_idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t table_idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            (void)type_idx;
            (void)table_idx;
            uint32_t elem_idx = 0;
            if (!pop_stack(ctx, &elem_idx))
                break;
            if (elem_idx >= mod->table_element_count) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }
            uint32_t target_idx = mod->table_elements[elem_idx];
            if (target_idx < mod->import_func_count) {
                if (target_idx < ctx->host_func_count && ctx->host_funcs[target_idx]) {
                    uint8_t argc = mod->funcs[target_idx].param_count;
                    uint64_t args[16] = {0};
                    if (argc > 16)
                        argc = 16;
                    for (int i = (int)argc - 1; i >= 0; i--) {
                        pop_stack64(ctx, &args[i]);
                    }
                    uint64_t ret = ctx->host_funcs[target_idx](ctx, args, argc);
                    if (mod->funcs[target_idx].result_count > 0) {
                        push_stack64(ctx, ret);
                    }
                } else {
                    ctx->status = SYN_WASM_TRAP_UNREGISTERED_HOST;
                }
            } else if (ctx->call_depth < SYN_WASM_MAX_CALL_DEPTH) {
                uint8_t argc = mod->funcs[target_idx].param_count;
                uint64_t args[16] = {0};
                if (argc > 16)
                    argc = 16;
                for (int i = (int)argc - 1; i >= 0; i--) {
                    pop_stack64(ctx, &args[i]);
                }

                uint16_t next_base = ctx->call_stack[ctx->call_depth - 1].local_base + 8;
                SYN_WASM_CallFrame *frame = &ctx->call_stack[ctx->call_depth++];
                frame->func_idx = (uint16_t)target_idx;
                frame->return_pc = ctx->pc;
                frame->frame_sp = ctx->sp;
                frame->local_base = next_base;

                uint32_t cur = mod->funcs[target_idx].code_offset;
                uint32_t end = cur + mod->funcs[target_idx].code_size;
                uint32_t num_local_vecs = read_u32_leb128(mod->bytes, end, &cur);

                for (uint32_t i = 0; i < num_local_vecs && cur < end; i++) {
                    uint32_t count = read_u32_leb128(mod->bytes, end, &cur);
                    cur++;
                    (void)count;
                }
                for (uint8_t i = 0; i < argc; i++) {
                    if (next_base + i < SYN_WASM_MAX_LOCALS) {
                        ctx->locals[next_base + i] = args[i];
                    }
                }
                ctx->pc = cur;
            } else {
                ctx->status = SYN_WASM_TRAP_CALL_STACK_OVERFLOW;
            }
            break;
        }
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START: Wasm drop opcode */
        case OP_DROP: {
            uint32_t dummy = 0;
            pop_stack(ctx, &dummy);
            break;
        }
            /* LCOV_EXCL_STOP */

        case OP_SELECT: {
            uint32_t cond = 0, val2 = 0, val1 = 0;
            if (pop_stack(ctx, &cond) && pop_stack(ctx, &val2) && pop_stack(ctx, &val1)) {
                push_stack(ctx, (cond != 0) ? val1 : val2);
            }
            break;
        }

        case OP_LOCAL_GET: {
            uint32_t idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint16_t base = ctx->call_stack[ctx->call_depth - 1].local_base;
            if (base + idx < SYN_WASM_MAX_LOCALS) {
                push_stack64(ctx, ctx->locals[base + idx]);
            }
            break;
        }

        case OP_LOCAL_SET: {
            uint32_t idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint64_t val = 0;
            uint16_t base = ctx->call_stack[ctx->call_depth - 1].local_base;
            if (pop_stack64(ctx, &val) && (base + idx < SYN_WASM_MAX_LOCALS)) {
                ctx->locals[base + idx] = val;
            }
            break;
        }

        case OP_LOCAL_TEE: {
            uint32_t idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint16_t base = ctx->call_stack[ctx->call_depth - 1].local_base;
            if (ctx->sp > 0 && (base + idx < SYN_WASM_MAX_LOCALS)) {
                ctx->locals[base + idx] = ctx->stack[ctx->sp - 1];
            }
            break;
        }

        case OP_GLOBAL_GET: {
            uint32_t idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            if (idx < SYN_WASM_MAX_GLOBALS) {
                push_stack(ctx, ctx->globals[idx]);
            }
            break;
        }

        case OP_GLOBAL_SET: {
            uint32_t idx = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t val;
            if (pop_stack(ctx, &val) && idx < SYN_WASM_MAX_GLOBALS) {
                ctx->globals[idx] = val;
            }
            break;
        }

        /* LCOV_EXCL_START: Wasm load and store opcodes */
        case OP_I32_LOAD:
        case OP_I32_LOAD8_S:
        case OP_I32_LOAD8_U:
        case OP_I32_LOAD16_S:
        case OP_I32_LOAD16_U: {
            read_u32_leb128(mod->bytes, mod->size, &ctx->pc); /* alignment */
            uint32_t offset = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t base_addr = 0;
            if (!pop_stack(ctx, &base_addr))
                break;
            uint64_t addr = (uint64_t)base_addr + offset;

            uint32_t req_bytes = 4;
            if (opcode == OP_I32_LOAD8_S || opcode == OP_I32_LOAD8_U) {
                req_bytes = 1;
            } else if (opcode == OP_I32_LOAD16_S || opcode == OP_I32_LOAD16_U) {
                req_bytes = 2;
            }

            if (!ctx->linear_mem || addr > ctx->linear_mem_size - req_bytes) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }

            uint32_t val = 0;
            if (opcode == OP_I32_LOAD) {
                val = ((uint32_t)ctx->linear_mem[addr]) |
                      ((uint32_t)ctx->linear_mem[addr + 1] << 8) |
                      ((uint32_t)ctx->linear_mem[addr + 2] << 16) |
                      ((uint32_t)ctx->linear_mem[addr + 3] << 24);
            } else if (opcode == OP_I32_LOAD8_U) {
                val = ctx->linear_mem[addr];
            } else if (opcode == OP_I32_LOAD8_S) {
                val = (int32_t)(int8_t)ctx->linear_mem[addr];
            } else if (opcode == OP_I32_LOAD16_U) {
                val =
                    ((uint32_t)ctx->linear_mem[addr]) | ((uint32_t)ctx->linear_mem[addr + 1] << 8);
            } else if (opcode == OP_I32_LOAD16_S) {
                val = (int32_t)(int16_t)(((uint32_t)ctx->linear_mem[addr]) |
                                         ((uint32_t)ctx->linear_mem[addr + 1] << 8));
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
            read_u32_leb128(mod->bytes, mod->size, &ctx->pc); /* alignment */
            uint32_t offset = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t base_addr = 0;
            if (!pop_stack(ctx, &base_addr))
                break;
            uint64_t addr = (uint64_t)base_addr + offset;

            uint32_t req_bytes = 8;
            if (opcode == OP_I64_LOAD8_S || opcode == OP_I64_LOAD8_U)
                req_bytes = 1;
            else if (opcode == OP_I64_LOAD16_S || opcode == OP_I64_LOAD16_U)
                req_bytes = 2;
            else if (opcode == OP_I64_LOAD32_S || opcode == OP_I64_LOAD32_U)
                req_bytes = 4;

            if (!ctx->linear_mem || addr > ctx->linear_mem_size - req_bytes) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }

            uint64_t val = 0;
            if (opcode == OP_I64_LOAD) {
                val = ((uint64_t)ctx->linear_mem[addr]) |
                      ((uint64_t)ctx->linear_mem[addr + 1] << 8) |
                      ((uint64_t)ctx->linear_mem[addr + 2] << 16) |
                      ((uint64_t)ctx->linear_mem[addr + 3] << 24) |
                      ((uint64_t)ctx->linear_mem[addr + 4] << 32) |
                      ((uint64_t)ctx->linear_mem[addr + 5] << 40) |
                      ((uint64_t)ctx->linear_mem[addr + 6] << 48) |
                      ((uint64_t)ctx->linear_mem[addr + 7] << 56);
            } else if (opcode == OP_I64_LOAD8_U) {
                val = ctx->linear_mem[addr];
            } else if (opcode == OP_I64_LOAD8_S) {
                val = (uint64_t)(int64_t)(int8_t)ctx->linear_mem[addr];
            } else if (opcode == OP_I64_LOAD16_U) {
                val =
                    ((uint64_t)ctx->linear_mem[addr]) | ((uint64_t)ctx->linear_mem[addr + 1] << 8);
            } else if (opcode == OP_I64_LOAD16_S) {
                val = (uint64_t)(int64_t)(int16_t)(((uint32_t)ctx->linear_mem[addr]) |
                                                   ((uint32_t)ctx->linear_mem[addr + 1] << 8));
            } else if (opcode == OP_I64_LOAD32_U) {
                val = ((uint64_t)ctx->linear_mem[addr]) |
                      ((uint64_t)ctx->linear_mem[addr + 1] << 8) |
                      ((uint64_t)ctx->linear_mem[addr + 2] << 16) |
                      ((uint64_t)ctx->linear_mem[addr + 3] << 24);
            } else if (opcode == OP_I64_LOAD32_S) {
                val = (uint64_t)(int64_t)(int32_t)(((uint32_t)ctx->linear_mem[addr]) |
                                                   ((uint32_t)ctx->linear_mem[addr + 1] << 8) |
                                                   ((uint32_t)ctx->linear_mem[addr + 2] << 16) |
                                                   ((uint32_t)ctx->linear_mem[addr + 3] << 24));
            }
            push_stack64(ctx, val);
            break;
        }

        case OP_I32_STORE:
        case OP_I32_STORE8:
        case OP_I32_STORE16: {
            read_u32_leb128(mod->bytes, mod->size, &ctx->pc); /* alignment */
            uint32_t offset = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t val = 0, base_addr = 0;
            if (!pop_stack(ctx, &val) || !pop_stack(ctx, &base_addr))
                break;
            uint64_t addr = (uint64_t)base_addr + offset;

            uint32_t req_bytes = 4;
            if (opcode == OP_I32_STORE8) {
                req_bytes = 1;
            } else if (opcode == OP_I32_STORE16) {
                req_bytes = 2;
            }

            if (!ctx->linear_mem || addr > ctx->linear_mem_size - req_bytes) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }

            if (opcode == OP_I32_STORE) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFF);
                ctx->linear_mem[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
                ctx->linear_mem[addr + 2] = (uint8_t)((val >> 16) & 0xFF);
                ctx->linear_mem[addr + 3] = (uint8_t)((val >> 24) & 0xFF);
            } else if (opcode == OP_I32_STORE8) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFF);
            } else if (opcode == OP_I32_STORE16) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFF);
                ctx->linear_mem[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
            }
            break;
        }

        case OP_I64_STORE:
        case OP_I64_STORE8:
        case OP_I64_STORE16:
        case OP_I64_STORE32: {
            read_u32_leb128(mod->bytes, mod->size, &ctx->pc); /* alignment */
            uint32_t offset = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint64_t val = 0;
            uint32_t base_addr = 0;
            if (!pop_stack64(ctx, &val) || !pop_stack(ctx, &base_addr))
                break;
            uint64_t addr = (uint64_t)base_addr + offset;

            uint32_t req_bytes = 8;
            if (opcode == OP_I64_STORE8)
                req_bytes = 1;
            else if (opcode == OP_I64_STORE16)
                req_bytes = 2;
            else if (opcode == OP_I64_STORE32)
                req_bytes = 4;

            if (!ctx->linear_mem || addr > ctx->linear_mem_size - req_bytes) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }

            if (opcode == OP_I64_STORE) {
                for (int i = 0; i < 8; i++) {
                    ctx->linear_mem[addr + i] = (uint8_t)((val >> (i * 8)) & 0xFF);
                }
            } else if (opcode == OP_I64_STORE8) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFF);
            } else if (opcode == OP_I64_STORE16) {
                ctx->linear_mem[addr] = (uint8_t)(val & 0xFF);
                ctx->linear_mem[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
            } else if (opcode == OP_I64_STORE32) {
                for (int i = 0; i < 4; i++) {
                    ctx->linear_mem[addr + i] = (uint8_t)((val >> (i * 8)) & 0xFF);
                }
            }
            break;
        }
            /* LCOV_EXCL_STOP */

        case OP_I32_CONST: {
            int32_t val = read_i32_leb128(mod->bytes, mod->size, &ctx->pc);
            push_stack(ctx, (uint32_t)val);
            break;
        }

        case OP_I64_CONST: {
            int64_t val = read_i64_leb128(mod->bytes, mod->size, &ctx->pc);
            push_stack64(ctx, (uint64_t)val);
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
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = (uint64_t)((int64_t)a / (int64_t)b);
                break;
            case OP_I64_DIV_U:
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = a / b;
                break;
            case OP_I64_REM_S:
                if (b == 0) {
                    ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                    break;
                }
                res = (uint64_t)((int64_t)a % (int64_t)b);
                break;
            case OP_I64_REM_U:
                if (b == 0) {
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
                res = a << (b % 64);
                break;
            case OP_I64_SHR_S:
                res = (uint64_t)((int64_t)a >> (b % 64));
                break;
            case OP_I64_SHR_U:
                res = a >> (b % 64);
                break;
            }
            push_stack64(ctx, res);
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
            uint32_t val = mod->bytes[ctx->pc] | ((uint32_t)mod->bytes[ctx->pc + 1] << 8) |
                           ((uint32_t)mod->bytes[ctx->pc + 2] << 16) |
                           ((uint32_t)mod->bytes[ctx->pc + 3] << 24);
            ctx->pc += 4;
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
            uint64_t val = 0;
            for (int i = 0; i < 8; i++) {
                val |= ((uint64_t)mod->bytes[ctx->pc + i]) << (i * 8);
            }
            ctx->pc += 8;
            push_stack64(ctx, val);
            break;
        }

        /* LCOV_EXCL_START: Wasm floating-point & bulk memory opcode interpreter fallbacks */
        case OP_F32_LOAD:
        case OP_F64_LOAD: {
            read_u32_leb128(mod->bytes, mod->size, &ctx->pc); /* alignment */
            uint32_t offset = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint32_t base_addr = 0;
            if (!pop_stack(ctx, &base_addr))
                break;
            uint64_t addr = (uint64_t)base_addr + offset;
            uint32_t req = (opcode == OP_F32_LOAD) ? 4 : 8;
            if (!ctx->linear_mem || addr > ctx->linear_mem_size - req) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }
            uint64_t val = 0;
            for (uint32_t i = 0; i < req; i++) {
                val |= ((uint64_t)ctx->linear_mem[addr + i]) << (i * 8);
            }
            push_stack64(ctx, val);
            break;
        }

        case OP_F32_STORE:
        case OP_F64_STORE: {
            read_u32_leb128(mod->bytes, mod->size, &ctx->pc); /* alignment */
            uint32_t offset = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            uint64_t val = 0;
            uint32_t base_addr = 0;
            if (!pop_stack64(ctx, &val) || !pop_stack(ctx, &base_addr))
                break;
            uint64_t addr = (uint64_t)base_addr + offset;
            uint32_t req = (opcode == OP_F32_STORE) ? 4 : 8;
            if (!ctx->linear_mem || addr > ctx->linear_mem_size - req) {
                ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                break;
            }
            for (uint32_t i = 0; i < req; i++) {
                ctx->linear_mem[addr + i] = (uint8_t)((val >> (i * 8)) & 0xFF);
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
            /* Q16.16 Fixed-Point Mode for F32 */
            if (opcode == OP_F32_ABS || opcode == OP_F32_NEG || opcode == OP_F32_CEIL ||
                opcode == OP_F32_FLOOR || opcode == OP_F32_TRUNC || opcode == OP_F32_NEAREST ||
                opcode == OP_F32_SQRT) {
                uint32_t a = 0;
                if (!pop_stack(ctx, &a))
                    break;
                q16_t qa = (q16_t)a;
                q16_t qres = 0;
                if (opcode == OP_F32_ABS)
                    qres = (qa < 0) ? -qa : qa;
                else if (opcode == OP_F32_NEG)
                    qres = -qa;
                else if (opcode == OP_F32_CEIL)
                    qres = q16_ceil(qa);
                else if (opcode == OP_F32_FLOOR)
                    qres = q16_floor(qa);
                else if (opcode == OP_F32_TRUNC || opcode == OP_F32_NEAREST)
                    qres = q16_round(qa);
                else if (opcode == OP_F32_SQRT)
                    qres = q16_sqrt(qa);
                push_stack(ctx, (uint32_t)qres);
            } else {
                uint32_t b = 0, a = 0;
                if (!pop_stack(ctx, &b) || !pop_stack(ctx, &a))
                    break;
                q16_t qa = (q16_t)a, qb = (q16_t)b;
                q16_t qres = 0;
                if (opcode == OP_F32_ADD)
                    qres = q16_add(qa, qb);
                else if (opcode == OP_F32_SUB)
                    qres = q16_sub(qa, qb);
                else if (opcode == OP_F32_MUL)
                    qres = q16_mul(qa, qb);
                else if (opcode == OP_F32_DIV) {
                    if (qb == 0) {
                        ctx->status = SYN_WASM_TRAP_DIV_ZERO;
                        break;
                    }
                    qres = q16_div(qa, qb);
                } else if (opcode == OP_F32_MIN)
                    qres = (qa < qb) ? qa : qb;
                else if (opcode == OP_F32_MAX)
                    qres = (qa > qb) ? qa : qb;
                else if (opcode == OP_F32_COPYSIGN)
                    qres = (qb < 0) ? -q16_abs(qa) : q16_abs(qa);
                push_stack(ctx, (uint32_t)qres);
            }
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

        case OP_I32_TRUNC_F32_S:
        case OP_I32_TRUNC_F32_U: {
            float fa = 0.0f;
            if (!pop_f32(ctx, &fa))
                break;
            uint32_t res = (opcode == OP_I32_TRUNC_F32_S) ? (uint32_t)(int32_t)fa : (uint32_t)fa;
            push_stack(ctx, res);
            break;
        }

        case OP_I32_TRUNC_F64_S:
        case OP_I32_TRUNC_F64_U: {
            double da = 0.0;
            if (!pop_f64(ctx, &da))
                break;
            uint32_t res = (opcode == OP_I32_TRUNC_F64_S) ? (uint32_t)(int32_t)da : (uint32_t)da;
            push_stack(ctx, res);
            break;
        }

        case OP_I64_TRUNC_F32_S:
        case OP_I64_TRUNC_F32_U: {
            float fa = 0.0f;
            if (!pop_f32(ctx, &fa))
                break;
            uint64_t res = (opcode == OP_I64_TRUNC_F32_S) ? (uint64_t)(int64_t)fa : (uint64_t)fa;
            push_stack64(ctx, res);
            break;
        }

        case OP_I64_TRUNC_F64_S:
        case OP_I64_TRUNC_F64_U: {
            double da = 0.0;
            if (!pop_f64(ctx, &da))
                break;
            uint64_t res = (opcode == OP_I64_TRUNC_F64_S) ? (uint64_t)(int64_t)da : (uint64_t)da;
            push_stack64(ctx, res);
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

        case OP_PREFIX_MISC: {
            uint32_t subop = read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            if (subop <= 7) {
                if (subop == 0 || subop == 1) {
                    float f = 0.0f;
                    if (pop_f32(ctx, &f))
                        push_stack(ctx, (uint32_t)(int32_t)f);
                } else if (subop == 2 || subop == 3) {
                    double d = 0.0;
                    if (pop_f64(ctx, &d))
                        push_stack(ctx, (uint32_t)(int32_t)d);
                } else if (subop == 4 || subop == 5) {
                    float f = 0.0f;
                    if (pop_f32(ctx, &f))
                        push_stack64(ctx, (uint64_t)(int64_t)f);
                } else {
                    double d = 0.0;
                    if (pop_f64(ctx, &d))
                        push_stack64(ctx, (uint64_t)(int64_t)d);
                }
            } else if (subop == 8) {
                (void)read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
                (void)read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
                uint32_t n = 0;
                uint32_t src = 0;
                uint32_t dst = 0;
                pop_stack(ctx, &n);
                pop_stack(ctx, &src);
                pop_stack(ctx, &dst);
            } else if (subop == 9) {
                (void)read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
            } else if (subop == 10) {
                (void)read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
                (void)read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
                uint32_t len = 0;
                uint32_t src = 0;
                uint32_t dst = 0;
                if (pop_stack(ctx, &len) && pop_stack(ctx, &src) && pop_stack(ctx, &dst)) {
                    if (ctx->linear_mem && dst + len <= ctx->linear_mem_size &&
                        src + len <= ctx->linear_mem_size) {
                        memmove(&ctx->linear_mem[dst], &ctx->linear_mem[src], len);
                    } else {
                        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                    }
                }
            } else if (subop == 11) {
                (void)read_u32_leb128(mod->bytes, mod->size, &ctx->pc);
                uint32_t len = 0;
                uint32_t val = 0;
                uint32_t dst = 0;
                if (pop_stack(ctx, &len) && pop_stack(ctx, &val) && pop_stack(ctx, &dst)) {
                    if (ctx->linear_mem && dst + len <= ctx->linear_mem_size) {
                        memset(&ctx->linear_mem[dst], (uint8_t)val, len);
                    } else {
                        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
                    }
                }
            } else {
                ctx->status = SYN_WASM_TRAP_BAD_OPCODE;
            }
            break;
        }
            /* LCOV_EXCL_STOP */

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

```


