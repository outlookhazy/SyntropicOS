/**
 * @file test_wasm.c
 * @brief Unit tests for 32-bit WebAssembly (Wasm MVP) cooperative interpreter.
 */

#include "syntropic/vm/syn_wasm.h"
#include "unity/unity.h"

#include <string.h>

static uint8_t g_linear_mem[4096];
static SYN_WASM_Module g_wasm_mod;
static SYN_WASM_Context g_wasm_ctx;

/* Sample valid minimal Wasm module with an export 'add' (i32.add) */
/* Binary generated from WebAssembly text:
 * (module
 *   (func (export "add") (param i32 i32) (result i32)
 *     local.get 0
 *     local.get 1
 *     i32.add)
 * )
 */
static const uint8_t g_wasm_add_binary[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,       /* magic & version */
    0x01, 0x07, 0x01, 0x60, 0x02, 0x7f, 0x7f, 0x01, 0x7f, /* Type: (i32, i32) -> i32 */
    0x03, 0x02, 0x01, 0x00,                               /* Func 0: type 0 */
    0x07, 0x07, 0x01, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, /* Export "add" -> func 0 */
    0x0a, 0x09, 0x01, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01,
    0x6a, 0x0b /* Code: local.get 0, local.get 1, i32.add, end */
};

/* Sample Wasm module with loop counting down from N */
/*
 * (module
 *   (func (export "loop_count") (param i32) (result i32)
 *     (local i32)
 *     local.get 0
 *     local.set 1
 *     block
 *       loop
 *         local.get 1
 *         i32.eqz
 *         br_if 1
 *         local.get 1
 *         i32.const 1
 *         i32.sub
 *         local.set 1
 *         br 0
 *       end
 *     end
 *     local.get 1)
 * )
 */
static const uint8_t g_wasm_loop_binary[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x60, 0x01, 0x7f,
    0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07, 0x0e, 0x01, 0x0a, 0x6c, 0x6f, 0x6f, 0x70,
    0x5f, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x00, 0x00, 0x0a, 0x20, 0x01, 0x1e, 0x01, 0x01,
    0x7f, 0x20, 0x00, 0x21, 0x01, 0x02, 0x40, 0x03, 0x40, 0x20, 0x01, 0x45, 0x0d, 0x01,
    0x20, 0x01, 0x41, 0x01, 0x6b, 0x21, 0x01, 0x0c, 0x00, 0x0b, 0x0b, 0x20, 0x01, 0x0b};

/* Host function callback counter */
static uint32_t g_host_call_count = 0;
static uint64_t mock_host_func(SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc)
{
    (void)ctx;
    (void)args;
    (void)argc;
    g_host_call_count++;
    return 42;
}

static void test_wasm_load_null_and_invalid(void)
{
    TEST_ASSERT_FALSE(syn_wasm_module_load(NULL, g_wasm_add_binary, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, NULL, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, g_wasm_add_binary, 4));

    uint8_t bad_hdr[8] = {0x00, 0x61, 0x73, 0x6d, 0x02, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, bad_hdr, sizeof(bad_hdr)));
}

static void test_wasm_load_and_export_lookup(void)
{
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, g_wasm_add_binary, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_EQUAL_UINT16(1, g_wasm_mod.export_count);

    int32_t add_idx = syn_wasm_find_export(&g_wasm_mod, "add");
    TEST_ASSERT_EQUAL_INT32(0, add_idx);

    int32_t missing_idx = syn_wasm_find_export(&g_wasm_mod, "nonexistent");
    TEST_ASSERT_EQUAL_INT32(-1, missing_idx);
}

static void test_wasm_init_and_execution(void)
{
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, g_wasm_add_binary, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0));

    /* Populate arguments into local variables 0 and 1 */
    g_wasm_ctx.locals[0] = 15;
    g_wasm_ctx.locals[1] = 27;

    SYN_WASM_Status st = syn_wasm_step(&g_wasm_ctx, 100);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, st);
    TEST_ASSERT_EQUAL_UINT32(42, syn_wasm_result(&g_wasm_ctx));
}

static void test_wasm_loop_and_yielding(void)
{
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, g_wasm_loop_binary, sizeof(g_wasm_loop_binary)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0));
    g_wasm_ctx.locals[0] = 10; /* Loop 10 iterations */

    /* Step with small instruction budget (5 opcodes) -> Expect YIELDED */
    SYN_WASM_Status st = syn_wasm_step(&g_wasm_ctx, 5);
    TEST_ASSERT_EQUAL(SYN_WASM_YIELDED, st);

    /* Finish remaining execution */
    st = syn_wasm_step(&g_wasm_ctx, 1000);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, st);
    TEST_ASSERT_EQUAL_UINT32(0, syn_wasm_result(&g_wasm_ctx));
}

static void test_wasm_host_function_registration(void)
{
    g_host_call_count = 0;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0, mock_host_func));

    /* Manually invoke host func registration slot 0 */
    uint32_t ret = g_wasm_ctx.host_funcs[0](&g_wasm_ctx, NULL, 0);
    TEST_ASSERT_EQUAL_UINT32(42, ret);
    TEST_ASSERT_EQUAL_UINT32(1, g_host_call_count);
}

static void test_wasm_float_and_bulk_memory(void)
{
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    memset(g_linear_mem, 0, sizeof(g_linear_mem));
    memcpy(&g_linear_mem[100], "WORLD", 5);

    /* Test 0xFC subop 10 memory.copy in linear_mem */
    g_wasm_ctx.sp = 0;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 0;   /* dst = 0 */
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 100; /* src = 100 */
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 5;   /* len = 5 */

    static const uint8_t code_copy[] = {0xFC, 0x0A, 0x00, 0x00, 0x0B};
    SYN_WASM_Module mod_copy;
    memset(&mod_copy, 0, sizeof(mod_copy));
    mod_copy.bytes = code_copy;
    mod_copy.size = sizeof(code_copy);
    mod_copy.func_count = 1;
    mod_copy.funcs[0].code_offset = 0;
    mod_copy.funcs[0].code_size = sizeof(code_copy);

    g_wasm_ctx.module = &mod_copy;
    g_wasm_ctx.call_depth = 1;
    g_wasm_ctx.call_stack[0].func_idx = 0;
    g_wasm_ctx.call_stack[0].return_pc = 0;
    g_wasm_ctx.pc = 0;
    g_wasm_ctx.status = SYN_WASM_OK;

    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 100));
    TEST_ASSERT_EQUAL_MEMORY("WORLD", g_linear_mem, 5);
}

static void test_wasm_fixed_point_float_ops(void)
{
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    /* Test F32 opcodes in bytecode: f32.const 3.0f, f32.const 4.0f, f32.add */
    static const uint8_t code_f32_add[] = {0x43, 0x00, 0x00, 0x40, 0x40, 0x43,
                                           0x00, 0x00, 0x80, 0x40, 0x92, 0x0B};
    SYN_WASM_Module mod_add;
    memset(&mod_add, 0, sizeof(mod_add));
    mod_add.bytes = code_f32_add;
    mod_add.size = sizeof(code_f32_add);
    mod_add.func_count = 1;
    mod_add.funcs[0].code_offset = 0;
    mod_add.funcs[0].code_size = sizeof(code_f32_add);
    mod_add.funcs[0].result_count = 1U;

    g_wasm_ctx.module = &mod_add;
    g_wasm_ctx.call_depth = 1;
    g_wasm_ctx.call_stack[0].func_idx = 0;
    g_wasm_ctx.call_stack[0].return_pc = 0;
    g_wasm_ctx.pc = 0;
    g_wasm_ctx.sp = 0;
    g_wasm_ctx.status = SYN_WASM_OK;

    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 100));
#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
    /* 3.0 + 4.0 = 7.0 in Q16.16 (7 * 65536 = 458752) */
    TEST_ASSERT_EQUAL_UINT32(458752, syn_wasm_result(&g_wasm_ctx));
#else
    /* 3.0f + 4.0f = 7.0f in IEEE 754 (0x40e00000) */
    TEST_ASSERT_EQUAL_HEX32(0x40e00000, (uint32_t)syn_wasm_result(&g_wasm_ctx));
#endif
}

#include "wasm/test_arithmetic.wasm.h"
#include "wasm/test_control_flow.wasm.h"
#include "wasm/test_conversions.wasm.h"
#include "wasm/test_embedded.wasm.h"
#include "wasm/test_functions.wasm.h"
#include "wasm/test_host_api.wasm.h"
#include "wasm/test_memory.wasm.h"
#include "wasm/test_recursion.wasm.h"
#include "wasm/test_structs.wasm.h"
#include "wasm/test_traps.wasm.h"

static uint32_t g_host_log_called = 0;
static uint32_t g_host_log_code = 0;
static uint32_t g_host_log_val = 0;

static uint64_t mock_host_log(SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc)
{
    (void)ctx;
    (void)argc;
    g_host_log_called++;
    g_host_log_code = (uint32_t)args[0];
    g_host_log_val = (uint32_t)args[1];
    return g_host_log_code + g_host_log_val;
}

static uint32_t run_wasm_fixture(const uint8_t *bytes, uint32_t size)
{
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, bytes, size));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    int32_t fn = syn_wasm_find_export(&g_wasm_mod, "run_tests");
    TEST_ASSERT_TRUE(fn >= 0);

    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)fn));
    SYN_WASM_Status st = syn_wasm_step(&g_wasm_ctx, 10000);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, st);
    return (uint32_t)syn_wasm_result(&g_wasm_ctx);
}

static void test_wasm_fixture_arithmetic(void)
{
    uint32_t res = run_wasm_fixture(test_arithmetic_wasm, test_arithmetic_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_arithmetic failed at case #");
}

static void test_wasm_fixture_memory(void)
{
    uint32_t res = run_wasm_fixture(test_memory_wasm, test_memory_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_memory failed at case #");
}

static void test_wasm_fixture_functions(void)
{
    uint32_t res = run_wasm_fixture(test_functions_wasm, test_functions_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_functions failed at case #");
}

static void test_wasm_fixture_structs(void)
{
    uint32_t res = run_wasm_fixture(test_structs_wasm, test_structs_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_structs failed at case #");
}

static void test_wasm_fixture_control_flow(void)
{
    uint32_t res = run_wasm_fixture(test_control_flow_wasm, test_control_flow_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_control_flow failed at case #");
}

static void test_wasm_fixture_conversions(void)
{
    uint32_t res = run_wasm_fixture(test_conversions_wasm, test_conversions_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_conversions failed at case #");
}

static void test_wasm_fixture_recursion(void)
{
    uint32_t res = run_wasm_fixture(test_recursion_wasm, test_recursion_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_recursion failed at case #");
}

static void test_wasm_fixture_embedded(void)
{
    uint32_t res = run_wasm_fixture(test_embedded_wasm, test_embedded_wasm_len);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, res, "WASM test_embedded failed at case #");
}

static void test_wasm_fixture_host_api(void)
{
    g_host_log_called = 0;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, test_host_api_wasm, test_host_api_wasm_len));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0, mock_host_log));

    int32_t fn = syn_wasm_find_export(&g_wasm_mod, "run_tests");
    TEST_ASSERT_TRUE(fn >= 0);

    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)fn));
    SYN_WASM_Status st = syn_wasm_step(&g_wasm_ctx, 10000);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, st);
    TEST_ASSERT_EQUAL_UINT32(0, syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL_UINT32(1, g_host_log_called);
    TEST_ASSERT_EQUAL_UINT32(42, g_host_log_code);
    TEST_ASSERT_EQUAL_UINT32(150, g_host_log_val);
}

static void test_wasm_fixture_traps(void)
{
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, test_traps_wasm, test_traps_wasm_len));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    /* 1. Divide by zero trap */
    int32_t fn_div = syn_wasm_find_export(&g_wasm_mod, "trigger_div_zero");
    TEST_ASSERT_TRUE(fn_div >= 0);
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)fn_div));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, syn_wasm_step(&g_wasm_ctx, 1000));

    /* 2. Out of bounds memory access trap */
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    int32_t fn_oob = syn_wasm_find_export(&g_wasm_mod, "trigger_oob_read");
    TEST_ASSERT_TRUE(fn_oob >= 0);
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)fn_oob));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, syn_wasm_step(&g_wasm_ctx, 1000));

    /* 3. Direct i32.div_s signed overflow trap (INT32_MIN / -1) */
    static const uint8_t code_div_overflow[] = {
        0x41, 0x80, 0x80, 0x80, 0x80, 0x78, /* i32.const -2147483648 (INT32_MIN) */
        0x41, 0x7F,                         /* i32.const -1 */
        0x6D,                               /* i32.div_s */
        0x0B                                /* end */
    };
    SYN_WASM_Module mod_div_ovf;
    memset(&mod_div_ovf, 0, sizeof(mod_div_ovf));
    mod_div_ovf.bytes = code_div_overflow;
    mod_div_ovf.size = sizeof(code_div_overflow);
    mod_div_ovf.func_count = 1;
    mod_div_ovf.funcs[0].code_offset = 0;
    mod_div_ovf.funcs[0].code_size = sizeof(code_div_overflow);

    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod_div_ovf, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1;
    g_wasm_ctx.call_stack[0].func_idx = 0;
    g_wasm_ctx.call_stack[0].return_pc = 0;
    g_wasm_ctx.pc = 0;
    g_wasm_ctx.sp = 0;
    g_wasm_ctx.status = SYN_WASM_OK;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, syn_wasm_step(&g_wasm_ctx, 100));
}

static void test_wasm_fixture_yield_resume(void)
{
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, test_control_flow_wasm, test_control_flow_wasm_len));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    int32_t fn = syn_wasm_find_export(&g_wasm_mod, "run_tests");
    TEST_ASSERT_TRUE(fn >= 0);

    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)fn));

    /* Step 3 instructions -> Expect YIELDED */
    SYN_WASM_Status st = syn_wasm_step(&g_wasm_ctx, 3);
    TEST_ASSERT_EQUAL(SYN_WASM_YIELDED, st);

    /* Resume execution -> Expect HALTED */
    st = syn_wasm_step(&g_wasm_ctx, 10000);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, st);
    TEST_ASSERT_EQUAL_UINT32(0, syn_wasm_result(&g_wasm_ctx));
}

static void test_wasm_extended_traps_and_nulls(void)
{
    SYN_WASM_Module mod;
    SYN_WASM_Context ctx;
    uint8_t mem[1024];

    /* Null & boundary API calls */
    TEST_ASSERT_FALSE(syn_wasm_module_load(NULL, g_wasm_add_binary, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, NULL, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, g_wasm_add_binary, 0));
    TEST_ASSERT_FALSE(syn_wasm_init(NULL, &g_wasm_mod, mem, sizeof(mem)));
    TEST_ASSERT_FALSE(syn_wasm_init(&ctx, NULL, mem, sizeof(mem)));
    TEST_ASSERT_TRUE(syn_wasm_init(&ctx, &g_wasm_mod, NULL, 0));
    TEST_ASSERT_FALSE(syn_wasm_register_host(NULL, 0, mock_host_log));
    TEST_ASSERT_FALSE(syn_wasm_register_host(&ctx, 999, mock_host_log));
    TEST_ASSERT_EQUAL(-1, syn_wasm_find_export(NULL, "add"));
    TEST_ASSERT_EQUAL(-1, syn_wasm_find_export(&g_wasm_mod, NULL));
    TEST_ASSERT_EQUAL(-1, syn_wasm_find_export(&g_wasm_mod, "non_existent"));
    TEST_ASSERT_FALSE(syn_wasm_call(NULL, 0));
    TEST_ASSERT_FALSE(syn_wasm_call(&ctx, 9999));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, syn_wasm_step(NULL, 10));
    TEST_ASSERT_EQUAL_UINT64(0, syn_wasm_result(NULL));

    /* Invalid Magic Header check */
    uint8_t bad_magic[8] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, bad_magic, sizeof(bad_magic)));

    /* Load valid minimal module and test step traps */
    SYN_WASM_Module test_mod;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&test_mod, g_wasm_add_binary, sizeof(g_wasm_add_binary)));
    TEST_ASSERT_TRUE(syn_wasm_init(&ctx, &test_mod, mem, sizeof(mem)));
    TEST_ASSERT_TRUE(syn_wasm_call(&ctx, 0));
    syn_wasm_step(&ctx, 10);

    /* Test step on non-OK status returns current status immediately */
    ctx.status = SYN_WASM_HALTED;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&ctx, 10));

    /* Test register_host at max host func index boundary */
    TEST_ASSERT_TRUE(syn_wasm_register_host(&ctx, SYN_WASM_MAX_HOST_FUNCS - 1, mock_host_func));
}

static void test_wasm_native_f64_ops(void)
{
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    /* Bytecode: i64.const 42, f64.const 2.0, f64.abs, end */
    double v1 = -2.0;
    uint8_t code_f64[30];
    code_f64[0] = 0x42; /* OP_I64_CONST */
    code_f64[1] = 42;   /* LEB128 42 */
    code_f64[2] = 0x44; /* OP_F64_CONST */
    memcpy(&code_f64[3], &v1, 8);
    code_f64[11] = 0x99; /* OP_F64_ABS */
    code_f64[12] = 0x0B; /* OP_END */

    SYN_WASM_Module mod_f64;
    memset(&mod_f64, 0, sizeof(mod_f64));
    mod_f64.bytes = code_f64;
    mod_f64.size = 13;
    mod_f64.func_count = 1;
    mod_f64.funcs[0].code_offset = 0;
    mod_f64.funcs[0].code_size = 13;

    g_wasm_ctx.module = &mod_f64;
    g_wasm_ctx.call_depth = 1;
    g_wasm_ctx.call_stack[0].func_idx = 0;
    g_wasm_ctx.call_stack[0].return_pc = 0;
    g_wasm_ctx.pc = 0;
    g_wasm_ctx.sp = 0;
    g_wasm_ctx.status = SYN_WASM_OK;

    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 100));
}

static void test_wasm_opcodes_branch_coverage(void)
{
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    /* Bytecode: i32.const 10, i32.const 20, i32.const 0, select, end (selects 2nd operand 20 when
     * cond=0) */
    static const uint8_t code_select_false[] = {0x41, 0x0A, 0x41, 0x14, 0x41, 0x00, 0x1B, 0x0B};
    SYN_WASM_Module mod_select;
    memset(&mod_select, 0, sizeof(mod_select));
    mod_select.bytes = code_select_false;
    mod_select.size = sizeof(code_select_false);
    mod_select.func_count = 1;
    mod_select.funcs[0].code_offset = 0;
    mod_select.funcs[0].code_size = sizeof(code_select_false);
    mod_select.funcs[0].result_count = 1U;

    g_wasm_ctx.module = &mod_select;
    g_wasm_ctx.call_depth = 1;
    g_wasm_ctx.call_stack[0].func_idx = 0;
    g_wasm_ctx.call_stack[0].return_pc = 0;
    g_wasm_ctx.pc = 0;
    g_wasm_ctx.sp = 0;
    g_wasm_ctx.status = SYN_WASM_OK;

    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 100));
    TEST_ASSERT_EQUAL_UINT32(20, syn_wasm_result(&g_wasm_ctx));

    /* Test local.tee and global.set/get */
    static const uint8_t code_globals[] = {0x41, 0x2A, 0x24, 0x00, 0x23, 0x00, 0x22, 0x01, 0x0B};
    SYN_WASM_Module mod_glob;
    memset(&mod_glob, 0, sizeof(mod_glob));
    mod_glob.bytes = code_globals;
    mod_glob.size = sizeof(code_globals);
    mod_glob.func_count = 1;
    mod_glob.funcs[0].code_offset = 0;
    mod_glob.funcs[0].code_size = sizeof(code_globals);

    g_wasm_ctx.module = &mod_glob;
    g_wasm_ctx.call_depth = 1;
    g_wasm_ctx.call_stack[0].func_idx = 0;
    g_wasm_ctx.call_stack[0].return_pc = 0;
    g_wasm_ctx.call_stack[0].local_base = 0;
    g_wasm_ctx.local_count = SYN_WASM_MAX_LOCALS;
    g_wasm_ctx.pc = 0;
    g_wasm_ctx.sp = 0;
    g_wasm_ctx.status = SYN_WASM_OK;

    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 100));
    TEST_ASSERT_EQUAL_UINT32(42, g_wasm_ctx.globals[0]);
    TEST_ASSERT_EQUAL_UINT32(42, g_wasm_ctx.locals[1]);

    /* Test Table (kind 1), Memory (kind 2), Global (kind 3) imports and Element section (sec 9) */
    static const uint8_t wasm_extra_sections[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, /* magic & version */
        0x01, 0x04, 0x01, 0x60, 0x00, 0x00,             /* Type: () -> () */
        0x02, 0x26, 0x03, /* Import Section (3 entries, 38 bytes payload) */
        0x03, 'e',  'n',  'v',  0x05, 't',  'a',  'b',  'l',  'e',
        0x01, 0x70, 0x00, /* Table import: funcref, min 0 */
        0x03, 'e',  'n',  'v',  0x03, 'm',  'e',  'm',  0x02, 0x01,
        0x01, 0x02, /* Memory import: flags=1, min 1, max 2 */
        0x03, 'e',  'n',  'v',  0x04, 'g',  'l',  'o',  'b',  0x03,
        0x7f, 0x00,                                           /* Global import: i32, const */
        0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01, 0x00, /* Element section: table 0, offset
                                                                 i32.const 0, func 0 */
    };
    SYN_WASM_Module extra_mod;
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&extra_mod, wasm_extra_sections, sizeof(wasm_extra_sections)));
    TEST_ASSERT_EQUAL_UINT16(1, extra_mod.table_element_count);
}

static SYN_WASM_Status run_raw_snippet_ex(const uint8_t *code, uint32_t size, uint8_t *mem,
                                          uint32_t mem_size)
{
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = code;
    g_wasm_mod.size = size;
    g_wasm_mod.func_count = 1U;
    g_wasm_mod.funcs[0].code_offset = 0U;
    g_wasm_mod.funcs[0].code_size = size;
    g_wasm_mod.funcs[0].result_count = 1U;

    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, mem, mem_size));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.call_stack[0].func_idx = 0U;
    g_wasm_ctx.pc = 0U;
    g_wasm_ctx.sp = 0U;
    return syn_wasm_step(&g_wasm_ctx, 500U);
}

static SYN_WASM_Status run_raw_snippet(const uint8_t *code, uint32_t size)
{
    return run_raw_snippet_ex(code, size, g_linear_mem, sizeof(g_linear_mem));
}

static void test_r5_wasm_module_and_data_bounds(void)
{
    /* 1. Section length overflow in syn_wasm_module_load */
    static const uint8_t bad_sec_len[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                          0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, bad_sec_len, sizeof(bad_sec_len)));

    /* 2. Malformed import mod_len & field_len */
    static const uint8_t bad_imp_mod[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                          0x00, 0x02, 0x03, 0x01, 0x10, 'a'};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, bad_imp_mod, sizeof(bad_imp_mod)));

    static const uint8_t bad_imp_fld[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                          0x00, 0x02, 0x04, 0x01, 0x01, 'a',  0x05};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, bad_imp_fld, sizeof(bad_imp_fld)));

    /* 3. Malformed export name_len & code body_size */
    static const uint8_t bad_exp[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                      0x00, 0x07, 0x03, 0x01, 0x05, 'x'};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, bad_exp, sizeof(bad_exp)));

    static const uint8_t bad_code_sz[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                          0x00, 0x0A, 0x03, 0x01, 0x10, 0x0B};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&g_wasm_mod, bad_code_sz, sizeof(bad_code_sz)));

    /* 4. Negative data_offset (-4 = 0x7C) in DATA section must trap OUT_OF_BOUNDS */
    static const uint8_t neg_data_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x0B,
                                            0x07, 0x01, 0x00, 0x41, 0x7C, 0x0B, 0x01, 0xAA};
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, neg_data_wasm, sizeof(neg_data_wasm)));
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, g_wasm_ctx.status);

    /* 5. Positive data_offset + data_size > mem_size must trap OUT_OF_BOUNDS */
    static const uint8_t oob_data_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                            0x00, 0x0B, 0x09, 0x01, 0x00, 0x41, 0x06,
                                            0x0B, 0x03, 'A',  'B',  'C'};
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, oob_data_wasm, sizeof(oob_data_wasm)));
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 8U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, g_wasm_ctx.status);

    /* Valid fit in 16 bytes */
    memset(g_linear_mem, 0, 16U);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL_MEMORY("ABC", &g_linear_mem[6], 3U);

    /* 6. Truncated data segment payload & corrupt init section_size & truncated global */
    static const uint8_t trunc_data_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x0B,
                                              0x07, 0x01, 0x00, 0x41, 0x00, 0x0B, 0x09, 'A'};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = trunc_data_wasm;
    g_wasm_mod.size = sizeof(trunc_data_wasm);
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);

    g_wasm_mod.bytes = bad_sec_len;
    g_wasm_mod.size = sizeof(bad_sec_len);
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);

    static const uint8_t trunc_glob_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00,
                                              0x00, 0x00, 0x06, 0x02, 0x01, 0x7F};
    g_wasm_mod.bytes = trunc_glob_wasm;
    g_wasm_mod.size = sizeof(trunc_glob_wasm);
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);
}

static void test_r5_wasm_if_else_return_and_locals(void)
{
    /*
     * Module with 3 functions:
     *   func 0 (caller): () -> i32, 12 i32 locals (0..11).
     *     Sets local 10 = 77, calls func 1(5) (which has 4 locals, must NOT clobber local 10!),
     *     adds local 10 (77 + 50 = 127).
     *   func 1 (callee): (param i32) -> i32, 3 i32 locals (total 4).
     *     Writes 999 to local 2, checks local 3 == 0 (zero-initialized), enters block+loop,
     *     pushes junk 888, pushes param*10 + local3, executes OP_RETURN (unwinds sp & labels!).
     *   func 2 (if_else_br): (param i32) -> i32
     *     if (param) { 11 } else { i32.const 33; br 0; i32.const 99 } -> returns 33 when param==0!
     */
    static const uint8_t mod_bin[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
        /* Type section (13 bytes): 0: ()->i32, 1: (i32)->i32, 2: ()->() */
        0x01, 0x0D, 0x03, 0x60, 0x00, 0x01, 0x7F, 0x60, 0x01, 0x7F, 0x01, 0x7F, 0x60, 0x00, 0x00,
        /* Function section: f0:t0, f1:t1, f2:t1, f3:t2 */
        0x03, 0x05, 0x04, 0x00, 0x01, 0x01, 0x02,
        /* Element section: table[0] = f1 */
        0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0B, 0x01, 0x01,
        /* Code section (70 = 0x46 bytes, 4 bodies) */
        0x0A, 0x46, 0x04,
        /* f0 body (18 = 0x12 bytes): 12 locals; l10=77; f3(); f1(5); l10 + res */
        0x12, 0x01, 0x0C, 0x7F, 0x41, 0xCD, 0x00, 0x21, 0x0A, 0x10, 0x03, 0x41, 0x05, 0x10, 0x01,
        0x20, 0x0A, 0x6A, 0x0B,
        /* f1 body (26 = 0x1A bytes): 3 locals; l2=999; block { loop { push 88; l0*10+l3; return } }
         */
        0x1A, 0x01, 0x03, 0x7F, 0x41, 0xE7, 0x07, 0x21, 0x02, 0x02, 0x40, 0x03, 0x40, 0x41, 0x58,
        0x20, 0x00, 0x41, 0x0A, 0x6C, 0x20, 0x03, 0x6A, 0x0F, 0x0B, 0x0B, 0x0B,
        /* f2 body (16 = 0x10 bytes): 0 locals; l0; if(result i32) { 11 } else { 33; br 0; 99 } end
         */
        0x10, 0x00, 0x20, 0x00, 0x04, 0x7F, 0x41, 0x0B, 0x05, 0x41, 0x21, 0x0C, 0x00, 0x41, 0x63,
        0x0B, 0x0B,
        /* f3 body (5 = 0x05 bytes): void func pushing junk 7 and returning */
        0x05, 0x00, 0x41, 0x07, 0x0F, 0x0B};

    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, mod_bin, sizeof(mod_bin)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    /* Run f0: verifies +8 local non-clobber, void return sp unwind, and OP_RETURN unwind */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 500U));
    TEST_ASSERT_EQUAL_UINT32(127U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL_UINT32(1U, g_wasm_ctx.sp);
    TEST_ASSERT_EQUAL_UINT8(0U, g_wasm_ctx.label_depth);

    /* Run f2 with param=0 (false jump into OP_ELSE with br 0) */
    g_wasm_ctx.sp = 0U;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 0ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 500U));
    TEST_ASSERT_EQUAL_UINT32(33U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Run f2 with param=1 (true branch hitting OP_ELSE skip) */
    g_wasm_ctx.sp = 0U;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 1ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 500U));
    TEST_ASSERT_EQUAL_UINT32(11U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Normal OP_ELSE fallthrough to OP_END without br 0 */
    static const uint8_t else_end[] = {0x41, 0x00, 0x04, 0x7F, 0x41, 0x01,
                                       0x05, 0x41, 0x2A, 0x0B, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(else_end, sizeof(else_end)));
    TEST_ASSERT_EQUAL_UINT32(42U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* call_indirect valid & OOB */
    static const uint8_t ci_ok[] = {0x41, 0x06, 0x41, 0x00, 0x11, 0x01, 0x00, 0x0B};
    uint8_t combo[sizeof(mod_bin) + sizeof(ci_ok)];
    memcpy(combo, mod_bin, sizeof(mod_bin));
    memcpy(&combo[sizeof(mod_bin)], ci_ok, sizeof(ci_ok));
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, mod_bin, sizeof(mod_bin)));
    g_wasm_mod.bytes = combo;
    g_wasm_mod.size = (uint32_t)sizeof(combo);
    g_wasm_mod.funcs[0].code_offset = (uint32_t)sizeof(mod_bin);
    g_wasm_mod.funcs[0].code_size = (uint32_t)sizeof(ci_ok);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.call_stack[0].func_idx = 0U;
    g_wasm_ctx.local_count = 4U;
    g_wasm_ctx.pc = (uint32_t)sizeof(mod_bin);
    g_wasm_ctx.sp = 0U;
    g_wasm_ctx.status = SYN_WASM_OK;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 500U));
    TEST_ASSERT_EQUAL_UINT32(60U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* call_indirect OOB & truncated/underflow */
    static const uint8_t ci_oob[] = {0x41, 0x09, 0x11, 0x01, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, run_raw_snippet(ci_oob, sizeof(ci_oob)));
    static const uint8_t ci_tr[] = {0x11, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(ci_tr, sizeof(ci_tr)));
    static const uint8_t call_tr[] = {0x10};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(call_tr, sizeof(call_tr)));
    static const uint8_t br_tr[] = {0x0C};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(br_tr, sizeof(br_tr)));
    static const uint8_t brif_tr[] = {0x0D};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(brif_tr, sizeof(brif_tr)));
    static const uint8_t ms_tr[] = {0x3F};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(ms_tr, sizeof(ms_tr)));
    static const uint8_t mg_tr[] = {0x40};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(mg_tr, sizeof(mg_tr)));
    static const uint8_t else_tr[] = {0x41, 0x01, 0x04, 0x40, 0x05};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(else_tr, sizeof(else_tr)));
}

static void test_r5_wasm_memory_and_bytecode_bounds(void)
{
    /* 1. Linear memory size < req_bytes (mem_size = 2, i32.load needs 4: must NOT underflow!) */
    static const uint8_t load32_zero[] = {0x41, 0x00, 0x28, 0x02, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS,
                      run_raw_snippet_ex(load32_zero, sizeof(load32_zero), g_linear_mem, 2U));

    static const uint8_t store32_zero[] = {0x41, 0x00, 0x41, 0x01, 0x36, 0x02, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS,
                      run_raw_snippet_ex(store32_zero, sizeof(store32_zero), g_linear_mem, 2U));

    /* 2. base_addr + offset 32-bit wraparound on i32.load (0xFFFFFFFC + 8) */
    static const uint8_t load_wrap[] = {0x41, 0x7C, 0x28, 0x02, 0x08, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, run_raw_snippet(load_wrap, sizeof(load_wrap)));

    /* 3. memory.copy & memory.fill 32-bit wraparound (dst = 0xFFFFFFFE = -2, len = 8) */
    static const uint8_t mcopy_wrap[] = {0x41, 0x7E, 0x41, 0x00, 0x41, 0x08,
                                         0xFC, 0x0A, 0x00, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, run_raw_snippet(mcopy_wrap, sizeof(mcopy_wrap)));

    static const uint8_t mfill_wrap[] = {0x41, 0x7E, 0x41, 0x2A, 0x41,
                                         0x08, 0xFC, 0x0B, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, run_raw_snippet(mfill_wrap, sizeof(mfill_wrap)));

    /* Valid memory.fill, memory.init (subop 8), data.drop (subop 9), memory.size & memory.grow */
    static const uint8_t mfill_ok[] = {0x41, 0x10, 0x41, 0x3A, 0x41, 0x04, 0xFC, 0x0B, 0x00,
                                       0x41, 0x00, 0x41, 0x00, 0x41, 0x00, 0xFC, 0x08, 0x00,
                                       0x00, 0xFC, 0x09, 0x00, 0x3F, 0x00, 0x41, 0x00, 0x40,
                                       0x00, 0x41, 0x01, 0x40, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(mfill_ok, sizeof(mfill_ok)));
    TEST_ASSERT_EQUAL_HEX8(0x3AU, g_linear_mem[0x10]);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* 4. Truncated & invalid LEB128 / immediate reads at code_end */
    static const uint8_t empty_code[] = {0x01};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = empty_code;
    g_wasm_mod.size = 0U;
    g_wasm_mod.func_count = 1U;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    g_wasm_ctx.call_depth = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, syn_wasm_step(&g_wasm_ctx, 10U));

    static const uint8_t trunc_u32[] = {0x20, 0x80};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_u32, sizeof(trunc_u32)));

    static const uint8_t ovf_u32[] = {0x20, 0x80, 0x80, 0x80, 0x80, 0x10};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(ovf_u32, sizeof(ovf_u32)));

    static const uint8_t trunc_i32[] = {0x41, 0x80};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_i32, sizeof(trunc_i32)));

    static const uint8_t ovf_i32[] = {0x41, 0x80, 0x80, 0x80, 0x80, 0x80};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(ovf_i32, sizeof(ovf_i32)));

    static const uint8_t trunc_i64[] = {0x42, 0x80};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_i64, sizeof(trunc_i64)));

    static const uint8_t ovf_i64[] = {0x42, 0x80, 0x80, 0x80, 0x80, 0x80,
                                      0x80, 0x80, 0x80, 0x80, 0x80};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(ovf_i64, sizeof(ovf_i64)));

    static const uint8_t trunc_f32[] = {0x43, 0x00, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_f32, sizeof(trunc_f32)));

    static const uint8_t trunc_f64[] = {0x44, 0x00, 0x00, 0x00, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_f64, sizeof(trunc_f64)));

    static const uint8_t trunc_memarg[] = {0x41, 0x00, 0x28, 0x02};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(trunc_memarg, sizeof(trunc_memarg)));

    static const uint8_t trunc_fc[] = {0xFC};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_fc, sizeof(trunc_fc)));

    static const uint8_t bad_fc[] = {0xFC, 0x63};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_BAD_OPCODE, run_raw_snippet(bad_fc, sizeof(bad_fc)));

    static const uint8_t trunc_fc8[] = {0xFC, 0x08, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(trunc_fc8, sizeof(trunc_fc8)));

    static const uint8_t trunc_fc10[] = {0xFC, 0x0A, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(trunc_fc10, sizeof(trunc_fc10)));

    static const uint8_t trunc_fc11[] = {0xFC, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(trunc_fc11, sizeof(trunc_fc11)));
}

static void test_r5_wasm_skip_and_control_edge_cases(void)
{
    /* Skipped if(0) block exercising every immediate class in wasm_skip_immediates_checked */
    static const uint8_t skip_all[] = {0x41, 0x00, 0x04, 0x40, 0x3F, 0x00, /* memory.size */
                                       0x41, 0x01,                         /* i32.const */
                                       0x42, 0x7F,                         /* i64.const -1 */
                                       0x43, 0x00, 0x00, 0x80, 0x3F,       /* f32.const */
                                       0x44, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0xF0, 0x3F,       /* f64.const */
                                       0x0E, 0x01, 0x00, 0x00, /* br_table */
                                       0xFC, 0x00,             /* misc 0 */
                                       0xFC, 0x08, 0x00, 0x00, /* misc 8 */
                                       0xFC, 0x09, 0x00,       /* misc 9 */
                                       0x20, 0x00,             /* local.get (1 leb) */
                                       0x28, 0x02, 0x00,       /* i32.load (2 lebs) */
                                       0x01,                   /* nop (0 lebs) */
                                       0x02, 0x40, 0x0B,       /* nested block */
                                       0x0B, 0x41, 0x37, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(skip_all, sizeof(skip_all)));
    TEST_ASSERT_EQUAL_UINT32(55U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Truncated skipped instructions inside if(0) */
    static const uint8_t sk_t1[] = {0x41, 0x00, 0x04, 0x40};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t1, sizeof(sk_t1)));

    static const uint8_t sk_t2[] = {0x41, 0x00, 0x04, 0x40, 0x02};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t2, sizeof(sk_t2)));

    static const uint8_t sk_t3[] = {0x41, 0x00, 0x04, 0x40, 0x43, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t3, sizeof(sk_t3)));

    static const uint8_t sk_t4[] = {0x41, 0x00, 0x04, 0x40, 0x0E};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t4, sizeof(sk_t4)));

    static const uint8_t sk_t5[] = {0x41, 0x00, 0x04, 0x40, 0x0E, 0x01, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t5, sizeof(sk_t5)));

    static const uint8_t sk_t6[] = {0x41, 0x00, 0x04, 0x40, 0xFC};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t6, sizeof(sk_t6)));

    static const uint8_t sk_t7[] = {0x41, 0x00, 0x04, 0x40, 0xFC, 0x09};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t7, sizeof(sk_t7)));

    static const uint8_t sk_t8[] = {0x41, 0x00, 0x04, 0x40, 0x20};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(sk_t8, sizeof(sk_t8)));

    /* Control edge cases: truncated block, IF underflow, stray ELSE, root BR & OOB BR, label
     * overflow */
    static const uint8_t tr_blk[] = {0x02};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(tr_blk, sizeof(tr_blk)));

    static const uint8_t if_uf[] = {0x04, 0x40, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(if_uf, sizeof(if_uf)));

    static const uint8_t stray_else[] = {0x05};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(stray_else, sizeof(stray_else)));

    static const uint8_t root_br[] = {0x41, 0x19, 0x0C, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(root_br, sizeof(root_br)));
    TEST_ASSERT_EQUAL_UINT32(25U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t oob_br[] = {0x0C, 0x05};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(oob_br, sizeof(oob_br)));

    /* 17 nested blocks -> label_stack overflow */
    uint8_t deep_blk[36];
    for (uint32_t i = 0U; i < 17U; i++) {
        deep_blk[i * 2U] = 0x02U;
        deep_blk[i * 2U + 1U] = 0x40U;
    }
    deep_blk[34] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_CALL_STACK_OVERFLOW, run_raw_snippet(deep_blk, 35U));

    /* br_table valid (hit + default), underflow, and truncated */
    static const uint8_t brt_hit[] = {0x02, 0x7F, 0x02, 0x40, 0x41, 0x00, 0x0E, 0x01, 0x00,
                                      0x01, 0x0B, 0x41, 0x09, 0x0C, 0x00, 0x0B, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(brt_hit, sizeof(brt_hit)));
    TEST_ASSERT_EQUAL_UINT32(9U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Branch stack unwinding: typed block (result i32) unwinds extra operands; empty br traps */
    static const uint8_t br_unwind_typed[] = {0x02, 0x7F, 0x41, 0x77, 0x41, 0xCD, 0x00,
                                              0x0C, 0x00, 0x0B, 0x41, 0x0A, 0x6A, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(br_unwind_typed, sizeof(br_unwind_typed)));
    TEST_ASSERT_EQUAL_UINT32(87U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL_UINT32(1U, g_wasm_ctx.sp);

    static const uint8_t br_typed_uf[] = {0x02, 0x7F, 0x0C, 0x00, 0x0B, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW,
                      run_raw_snippet(br_typed_uf, sizeof(br_typed_uf)));

    /* Loop backward branch unwinds stack each iteration (> SYN_WASM_MAX_STACK iterations) */
    static const uint8_t loop_unwind[] = {0x41, 0xD0, 0x00, 0x21, 0x00, 0x03, 0x40, 0x41,
                                          0x7F, 0x20, 0x00, 0x41, 0x01, 0x6B, 0x22, 0x00,
                                          0x0D, 0x00, 0x0B, 0x20, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(loop_unwind, sizeof(loop_unwind)));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL_UINT32(1U, g_wasm_ctx.sp);

    static const uint8_t brt_def[] = {0x02, 0x40, 0x02, 0x40, 0x41, 0x05, 0x0E, 0x01, 0x00,
                                      0x01, 0x0B, 0x41, 0x09, 0x0B, 0x41, 0x11, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(brt_def, sizeof(brt_def)));
    TEST_ASSERT_EQUAL_UINT32(17U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t brt_uf[] = {0x0E, 0x00, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(brt_uf, sizeof(brt_uf)));

    static const uint8_t brt_tr1[] = {0x0E};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(brt_tr1, sizeof(brt_tr1)));

    static const uint8_t brt_tr2[] = {0x41, 0x00, 0x0E, 0x01};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(brt_tr2, sizeof(brt_tr2)));

    static const uint8_t brt_tr3[] = {0x41, 0x00, 0x0E, 0x01, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(brt_tr3, sizeof(brt_tr3)));

    /* Call frame / local / global error guards */
    static const uint8_t lget_oob[] = {0x20, 0x05, 0x0B};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = lget_oob;
    g_wasm_mod.size = sizeof(lget_oob);
    g_wasm_mod.func_count = 1U;
    g_wasm_mod.funcs[0].code_size = sizeof(lget_oob);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.local_count = 2U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, syn_wasm_step(&g_wasm_ctx, 10U));

    static const uint8_t lset_oob[] = {0x41, 0x01, 0x21, 0x05, 0x0B};
    g_wasm_mod.bytes = lset_oob;
    g_wasm_mod.size = sizeof(lset_oob);
    g_wasm_mod.funcs[0].code_size = sizeof(lset_oob);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.local_count = 2U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, syn_wasm_step(&g_wasm_ctx, 10U));

    static const uint8_t ltee_uf[] = {0x22, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(ltee_uf, sizeof(ltee_uf)));

    static const uint8_t ltee_oob[] = {0x41, 0x01, 0x22, 0x05, 0x0B};
    g_wasm_mod.bytes = ltee_oob;
    g_wasm_mod.size = sizeof(ltee_oob);
    g_wasm_mod.funcs[0].code_size = sizeof(ltee_oob);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.local_count = 2U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, syn_wasm_step(&g_wasm_ctx, 10U));

    static const uint8_t gget_oob[] = {0x23, 0x20, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(gget_oob, sizeof(gget_oob)));

    static const uint8_t gset_oob[] = {0x41, 0x01, 0x24, 0x20, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(gset_oob, sizeof(gset_oob)));

    static const uint8_t gget_tr[] = {0x23};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(gget_tr, sizeof(gget_tr)));

    static const uint8_t gset_tr[] = {0x24};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(gset_tr, sizeof(gset_tr)));

    static const uint8_t lset_tr[] = {0x21};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(lset_tr, sizeof(lset_tr)));

    static const uint8_t ltee_tr[] = {0x22};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(ltee_tr, sizeof(ltee_tr)));

    /* Call & frame error guards: bad func idx, unregistered host, host underflow, return underflow,
     * call stack overflow, locals overflow, corrupt local headers */
    static const uint8_t call_bad[] = {0x10, 0x09};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, run_raw_snippet(call_bad, sizeof(call_bad)));

    /* Host call unregistered & underflow */
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    static const uint8_t call_imp[] = {0x10, 0x00, 0x0B};
    g_wasm_mod.bytes = call_imp;
    g_wasm_mod.size = sizeof(call_imp);
    g_wasm_mod.func_count = 2U;
    g_wasm_mod.import_func_count = 1U;
    g_wasm_mod.funcs[0].param_count = 1U;
    g_wasm_mod.funcs[1].code_size = sizeof(call_imp);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.call_stack[0].func_idx = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_UNREGISTERED_HOST, syn_wasm_step(&g_wasm_ctx, 10U));

    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, mock_host_func));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.call_stack[0].func_idx = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, syn_wasm_step(&g_wasm_ctx, 10U));

    /* Callee param underflow, call stack overflow, return underflow, locals overflow */
    static const uint8_t rec_body[] = {0x00, 0x10, 0x00, 0x0B};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = rec_body;
    g_wasm_mod.size = sizeof(rec_body);
    g_wasm_mod.func_count = 1U;
    g_wasm_mod.funcs[0].code_size = sizeof(rec_body);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_CALL_STACK_OVERFLOW, syn_wasm_step(&g_wasm_ctx, 200U));

    /* Callee param underflow */
    g_wasm_mod.funcs[0].param_count = 2U;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.pc = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, syn_wasm_step(&g_wasm_ctx, 10U));

    /* Return stack underflow on result_count = 1 */
    static const uint8_t ret_uf[] = {0x00, 0x0F};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = ret_uf;
    g_wasm_mod.size = sizeof(ret_uf);
    g_wasm_mod.func_count = 1U;
    g_wasm_mod.funcs[0].code_size = sizeof(ret_uf);
    g_wasm_mod.funcs[0].result_count = 1U;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, syn_wasm_step(&g_wasm_ctx, 10U));

    /* Parse locals errors: end64 > size, truncated vecs, truncated entry, > MAX_LOCALS, frame sum >
     * MAX_LOCALS */
    g_wasm_mod.funcs[0].code_size = 99U;
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);

    static const uint8_t loc_tr1[] = {0x80};
    g_wasm_mod.bytes = loc_tr1;
    g_wasm_mod.size = sizeof(loc_tr1);
    g_wasm_mod.funcs[0].code_size = sizeof(loc_tr1);
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 0U));

    static const uint8_t loc_tr2[] = {0x01, 0x01};
    g_wasm_mod.bytes = loc_tr2;
    g_wasm_mod.size = sizeof(loc_tr2);
    g_wasm_mod.funcs[0].code_size = sizeof(loc_tr2);
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 0U));

    static const uint8_t loc_big[] = {0x01, 0xAC, 0x02, 0x7F, 0x0B}; /* 300 locals */
    g_wasm_mod.bytes = loc_big;
    g_wasm_mod.size = sizeof(loc_big);
    g_wasm_mod.funcs[0].code_size = sizeof(loc_big);
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_OVERFLOW, g_wasm_ctx.status);

    static const uint8_t loc_200[] = {0x01, 0xC8, 0x01, 0x7F,
                                      0x10, 0x00, 0x0B}; /* 200 locals recursive */
    g_wasm_mod.bytes = loc_200;
    g_wasm_mod.size = sizeof(loc_200);
    g_wasm_mod.funcs[0].code_size = sizeof(loc_200);
    g_wasm_mod.funcs[0].result_count = 0U;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, 16U));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_OVERFLOW, syn_wasm_step(&g_wasm_ctx, 10U));
}

static void test_r5_wasm_int64_div_and_float_trunc(void)
{
    /* 1. i64.div_s(INT64_MIN, -1) MUST trap, i64.rem_s(INT64_MIN, -1) MUST return 0 */
    static const uint8_t i64_div_ovf[] = {0x42, 0x80, 0x80, 0x80, 0x80, 0x80,
                                          0x80, 0x80, 0x80, 0x80, 0x7F, /* INT64_MIN */
                                          0x42, 0x7F,                   /* -1 */
                                          0x7F,                         /* i64.div_s */
                                          0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(i64_div_ovf, sizeof(i64_div_ovf)));

    static const uint8_t i64_rem_min[] = {0x42, 0x80, 0x80, 0x80, 0x80, 0x80,
                                          0x80, 0x80, 0x80, 0x80, 0x7F, /* INT64_MIN */
                                          0x42, 0x7F,                   /* -1 */
                                          0x81,                         /* i64.rem_s */
                                          0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(i64_rem_min, sizeof(i64_rem_min)));
    TEST_ASSERT_EQUAL_UINT64(0ULL, syn_wasm_result(&g_wasm_ctx));

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
    /* 2. Trapping trunc (0xA8..0xAB, 0xAE..0xB1): valid + NaN/Inf/out-of-range + underflow */
    static const uint8_t tr_uf32[] = {0xA8};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(tr_uf32, sizeof(tr_uf32)));

    static const uint8_t tr_uf64[] = {0xAA};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(tr_uf64, sizeof(tr_uf64)));

    /* NaN & overflow traps across i32/i64 signed & unsigned */
    static const uint8_t f32_nan_i32s[] = {0x43, 0x00, 0x00, 0xC0, 0x7F, 0xA8, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(f32_nan_i32s, sizeof(f32_nan_i32s)));

    static const uint8_t f32_ovf_i32s[] = {0x43, 0x00, 0x00, 0x00, 0x4F, 0xA8, 0x0B}; /* 2^31f */
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(f32_ovf_i32s, sizeof(f32_ovf_i32s)));

    static const uint8_t f32_neg_i32u[] = {0x43, 0x00, 0x00, 0x00, 0xC0, 0xA9, 0x0B}; /* -2.0f */
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(f32_neg_i32u, sizeof(f32_neg_i32u)));

    static const uint8_t f32_nan_i64s[] = {0x43, 0x00, 0x00, 0xC0, 0x7F, 0xAE, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(f32_nan_i64s, sizeof(f32_nan_i64s)));

    static const uint8_t f32_ovf_i64s[] = {0x43, 0x00, 0x00, 0x00, 0x5F, 0xAE, 0x0B}; /* 2^63f */
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(f32_ovf_i64s, sizeof(f32_ovf_i64s)));

    static const uint8_t f32_neg_i64u[] = {0x43, 0x00, 0x00, 0x00, 0xC0, 0xAF, 0x0B}; /* -2.0f */
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, run_raw_snippet(f32_neg_i64u, sizeof(f32_neg_i64u)));

    /* Valid i64.trunc_f32_s(-2^63f = 0xDF000000) == INT64_MIN, i64.trunc_f64_u(42.9),
     * i32.trunc_f64_u(3000000000.0) */
    static const uint8_t f32_min_i64s[] = {0x43, 0x00, 0x00, 0x00, 0xDF, 0xAE, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f32_min_i64s, sizeof(f32_min_i64s)));
    TEST_ASSERT_EQUAL_HEX64(0x8000000000000000ULL, syn_wasm_result(&g_wasm_ctx));

    double d3b = 3000000000.0;
    uint8_t f64_u32_ok[11] = {0x44};
    memcpy(&f64_u32_ok[1], &d3b, 8U);
    f64_u32_ok[9] = 0xABU;
    f64_u32_ok[10] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_u32_ok, sizeof(f64_u32_ok)));
    TEST_ASSERT_EQUAL_UINT32(3000000000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    double d42 = 42.9;
    uint8_t f64_u64_ok[11] = {0x44};
    memcpy(&f64_u64_ok[1], &d42, 8U);
    f64_u64_ok[9] = 0xB1U;
    f64_u64_ok[10] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_u64_ok, sizeof(f64_u64_ok)));
    TEST_ASSERT_EQUAL_UINT64(42ULL, syn_wasm_result(&g_wasm_ctx));

    double d_m7 = -7.5;
    uint8_t f64_s64_ok[11] = {0x44};
    memcpy(&f64_s64_ok[1], &d_m7, 8U);
    f64_s64_ok[9] = 0xB0U;
    f64_s64_ok[10] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_s64_ok, sizeof(f64_s64_ok)));
    TEST_ASSERT_EQUAL_INT64(-7LL, (int64_t)syn_wasm_result(&g_wasm_ctx));

    uint8_t f64_s32_ok[11] = {0x44};
    memcpy(&f64_s32_ok[1], &d_m7, 8U);
    f64_s32_ok[9] = 0xAAU;
    f64_s32_ok[10] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_s32_ok, sizeof(f64_s32_ok)));
    TEST_ASSERT_EQUAL_INT32(-7, (int32_t)syn_wasm_result(&g_wasm_ctx));

    /* 3. Saturating 0xFC 0..7 trunc_sat (NaN, clamps, mid-range, underflow) */
    static const uint8_t sat_uf32[] = {0xFC, 0x00};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(sat_uf32, sizeof(sat_uf32)));

    static const uint8_t sat_uf64[] = {0xFC, 0x02};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, run_raw_snippet(sat_uf64, sizeof(sat_uf64)));

    /* f32 unsigned sat (subop 1): 5e9f (0x501502F9) -> UINT32_MAX, -3.0f -> 0, NaN -> 0, 12.5f ->
     * 12 */
    static const uint8_t sat_u32_hi[] = {0x43, 0xF9, 0x02, 0x15, 0x50, 0xFC, 0x01, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u32_hi, sizeof(sat_u32_hi)));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFFU, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_u32_lo[] = {0x43, 0x00, 0x00, 0x40, 0xC0, 0xFC, 0x01, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u32_lo, sizeof(sat_u32_lo)));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_u32_nan[] = {0x43, 0x00, 0x00, 0xC0, 0x7F, 0xFC, 0x01, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u32_nan, sizeof(sat_u32_nan)));
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_u32_mid[] = {0x43, 0x00, 0x00, 0x48, 0x41, 0xFC, 0x01, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u32_mid, sizeof(sat_u32_mid)));
    TEST_ASSERT_EQUAL_UINT32(12U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* f32 signed sat (subop 0): -5e9f -> INT32_MIN, +5e9f -> INT32_MAX, -12.5f -> -12 */
    static const uint8_t sat_s32_lo[] = {0x43, 0xF9, 0x02, 0x15, 0xD0, 0xFC, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_s32_lo, sizeof(sat_s32_lo)));
    TEST_ASSERT_EQUAL_HEX32(0x80000000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_s32_hi[] = {0x43, 0xF9, 0x02, 0x15, 0x50, 0xFC, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_s32_hi, sizeof(sat_s32_hi)));
    TEST_ASSERT_EQUAL_HEX32(0x7FFFFFFFU, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_s32_mid[] = {0x43, 0x00, 0x00, 0x48, 0xC1, 0xFC, 0x00, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_s32_mid, sizeof(sat_s32_mid)));
    TEST_ASSERT_EQUAL_INT32(-12, (int32_t)syn_wasm_result(&g_wasm_ctx));

    /* f64 i64 sat (subop 6 signed, subop 7 unsigned): NaN, low, high, mid */
    static const uint8_t sat_u64_nan[] = {0x43, 0x00, 0x00, 0xC0, 0x7F, 0xFC, 0x05, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u64_nan, sizeof(sat_u64_nan)));
    TEST_ASSERT_EQUAL_UINT64(0ULL, syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_u64_lo[] = {0x43, 0x00, 0x00, 0x40, 0xC0, 0xFC, 0x05, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u64_lo, sizeof(sat_u64_lo)));
    TEST_ASSERT_EQUAL_UINT64(0ULL, syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_u64_hi[] = {0x43, 0x00, 0x00, 0x80,
                                         0x5F, 0xFC, 0x05, 0x0B}; /* 2^64f */
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u64_hi, sizeof(sat_u64_hi)));
    TEST_ASSERT_EQUAL_HEX64(0xFFFFFFFFFFFFFFFFULL, syn_wasm_result(&g_wasm_ctx));

    uint8_t sat_u64_mid[12] = {0x44};
    memcpy(&sat_u64_mid[1], &d3b, 8U);
    sat_u64_mid[9] = 0xFCU;
    sat_u64_mid[10] = 0x07U;
    sat_u64_mid[11] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_u64_mid, sizeof(sat_u64_mid)));
    TEST_ASSERT_EQUAL_UINT64(3000000000ULL, syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_s64_lo[] = {0x43, 0x00, 0x00, 0x80,
                                         0xDF, 0xFC, 0x04, 0x0B}; /* -2^64f */
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_s64_lo, sizeof(sat_s64_lo)));
    TEST_ASSERT_EQUAL_HEX64(0x8000000000000000ULL, syn_wasm_result(&g_wasm_ctx));

    static const uint8_t sat_s64_hi[] = {0x43, 0x00, 0x00, 0x00,
                                         0x5F, 0xFC, 0x04, 0x0B}; /* +2^63f */
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_s64_hi, sizeof(sat_s64_hi)));
    TEST_ASSERT_EQUAL_HEX64(0x7FFFFFFFFFFFFFFFULL, syn_wasm_result(&g_wasm_ctx));

    uint8_t sat_s64_mid[12] = {0x44};
    memcpy(&sat_s64_mid[1], &d_m7, 8U);
    sat_s64_mid[9] = 0xFCU;
    sat_s64_mid[10] = 0x06U;
    sat_s64_mid[11] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(sat_s64_mid, sizeof(sat_s64_mid)));
    TEST_ASSERT_EQUAL_INT64(-7LL, (int64_t)syn_wasm_result(&g_wasm_ctx));
#endif
}

static void test_r5_wasm_i64_global_init(void)
{
    static const uint8_t i64_glob_wasm[] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U,
                                            0x00U, 0x00U, 0x06U, 0x07U, 0x01U, 0x7EU,
                                            0x01U, 0x42U, 0x85U, 0x01U, 0x0BU};
    SYN_WASM_Module mod;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&mod, i64_glob_wasm, (uint32_t)sizeof(i64_glob_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_EQUAL_UINT64(133ULL, g_wasm_ctx.globals[0]);

    /* Globals also initialize when linear_mem == NULL; non-empty data section with NULL mem traps
     */
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, NULL, 0U));
    TEST_ASSERT_EQUAL_UINT64(133ULL, g_wasm_ctx.globals[0]);
    TEST_ASSERT_EQUAL_UINT16(1U, g_wasm_ctx.global_count);

    static const uint8_t data_wasm[] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U,
                                        0x00U, 0x00U, 0x0BU, 0x07U, 0x01U, 0x00U,
                                        0x41U, 0x00U, 0x0BU, 0x01U, 0xAAU};
    SYN_WASM_Module data_mod;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&data_mod, data_wasm, (uint32_t)sizeof(data_wasm)));
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &data_mod, NULL, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, g_wasm_ctx.status);

    static const uint8_t i64_glob_trunc[] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U,
                                             0x06U, 0x05U, 0x01U, 0x7EU, 0x01U, 0x42U, 0x85U};
    TEST_ASSERT_TRUE(syn_wasm_module_load(&mod, i64_glob_trunc, (uint32_t)sizeof(i64_glob_trunc)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_EQUAL_UINT64(5ULL, g_wasm_ctx.globals[0]);
}

static uint64_t tier_a_host_add64(SYN_WASM_Context *ctx, const uint64_t *args, uint8_t argc)
{
    (void)ctx;
    if (args == NULL || argc != 2U) {
        return 0ULL;
    }
    return args[0] + args[1];
}

static void tier_a_expect_bad_sec(const uint8_t *sec, uint32_t sec_len, bool with_decl)
{
    static const uint8_t hdr[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    static const uint8_t decl[] = {0x01, 0x04, 0x01, 0x60, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00};
    uint8_t buf[64];
    uint32_t pos = 0U;
    SYN_WASM_Module mod;

    memcpy(&buf[pos], hdr, sizeof(hdr));
    pos += (uint32_t)sizeof(hdr);
    if (with_decl) {
        memcpy(&buf[pos], decl, sizeof(decl));
        pos += (uint32_t)sizeof(decl);
    }
    memcpy(&buf[pos], sec, sec_len);
    pos += sec_len;
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, buf, pos));
}

static void test_tier_a_wasm_select64_and_host_call(void)
{
    /* WASM-1: OP_SELECT must preserve full 64-bit i64/f64 stack operands */
    static const uint8_t sel_code[] = {0x1b, 0x0b};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = sel_code;
    g_wasm_mod.size = (uint32_t)sizeof(sel_code);
    g_wasm_mod.func_count = 1U;
    g_wasm_mod.funcs[0].code_offset = 0U;
    g_wasm_mod.funcs[0].code_size = (uint32_t)sizeof(sel_code);
    g_wasm_mod.funcs[0].result_count = 1U;

    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.pc = 0U;
    g_wasm_ctx.stack[0] = 0x123456789ABCDEF0ULL;
    g_wasm_ctx.stack[1] = 0xFEDCBA9876543210ULL;
    g_wasm_ctx.stack[2] = 1ULL;
    g_wasm_ctx.sp = 3U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL_HEX64(0x123456789ABCDEF0ULL, syn_wasm_result(&g_wasm_ctx));

    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.pc = 0U;
    g_wasm_ctx.stack[0] = 0x123456789ABCDEF0ULL;
    g_wasm_ctx.stack[1] = 0xFEDCBA9876543210ULL;
    g_wasm_ctx.stack[2] = 0ULL;
    g_wasm_ctx.sp = 3U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL_HEX64(0xFEDCBA9876543210ULL, syn_wasm_result(&g_wasm_ctx));

    /* WASM-8: direct syn_wasm_call on imported host function respects signature & 64-bit return */
    static const uint8_t hdr[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    static const uint8_t h_type[] = {0x01, 0x07, 0x01, 0x60, 0x02, 0x7e, 0x7e, 0x01, 0x7e};
    static const uint8_t h_imp[] = {0x02, 0x07, 0x01, 0x01, 0x65, 0x01, 0x66, 0x00, 0x00};
    uint8_t host64_wasm[sizeof(hdr) + sizeof(h_type) + sizeof(h_imp)];
    memcpy(host64_wasm, hdr, sizeof(hdr));
    memcpy(&host64_wasm[sizeof(hdr)], h_type, sizeof(h_type));
    memcpy(&host64_wasm[sizeof(hdr) + sizeof(h_type)], h_imp, sizeof(h_imp));

    SYN_WASM_Module hmod;
    SYN_WASM_Context hctx;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&hmod, host64_wasm, (uint32_t)sizeof(host64_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&hctx, &hmod, g_linear_mem, sizeof(g_linear_mem)));

    TEST_ASSERT_FALSE(syn_wasm_call(&hctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_UNREGISTERED_HOST, hctx.status);

    TEST_ASSERT_TRUE(syn_wasm_register_host(&hctx, 0U, tier_a_host_add64));
    hctx.sp = 0U;
    TEST_ASSERT_FALSE(syn_wasm_call(&hctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW, hctx.status);

    hctx.stack[0] = 0x0100000000000005ULL;
    hctx.stack[1] = 0x0200000000000007ULL;
    hctx.sp = 2U;
    TEST_ASSERT_TRUE(syn_wasm_call(&hctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, hctx.status);
    TEST_ASSERT_EQUAL_HEX64(0x030000000000000CULL, syn_wasm_result(&hctx));

#if defined(SYN_WASM_USE_FIXED) && SYN_WASM_USE_FIXED
    static const uint8_t f_abs[] = {0x8b, 0x0b};
    static const uint8_t f_neg[] = {0x8c, 0x0b};
    static const uint8_t f_trunc[] = {0x8f, 0x0b};

    g_wasm_mod.bytes = f_abs;
    g_wasm_mod.size = 2U;
    g_wasm_mod.funcs[0].code_size = 2U;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.stack[0] = 0x80000000ULL;
    g_wasm_ctx.sp = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL_HEX32(0x7FFFFFFFU, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    g_wasm_mod.bytes = f_neg;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.stack[0] = 0x80000000ULL;
    g_wasm_ctx.sp = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL_HEX32(0x7FFFFFFFU, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    g_wasm_mod.bytes = f_trunc;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    g_wasm_ctx.stack[0] = 0xFFFE4000ULL;
    g_wasm_ctx.sp = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL_HEX32(0xFFFF0000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
#endif
}

static void test_tier_a_wasm_loader_bounds(void)
{
    static const uint8_t hdr[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    static const uint8_t m_imp[] = {0x02, 0x08, 0x01, 0x01, 0x65, 0x01, 0x6d, 0x02, 0x00, 0x01};
    uint8_t mem_ok[sizeof(hdr) + sizeof(m_imp)];
    SYN_WASM_Module mod;

    memcpy(mem_ok, hdr, sizeof(hdr));
    memcpy(&mem_ok[sizeof(hdr)], m_imp, sizeof(m_imp));
    TEST_ASSERT_TRUE(syn_wasm_module_load(&mod, mem_ok, (uint32_t)sizeof(mem_ok)));

    static const uint8_t s_trail[] = {0x01};
    static const uint8_t s_t_emp[] = {0x01, 0x00};
    static const uint8_t s_t_frm[] = {0x01, 0x02, 0x01, 0x61};
    static const uint8_t s_t_pov[] = {0x01, 0x03, 0x01, 0x60, 0x45};
    static const uint8_t s_t_poo[] = {0x01, 0x03, 0x01, 0x60, 0x02};
    static const uint8_t s_t_rov[] = {0x01, 0x04, 0x01, 0x60, 0x00, 0x45};
    static const uint8_t s_t_roo[] = {0x01, 0x04, 0x01, 0x60, 0x00, 0x02};
    static const uint8_t s_i_emp[] = {0x02, 0x00};
    static const uint8_t s_i_bty[] = {0x02, 0x07, 0x01, 0x01, 0x61, 0x01, 0x62, 0x00, 0x00};
    static const uint8_t s_i_tbl[] = {0x02, 0x07, 0x01, 0x01, 0x61, 0x01, 0x62, 0x01, 0x70};
    static const uint8_t s_i_glb[] = {0x02, 0x07, 0x01, 0x01, 0x61, 0x01, 0x62, 0x03, 0x7f};
    static const uint8_t s_i_mfl[] = {0x02, 0x06, 0x01, 0x01, 0x61, 0x01, 0x62, 0x02};
    static const uint8_t s_i_mmn[] = {0x02, 0x07, 0x01, 0x01, 0x61, 0x01, 0x62, 0x02, 0x00};
    static const uint8_t s_i_mmx[] = {0x02, 0x08, 0x01, 0x01, 0x61, 0x01, 0x62, 0x02, 0x01, 0x01};
    static const uint8_t s_i_bkd[] = {0x02, 0x06, 0x01, 0x01, 0x61, 0x01, 0x62, 0x04};
    static const uint8_t s_f_emp[] = {0x03, 0x00};
    static const uint8_t s_f_bty[] = {0x03, 0x02, 0x01, 0x00};
    static const uint8_t s_x_emp[] = {0x07, 0x00};
    static const uint8_t s_x_bkd[] = {0x07, 0x04, 0x01, 0x01, 0x78, 0x04};
    static const uint8_t s_x_nid[] = {0x07, 0x04, 0x01, 0x01, 0x78, 0x00};
    static const uint8_t s_x_bfn[] = {0x07, 0x05, 0x01, 0x01, 0x78, 0x00, 0x00};
    static const uint8_t s_e_emp[] = {0x09, 0x00};
    static const uint8_t s_e_flg[] = {0x09, 0x02, 0x01, 0x01};
    static const uint8_t s_e_bop[] = {0x09, 0x04, 0x01, 0x00, 0x42, 0x00};
    static const uint8_t s_e_neg[] = {0x09, 0x05, 0x01, 0x00, 0x41, 0x7f, 0x0b};
    static const uint8_t s_e_ned[] = {0x09, 0x05, 0x01, 0x00, 0x41, 0x00, 0x01};
    static const uint8_t s_e_tov[] = {0x09, 0x06, 0x01, 0x00, 0x41, 0x3f, 0x0b, 0x02};
    static const uint8_t s_e_bfi[] = {0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01, 0x05};
    static const uint8_t s_c_zbd[] = {0x0a, 0x02, 0x01, 0x00};
    static const uint8_t s_c_oob[] = {0x0a, 0x03, 0x01, 0x05, 0x0b};

    tier_a_expect_bad_sec(s_trail, (uint32_t)sizeof(s_trail), false);
    tier_a_expect_bad_sec(s_t_emp, (uint32_t)sizeof(s_t_emp), false);
    tier_a_expect_bad_sec(s_t_frm, (uint32_t)sizeof(s_t_frm), false);
    tier_a_expect_bad_sec(s_t_pov, (uint32_t)sizeof(s_t_pov), false);
    tier_a_expect_bad_sec(s_t_poo, (uint32_t)sizeof(s_t_poo), false);
    tier_a_expect_bad_sec(s_t_rov, (uint32_t)sizeof(s_t_rov), false);
    tier_a_expect_bad_sec(s_t_roo, (uint32_t)sizeof(s_t_roo), false);
    tier_a_expect_bad_sec(s_i_emp, (uint32_t)sizeof(s_i_emp), false);
    tier_a_expect_bad_sec(s_i_bty, (uint32_t)sizeof(s_i_bty), false);
    tier_a_expect_bad_sec(s_i_tbl, (uint32_t)sizeof(s_i_tbl), false);
    tier_a_expect_bad_sec(s_i_glb, (uint32_t)sizeof(s_i_glb), false);
    tier_a_expect_bad_sec(s_i_mfl, (uint32_t)sizeof(s_i_mfl), false);
    tier_a_expect_bad_sec(s_i_mmn, (uint32_t)sizeof(s_i_mmn), false);
    tier_a_expect_bad_sec(s_i_mmx, (uint32_t)sizeof(s_i_mmx), false);
    tier_a_expect_bad_sec(s_i_bkd, (uint32_t)sizeof(s_i_bkd), false);
    tier_a_expect_bad_sec(s_f_emp, (uint32_t)sizeof(s_f_emp), false);
    tier_a_expect_bad_sec(s_f_bty, (uint32_t)sizeof(s_f_bty), false);
    tier_a_expect_bad_sec(s_x_emp, (uint32_t)sizeof(s_x_emp), false);
    tier_a_expect_bad_sec(s_x_bkd, (uint32_t)sizeof(s_x_bkd), false);
    tier_a_expect_bad_sec(s_x_nid, (uint32_t)sizeof(s_x_nid), false);
    tier_a_expect_bad_sec(s_x_bfn, (uint32_t)sizeof(s_x_bfn), false);
    tier_a_expect_bad_sec(s_e_emp, (uint32_t)sizeof(s_e_emp), false);
    tier_a_expect_bad_sec(s_e_flg, (uint32_t)sizeof(s_e_flg), false);
    tier_a_expect_bad_sec(s_e_bop, (uint32_t)sizeof(s_e_bop), false);
    tier_a_expect_bad_sec(s_e_neg, (uint32_t)sizeof(s_e_neg), false);
    tier_a_expect_bad_sec(s_e_ned, (uint32_t)sizeof(s_e_ned), false);
    tier_a_expect_bad_sec(s_e_tov, (uint32_t)sizeof(s_e_tov), false);
    tier_a_expect_bad_sec(s_e_bfi, (uint32_t)sizeof(s_e_bfi), true);
    tier_a_expect_bad_sec(s_c_zbd, (uint32_t)sizeof(s_c_zbd), true);
    tier_a_expect_bad_sec(s_c_oob, (uint32_t)sizeof(s_c_oob), true);
}

void run_wasm_tests(void)
{
    RUN_TEST(test_wasm_load_null_and_invalid);
    RUN_TEST(test_wasm_load_and_export_lookup);
    RUN_TEST(test_wasm_init_and_execution);
    RUN_TEST(test_wasm_loop_and_yielding);
    RUN_TEST(test_wasm_host_function_registration);
    RUN_TEST(test_wasm_float_and_bulk_memory);
    RUN_TEST(test_wasm_fixed_point_float_ops);
    RUN_TEST(test_wasm_native_f64_ops);
    RUN_TEST(test_wasm_fixture_arithmetic);
    RUN_TEST(test_wasm_fixture_memory);
    RUN_TEST(test_wasm_fixture_functions);
    RUN_TEST(test_wasm_fixture_structs);
    RUN_TEST(test_wasm_fixture_control_flow);
    RUN_TEST(test_wasm_fixture_conversions);
    RUN_TEST(test_wasm_fixture_recursion);
    RUN_TEST(test_wasm_fixture_embedded);
    RUN_TEST(test_wasm_fixture_host_api);
    RUN_TEST(test_wasm_fixture_traps);
    RUN_TEST(test_wasm_fixture_yield_resume);
    RUN_TEST(test_wasm_extended_traps_and_nulls);
    RUN_TEST(test_wasm_opcodes_branch_coverage);
    RUN_TEST(test_r5_wasm_module_and_data_bounds);
    RUN_TEST(test_r5_wasm_if_else_return_and_locals);
    RUN_TEST(test_r5_wasm_memory_and_bytecode_bounds);
    RUN_TEST(test_r5_wasm_skip_and_control_edge_cases);
    RUN_TEST(test_r5_wasm_int64_div_and_float_trunc);
    RUN_TEST(test_r5_wasm_i64_global_init);
    RUN_TEST(test_tier_a_wasm_select64_and_host_call);
    RUN_TEST(test_tier_a_wasm_loader_bounds);
}
/* touch test_wasm.c */
