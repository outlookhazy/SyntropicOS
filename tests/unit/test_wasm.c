/**
 * @file test_wasm.c
 * @brief Unit tests for 32-bit WebAssembly (Wasm MVP) cooperative interpreter.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_mutex.h"
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
SYN_WASM_HOST_SYNC(mock_host_func, ctx, args, argc)
{
    (void)ctx;
    (void)args;
    (void)argc;
    g_host_call_count++;
    return 42ULL;
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
    SYN_PT hpt;
    PT_INIT(&hpt);
    uint64_t ret = 0ULL;
    TEST_ASSERT_EQUAL(PT_ENDED, g_wasm_ctx.host_funcs[0](&hpt, &g_wasm_ctx, NULL, 0U, &ret));
    TEST_ASSERT_EQUAL_UINT64(42ULL, ret);
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

SYN_WASM_HOST_SYNC(mock_host_log, ctx, args, argc)
{
    (void)ctx;
    (void)argc;
    g_host_log_called++;
    g_host_log_code = (uint32_t)args[0];
    g_host_log_val = (uint32_t)args[1];
    return (uint64_t)(g_host_log_code + g_host_log_val);
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

    /* Test Func/Table/Memory/Global imports, Start section (sec 8), and Element section (sec 9) */
    static const uint8_t wasm_extra_sections[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, /* magic & version */
        0x01, 0x04, 0x01, 0x60, 0x00, 0x00,             /* Type: () -> () */
        0x02, 0x30, 0x04, /* Import Section (4 entries, 48 bytes payload) */
        0x03, 'e',  'n',  'v',  0x02, 'f',  'n',  0x00, 0x00, /* Func import: type 0 */
        0x03, 'e',  'n',  'v',  0x05, 't',  'a',  'b',  'l',  'e',  0x01, 0x70, 0x00,
        0x01, /* Table import: funcref, flags=0, min 1 */
        0x03, 'e',  'n',  'v',  0x03, 'm',  'e',  'm',  0x02, 0x01, 0x01, 0x02, /* Memory import:
                                                                                   flags=1, min 1,
                                                                                   max 2 */
        0x03, 'e',  'n',  'v',  0x04, 'g',  'l',  'o',  'b',  0x03, 0x7f, 0x00, /* Global import:
                                                                                   i32, const */
        0x08, 0x01, 0x00,                                     /* Start section: func 0 */
        0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01, 0x00, /* Element section: table 0, offset
                                                                 i32.const 0, func 0 */
    };
    SYN_WASM_Module extra_mod;
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&extra_mod, wasm_extra_sections, sizeof(wasm_extra_sections)));
    TEST_ASSERT_TRUE(extra_mod.has_start_func);
    TEST_ASSERT_EQUAL_UINT32(0U, extra_mod.start_func_idx);
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
    static const uint8_t neg_data_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                            0x05, 0x03, 0x01, 0x00, 0x01, 0x0B, 0x07, 0x01,
                                            0x00, 0x41, 0x7C, 0x0B, 0x01, 0xAA};
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, neg_data_wasm, sizeof(neg_data_wasm)));
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, g_wasm_ctx.status);

    /* 5. Positive data_offset + data_size > mem_size must trap OUT_OF_BOUNDS */
    static const uint8_t oob_data_wasm[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                            0x05, 0x03, 0x01, 0x00, 0x01, 0x0B, 0x09, 0x01,
                                            0x00, 0x41, 0x06, 0x0B, 0x03, 'A',  'B',  'C'};
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
        /* Table section: 1 funcref table, min 1 */
        0x04, 0x04, 0x01, 0x70, 0x00, 0x01,
        /* Element section: table[0] = f1 */
        0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0B, 0x01, 0x01,
        /* Code section (71 = 0x47 bytes, 4 bodies) */
        0x0A, 0x47, 0x04,
        /* f0 body (18 = 0x12 bytes): 12 locals; l10=77; f3(); f1(5); l10 + res */
        0x12, 0x01, 0x0C, 0x7F, 0x41, 0xCD, 0x00, 0x21, 0x0A, 0x10, 0x03, 0x41, 0x05, 0x10, 0x01,
        0x20, 0x0A, 0x6A, 0x0B,
        /* f1 body (27 = 0x1B bytes): 3 locals; l2=999; block { loop { push 88; l0*10+l3; return } }
         */
        0x1B, 0x01, 0x03, 0x7F, 0x41, 0xE7, 0x07, 0x21, 0x02, 0x02, 0x40, 0x03, 0x40, 0x41, 0x58,
        0x20, 0x00, 0x41, 0x0A, 0x6C, 0x20, 0x03, 0x6A, 0x0F, 0x0B, 0x0B, 0x00, 0x0B,
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

    static const uint8_t data_wasm[] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U,
                                        0x05U, 0x03U, 0x01U, 0x00U, 0x01U, 0x0BU, 0x07U, 0x01U,
                                        0x00U, 0x41U, 0x00U, 0x0BU, 0x01U, 0xAAU};
    SYN_WASM_Module data_mod;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&data_mod, data_wasm, (uint32_t)sizeof(data_wasm)));
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &data_mod, NULL, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, g_wasm_ctx.status);

    static const uint8_t i64_glob_trunc[] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U,
                                             0x06U, 0x05U, 0x01U, 0x7EU, 0x01U, 0x42U, 0x85U};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, i64_glob_trunc, (uint32_t)sizeof(i64_glob_trunc)));
    memset(&mod, 0, sizeof(mod));
    mod.bytes = i64_glob_trunc;
    mod.size = (uint32_t)sizeof(i64_glob_trunc);
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);
}

SYN_WASM_HOST_SYNC(tier_a_host_add64, ctx, args, argc)
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

    /* AUD-M5: Strict LEB128, tabletype, >32 func/type caps, 0-func elem, Start, and Data checks */
    static const uint8_t s_u32_ovf[] = {0x01, 0x05, 0x80, 0x80, 0x80, 0x80, 0x10};
    static const uint8_t s_u32_trc[] = {0x01, 0x01, 0x80};
    static const uint8_t s_t_gt32[] = {0x01, 0x01, 33U};
    static const uint8_t s_f_gt32[] = {0x03, 0x01, 33U};
    static const uint8_t s_i_tbad[] = {0x02, 0x09, 0x01, 0x01, 0x61, 0x01,
                                       0x62, 0x01, 0x6F, 0x00, 0x01};
    static const uint8_t s_i_lflg[] = {0x02, 0x08, 0x01, 0x01, 0x61, 0x01, 0x62, 0x02, 0x02, 0x01};
    static const uint8_t s_i_lmax[] = {0x02, 0x09, 0x01, 0x01, 0x61, 0x01,
                                       0x62, 0x02, 0x01, 0x05, 0x02};
    static const uint8_t s_e_zfn[] = {0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01, 0x00};
    static const uint8_t s_e_itrc[] = {0x09, 0x04, 0x01, 0x00, 0x41, 0x80};
    static const uint8_t s_e_iovf[] = {0x09, 0x09, 0x01, 0x00, 0x41, 0x80,
                                       0x80, 0x80, 0x80, 0x08, 0x0b};
    static const uint8_t s_e_i5ok[] = {0x09, 0x0b, 0x01, 0x00, 0x41, 0x80,
                                       0x80, 0x80, 0x80, 0x78, 0x0b, 0x00};
    static const uint8_t s_st_oob[] = {0x08, 0x01, 0x05};
    static const uint8_t s_st_trl[] = {0x08, 0x02, 0x00, 0x00};
    static const uint8_t s_st_dup[] = {0x08, 0x01, 0x00, 0x08, 0x01, 0x00};

    tier_a_expect_bad_sec(s_u32_ovf, (uint32_t)sizeof(s_u32_ovf), false);
    tier_a_expect_bad_sec(s_u32_trc, (uint32_t)sizeof(s_u32_trc), false);
    tier_a_expect_bad_sec(s_t_gt32, (uint32_t)sizeof(s_t_gt32), false);
    tier_a_expect_bad_sec(s_f_gt32, (uint32_t)sizeof(s_f_gt32), true);
    tier_a_expect_bad_sec(s_i_tbad, (uint32_t)sizeof(s_i_tbad), false);
    tier_a_expect_bad_sec(s_i_lflg, (uint32_t)sizeof(s_i_lflg), false);
    tier_a_expect_bad_sec(s_i_lmax, (uint32_t)sizeof(s_i_lmax), false);
    tier_a_expect_bad_sec(s_e_zfn, (uint32_t)sizeof(s_e_zfn), false);
    tier_a_expect_bad_sec(s_e_itrc, (uint32_t)sizeof(s_e_itrc), true);
    tier_a_expect_bad_sec(s_e_iovf, (uint32_t)sizeof(s_e_iovf), true);
    tier_a_expect_bad_sec(s_e_i5ok, (uint32_t)sizeof(s_e_i5ok), true);
    tier_a_expect_bad_sec(s_st_oob, (uint32_t)sizeof(s_st_oob), true);
    tier_a_expect_bad_sec(s_st_trl, (uint32_t)sizeof(s_st_trl), true);
    tier_a_expect_bad_sec(s_st_dup, (uint32_t)sizeof(s_st_dup), true);

    /* >32 imported functions rejected */
    uint8_t imp33[256];
    uint32_t ipos = 0U;
    static const uint8_t tsec[] = {0x01, 0x04, 0x01, 0x60, 0x00, 0x00};
    memcpy(&imp33[ipos], hdr, sizeof(hdr));
    ipos += (uint32_t)sizeof(hdr);
    memcpy(&imp33[ipos], tsec, sizeof(tsec));
    ipos += (uint32_t)sizeof(tsec);
    imp33[ipos++] = 0x02U;
    imp33[ipos++] = (uint8_t)(1U + (33U * 6U)); /* 199 = 0xC7 -> 2-byte LEB128! */
    imp33[ipos - 1U] = 0xC7U;
    imp33[ipos++] = 0x01U;
    imp33[ipos++] = 33U;
    for (uint32_t i = 0U; i < 33U; i++) {
        imp33[ipos++] = 0x01U;
        imp33[ipos++] = 0x65U;
        imp33[ipos++] = 0x01U;
        imp33[ipos++] = (uint8_t)(0x41U + (i & 15U));
        imp33[ipos++] = 0x00U;
        imp33[ipos++] = 0x00U;
    }
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, imp33, ipos));

    /* Malformed Data sections rejected at load AND guarded in syn_wasm_init */
    static const uint8_t d_emp[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00};
    static const uint8_t d_bad[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                    0x00, 0x0b, 0x04, 0x01, 0x03, 0x01, 0xAA};
    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, d_emp, (uint32_t)sizeof(d_emp)));
    memset(&mod, 0, sizeof(mod));
    mod.bytes = d_emp;
    mod.size = (uint32_t)sizeof(d_emp);
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);

    TEST_ASSERT_FALSE(syn_wasm_module_load(&mod, d_bad, (uint32_t)sizeof(d_bad)));
    memset(&mod, 0, sizeof(mod));
    mod.bytes = d_bad;
    mod.size = (uint32_t)sizeof(d_bad);
    TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);

    /* Negative i32.const (-5 = 0x7b) global initializer sign-extension */
    static const uint8_t g_neg[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                    0x06, 0x06, 0x01, 0x7f, 0x01, 0x41, 0x7b, 0x0b};
    TEST_ASSERT_TRUE(syn_wasm_module_load(&mod, g_neg, (uint32_t)sizeof(g_neg)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, 16U));
    TEST_ASSERT_EQUAL_HEX64((uint64_t)(int64_t)-5, g_wasm_ctx.globals[0]);
}

typedef struct {
    SYN_PT child_pt;
    SYN_WASM_Context *ctx;
    uint16_t quantum;
    uint32_t completed;
} WasmSpawnTaskCtx;

static SYN_PT_Status wasm_spawn_parent_task(SYN_PT *pt, SYN_Task *task)
{
    WasmSpawnTaskCtx *w = (WasmSpawnTaskCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_SPAWN_IF_NEEDED(pt, task, &w->child_pt,
                       syn_wasm_exec_pt(&w->child_pt, task, w->ctx, w->quantum));
    w->completed++;
    PT_END(pt);
}

static SYN_PT_Status wasm_peer_more_task(SYN_PT *pt, SYN_Task *task)
{
    uint32_t *runs = (uint32_t *)task->user_data;
    (*runs)++;
    PT_BEGIN(pt);
    for (;;) {
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void test_wasm_cooperative_exec_pt_and_task(void)
{
    SYN_PT pt;
    PT_INIT(&pt);

    /* 1. Parameter & trap guards */
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_exec_pt(NULL, NULL, &g_wasm_ctx, 8U));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_exec_pt(&pt, NULL, NULL, 8U));
    memset(&g_wasm_ctx, 0, sizeof(g_wasm_ctx));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_exec_pt(&pt, NULL, &g_wasm_ctx, 8U));

    SYN_Task dummy_task;
    memset(&dummy_task, 0, sizeof(dummy_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_task(NULL, &dummy_task));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_task(&pt, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_task(&pt, &dummy_task));

    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, g_wasm_loop_binary, sizeof(g_wasm_loop_binary)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));

    /* Calling syn_wasm_exec_pt when not HALTED and call_depth == 0 exits with PT_EXITED */
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_exec_pt(&pt, NULL, &g_wasm_ctx, 0U));

    /* 2. Standalone syn_wasm_exec_pt with default quantum (0U) and small quantum (5U) */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    g_wasm_ctx.locals[0] = 3U;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wasm_exec_pt(&pt, NULL, &g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);

    /* 3. PT_SPAWN_IF_NEEDED: uncontended (1 turn) vs contended (interleaved quanta) */
    WasmSpawnTaskCtx wctx = {.ctx = &g_wasm_ctx, .quantum = 5U, .completed = 0U};
    uint32_t peer_runs = 0U;
    SYN_Task tasks[2];
    SYN_Sched sched;

    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    g_wasm_ctx.locals[0] = 10U;
    syn_task_create(&tasks[0], "wasm_p", wasm_spawn_parent_task, SYN_PRIO_HIGH, &wctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_task_suspend(&tasks[1]);
    syn_sched_init(&sched, tasks, 2U);

    /* Uncontended: all 5-opcode quanta execute back-to-back in a single dispatch */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1U, wctx.completed);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);

    /* Contended: yields after each 5-opcode quantum so peer runs between slices */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    g_wasm_ctx.locals[0] = 10U;
    wctx.completed = 0U;
    syn_task_create(&tasks[0], "wasm_p", wasm_spawn_parent_task, SYN_PRIO_HIGH, &wctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched); /* wasm_p runs 1 quantum (5 opcodes) and yields PT_MORE */
    TEST_ASSERT_EQUAL_UINT32(0U, wctx.completed);
    TEST_ASSERT_EQUAL(SYN_WASM_OK, g_wasm_ctx.status);
    syn_sched_run(&sched); /* peer runs */
    TEST_ASSERT_EQUAL_UINT32(1U, peer_runs);

    while (wctx.completed == 0U) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_TRUE(peer_runs > 5U);

    /* 4. syn_wasm_task: idle before call, contended quantum yielding, and uncontended burst */
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    peer_runs = 0U;
    syn_task_create(&tasks[0], "wasm_t", syn_wasm_task, SYN_PRIO_HIGH, &g_wasm_ctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_sched_init(&sched, tasks, 2U);

    /* Before syn_wasm_call, call_depth == 0 -> syn_wasm_task yields PT_IDLE */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_WASM_OK, g_wasm_ctx.status);

    /* Start a 30-iteration loop (~210 opcodes > 64-opcode default quantum) while contended */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    g_wasm_ctx.locals[0] = 30U;
    mock_tick_advance(10U);
    syn_sched_run(&sched); /* peer (overdue) */
    syn_sched_run(&sched); /* wasm_t runs 1 quantum (64 opcodes) -> yields PT_MORE */
    TEST_ASSERT_EQUAL(SYN_WASM_OK, g_wasm_ctx.status);
    TEST_ASSERT_TRUE(g_wasm_ctx.locals[1] > 0U && g_wasm_ctx.locals[1] < 30U);

    while (g_wasm_ctx.status == SYN_WASM_OK) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(0U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Uncontended syn_wasm_task runs multi-quantum call to HALTED in a single turn */
    syn_task_suspend(&tasks[1]);
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    g_wasm_ctx.locals[0] = 30U;
    mock_tick_advance(10U);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);

    /* Fault trap terminates syn_wasm_task with PT_ENDED */
    g_wasm_ctx.status = SYN_WASM_TRAP_UNREACHABLE;
    mock_tick_advance(10U);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL(SYN_TASK_DEAD, tasks[0].state);
}

static void test_wasm_spec_profile_sign_ext_and_float_ieee754(void)
{
    /* 1. All 5 sign-ext opcodes (0xC0..0xC4) + empty-stack underflow */
    static const uint8_t c_ext8_32[] = {0x41, 0xAB, 0x25, 0xC0, 0x0B};        /* 0x12AB -> -85 */
    static const uint8_t c_ext16_32[] = {0x41, 0x80, 0x80, 0x03, 0xC1, 0x0B}; /* 0xC000 -> -16384 */
    static const uint8_t c_ext8_64[] = {0x42, 0xFF, 0x01, 0xC2, 0x0B};        /* 0xFF -> -1LL */
    static const uint8_t c_ext16_64[] = {0x42, 0x81, 0x80,
                                         0x06, 0xC3, 0x0B}; /* 0x18001 -> -32767LL */
    static const uint8_t c_ext32_64[] = {0x42, 0x80, 0x80, 0x80, 0x80, 0x18, 0xC4, 0x0B};
    static const uint8_t c_uf32[] = {0xC0, 0x0B};
    static const uint8_t c_uf64[] = {0xC2, 0x0B};

    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(c_ext8_32, (uint32_t)sizeof(c_ext8_32)));
    TEST_ASSERT_EQUAL_INT32(-85, (int32_t)(uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(c_ext16_32, (uint32_t)sizeof(c_ext16_32)));
    TEST_ASSERT_EQUAL_INT32(-16384, (int32_t)(uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(c_ext8_64, (uint32_t)sizeof(c_ext8_64)));
    TEST_ASSERT_EQUAL_INT64(-1LL, (int64_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(c_ext16_64, (uint32_t)sizeof(c_ext16_64)));
    TEST_ASSERT_EQUAL_INT64(-32767LL, (int64_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(c_ext32_64, (uint32_t)sizeof(c_ext32_64)));
    TEST_ASSERT_EQUAL_INT64(INT32_MIN, (int64_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW,
                      run_raw_snippet(c_uf32, (uint32_t)sizeof(c_uf32)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW,
                      run_raw_snippet(c_uf64, (uint32_t)sizeof(c_uf64)));

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
    /* 2. IEEE-754 f32/f64 nearest ties-to-even, -0.0 sign preservation, sNaN, and min/max */
    static const struct {
        uint32_t in_bits;
        uint32_t exp_nearest;
    } f32_near_cases[] = {
        {0x3F000000U, 0x00000000U}, /* +0.5 -> +0.0 */
        {0xBF000000U, 0x80000000U}, /* -0.5 -> -0.0 */
        {0x3FC00000U, 0x40000000U}, /* +1.5 -> +2.0 */
        {0x40200000U, 0x40000000U}, /* +2.5 -> +2.0 */
        {0x40600000U, 0x40800000U}, /* +3.5 -> +4.0 */
        {0x3EFFFFFFU, 0x00000000U}, /* 0.49999997 -> +0.0 (no double-round) */
        {0x4B000001U, 0x4B000001U}, /* >= 2^23 already integral */
        {0x7F800000U, 0x7F800000U}, /* +Inf -> +Inf */
    };
    for (size_t i = 0U; i < sizeof(f32_near_cases) / sizeof(f32_near_cases[0]); i++) {
        uint8_t code[7] = {0x43U, 0U, 0U, 0U, 0U, 0x90U, 0x0BU};
        memcpy(&code[1], &f32_near_cases[i].in_bits, 4U);
        TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(code, 7U));
        TEST_ASSERT_EQUAL_HEX32(f32_near_cases[i].exp_nearest,
                                (uint32_t)syn_wasm_result(&g_wasm_ctx));
    }
    /* f32 sNaN nearest/ceil/floor/trunc quiet bit, abs/neg/copysign preserve sNaN payload */
    uint32_t snan32 = 0xFF800013U;
    uint8_t snan_abs[7] = {0x43U, 0U, 0U, 0U, 0U, 0x8BU, 0x0BU};
    memcpy(&snan_abs[1], &snan32, 4U);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(snan_abs, 7U));
    TEST_ASSERT_EQUAL_HEX32(0x7F800013U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    snan_abs[5] = 0x90U; /* f32.nearest on sNaN sets quiet bit */
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(snan_abs, 7U));
    TEST_ASSERT_TRUE(((uint32_t)syn_wasm_result(&g_wasm_ctx) & 0x7FC00000U) == 0x7FC00000U);

    /* f32 min/max: -0.0 vs +0.0 and NaN propagation */
    static const uint8_t f32_min_z[] = {0x43, 0x00, 0x00, 0x00, 0x00, 0x43,
                                        0x00, 0x00, 0x00, 0x80, 0x96, 0x0B};
    static const uint8_t f32_max_z[] = {0x43, 0x00, 0x00, 0x00, 0x80, 0x43,
                                        0x00, 0x00, 0x00, 0x00, 0x97, 0x0B};
    static const uint8_t f32_min_nan[] = {0x43, 0x00, 0x00, 0x80, 0x3F, 0x43,
                                          0x01, 0x00, 0x80, 0x7F, 0x96, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f32_min_z, (uint32_t)sizeof(f32_min_z)));
    TEST_ASSERT_EQUAL_HEX32(0x80000000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f32_max_z, (uint32_t)sizeof(f32_max_z)));
    TEST_ASSERT_EQUAL_HEX32(0x00000000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f32_min_nan, (uint32_t)sizeof(f32_min_nan)));
    TEST_ASSERT_TRUE(((uint32_t)syn_wasm_result(&g_wasm_ctx) & 0x7FC00000U) == 0x7FC00000U);

    /* f64 nearest (-0.5 -> -0.0, 2.5 -> 2.0, 3.5 -> 4.0, >=2^52, sNaN) & min/max */
    static const struct {
        uint64_t in_bits;
        uint64_t exp_nearest;
    } f64_near_cases[] = {
        {0xBFE0000000000000ULL, 0x8000000000000000ULL}, /* -0.5 -> -0.0 */
        {0x4004000000000000ULL, 0x4000000000000000ULL}, /* +2.5 -> +2.0 */
        {0x400C000000000000ULL, 0x4010000000000000ULL}, /* +3.5 -> +4.0 */
        {0x4330000000000001ULL, 0x4330000000000001ULL}, /* >= 2^52 */
    };
    for (size_t i = 0U; i < sizeof(f64_near_cases) / sizeof(f64_near_cases[0]); i++) {
        uint8_t code[11] = {0x44U};
        memcpy(&code[1], &f64_near_cases[i].in_bits, 8U);
        code[9] = 0x9EU;
        code[10] = 0x0BU;
        TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(code, 11U));
        TEST_ASSERT_EQUAL_HEX64(f64_near_cases[i].exp_nearest, syn_wasm_result(&g_wasm_ctx));
    }
    uint64_t snan64 = 0xFFF0000000000005ULL;
    uint8_t snan64_code[11] = {0x44U};
    memcpy(&snan64_code[1], &snan64, 8U);
    snan64_code[9] = 0x99U; /* f64.abs preserves sNaN payload */
    snan64_code[10] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(snan64_code, 11U));
    TEST_ASSERT_EQUAL_HEX64(0x7FF0000000000005ULL, syn_wasm_result(&g_wasm_ctx));
    snan64_code[9] = 0x9EU; /* f64.nearest quiets sNaN */
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(snan64_code, 11U));
    TEST_ASSERT_TRUE((syn_wasm_result(&g_wasm_ctx) & 0x7FF8000000000000ULL) ==
                     0x7FF8000000000000ULL);

    uint8_t f64_mm[20] = {0x44U};
    uint64_t pz64 = 0x0000000000000000ULL;
    uint64_t nz64 = 0x8000000000000000ULL;
    memcpy(&f64_mm[1], &pz64, 8U);
    f64_mm[9] = 0x44U;
    memcpy(&f64_mm[10], &nz64, 8U);
    f64_mm[18] = 0xA4U; /* f64.min(+0.0, -0.0) -> -0.0 */
    f64_mm[19] = 0x0BU;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_mm, 20U));
    TEST_ASSERT_EQUAL_HEX64(nz64, syn_wasm_result(&g_wasm_ctx));
    f64_mm[18] = 0xA5U; /* f64.max(+0.0, -0.0) -> +0.0 */
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_mm, 20U));
    TEST_ASSERT_EQUAL_HEX64(pz64, syn_wasm_result(&g_wasm_ctx));
    memcpy(&f64_mm[1], &snan64, 8U);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_mm, 20U));
    TEST_ASSERT_TRUE((syn_wasm_result(&g_wasm_ctx) & 0x7FF8000000000000ULL) ==
                     0x7FF8000000000000ULL);

    /* Non-zero finite f32.min/f32.max and f64.min/f64.max */
    static const uint8_t f32_fin_min[] = {0x43, 0x00, 0x00, 0xC0, 0x3F, 0x43,
                                          0x00, 0x00, 0x20, 0x40, 0x96, 0x0B};
    static const uint8_t f32_fin_max[] = {0x43, 0x00, 0x00, 0xC0, 0x3F, 0x43,
                                          0x00, 0x00, 0x20, 0x40, 0x97, 0x0B};
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f32_fin_min, (uint32_t)sizeof(f32_fin_min)));
    TEST_ASSERT_EQUAL_HEX32(0x3FC00000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f32_fin_max, (uint32_t)sizeof(f32_fin_max)));
    TEST_ASSERT_EQUAL_HEX32(0x40200000U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    uint64_t f64_1p5 = 0x3FF8000000000000ULL;
    uint64_t f64_2p5 = 0x4004000000000000ULL;
    memcpy(&f64_mm[1], &f64_1p5, 8U);
    memcpy(&f64_mm[10], &f64_2p5, 8U);
    f64_mm[18] = 0xA4U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_mm, 20U));
    TEST_ASSERT_EQUAL_HEX64(f64_1p5, syn_wasm_result(&g_wasm_ctx));
    f64_mm[18] = 0xA5U;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, run_raw_snippet(f64_mm, 20U));
    TEST_ASSERT_EQUAL_HEX64(f64_2p5, syn_wasm_result(&g_wasm_ctx));
#endif
}

static void test_wasm_spec_profile_call_indirect_and_table(void)
{
    /* Module with 3 types: T0=(i32)->i32, T1=(i32)->i32 (distinct idx, structurally equal!),
     * T2=()->i32 (mismatch); table min=4 with elem at slot 1 = fn0 (T0); fn1 calls call_indirect */
    static const uint8_t ci_wasm[] = {
        0x00,
        0x61,
        0x73,
        0x6d,
        0x01,
        0x00,
        0x00,
        0x00,
        /* Type sec: T0=(i32)->i32, T1=(i32)->i32, T2=()->i32 */
        0x01,
        0x0f,
        0x03,
        0x60,
        0x01,
        0x7f,
        0x01,
        0x7f,
        0x60,
        0x01,
        0x7f,
        0x01,
        0x7f,
        0x60,
        0x00,
        0x01,
        0x7f,
        /* Func sec: fn0:T0, fn1:T1 (calls T1), fn2:T0 (calls T2 mismatch) */
        0x03,
        0x04,
        0x03,
        0x00,
        0x01,
        0x00,
        /* Table sec: 1 table funcref min=4 */
        0x04,
        0x04,
        0x01,
        0x70,
        0x00,
        0x04,
        /* Elem sec: slot 1 = fn0 (slots 0, 2, 3 remain SYN_WASM_TABLE_NULL!) */
        0x09,
        0x07,
        0x01,
        0x00,
        0x41,
        0x01,
        0x0b,
        0x01,
        0x00,
        /* Code sec */
        0x0a,
        0x1b,
        0x03,
        /* fn0(x): x + 10 */
        0x07,
        0x00,
        0x20,
        0x00,
        0x41,
        0x0a,
        0x6a,
        0x0b,
        /* fn1(slot): call_indirect T1(arg=32, idx=slot) */
        0x09,
        0x00,
        0x41,
        0x20,
        0x20,
        0x00,
        0x11,
        0x01,
        0x00,
        0x0b,
        /* fn2(slot): call_indirect T2(idx=slot) -> signature mismatch when slot==1 */
        0x07,
        0x00,
        0x20,
        0x00,
        0x11,
        0x02,
        0x00,
        0x0b,
    };
    SYN_WASM_Module mod;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&mod, ci_wasm, (uint32_t)sizeof(ci_wasm)));
    TEST_ASSERT_EQUAL_UINT16(SYN_WASM_TABLE_NULL, mod.table_elements[0]);
    TEST_ASSERT_EQUAL_UINT16(0U, mod.table_elements[1]);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, sizeof(g_linear_mem)));

    /* Structural equivalence: fn0 has T0, called through T1 at slot 1 -> 32 + 10 = 42 */
    g_wasm_ctx.sp = 1U;
    g_wasm_ctx.stack[0] = 1ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 64U));
    TEST_ASSERT_EQUAL_UINT32(42U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Uninitialized sparse slot 0 (SYN_WASM_TABLE_NULL) -> TRAP_OUT_OF_BOUNDS */
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.sp = 1U;
    g_wasm_ctx.stack[0] = 0ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, syn_wasm_step(&g_wasm_ctx, 64U));

    /* Signature mismatch (fn2 expects T2 ()->i32, slot 1 holds fn0 (i32)->i32) */
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.sp = 1U;
    g_wasm_ctx.stack[0] = 1ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_TYPE_MISMATCH, syn_wasm_step(&g_wasm_ctx, 64U));

    /* Runtime guard: non-zero table_idx in raw bytecode traps with TRAP_INVALID_MODULE */
    static const uint8_t bad_tbl_ci[] = {0x41, 0x00, 0x11, 0x00, 0x01, 0x0b};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(bad_tbl_ci, (uint32_t)sizeof(bad_tbl_ci)));
}

static void test_wasm_spec_profile_passive_data_mem_grow_and_init_ex(void)
{
    /* 1. Passive data segment + DataCount (sec 12) + memory.init + data.drop + memory.grow */
    static uint8_t two_pages[2U * 65536U];
    static const uint8_t bulk_wasm[] = {
        0x00,
        0x61,
        0x73,
        0x6d,
        0x01,
        0x00,
        0x00,
        0x00,
        /* Type 0: ()->i32, Type 1: (i32)->i32 */
        0x01,
        0x0a,
        0x02,
        0x60,
        0x00,
        0x01,
        0x7f,
        0x60,
        0x01,
        0x7f,
        0x01,
        0x7f,
        /* Func 0: T0 (passive init + drop), Func 1: T1 (memory.grow + size), Func 2: T0 */
        0x03,
        0x04,
        0x03,
        0x00,
        0x01,
        0x00,
        /* Memory: min=1, max=2 */
        0x05,
        0x04,
        0x01,
        0x01,
        0x01,
        0x02,
        /* DataCount: 1 segment */
        0x0c,
        0x01,
        0x01,
        /* Code */
        0x0a,
        0x3c,
        0x03,
        /* fn0: memory.init 0,0(dst=4, src=1, n=3); data.drop 0; memory.init 0,0(0,0,0); load8_u(5)
         */
        0x1e,
        0x00,
        0x41,
        0x04,
        0x41,
        0x01,
        0x41,
        0x03,
        0xfc,
        0x08,
        0x00,
        0x00,
        0xfc,
        0x09,
        0x00,
        0x41,
        0x00,
        0x41,
        0x00,
        0x41,
        0x00,
        0xfc,
        0x08,
        0x00,
        0x00,
        0x41,
        0x05,
        0x2d,
        0x00,
        0x00,
        0x0b,
        /* fn1(delta): memory.grow(delta) * 32 + memory.size() */
        0x0c,
        0x00,
        0x20,
        0x00,
        0x40,
        0x00,
        0x41,
        0x20,
        0x6c,
        0x3f,
        0x00,
        0x6a,
        0x0b,
        /* fn2: memory.init 0,0(dst=0, src=0, n=1) -> traps after fn0 dropped seg 0 */
        0x0e,
        0x00,
        0x41,
        0x00,
        0x41,
        0x00,
        0x41,
        0x01,
        0xfc,
        0x08,
        0x00,
        0x00,
        0x41,
        0x00,
        0x0b,
        /* Data section: 1 passive segment (flags=1) "ABCD" */
        0x0b,
        0x07,
        0x01,
        0x01,
        0x04,
        0x41,
        0x42,
        0x43,
        0x44,
    };
    SYN_WASM_Module mod;
    SYN_WASM_LoadError lerr = SYN_WASM_LOAD_ERR_MALFORMED;
    TEST_ASSERT_TRUE(syn_wasm_module_load_ex(&mod, bulk_wasm, (uint32_t)sizeof(bulk_wasm), &lerr));
    TEST_ASSERT_EQUAL(SYN_WASM_LOAD_OK, lerr);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, two_pages, (uint32_t)sizeof(two_pages)));
    TEST_ASSERT_EQUAL_UINT32(65536U, g_wasm_ctx.linear_mem_size);

    /* fn0 copies "BCD" to offset 4..6, drops seg 0, zero-len init ok, returns 'C' (0x43) */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 64U));
    TEST_ASSERT_EQUAL_UINT32(0x43U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Non-zero memory.init on dropped seg 0 traps with TRAP_OUT_OF_BOUNDS */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, syn_wasm_step(&g_wasm_ctx, 64U));

    /* fn1 memory.grow: delta=0 -> 1*32+1=33; delta=1 -> 1*32+2=34; delta=1 (>max=2) -> -1*32+2 */
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &mod, two_pages, (uint32_t)sizeof(two_pages)));
    g_wasm_ctx.sp = 1U;
    g_wasm_ctx.stack[0] = 0ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 64U));
    TEST_ASSERT_EQUAL_UINT32(33U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    g_wasm_ctx.sp = 1U;
    g_wasm_ctx.stack[0] = 1ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 64U));
    TEST_ASSERT_EQUAL_UINT32(34U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
    TEST_ASSERT_EQUAL_UINT32(131072U, g_wasm_ctx.linear_mem_size);

    g_wasm_ctx.sp = 1U;
    g_wasm_ctx.stack[0] = 1ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 64U));
    TEST_ASSERT_EQUAL_INT32(-30, (int32_t)(uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* 2. syn_wasm_init_ex with imported globals used in Global, Element, and Data const exprs */
    static const uint8_t imp_glob_wasm[] = {
        0x00,
        0x61,
        0x73,
        0x6d,
        0x01,
        0x00,
        0x00,
        0x00,
        0x01,
        0x04,
        0x01,
        0x60,
        0x00,
        0x00,
        /* Import 2 immutable i32 globals (g0=elem_off, g1=data_off) */
        0x02,
        0x11,
        0x02,
        0x01,
        0x65,
        0x02,
        0x67,
        0x30,
        0x03,
        0x7f,
        0x00,
        0x01,
        0x65,
        0x02,
        0x67,
        0x31,
        0x03,
        0x7f,
        0x00,
        0x03,
        0x02,
        0x01,
        0x00,
        0x04,
        0x04,
        0x01,
        0x70,
        0x00,
        0x04,
        0x05,
        0x03,
        0x01,
        0x00,
        0x01,
        /* Internal globals: g2 = global.get 0, g3 = f32.const 1.5, g4 = f64.const 2.5 */
        0x06,
        0x1a,
        0x03,
        0x7f,
        0x00,
        0x23,
        0x00,
        0x0b,
        0x7d,
        0x00,
        0x43,
        0x00,
        0x00,
        0xc0,
        0x3f,
        0x0b,
        0x7c,
        0x00,
        0x44,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x04,
        0x40,
        0x0b,
        /* Elem at global.get 0 -> fn0 */
        0x09,
        0x07,
        0x01,
        0x00,
        0x23,
        0x00,
        0x0b,
        0x01,
        0x00,
        0x0a,
        0x04,
        0x01,
        0x02,
        0x00,
        0x0b,
        /* Data at global.get 1 -> 0x5A */
        0x0b,
        0x07,
        0x01,
        0x00,
        0x23,
        0x01,
        0x0b,
        0x01,
        0x5a,
    };
#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
    TEST_ASSERT_TRUE(syn_wasm_module_load(&mod, imp_glob_wasm, (uint32_t)sizeof(imp_glob_wasm)));
    uint64_t imp_vals[2] = {2ULL, 8ULL};
    TEST_ASSERT_TRUE(syn_wasm_init_ex(&g_wasm_ctx, &mod, two_pages, 65536U, imp_vals, 2U));
    TEST_ASSERT_EQUAL_UINT64(2ULL, g_wasm_ctx.globals[2]);
    TEST_ASSERT_EQUAL_HEX64(0x3FC00000ULL, g_wasm_ctx.globals[3]);
    TEST_ASSERT_EQUAL_HEX64(0x4004000000000000ULL, g_wasm_ctx.globals[4]);
    TEST_ASSERT_EQUAL_UINT16(0U, g_wasm_ctx.table_elements[2]);
    TEST_ASSERT_EQUAL_HEX8(0x5AU, two_pages[8]);

    /* OOB elem offset via imported global traps at syn_wasm_init_ex */
    imp_vals[0] = 10ULL;
    TEST_ASSERT_FALSE(syn_wasm_init_ex(&g_wasm_ctx, &mod, two_pages, 65536U, imp_vals, 2U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, g_wasm_ctx.status);
#else
    (void)imp_glob_wasm;
    (void)two_pages;
#endif
}

static void expect_load_err(const uint8_t *wasm, uint32_t len, SYN_WASM_LoadError expected)
{
    SYN_WASM_Module mod;
    SYN_WASM_LoadError err = SYN_WASM_LOAD_OK;
    TEST_ASSERT_FALSE(syn_wasm_module_load_ex(&mod, wasm, len, &err));
    TEST_ASSERT_EQUAL(expected, err);
}

static void test_wasm_spec_profile_module_load_ex_validation(void)
{
    /* 1. Error strings + NULL / header checks */
    TEST_ASSERT_EQUAL_STRING("OK", syn_wasm_load_error_str(SYN_WASM_LOAD_OK));
    TEST_ASSERT_NOT_NULL(syn_wasm_load_error_str(SYN_WASM_LOAD_ERR_INVALID_ARG));
    TEST_ASSERT_NOT_NULL(syn_wasm_load_error_str(SYN_WASM_LOAD_ERR_BAD_HEADER));
    TEST_ASSERT_NOT_NULL(syn_wasm_load_error_str(SYN_WASM_LOAD_ERR_MALFORMED));
    TEST_ASSERT_NOT_NULL(syn_wasm_load_error_str(SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED));
    TEST_ASSERT_NOT_NULL(syn_wasm_load_error_str(SYN_WASM_LOAD_ERR_UNSUPPORTED));
    TEST_ASSERT_NOT_NULL(syn_wasm_load_error_str((SYN_WASM_LoadError)99));

    SYN_WASM_Module mod;
    SYN_WASM_LoadError err = SYN_WASM_LOAD_OK;
    static const uint8_t bad_ver[] = {0x00, 0x61, 0x73, 0x6d, 0x02, 0x00, 0x00, 0x00};
    TEST_ASSERT_FALSE(syn_wasm_module_load_ex(NULL, bad_ver, 8U, &err));
    TEST_ASSERT_EQUAL(SYN_WASM_LOAD_ERR_INVALID_ARG, err);
    TEST_ASSERT_FALSE(syn_wasm_module_load_ex(&mod, NULL, 8U, NULL));
    expect_load_err(bad_ver, 4U, SYN_WASM_LOAD_ERR_INVALID_ARG);
    expect_load_err(bad_ver, 8U, SYN_WASM_LOAD_ERR_BAD_HEADER);

    /* 2. Custom section (UTF-8 valid vs invalid), duplicate section, out-of-order section */
    static const uint8_t custom_ok[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                        0x00, 0x00, 0x04, 0x02, 0x6f, 0x6b, 0xff};
    static const uint8_t custom_bad_utf8[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00,
                                              0x00, 0x00, 0x00, 0x02, 0x01, 0xC0};
    static const uint8_t out_of_order[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                           0x00, 0x05, 0x03, 0x01, 0x00, 0x01, 0x04,
                                           0x04, 0x01, 0x70, 0x00, 0x01};
    TEST_ASSERT_TRUE(syn_wasm_module_load_ex(&mod, custom_ok, (uint32_t)sizeof(custom_ok), &err));
    expect_load_err(custom_bad_utf8, (uint32_t)sizeof(custom_bad_utf8),
                    SYN_WASM_LOAD_ERR_MALFORMED);
    expect_load_err(out_of_order, (uint32_t)sizeof(out_of_order), SYN_WASM_LOAD_ERR_MALFORMED);

    /* 3. Excluded WebAssembly 2.0+ extensions rejected with SYN_WASM_LOAD_ERR_UNSUPPORTED */
    /* Multi-result functype ()->(i32,i32) */
    static const uint8_t u_multiret[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                         0x01, 0x06, 0x01, 0x60, 0x00, 0x02, 0x7f, 0x7f};
    /* v128 param type (0x7B) */
    static const uint8_t u_v128[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                     0x01, 0x05, 0x01, 0x60, 0x01, 0x7b, 0x00};
    /* Shared memory flags = 3 */
    static const uint8_t u_shared_mem[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00,
                                           0x00, 0x05, 0x04, 0x01, 0x03, 0x01, 0x02};
    /* Multi-memory (2 memories) */
    static const uint8_t u_multimem[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                         0x05, 0x05, 0x02, 0x00, 0x01, 0x00, 0x01};
    /* Multi-table (2 tables) */
    static const uint8_t u_multitbl[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x04,
                                         0x07, 0x02, 0x70, 0x00, 0x01, 0x70, 0x00, 0x01};
    /* ref.null funcref in global const expr */
    static const uint8_t u_refnull[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                        0x06, 0x06, 0x01, 0x7f, 0x00, 0xd0, 0x70, 0x0b};
    expect_load_err(u_multiret, (uint32_t)sizeof(u_multiret), SYN_WASM_LOAD_ERR_UNSUPPORTED);
    expect_load_err(u_v128, (uint32_t)sizeof(u_v128), SYN_WASM_LOAD_ERR_UNSUPPORTED);
    expect_load_err(u_shared_mem, (uint32_t)sizeof(u_shared_mem), SYN_WASM_LOAD_ERR_UNSUPPORTED);
    expect_load_err(u_multimem, (uint32_t)sizeof(u_multimem), SYN_WASM_LOAD_ERR_UNSUPPORTED);
    expect_load_err(u_multitbl, (uint32_t)sizeof(u_multitbl), SYN_WASM_LOAD_ERR_UNSUPPORTED);
    expect_load_err(u_refnull, (uint32_t)sizeof(u_refnull), SYN_WASM_LOAD_ERR_UNSUPPORTED);

    /* Excluded opcodes inside Code body: SIMD (0xFD), Atomics (0xFE), Tail-call (0x12),
     * multivalue s33 blocktype (0x02 0x00), table.grow (0xFC 0x0F) */
    static const uint8_t op_cases[][4] = {
        {0xfd, 0x00, 0x0b, 3U}, /* SIMD */
        {0xfe, 0x00, 0x0b, 3U}, /* Threads/Atomics */
        {0x12, 0x00, 0x0b, 3U}, /* return_call */
        {0x02, 0x00, 0x0b, 3U}, /* s33 block type_idx 0 */
        {0xfc, 0x0f, 0x0b, 3U}, /* table.grow */
    };
    for (size_t i = 0U; i < sizeof(op_cases) / sizeof(op_cases[0]); i++) {
        uint8_t clen = op_cases[i][3];
        uint8_t wbuf[32] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04,
                            0x01, 0x60, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a};
        wbuf[19] = (uint8_t)(3U + clen);
        wbuf[20] = 0x01U;
        wbuf[21] = (uint8_t)(1U + clen);
        wbuf[22] = 0x00U;
        memcpy(&wbuf[23], op_cases[i], clen);
        expect_load_err(wbuf, (uint32_t)(23U + clen), SYN_WASM_LOAD_ERR_UNSUPPORTED);
    }

    /* 4. Duplicate export name & invalid bytecode operand types rejected with MALFORMED */
    static const uint8_t dup_exp[] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00,
                                      0x05, 0x03, 0x01, 0x00, 0x01, 0x07, 0x09, 0x02,
                                      0x01, 0x6d, 0x02, 0x00, 0x01, 0x6d, 0x02, 0x00};
    /* i32.add with only one i32 on stack */
    static const uint8_t bad_type_code[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x60, 0x00, 0x01,
        0x7f, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x07, 0x01, 0x05, 0x00, 0x41, 0x01, 0x6a, 0x0b};
    /* Polymorphic Wasm 2.0 br_table meet-bottom (block f64 (block f32 unreachable br_table 0 1)) */
    static const uint8_t meet_bottom_ok[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x04, 0x01, 0x60,
        0x00, 0x00, 0x03, 0x02, 0x01, 0x00, 0x0a, 0x1c, 0x01, 0x1a, 0x00, 0x02,
        0x7c, 0x02, 0x7d, 0x00, 0x41, 0x01, 0x0e, 0x01, 0x00, 0x01, 0x0b, 0x1a,
        0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x1a, 0x0b,
    };
    expect_load_err(dup_exp, (uint32_t)sizeof(dup_exp), SYN_WASM_LOAD_ERR_MALFORMED);
    expect_load_err(bad_type_code, (uint32_t)sizeof(bad_type_code), SYN_WASM_LOAD_ERR_MALFORMED);
#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
    TEST_ASSERT_TRUE(
        syn_wasm_module_load_ex(&mod, meet_bottom_ok, (uint32_t)sizeof(meet_bottom_ok), &err));
#else
    expect_load_err(meet_bottom_ok, (uint32_t)sizeof(meet_bottom_ok),
                    SYN_WASM_LOAD_ERR_UNSUPPORTED);
#endif
}

static uint32_t write_leb_u32(uint8_t *dst, uint32_t val)
{
    uint32_t n = 0U;
    do {
        uint8_t b = (uint8_t)(val & 0x7FU);
        val >>= 7U;
        if (val != 0U) {
            b |= 0x80U;
        }
        dst[n++] = b;
    } while (val != 0U);
    return n;
}

static void append_sec(uint8_t *buf, uint32_t *pos, uint8_t sec_id, const uint8_t *payload,
                       uint32_t payload_len)
{
    buf[(*pos)++] = sec_id;
    *pos += write_leb_u32(&buf[*pos], payload_len);
    if (payload_len > 0U) {
        memcpy(&buf[*pos], payload, payload_len);
        *pos += payload_len;
    }
}

#define CB_TBL 0x01U
#define CB_MEM 0x02U
#define CB_GLB 0x04U
#define CB_SIG_I32_I32 0x08U
#define CB_SIG_VOID_I32 0x10U

static void expect_code_body(uint8_t flags, const uint8_t *body, uint32_t body_len,
                             SYN_WASM_LoadError expected)
{
    static uint8_t wbuf[1024];
    static const uint8_t hdr[8] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};
    static const uint8_t t_vv[] = {0x01, 0x60, 0x00, 0x00};
    static const uint8_t t_ii[] = {0x01, 0x60, 0x01, 0x7f, 0x01, 0x7f};
    static const uint8_t t_vi[] = {0x01, 0x60, 0x00, 0x01, 0x7f};
    static const uint8_t f_sec[] = {0x01, 0x00};
    static const uint8_t tbl_sec[] = {0x01, 0x70, 0x00, 0x02};
    static const uint8_t mem_sec[] = {0x01, 0x00, 0x01};
    static const uint8_t glb_sec[] = {0x01, 0x7f, 0x00, 0x41, 0x00, 0x0b};

    uint32_t pos = 0U;
    memcpy(&wbuf[pos], hdr, sizeof(hdr));
    pos += (uint32_t)sizeof(hdr);

    if ((flags & CB_SIG_I32_I32) != 0U) {
        append_sec(wbuf, &pos, 0x01U, t_ii, (uint32_t)sizeof(t_ii));
    } else if ((flags & CB_SIG_VOID_I32) != 0U) {
        append_sec(wbuf, &pos, 0x01U, t_vi, (uint32_t)sizeof(t_vi));
    } else {
        append_sec(wbuf, &pos, 0x01U, t_vv, (uint32_t)sizeof(t_vv));
    }
    append_sec(wbuf, &pos, 0x03U, f_sec, (uint32_t)sizeof(f_sec));
    if ((flags & CB_TBL) != 0U) {
        append_sec(wbuf, &pos, 0x04U, tbl_sec, (uint32_t)sizeof(tbl_sec));
    }
    if ((flags & CB_MEM) != 0U) {
        append_sec(wbuf, &pos, 0x05U, mem_sec, (uint32_t)sizeof(mem_sec));
    }
    if ((flags & CB_GLB) != 0U) {
        append_sec(wbuf, &pos, 0x06U, glb_sec, (uint32_t)sizeof(glb_sec));
    }

    static uint8_t c_pay[768];
    uint32_t cpos = 0U;
    c_pay[cpos++] = 0x01U;
    cpos += write_leb_u32(&c_pay[cpos], body_len);
    memcpy(&c_pay[cpos], body, body_len);
    cpos += body_len;
    append_sec(wbuf, &pos, 0x0AU, c_pay, cpos);

    SYN_WASM_Module mod;
    SYN_WASM_LoadError err = SYN_WASM_LOAD_OK;
    bool ok = syn_wasm_module_load_ex(&mod, wbuf, pos, &err);
    TEST_ASSERT_EQUAL(expected == SYN_WASM_LOAD_OK, ok);
    TEST_ASSERT_EQUAL(expected, err);
}

static void test_wasm_spec_profile_section_and_limit_guards(void)
{
    static uint8_t wbuf[1024];
    static uint8_t pay[768];
    static const uint8_t hdr[8] = {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00};

    /* 1. Signed LEB128 truncation/overflow, valtypes, limits, section headers */
    static const struct {
        uint8_t bytes[28];
        uint8_t len;
        SYN_WASM_LoadError err;
    } sec_cases[] = {
        /* Global i32 LEB truncated */
        {{0x06, 0x05, 0x01, 0x7f, 0x00, 0x41, 0x80}, 7U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Global i32 LEB 5-byte overflow */
        {{0x06, 0x0a, 0x01, 0x7f, 0x00, 0x41, 0x80, 0x80, 0x80, 0x80, 0x08, 0x0b},
         12U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        /* Global i64 LEB 10-byte overflow */
        {{0x06, 0x0f, 0x01, 0x7e, 0x00, 0x42, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
          0x02, 0x0b},
         17U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        /* Type bad param valtype 0x7a */
        {{0x01, 0x05, 0x01, 0x60, 0x01, 0x7a, 0x00}, 7U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Type unsupported result valtype v128 (0x7b) */
        {{0x01, 0x05, 0x01, 0x60, 0x00, 0x01, 0x7b}, 7U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        /* Memory min_pages = 65537 (> 65536) */
        {{0x05, 0x05, 0x01, 0x00, 0x81, 0x80, 0x04}, 7U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Table empty, truncated entry, and bad elemtype */
        {{0x04, 0x00}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x04, 0x01, 0x01}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x04, 0x04, 0x01, 0x7f, 0x00, 0x01}, 6U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Memory empty */
        {{0x05, 0x00}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Import global unsupported valtype (v128) & invalid mut_flag (2) */
        {{0x02, 0x08, 0x01, 0x01, 0x61, 0x01, 0x62, 0x03, 0x7b, 0x00},
         10U,
         SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {{0x02, 0x08, 0x01, 0x01, 0x61, 0x01, 0x62, 0x03, 0x7f, 0x02},
         10U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        /* Global empty, truncated header, truncated const expr, bad mut_flag */
        {{0x06, 0x00}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x06, 0x02, 0x01, 0x7f}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x06, 0x03, 0x01, 0x7f, 0x00}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x06, 0x06, 0x01, 0x7f, 0x02, 0x41, 0x00, 0x0b}, 8U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Export table OK (tested below) vs missing table */
        {{0x07, 0x05, 0x01, 0x01, 0x74, 0x01, 0x00}, 7U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Start function with non-void signature ()->i32 */
        {{0x01, 0x05, 0x01, 0x60, 0x00, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x08, 0x01, 0x00},
         14U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        /* Elem truncated flags, bad const expr WITH table, truncated num_funcs, OOB fn_idx */
        {{0x09, 0x01, 0x01}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x04, 0x04, 0x01, 0x70, 0x00, 0x02, 0x09, 0x06, 0x01, 0x00, 0x42, 0x00, 0x0b, 0x00},
         14U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x04, 0x04, 0x01, 0x70, 0x00, 0x02, 0x09, 0x05, 0x01, 0x00, 0x41, 0x00, 0x0b},
         13U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x04, 0x04, 0x01, 0x70, 0x00, 0x02, 0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x01, 0x05},
         15U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        /* DataCount empty, Data truncated flags, active Data without mem, active Data bad expr,
         * passive Data truncated payload */
        {{0x0c, 0x00}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x0b, 0x01, 0x01}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x0b, 0x06, 0x01, 0x00, 0x41, 0x00, 0x0b, 0x00}, 8U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x05, 0x03, 0x01, 0x00, 0x01, 0x0b, 0x06, 0x01, 0x00, 0x42, 0x00, 0x0b, 0x00},
         13U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x0b, 0x03, 0x01, 0x01, 0x05}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* Unknown section ID 99 & Function section missing Code section */
        {{0x63, 0x00}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {{0x01, 0x04, 0x01, 0x60, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00},
         10U,
         SYN_WASM_LOAD_ERR_MALFORMED},
    };
    for (size_t i = 0U; i < sizeof(sec_cases) / sizeof(sec_cases[0]); i++) {
        memcpy(wbuf, hdr, 8U);
        memcpy(&wbuf[8], sec_cases[i].bytes, sec_cases[i].len);
        expect_load_err(wbuf, 8U + (uint32_t)sec_cases[i].len, sec_cases[i].err);
    }

    /* 2. Resource limit checks (SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED) */
    /* >64 types */
    uint32_t pos = 8U;
    memcpy(wbuf, hdr, 8U);
    pay[0] = 65U;
    append_sec(wbuf, &pos, 0x01U, pay, 1U);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* >WASM_MAX_FUNC_PARAMS (SYN_WASM_MAX_STACK = 64) params in functype */
    pos = 8U;
    pay[0] = 1U;
    pay[1] = 0x60U;
    pay[2] = (uint8_t)(SYN_WASM_MAX_STACK + 1U);
    memset(&pay[3], 0x7f, SYN_WASM_MAX_STACK + 1U);
    pay[3U + SYN_WASM_MAX_STACK + 1U] = 0U;
    append_sec(wbuf, &pos, 0x01U, pay, 4U + (uint32_t)SYN_WASM_MAX_STACK + 1U);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* >16 globals */
    pos = 8U;
    uint32_t p = 0U;
    pay[p++] = 17U;
    for (uint32_t g = 0U; g < 17U; g++) {
        static const uint8_t gdef[5] = {0x7f, 0x00, 0x41, 0x00, 0x0b};
        memcpy(&pay[p], gdef, 5U);
        p += 5U;
    }
    append_sec(wbuf, &pos, 0x06U, pay, p);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* >64 functions */
    pos = 8U;
    static const uint8_t t1[] = {0x01, 0x60, 0x00, 0x00};
    append_sec(wbuf, &pos, 0x01U, t1, 4U);
    pay[0] = 65U;
    append_sec(wbuf, &pos, 0x03U, pay, 1U);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* >64 exports */
    pos = 8U;
    pay[0] = 65U;
    append_sec(wbuf, &pos, 0x07U, pay, 1U);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* Elem segment exceeding WASM_MAX_TABLE_ELEMS (table min=300, elem_off=256, 1 fn) */
    pos = 8U;
    append_sec(wbuf, &pos, 0x01U, t1, 4U);
    static const uint8_t f1[] = {0x01, 0x00};
    append_sec(wbuf, &pos, 0x03U, f1, 2U);
    static const uint8_t tbl300[] = {0x01, 0x70, 0x00, 0xAC, 0x02};
    append_sec(wbuf, &pos, 0x04U, tbl300, 5U);
    static const uint8_t el256[] = {0x01, 0x00, 0x41, 0x80, 0x02, 0x0b, 0x01, 0x00};
    append_sec(wbuf, &pos, 0x09U, el256, (uint32_t)sizeof(el256));
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* >64 passive data segments (without DataCount, and with DataCount=65) */
    pos = 8U;
    p = 0U;
    pay[p++] = 65U;
    for (uint32_t s = 0U; s < 65U; s++) {
        pay[p++] = 0x01U;
        pay[p++] = 0x00U;
    }
    append_sec(wbuf, &pos, 0x0bU, pay, p);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    pos = 8U;
    uint8_t dc65[1] = {65U};
    append_sec(wbuf, &pos, 0x0cU, dc65, 1U);
    append_sec(wbuf, &pos, 0x0bU, pay, p);
    expect_load_err(wbuf, pos, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);
}

static void test_wasm_spec_profile_bytecode_validator_and_raw_guards(void)
{
    static const struct {
        uint8_t flags;
        uint8_t body[20];
        uint8_t len;
        SYN_WASM_LoadError err;
    } cb_cases[] = {
        /* Valid nop + br_if on value-returning block (hits OP_NOP and push_back) */
        {0U,
         {0x00, 0x01, 0x02, 0x7f, 0x41, 0x07, 0x41, 0x01, 0x0d, 0x00, 0x0b, 0x1a, 0x0b},
         13U,
         SYN_WASM_LOAD_OK},
        /* Valid f32.const + drop */
        {0U, {0x00, 0x43, 0x00, 0x00, 0x80, 0x3f, 0x1a, 0x0b}, 8U, SYN_WASM_LOAD_OK},
        /* block (result i32) end with empty stack */
        {0U, {0x00, 0x02, 0x7f, 0x0b, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* block truncated bt, truncated s33, unsupported v128 bt, if without i32 */
        {0U, {0x00, 0x02}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x02, 0x80}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x02, 0x7b, 0x0b, 0x0b}, 5U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {0U, {0x00, 0x04, 0x40, 0x0b, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* else outside if, else stack height mismatch, if(result i32) without else, end extra */
        {0U, {0x00, 0x05, 0x0b}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U,
         {0x00, 0x41, 0x01, 0x04, 0x40, 0x41, 0x02, 0x05, 0x0b, 0x0b},
         10U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {0U,
         {0x00, 0x41, 0x01, 0x04, 0x7f, 0x41, 0x02, 0x0b, 0x1a, 0x0b},
         10U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x02, 0x40, 0x41, 0x01, 0x0b, 0x0b}, 7U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* br_table: OOB list lbl, void/val mismatch, no idx, trunc list, OOB def, want_val empty */
        {0U, {0x00, 0x41, 0x00, 0x0e, 0x01, 0x05, 0x00, 0x0b}, 8U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U,
         {0x00, 0x02, 0x7f, 0x02, 0x40, 0x41, 0x00, 0x0e, 0x01, 0x01, 0x00, 0x0b, 0x41, 0x00, 0x0b,
          0x1a, 0x0b},
         17U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x0e, 0x00, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x41, 0x00, 0x0e, 0x02, 0x00}, 6U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x41, 0x00, 0x0e, 0x00, 0x05, 0x0b}, 7U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U,
         {0x00, 0x02, 0x7f, 0x41, 0x00, 0x0e, 0x00, 0x00, 0x0b, 0x1a, 0x0b},
         11U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        /* call missing arg, br OOB, br_if empty, call OOB */
        {CB_SIG_I32_I32, {0x00, 0x10, 0x00, 0x0b}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x0c, 0x05, 0x0b}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x0d, 0x00, 0x0b}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x10, 0x09, 0x0b}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* call_indirect: bad type, non-zero table w/ & w/o table, empty stack */
        {CB_TBL, {0x00, 0x11, 0x09, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_TBL, {0x00, 0x11, 0x00, 0x01, 0x0b}, 5U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {0U, {0x00, 0x11, 0x00, 0x01, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_TBL, {0x00, 0x11, 0x00, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* select mismatch, select t* (0x1c), bad op 0x1d, local/global errors */
        {0U,
         {0x00, 0x41, 0x01, 0x42, 0x02, 0x41, 0x01, 0x1b, 0x1a, 0x0b},
         10U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x1c, 0x01, 0x7f, 0x0b}, 5U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {0U, {0x00, 0x1d, 0x0b}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x20}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x20, 0x05, 0x0b}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_SIG_I32_I32, {0x00, 0x21, 0x00, 0x41, 0x00, 0x0b}, 6U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x23, 0x05, 0x0b}, 4U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_GLB, {0x00, 0x41, 0x01, 0x24, 0x00, 0x0b}, 6U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* memory ops: no mem, bad mem_idx, mem.grow empty, over-aligned, store/load empty */
        {0U, {0x00, 0x41, 0x00, 0x28, 0x02, 0x00, 0x1a, 0x0b}, 8U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0x3f, 0x01, 0x1a, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0x40, 0x00, 0x1a, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0x41, 0x00, 0x28, 0x03, 0x00, 0x1a, 0x0b}, 8U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0x36, 0x02, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0x28, 0x02, 0x00, 0x1a, 0x0b}, 6U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* f32.const truncated, i32.add empty stack, 0xFC truncated, data.drop/mem.init/fill errs */
        {0U, {0x00, 0x43, 0x00}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x6a, 0x0b}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0xfc}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0xfc, 0x09, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0xfc, 0x08, 0x00, 0x00, 0x0b}, 6U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0xfc, 0x0b, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0xfc, 0x0b, 0x01, 0x0b}, 5U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {CB_MEM, {0x00, 0xfc, 0x0b, 0x80, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        {CB_MEM, {0x00, 0xfc, 0x0b, 0x00, 0x0b}, 5U, SYN_WASM_LOAD_ERR_MALFORMED},
        /* locals init errors & ref.null / invalid opcode / missing OP_END */
        {0U, {0x80}, 1U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x01, 0x01}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x01, 0x01, 0x7b, 0x0b}, 4U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {CB_SIG_I32_I32,
         {0x01, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x7f, 0x41, 0x00, 0x0b},
         10U,
         SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x01, 0x81, 0x02, 0x7f, 0x0b}, 5U, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED},
        {0U, {0x00, 0xd0, 0x70, 0x0b}, 4U, SYN_WASM_LOAD_ERR_UNSUPPORTED},
        {0U, {0x00, 0xc5, 0x0b}, 3U, SYN_WASM_LOAD_ERR_MALFORMED},
        {0U, {0x00, 0x01}, 2U, SYN_WASM_LOAD_ERR_MALFORMED},
    };
    for (size_t i = 0U; i < sizeof(cb_cases) / sizeof(cb_cases[0]); i++) {
        expect_code_body(cb_cases[i].flags, cb_cases[i].body, cb_cases[i].len, cb_cases[i].err);
    }

#if !defined(SYN_WASM_USE_FIXED) || !SYN_WASM_USE_FIXED
    static const uint8_t f64_trc[] = {0x00, 0x44, 0x00};
    static const uint8_t sat_ok[] = {0x00, 0x43, 0x00, 0x00, 0x80, 0x3f, 0xfc, 0x00, 0x1a, 0x0b};
    static const uint8_t sat_emp[] = {0x00, 0xfc, 0x00, 0x0b};
    expect_code_body(0U, f64_trc, (uint32_t)sizeof(f64_trc), SYN_WASM_LOAD_ERR_MALFORMED);
    expect_code_body(0U, sat_ok, (uint32_t)sizeof(sat_ok), SYN_WASM_LOAD_OK);
    expect_code_body(0U, sat_emp, (uint32_t)sizeof(sat_emp), SYN_WASM_LOAD_ERR_MALFORMED);
#endif

    /* Validator stack overflow (129 pushes, and 128 pushes + call ()->i32) & label overflow (65) */
    static uint8_t big[512];
    uint32_t bp = 0U;
    big[bp++] = 0x00U;
    for (uint32_t k = 0U; k < 129U; k++) {
        big[bp++] = 0x41U;
        big[bp++] = 0x00U;
    }
    big[bp++] = 0x0bU;
    expect_code_body(0U, big, bp, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    bp = 0U;
    big[bp++] = 0x00U;
    for (uint32_t k = 0U; k < (uint32_t)SYN_WASM_MAX_STACK; k++) {
        big[bp++] = 0x41U;
        big[bp++] = 0x00U;
    }
    big[bp++] = 0x10U;
    big[bp++] = 0x00U;
    big[bp++] = 0x0bU;
    expect_code_body(CB_SIG_VOID_I32, big, bp, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    bp = 0U;
    big[bp++] = 0x00U;
    for (uint32_t k = 0U; k < 65U; k++) {
        big[bp++] = 0x02U;
        big[bp++] = 0x40U;
    }
    for (uint32_t k = 0U; k < 66U; k++) {
        big[bp++] = 0x0bU;
    }
    expect_code_body(0U, big, bp, SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED);

    /* Raw unvalidated runtime & syn_wasm_init_ex defensive guards */
    static const uint8_t r_msz1[] = {0x3f, 0x01, 0x0b};
    static const uint8_t r_mgr1[] = {0x41, 0x00, 0x40, 0x01, 0x0b};
    static const uint8_t r_min1[] = {0xfc, 0x08, 0x00, 0x01, 0x0b};
    static const uint8_t r_minu[] = {0xfc, 0x08, 0x00, 0x00, 0x0b};
    static const uint8_t r_dd64[] = {0xfc, 0x09, 0x40, 0x0b};
    static const uint8_t r_mcp1[] = {0xfc, 0x0a, 0x00, 0x01, 0x0b};
    static const uint8_t r_mfl1[] = {0xfc, 0x0b, 0x01, 0x0b};
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(r_msz1, (uint32_t)sizeof(r_msz1)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(r_mgr1, (uint32_t)sizeof(r_mgr1)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(r_min1, (uint32_t)sizeof(r_min1)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_UNDERFLOW,
                      run_raw_snippet(r_minu, (uint32_t)sizeof(r_minu)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(r_dd64, (uint32_t)sizeof(r_dd64)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(r_mcp1, (uint32_t)sizeof(r_mcp1)));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE,
                      run_raw_snippet(r_mfl1, (uint32_t)sizeof(r_mfl1)));

    /* Raw call_indirect with type_count == 0 */
    static const uint8_t r_ci[] = {0x41, 0x00, 0x11, 0x00, 0x00, 0x0b};
    memset(&g_wasm_mod, 0, sizeof(g_wasm_mod));
    g_wasm_mod.bytes = r_ci;
    g_wasm_mod.size = (uint32_t)sizeof(r_ci);
    g_wasm_mod.func_count = 1U;
    g_wasm_mod.funcs[0].code_size = (uint32_t)sizeof(r_ci);
    g_wasm_mod.table_element_count = 1U;
    g_wasm_mod.table_elements[0] = 0U;
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    g_wasm_ctx.call_depth = 1U;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, syn_wasm_step(&g_wasm_ctx, 16U));

    /* Raw syn_wasm_init_ex malformed Global / Elem / Data sections */
    static const uint8_t bad_inits[][18] = {
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 10U},
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x06, 0x03, 0x01, 0x7f, 0x00, 13U},
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x09, 0x00, 10U},
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x09, 0x02, 0x01, 0x01, 12U},
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x09, 0x03, 0x01, 0x00, 0x42, 13U},
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x09, 0x07, 0x01, 0x00, 0x41, 0x00, 0x0b,
         0x01, 0x05, 17U},
        {0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x03, 0x01, 0x01, 0x05, 13U},
    };
    static const uint8_t bad_lens[] = {10U, 13U, 10U, 12U, 13U, 17U, 13U};
    for (size_t i = 0U; i < sizeof(bad_lens); i++) {
        SYN_WASM_Module rmod;
        memset(&rmod, 0, sizeof(rmod));
        rmod.bytes = bad_inits[i];
        rmod.size = bad_lens[i];
        rmod.table_min_elems = 4U;
        TEST_ASSERT_FALSE(syn_wasm_init(&g_wasm_ctx, &rmod, g_linear_mem, 16U));
        TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);
    }
}

/* ── Coroutine Host Function Tests ──────────────────────────────────────── */

static uint32_t g_sync_bias_calls = 0U;
SYN_WASM_HOST_SYNC(coro_sync_add_bias_host, ctx, args, argc)
{
    const uint32_t *bias = (const uint32_t *)syn_wasm_host_user(ctx);
    g_sync_bias_calls++;
    uint64_t addend = (bias != NULL) ? (uint64_t)(*bias) : 0ULL;
    uint64_t base = (argc > 0U && args != NULL) ? args[0] : 0ULL;
    return base + addend;
}

SYN_WASM_HOST_SYNC(coro_sync_void_host, ctx, args, argc)
{
    (void)ctx;
    (void)args;
    (void)argc;
    g_sync_bias_calls++;
    return 999ULL;
}

static void test_wasm_host_sync_multi_call_single_quantum(void)
{
    /* Module with:
     *   import 0: (i32) -> i32
     *   import 1: () -> ()
     *   func 2 (export "run4"):
     *     call 1
     *     i32.const 10
     *     call 0
     *     call 0
     *     call 0
     *     call 0
     *     end
     */
    static const uint8_t sync_multi_wasm[] = {
        0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x09U, 0x02U,
        0x60U, 0x01U, 0x7FU, 0x01U, 0x7FU, 0x60U, 0x00U, 0x00U, 0x02U, 0x0DU, 0x02U,
        0x01U, 0x65U, 0x01U, 0x61U, 0x00U, 0x00U, 0x01U, 0x65U, 0x01U, 0x76U, 0x00U,
        0x01U, 0x03U, 0x02U, 0x01U, 0x00U, 0x07U, 0x08U, 0x01U, 0x04U, 0x72U, 0x75U,
        0x6EU, 0x34U, 0x00U, 0x02U, 0x0AU, 0x10U, 0x01U, 0x0EU, 0x00U, 0x10U, 0x01U,
        0x41U, 0x0AU, 0x10U, 0x00U, 0x10U, 0x00U, 0x10U, 0x00U, 0x10U, 0x00U, 0x0BU,
    };

    uint32_t bias = 7U;
    g_sync_bias_calls = 0U;
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, sync_multi_wasm, (uint32_t)sizeof(sync_multi_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(
        syn_wasm_register_host_ex(&g_wasm_ctx, 0U, coro_sync_add_bias_host, NULL, &bias));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 1U, coro_sync_void_host));
    /* Re-registering existing index < host_func_count preserves host_func_count */
    TEST_ASSERT_TRUE(
        syn_wasm_register_host_ex(&g_wasm_ctx, 0U, coro_sync_add_bias_host, NULL, &bias));
    TEST_ASSERT_EQUAL_UINT16(2U, g_wasm_ctx.host_func_count);

    int32_t fn = syn_wasm_find_export(&g_wasm_mod, "run4");
    TEST_ASSERT_EQUAL_INT32(2, fn);
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)fn));

    /* All 5 synchronous host calls execute inside a single syn_wasm_step() quantum */
    SYN_WASM_Status st = syn_wasm_step(&g_wasm_ctx, 32U);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, st);
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL_UINT32(5U, g_sync_bias_calls);
    TEST_ASSERT_EQUAL_UINT32(38U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Stack overflow when pushing host return value onto a full operand stack */
    g_wasm_ctx.sp = SYN_WASM_MAX_STACK;
    g_wasm_ctx.stack[SYN_WASM_MAX_STACK - 1U] = 5ULL;
    g_wasm_mod.funcs[0].param_count = 0U;
    g_wasm_mod.funcs[0].result_count = 1U;
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_STACK_OVERFLOW, g_wasm_ctx.status);
}

static const uint8_t g_coro_uart_feed[8] = {'S', 'Y', 'N', '_', 'C', 'O', 'R', 'O'};
static uint32_t g_coro_sleep_polls = 0U;

static SYN_PT_Status coro_uart_read_host(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                         uint8_t argc, uint64_t *ret)
{
    if ((pt == NULL) || (ctx == NULL) || (args == NULL) || (argc < 2U) || (ret == NULL)) {
        return PT_EXITED;
    }
    if (syn_wasm_mem_ptr(ctx, (uint32_t)args[0], (uint32_t)args[1]) == NULL) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return PT_EXITED;
    }

    PT_BEGIN(pt);
    while (ctx->host_scratch.u32[0] < (uint32_t)args[1]) {
        {
            uint8_t *dst = (uint8_t *)syn_wasm_mem_ptr(ctx, (uint32_t)args[0], (uint32_t)args[1]);
            uint32_t idx = ctx->host_scratch.u32[0];
            uint32_t rem = (uint32_t)args[1] - idx;
            uint32_t chunk = (rem > 2U) ? 2U : rem;
            for (uint32_t i = 0U; i < chunk; i++) {
                dst[idx + i] = g_coro_uart_feed[(idx + i) & 7U];
            }
            ctx->host_scratch.u32[0] = idx + chunk;
        }
        if (ctx->host_scratch.u32[0] < (uint32_t)args[1]) {
            PT_PASS(pt);
        }
    }
    *ret = args[1];
    PT_END(pt);
}

static SYN_PT_Status coro_counted_sleep_host(SYN_PT *pt, SYN_WASM_Context *ctx,
                                             const uint64_t *args, uint8_t argc, uint64_t *ret)
{
    g_coro_sleep_polls++;
    return syn_wasm_host_sleep_ms(pt, ctx, args, argc, ret);
}

static SYN_PT_Status wasm_peer_idle_counter_task(SYN_PT *pt, SYN_Task *task)
{
    uint32_t *runs = (uint32_t *)task->user_data;
    (*runs)++;
    PT_BEGIN(pt);
    for (;;) {
        PT_PASS(pt);
    }
    PT_END(pt);
}

static void test_wasm_host_sleep_scheduler_and_builtins(void)
{
    /* 1. Built-in host coroutine NULL / invalid-argument guards */
    SYN_PT bpt;
    PT_INIT(&bpt);
    uint64_t bret = 0ULL;
    uint64_t barg = 10ULL;

    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_millis(NULL, &g_wasm_ctx, NULL, 0U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_millis(&bpt, &g_wasm_ctx, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_micros(NULL, &g_wasm_ctx, NULL, 0U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_micros(&bpt, &g_wasm_ctx, NULL, 0U, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_sleep_ms(NULL, &g_wasm_ctx, &barg, 1U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_sleep_ms(&bpt, NULL, &barg, 1U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, &barg, 0U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, NULL, 1U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, &barg, 1U, NULL));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_yield(NULL, &g_wasm_ctx, NULL, 0U, &bret));
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_host_yield(&bpt, &g_wasm_ctx, NULL, 0U, NULL));

    mock_tick_ms = 0U;
    mock_tick_us = 0U;
    mock_tick_advance(123U);
    PT_INIT(&bpt);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wasm_host_micros(&bpt, &g_wasm_ctx, NULL, 0U, &bret));
    TEST_ASSERT_EQUAL_UINT64(123000ULL, bret);

    /* 2. Standalone syn_wasm_host_sleep_ms (active_task == NULL -> uses host_scratch.u32[0]) */
    memset(&g_wasm_ctx, 0, sizeof(g_wasm_ctx));
    PT_INIT(&bpt);
    barg = 0ULL;
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, &barg, 1U, &bret));
    PT_INIT(&bpt);
    barg = 20ULL;
    mock_tick_ms = 500U;
    mock_tick_us = 500000U;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, &barg, 1U, &bret));
    mock_tick_advance(19U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, &barg, 1U, &bret));
    mock_tick_advance(1U);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wasm_host_sleep_ms(&bpt, &g_wasm_ctx, &barg, 1U, &bret));

    /* 3. Multi-tick sleeping host call inside syn_wasm_task alongside high-priority peer task */
    static const uint8_t sleep_guest_wasm[] = {
        0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x09U, 0x02U, 0x60U, 0x01U,
        0x7FU, 0x00U, 0x60U, 0x00U, 0x01U, 0x7FU, 0x02U, 0x0DU, 0x02U, 0x01U, 0x65U, 0x01U, 0x73U,
        0x00U, 0x00U, 0x01U, 0x65U, 0x01U, 0x6DU, 0x00U, 0x01U, 0x03U, 0x02U, 0x01U, 0x01U, 0x07U,
        0x07U, 0x01U, 0x03U, 0x72U, 0x75U, 0x6EU, 0x00U, 0x02U, 0x0AU, 0x0DU, 0x01U, 0x0BU, 0x00U,
        0x41U, 0x32U, 0x10U, 0x00U, 0x10U, 0x01U, 0x41U, 0x07U, 0x6AU, 0x0BU,
    };

    g_coro_sleep_polls = 0U;
    mock_tick_ms = 1000U;
    mock_tick_us = 1000000U;
    TEST_ASSERT_TRUE(
        syn_wasm_module_load(&g_wasm_mod, sleep_guest_wasm, (uint32_t)sizeof(sleep_guest_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, coro_counted_sleep_host));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 1U, syn_wasm_host_millis));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));

    uint32_t hi_peer_runs = 0U;
    SYN_Task tasks[2];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "hi_peer", wasm_peer_idle_counter_task, SYN_PRIO_HIGH,
                    &hi_peer_runs);
    syn_task_create(&tasks[1], "wasm_t", syn_wasm_task, SYN_PRIO_NORMAL, &g_wasm_ctx);
    syn_sched_init(&sched, tasks, 2U);

    /* Dispatch 1: hi_peer runs & yields PT_IDLE; Dispatch 2: wasm_t arms sleep_ms(50) */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1U, hi_peer_runs);
    TEST_ASSERT_EQUAL_UINT32(1U, g_coro_sleep_polls);
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL(SYN_WASM_OK, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(1050U, tasks[1].delay_until);
    TEST_ASSERT_TRUE((tasks[1].sched_flags & SYN_SCHED_FLAG_SLEEPING) != 0U);

    /* Verify tickless wakeup deadline when peer is suspended */
    syn_task_suspend(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT32(1050U, syn_sched_next_wakeup(&sched));
    syn_task_resume(&tasks[0]);

    /* Advance 49 ms across 7 ticks (2 dispatches/tick): hi_peer runs, wasm_t NEVER busy-polls */
    for (uint32_t i = 0U; i < 7U; i++) {
        mock_tick_advance(7U);
        syn_sched_run(&sched);
        syn_sched_run(&sched);
        TEST_ASSERT_EQUAL_UINT32(1U, g_coro_sleep_polls);
        TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    }
    TEST_ASSERT_TRUE(hi_peer_runs >= 8U);

    /* Advance final 1 ms to 1050 ms: hi_peer passes, wasm_t wakes, calls millis() + 7 */
    mock_tick_advance(1U);
    syn_sched_run(&sched);
    if (g_wasm_ctx.status == SYN_WASM_OK) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_UINT32(2U, g_coro_sleep_polls);
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(1057U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* 4. Compiled C guest pipeline (test_host_api.c: run_coro_pipeline) */
    mock_tick_ms = 200U;
    mock_tick_us = 200000U;
    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, test_host_api_wasm, test_host_api_wasm_len));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, mock_host_log));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 1U, syn_wasm_host_millis));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 2U, syn_wasm_host_sleep_ms));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 3U, coro_uart_read_host));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 4U, syn_wasm_host_yield));

    int32_t pipe_fn = syn_wasm_find_export(&g_wasm_mod, "run_coro_pipeline");
    TEST_ASSERT_TRUE(pipe_fn >= 0);
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, (uint16_t)pipe_fn));

    syn_task_create(&tasks[0], "wasm_pipe", syn_wasm_task, SYN_PRIO_NORMAL, &g_wasm_ctx);
    syn_sched_init(&sched, tasks, 1U);

    for (uint32_t step = 0U; (step < 32U) && (g_wasm_ctx.status == SYN_WASM_OK); step++) {
        syn_sched_run(&sched);
        if (tasks[0].delay_until != 0U) {
            mock_tick_advance(25U);
        } else {
            mock_tick_advance(1U);
        }
    }
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);

    uint32_t expected_acc = 0U;
    for (uint32_t i = 0U; i < 8U; i++) {
        expected_acc = (expected_acc * 31U) + (uint32_t)g_coro_uart_feed[i];
    }
    TEST_ASSERT_EQUAL_UINT32(expected_acc + 25U, (uint32_t)syn_wasm_result(&g_wasm_ctx));
}

static uint32_t g_chunk_polls = 0U;
static SYN_PT_Status coro_chunk_xor_host(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                         uint8_t argc, uint64_t *ret)
{
    if ((pt == NULL) || (ctx == NULL) || (args == NULL) || (argc < 3U) || (ret == NULL)) {
        return PT_EXITED;
    }
    if ((syn_wasm_mem_ptr(ctx, (uint32_t)args[0], (uint32_t)args[2]) == NULL) ||
        (syn_wasm_mem_cptr(ctx, (uint32_t)args[1], (uint32_t)args[2]) == NULL)) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
        return PT_EXITED;
    }

    g_chunk_polls++;
    PT_BEGIN(pt);
    while (ctx->host_scratch.u32[0] < (uint32_t)args[2]) {
        {
            uint8_t *dst = (uint8_t *)syn_wasm_mem_ptr(ctx, (uint32_t)args[0], (uint32_t)args[2]);
            const uint8_t *src =
                (const uint8_t *)syn_wasm_mem_cptr(ctx, (uint32_t)args[1], (uint32_t)args[2]);
            uint32_t idx = ctx->host_scratch.u32[0];
            uint32_t rem = (uint32_t)args[2] - idx;
            uint32_t chunk = (rem > 4U) ? 4U : rem;
            uint32_t sum = ctx->host_scratch.u32[1];
            for (uint32_t i = 0U; i < chunk; i++) {
                uint8_t v = (uint8_t)(src[idx + i] ^ 0x5AU);
                dst[idx + i] = v;
                sum += (uint32_t)v;
            }
            ctx->host_scratch.u32[0] = idx + chunk;
            ctx->host_scratch.u32[1] = sum;
        }
        if (ctx->host_scratch.u32[0] < (uint32_t)args[2]) {
            PT_CONTINUE(pt);
        }
    }
    *ret = (uint64_t)ctx->host_scratch.u32[1];
    PT_END(pt);
}

static void test_wasm_host_chunked_pt_more_contended_and_spawn(void)
{
    /* Module with:
     *   type 0: (i32, i32, i32) -> i32
     *   import 0: type 0 (coro_chunk_xor_host)
     *   table 0: funcref min 1 (elem[0] = func 0)
     *   memory 0: min 1 page
     *   func 1 (export "direct"): i32.const 64, i32.const 0, i32.const 16, call 0, i32.const 1, add
     *   func 2 (export "indir"):  i32.const 64, i32.const 0, i32.const 16, i32.const 0,
     *                             call_indirect 0 0
     */
    static uint8_t page_mem[65536];
    static const uint8_t chunk_wasm[] = {
        0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x08U, 0x01U, 0x60U,
        0x03U, 0x7FU, 0x7FU, 0x7FU, 0x01U, 0x7FU, 0x02U, 0x07U, 0x01U, 0x01U, 0x65U, 0x01U,
        0x78U, 0x00U, 0x00U, 0x03U, 0x03U, 0x02U, 0x00U, 0x00U, 0x04U, 0x04U, 0x01U, 0x70U,
        0x00U, 0x01U, 0x05U, 0x03U, 0x01U, 0x00U, 0x01U, 0x07U, 0x12U, 0x02U, 0x06U, 0x64U,
        0x69U, 0x72U, 0x65U, 0x63U, 0x74U, 0x00U, 0x01U, 0x05U, 0x69U, 0x6EU, 0x64U, 0x69U,
        0x72U, 0x00U, 0x02U, 0x09U, 0x07U, 0x01U, 0x00U, 0x41U, 0x00U, 0x0BU, 0x01U, 0x00U,
        0x0AU, 0x1FU, 0x02U, 0x0EU, 0x00U, 0x41U, 0xC0U, 0x00U, 0x41U, 0x00U, 0x41U, 0x10U,
        0x10U, 0x00U, 0x41U, 0x01U, 0x6AU, 0x0BU, 0x0EU, 0x00U, 0x41U, 0xC0U, 0x00U, 0x41U,
        0x00U, 0x41U, 0x10U, 0x41U, 0x00U, 0x11U, 0x00U, 0x00U, 0x0BU,
    };

    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, chunk_wasm, (uint32_t)sizeof(chunk_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, page_mem, sizeof(page_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, coro_chunk_xor_host));

    uint32_t exp_sum = 0U;
    for (uint32_t i = 0U; i < 16U; i++) {
        page_mem[i] = (uint8_t)(i + 1U);
        exp_sum += (uint32_t)((uint8_t)((i + 1U) ^ 0x5AU));
    }

    /* 1. Uncontended syn_wasm_task: all 4 PT_MORE chunks burst in a SINGLE scheduler dispatch */
    uint32_t peer_runs = 0U;
    SYN_Task tasks[2];
    SYN_Sched sched;
    g_chunk_polls = 0U;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    syn_task_create(&tasks[0], "wasm_t", syn_wasm_task, SYN_PRIO_HIGH, &g_wasm_ctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_task_suspend(&tasks[1]);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(4U, g_chunk_polls);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(exp_sum + 1U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* 2. Contended syn_wasm_task via call_indirect: yields after each chunk so peer runs */
    g_chunk_polls = 0U;
    peer_runs = 0U;
    memset(&page_mem[64], 0, 16U);
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    syn_task_create(&tasks[0], "wasm_t", syn_wasm_task, SYN_PRIO_HIGH, &g_wasm_ctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched); /* Chunk 1 -> PT_MORE -> yields because peer is ready */
    TEST_ASSERT_EQUAL_UINT32(1U, g_chunk_polls);
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL(SYN_WASM_OK, g_wasm_ctx.status);
    syn_sched_run(&sched); /* Peer runs */
    TEST_ASSERT_EQUAL_UINT32(1U, peer_runs);

    while (g_wasm_ctx.status == SYN_WASM_OK) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_UINT32(4U, g_chunk_polls);
    TEST_ASSERT_TRUE(peer_runs >= 3U);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(exp_sum, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* 3. PT_SPAWN_IF_NEEDED(..., syn_wasm_exec_pt(...)): uncontended burst vs contended */
    WasmSpawnTaskCtx wctx = {.ctx = &g_wasm_ctx, .quantum = 32U, .completed = 0U};
    g_chunk_polls = 0U;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    syn_task_create(&tasks[0], "wasm_sp", wasm_spawn_parent_task, SYN_PRIO_HIGH, &wctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_task_suspend(&tasks[1]);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1U, wctx.completed);
    TEST_ASSERT_EQUAL_UINT32(4U, g_chunk_polls);
    TEST_ASSERT_EQUAL_UINT32(exp_sum + 1U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    g_chunk_polls = 0U;
    peer_runs = 0U;
    wctx.completed = 0U;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    syn_task_create(&tasks[0], "wasm_sp", wasm_spawn_parent_task, SYN_PRIO_HIGH, &wctx);
    syn_task_create(&tasks[1], "peer", wasm_peer_more_task, SYN_PRIO_HIGH, &peer_runs);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(0U, wctx.completed);
    TEST_ASSERT_EQUAL_UINT32(1U, g_chunk_polls);
    while (wctx.completed == 0U) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_UINT32(4U, g_chunk_polls);
    TEST_ASSERT_TRUE(peer_runs >= 3U);

    /* 4. Direct syn_wasm_call() on yielding imported functions (call_depth == 0) */
    g_chunk_polls = 0U;
    g_wasm_ctx.sp = 0U;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 64ULL;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 0ULL;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 16ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL(SYN_WASM_OK, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL(SYN_WASM_YIELDED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_YIELDED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL_UINT32(exp_sum, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Direct syn_wasm_call() on PT_IDLE host coroutine driven by syn_wasm_exec_pt & syn_wasm_task
     */
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, syn_wasm_host_yield));
    g_wasm_mod.funcs[0].param_count = 0U;
    g_wasm_mod.funcs[0].result_count = 0U;
    g_wasm_ctx.sp = 0U;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    SYN_PT ept;
    PT_INIT(&ept);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wasm_exec_pt(&ept, NULL, &g_wasm_ctx, 8U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);

    /* Also exercise syn_wasm_exec_pt yielding PT_IDLE when pending host coroutine stays waiting */
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, syn_wasm_host_sleep_ms));
    g_wasm_mod.funcs[0].param_count = 1U;
    mock_tick_ms = 10U;
    mock_tick_us = 10000U;
    g_wasm_ctx.sp = 0U;
    g_wasm_ctx.stack[g_wasm_ctx.sp++] = 30ULL;
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 0U));
    PT_INIT(&ept);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_wasm_exec_pt(&ept, NULL, &g_wasm_ctx, 8U));
    mock_tick_advance(30U);
    TEST_ASSERT_EQUAL(PT_ENDED, syn_wasm_exec_pt(&ept, NULL, &g_wasm_ctx, 8U));
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
}

#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
typedef struct {
    SYN_Mutex mtx;
    uint32_t shared_val;
    uint32_t seq;
    uint32_t low_unlock_seq;
    uint32_t med_first_seq;
    uint8_t boosted_prio_observed;
} WasmMutexTestState;

static WasmMutexTestState g_wasm_mtx_state;

static SYN_PT_Status coro_mutex_inc_host(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                         uint8_t argc, uint64_t *ret)
{
    (void)args;
    (void)argc;
    PT_BEGIN(pt);
    PT_MUTEX_LOCK(pt, ctx->active_task, &g_wasm_mtx_state.mtx);
    g_wasm_mtx_state.shared_val += 15U;
    *ret = (uint64_t)g_wasm_mtx_state.shared_val;
    (void)PT_MUTEX_UNLOCK(&g_wasm_mtx_state.mtx, ctx->active_task);
    PT_END(pt);
}

static SYN_PT_Status wasm_mtx_low_owner_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_MUTEX_LOCK(pt, task, &g_wasm_mtx_state.mtx);
    PT_PASS(pt);
    g_wasm_mtx_state.boosted_prio_observed = task->priority;
    g_wasm_mtx_state.shared_val = 10U;
    g_wasm_mtx_state.low_unlock_seq = ++g_wasm_mtx_state.seq;
    (void)PT_MUTEX_UNLOCK(&g_wasm_mtx_state.mtx, task);
    PT_END(pt);
}

static SYN_PT_Status wasm_mtx_med_peer_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    if (g_wasm_mtx_state.med_first_seq == 0U) {
        g_wasm_mtx_state.med_first_seq = ++g_wasm_mtx_state.seq;
    }
    PT_PASS(pt);
    PT_END(pt);
}

static void test_wasm_host_mutex_priority_inheritance(void)
{
    /* Module: import 0: () -> i32; func 1 (export "run"): call 0, i32.const 100, i32.add, end */
    static const uint8_t mtx_wasm[] = {
        0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x05U, 0x01U, 0x60U, 0x00U,
        0x01U, 0x7FU, 0x02U, 0x07U, 0x01U, 0x01U, 0x65U, 0x01U, 0x6DU, 0x00U, 0x00U, 0x03U, 0x02U,
        0x01U, 0x00U, 0x07U, 0x07U, 0x01U, 0x03U, 0x72U, 0x75U, 0x6EU, 0x00U, 0x01U, 0x0AU, 0x0AU,
        0x01U, 0x08U, 0x00U, 0x10U, 0x00U, 0x41U, 0xE4U, 0x00U, 0x6AU, 0x0BU,
    };

    memset(&g_wasm_mtx_state, 0, sizeof(g_wasm_mtx_state));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_init(&g_wasm_mtx_state.mtx));

    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, mtx_wasm, (uint32_t)sizeof(mtx_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, coro_mutex_inc_host));

    SYN_Task tasks[3];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "wasm_hi", syn_wasm_task, SYN_PRIO_HIGH, &g_wasm_ctx);
    syn_task_create(&tasks[1], "med_peer", wasm_mtx_med_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&tasks[2], "low_own", wasm_mtx_low_owner_task, SYN_PRIO_LOW, NULL);
    syn_task_suspend(&tasks[0]);
    syn_task_suspend(&tasks[1]);
    syn_sched_init(&sched, tasks, 3U);

    /* Step 1: low_own acquires mutex and yields PT_PASS */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_PTR(&tasks[2], syn_mutex_get_owner(&g_wasm_mtx_state.mtx));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_LOW, tasks[2].priority);

    /* Step 2: wake both high-priority Wasm task and medium-priority peer task */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 1U));
    syn_task_resume(&tasks[0]);
    syn_task_resume(&tasks[1]);

    /* Dispatch 1: wasm_hi (SYN_PRIO_HIGH) runs, blocks on PT_MUTEX_LOCK, boosts low_own -> HIGH */
    syn_sched_run(&sched);
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_HIGH, tasks[2].priority);
    TEST_ASSERT_EQUAL_UINT32(0U, g_wasm_mtx_state.med_first_seq);

    /* Dispatch 2: low_own (boosted to SYN_PRIO_HIGH) preempts med_peer, sets shared_val = 10,
     * and unlocks mutex (demoting itself back to SYN_PRIO_LOW) */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_HIGH, g_wasm_mtx_state.boosted_prio_observed);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_LOW, tasks[2].priority);
    TEST_ASSERT_EQUAL_UINT32(1U, g_wasm_mtx_state.low_unlock_seq);
    TEST_ASSERT_EQUAL_UINT32(0U, g_wasm_mtx_state.med_first_seq);

    /* Dispatch 3: wasm_hi acquires unlocked mutex, adds 15 -> 25, guest adds 100 -> 125 */
    syn_sched_run(&sched);
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(125U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Dispatch 4: med_peer runs after both high-priority critical sections complete */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(2U, g_wasm_mtx_state.med_first_seq);
    TEST_ASSERT_TRUE(g_wasm_mtx_state.low_unlock_seq < g_wasm_mtx_state.med_first_seq);
}
#endif

typedef struct {
    SYN_WASM_Context *observed_ctx;
    void *observed_user;
    uint32_t cancel_calls;
} WasmCancelTrack;

static WasmCancelTrack g_cancel_track;

static void mock_host_cancel(SYN_WASM_Context *ctx, void *user)
{
    g_cancel_track.observed_ctx = ctx;
    g_cancel_track.observed_user = user;
    g_cancel_track.cancel_calls++;
}

static SYN_PT_Status coro_trap_mode_host(SYN_PT *pt, SYN_WASM_Context *ctx, const uint64_t *args,
                                         uint8_t argc, uint64_t *ret)
{
    (void)argc;
    PT_BEGIN(pt);
    if ((args[0] & 2ULL) != 0ULL) {
        PT_PASS(pt);
    }
    if ((args[0] & 4ULL) != 0ULL) {
        syn_wasm_abort(ctx);
        return PT_EXITED;
    }
    if ((args[0] & 1ULL) != 0ULL) {
        ctx->status = SYN_WASM_TRAP_OUT_OF_BOUNDS;
    }
    if ((args[0] & 8ULL) != 0ULL) {
        ctx->status = SYN_WASM_TRAP_DIV_ZERO;
        SYN_WASM_HOST_RETURN(pt, ret, 0ULL);
    }
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_wasm_host_traps_abort_bounds_and_limits(void)
{
    /* Module: import 0: (i32) -> i32; import 1: () -> (); func 2: local.get 0, call 0, end */
    static const uint8_t trap_wasm[] = {
        0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x09U, 0x02U, 0x60U,
        0x01U, 0x7FU, 0x01U, 0x7FU, 0x60U, 0x00U, 0x00U, 0x02U, 0x0DU, 0x02U, 0x01U, 0x65U,
        0x01U, 0x74U, 0x00U, 0x00U, 0x01U, 0x65U, 0x01U, 0x75U, 0x00U, 0x01U, 0x03U, 0x02U,
        0x01U, 0x00U, 0x0AU, 0x08U, 0x01U, 0x06U, 0x00U, 0x20U, 0x00U, 0x10U, 0x00U, 0x0BU,
    };

    TEST_ASSERT_TRUE(syn_wasm_module_load(&g_wasm_mod, trap_wasm, (uint32_t)sizeof(trap_wasm)));
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &g_wasm_mod, g_linear_mem, sizeof(g_linear_mem)));
    int user_cookie = 0x55AA;
    TEST_ASSERT_TRUE(syn_wasm_register_host_ex(&g_wasm_ctx, 0U, coro_trap_mode_host,
                                               mock_host_cancel, &user_cookie));

    /* 1. PT_EXITED on poll 1 with status == OK -> SYN_WASM_TRAP_HOST */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 0ULL;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_HOST, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);

    /* 2. PT_EXITED on poll 1 with custom trap -> preserves SYN_WASM_TRAP_OUT_OF_BOUNDS */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 1ULL;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_OUT_OF_BOUNDS, syn_wasm_step(&g_wasm_ctx, 16U));

    /* 3. PT_ENDED with custom trap -> preserves SYN_WASM_TRAP_DIV_ZERO */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 8ULL;
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_DIV_ZERO, syn_wasm_step(&g_wasm_ctx, 16U));

    /* 4. PT_EXITED on poll 2 (after PT_PASS) + syn_wasm_exec_pt returning PT_EXITED on trap */
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 2ULL;
    TEST_ASSERT_EQUAL(SYN_WASM_WAITING, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);

    /* syn_wasm_call() MUST fail while host_pending is true */
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 2U));
    /* syn_wasm_register_host_ex() rejects mutating active slot 0, allows slot 1 */
    TEST_ASSERT_FALSE(syn_wasm_register_host(&g_wasm_ctx, 0U, mock_host_func));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 1U, mock_host_func));
    TEST_ASSERT_EQUAL_PTR(&user_cookie, syn_wasm_host_user(&g_wasm_ctx));

    SYN_PT ept;
    PT_INIT(&ept);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_wasm_exec_pt(&ept, NULL, &g_wasm_ctx, 16U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_HOST, g_wasm_ctx.status);
    TEST_ASSERT_NULL(syn_wasm_host_user(&g_wasm_ctx));
    TEST_ASSERT_NULL(syn_wasm_host_user(NULL));

    /* 5. syn_wasm_abort() invokes cancel(ctx, user), clears task sleep, and allows fresh call */
    SYN_Task abort_task;
    SYN_Sched abort_sched;
    mock_tick_ms = 500U;
    mock_tick_us = 500000U;
    memset(&g_cancel_track, 0, sizeof(g_cancel_track));
    TEST_ASSERT_TRUE(syn_wasm_register_host_ex(&g_wasm_ctx, 0U, syn_wasm_host_sleep_ms,
                                               mock_host_cancel, &user_cookie));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 5000ULL;
    syn_task_create(&abort_task, "wasm_ab", syn_wasm_task, SYN_PRIO_NORMAL, &g_wasm_ctx);
    syn_sched_init(&abort_sched, &abort_task, 1U);

    /* Arm 5000 ms task sleep via syn_wasm_task */
    syn_sched_run(&abort_sched);
    TEST_ASSERT_TRUE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL_UINT32(5500U, abort_task.delay_until);

    syn_wasm_abort(NULL); /* Safe NULL no-op */
    syn_wasm_abort(&g_wasm_ctx);
    TEST_ASSERT_EQUAL_UINT32(1U, g_cancel_track.cancel_calls);
    TEST_ASSERT_EQUAL_PTR(&g_wasm_ctx, g_cancel_track.observed_ctx);
    TEST_ASSERT_EQUAL_PTR(&user_cookie, g_cancel_track.observed_user);
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);
    TEST_ASSERT_EQUAL_UINT16(0U, g_wasm_ctx.call_depth);
    TEST_ASSERT_EQUAL_UINT32(0U, g_wasm_ctx.sp);
    TEST_ASSERT_EQUAL_UINT32(0U, abort_task.delay_until);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);

    /* Without advancing clock (still 500 ms < 5500 ms), fresh call executes immediately! */
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, mock_host_func));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 1ULL;
    syn_sched_run(&abort_sched);
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, g_wasm_ctx.status);
    TEST_ASSERT_EQUAL_UINT32(42U, (uint32_t)syn_wasm_result(&g_wasm_ctx));

    /* Abort when not pending or when cancel == NULL */
    g_wasm_ctx.active_task = NULL;
    syn_wasm_abort(&g_wasm_ctx);
    TEST_ASSERT_EQUAL_UINT32(1U, g_cancel_track.cancel_calls);

    /* Self-aborting host coroutine (calls syn_wasm_abort(ctx) from inside wasm_poll_host) */
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, coro_trap_mode_host));
    TEST_ASSERT_TRUE(syn_wasm_call(&g_wasm_ctx, 2U));
    g_wasm_ctx.locals[0] = 4ULL;
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, syn_wasm_step(&g_wasm_ctx, 16U));
    TEST_ASSERT_FALSE(g_wasm_ctx.host_pending);

    /* Out-of-bounds synthetic host_import_idx guards in syn_wasm_host_user & syn_wasm_abort */
    g_wasm_ctx.host_pending = true;
    g_wasm_ctx.host_import_idx = SYN_WASM_MAX_HOST_FUNCS;
    TEST_ASSERT_NULL(syn_wasm_host_user(&g_wasm_ctx));
    syn_wasm_abort(&g_wasm_ctx);

    /* 6. Load-time & runtime SYN_WASM_MAX_HOST_ARGS enforcement */
    uint8_t big_imp_wasm[64];
    uint32_t p = 0U;
    static const uint8_t hdr8[8] = {0x00U, 0x61U, 0x73U, 0x6DU, 0x01U, 0x00U, 0x00U, 0x00U};
    memcpy(&big_imp_wasm[p], hdr8, 8U);
    p += 8U;
    uint8_t nparams = (uint8_t)(SYN_WASM_MAX_HOST_ARGS + 1U);
    big_imp_wasm[p++] = 0x01U;                   /* Type section */
    big_imp_wasm[p++] = (uint8_t)(4U + nparams); /* section size */
    big_imp_wasm[p++] = 0x01U;                   /* 1 type */
    big_imp_wasm[p++] = 0x60U;                   /* func */
    big_imp_wasm[p++] = nparams;                 /* 17 params */
    memset(&big_imp_wasm[p], 0x7FU, nparams);    /* all i32 */
    p += nparams;
    big_imp_wasm[p++] = 0x00U; /* 0 results */
    static const uint8_t imp_sec[9] = {0x02U, 0x07U, 0x01U, 0x01U, 0x65U,
                                       0x01U, 0x66U, 0x00U, 0x00U};
    memcpy(&big_imp_wasm[p], imp_sec, sizeof(imp_sec));
    p += (uint32_t)sizeof(imp_sec);

    SYN_WASM_Module bmod;
    SYN_WASM_LoadError lerr = SYN_WASM_LOAD_OK;
    TEST_ASSERT_FALSE(syn_wasm_module_load_ex(&bmod, big_imp_wasm, p, &lerr));
    TEST_ASSERT_EQUAL(SYN_WASM_LOAD_ERR_LIMIT_EXCEEDED, lerr);

    /* Runtime defence-in-depth guard in wasm_invoke_func on synthetic module */
    memset(&bmod, 0, sizeof(bmod));
    bmod.func_count = 1U;
    bmod.import_func_count = 1U;
    bmod.funcs[0].param_count = (uint8_t)(SYN_WASM_MAX_HOST_ARGS + 1U);
    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &bmod, g_linear_mem, sizeof(g_linear_mem)));
    TEST_ASSERT_TRUE(syn_wasm_register_host(&g_wasm_ctx, 0U, mock_host_func));
    TEST_ASSERT_FALSE(syn_wasm_call(&g_wasm_ctx, 0U));
    TEST_ASSERT_EQUAL(SYN_WASM_TRAP_INVALID_MODULE, g_wasm_ctx.status);

    /* 7. syn_wasm_mem_ptr / syn_wasm_mem_cptr bounds & uint64_t overflow checks */
    TEST_ASSERT_NULL(syn_wasm_mem_ptr(NULL, 0U, 1U));
    TEST_ASSERT_NULL(syn_wasm_mem_cptr(NULL, 0U, 1U));

    SYN_WASM_Context no_mem_ctx;
    TEST_ASSERT_TRUE(syn_wasm_init(&no_mem_ctx, &bmod, NULL, 0U));
    TEST_ASSERT_NULL(syn_wasm_mem_ptr(&no_mem_ctx, 0U, 0U));
    TEST_ASSERT_NULL(syn_wasm_mem_cptr(&no_mem_ctx, 0U, 0U));

    TEST_ASSERT_TRUE(syn_wasm_init(&g_wasm_ctx, &bmod, g_linear_mem, 256U));
    TEST_ASSERT_EQUAL_PTR(&g_linear_mem[0], syn_wasm_mem_ptr(&g_wasm_ctx, 0U, 256U));
    TEST_ASSERT_EQUAL_PTR(&g_linear_mem[128], syn_wasm_mem_cptr(&g_wasm_ctx, 128U, 128U));
    /* Zero-length slice at exact end of linear memory is valid; 1 byte past end is NULL */
    TEST_ASSERT_EQUAL_PTR(&g_linear_mem[256], syn_wasm_mem_ptr(&g_wasm_ctx, 256U, 0U));
    TEST_ASSERT_EQUAL_PTR(&g_linear_mem[256], syn_wasm_mem_cptr(&g_wasm_ctx, 256U, 0U));
    TEST_ASSERT_NULL(syn_wasm_mem_ptr(&g_wasm_ctx, 257U, 0U));
    TEST_ASSERT_NULL(syn_wasm_mem_cptr(&g_wasm_ctx, 257U, 0U));
    TEST_ASSERT_NULL(syn_wasm_mem_ptr(&g_wasm_ctx, 256U, 1U));
    TEST_ASSERT_NULL(syn_wasm_mem_cptr(&g_wasm_ctx, 256U, 1U));
    /* 32-bit wraparound overflow (0xFFFFFFFF + 4 wraps to 3 in 32-bit math!) MUST return NULL */
    TEST_ASSERT_NULL(syn_wasm_mem_ptr(&g_wasm_ctx, 0xFFFFFFFFU, 4U));
    TEST_ASSERT_NULL(syn_wasm_mem_cptr(&g_wasm_ctx, 0xFFFFFFFFU, 4U));
    TEST_ASSERT_NULL(syn_wasm_mem_ptr(&g_wasm_ctx, 0xFFFFFF00U, 0x200U));
    TEST_ASSERT_NULL(syn_wasm_mem_cptr(&g_wasm_ctx, 0xFFFFFF00U, 0x200U));
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
    RUN_TEST(test_wasm_cooperative_exec_pt_and_task);
    RUN_TEST(test_wasm_spec_profile_sign_ext_and_float_ieee754);
    RUN_TEST(test_wasm_spec_profile_call_indirect_and_table);
    RUN_TEST(test_wasm_spec_profile_passive_data_mem_grow_and_init_ex);
    RUN_TEST(test_wasm_spec_profile_module_load_ex_validation);
    RUN_TEST(test_wasm_spec_profile_section_and_limit_guards);
    RUN_TEST(test_wasm_spec_profile_bytecode_validator_and_raw_guards);
    RUN_TEST(test_wasm_host_sync_multi_call_single_quantum);
    RUN_TEST(test_wasm_host_sleep_scheduler_and_builtins);
    RUN_TEST(test_wasm_host_chunked_pt_more_contended_and_spawn);
#if !defined(SYN_USE_MUTEX) || SYN_USE_MUTEX
    RUN_TEST(test_wasm_host_mutex_priority_inheritance);
#endif
    RUN_TEST(test_wasm_host_traps_abort_bounds_and_limits);
}
