/**
 * @file test_chunk_budget.c
 * @brief Bare-metal Cortex-M4 QEMU cooperative chunk-latency budget verification gate.
 *
 * Clocks syn_port_get_tick_ms() and syn_port_get_tick_us() from Cortex-M4 hardware
 * SysTick (25 MHz MPS2-AN386 SYSCLK) under deterministic QEMU instruction timing
 * (-icount shift=4,align=off,sleep=off) and enforces that no scheduled cooperative
 * task exceeds its contended peer max_wait_ms budget (1000 us for SYN_PRIO_CRITICAL).
 */

#include "mocks/mock_port.h"
#include "syntropic/crypto/syn_blake2s.h"
#include "syntropic/crypto/syn_chacha20poly1305.h"
#include "syntropic/crypto/syn_ed25519.h"
#include "syntropic/crypto/syn_p256.h"
#include "syntropic/crypto/syn_sha256.h"
#include "syntropic/crypto/syn_x25519.h"
#include "syntropic/debug/syn_task_profile.h"
#include "syntropic/dsp/syn_fft.h"
#include "syntropic/net/syn_coap.h"
#include "syntropic/net/syn_dtls.h"
#include "syntropic/net/syn_httpd.h"
#include "syntropic/net/syn_mqtt.h"
#include "syntropic/net/syn_tls.h"
#include "syntropic/net/syn_wg.h"
#include "syntropic/proto/syn_lwm2m.h"
#include "syntropic/proto/syn_lwm2m_task.h"
#include "syntropic/proto/syn_opcua.h"
#include "syntropic/sched/syn_ao.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/util/syn_nn.h"
#include "syntropic/vm/syn_wasm.h"
#include "unity/unity.h"

#include <stdio.h>
#include <string.h>

/* ── Cortex-M4 Hardware SysTick (MPS2-AN386 @ 25 MHz SYSCLK) ────────────── */

#define SYST_CSR (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018UL)
#define SCB_ICSR (*(volatile uint32_t *)0xE000ED04UL)

#define SYST_CSR_ENABLE (1UL << 0)
#define SYST_CSR_TICKINT (1UL << 1)
#define SYST_CSR_CLKSOURCE (1UL << 2)
#define SCB_ICSR_PENDSTSET (1UL << 26)
#define SCB_ICSR_PENDSTCLR (1UL << 25)

#define MPS2_SYSCLK_HZ 25000000UL
#define MPS2_CYCLES_PER_US 25UL
#define SYST_RELOAD_1MS ((MPS2_SYSCLK_HZ / 1000UL) - 1UL)

static volatile uint32_t g_hw_systick_ms = 0U;

void SysTick_Handler(void);
void SysTick_Handler(void)
{
    g_hw_systick_ms++;
}

static void hw_systick_reset(void)
{
    SYST_CSR = 0UL;
    SCB_ICSR = SCB_ICSR_PENDSTCLR;
    g_hw_systick_ms = 0U;
    SYST_RVR = SYST_RELOAD_1MS;
    SYST_CVR = 0UL;
    SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
}

static uint32_t hw_get_tick_ms(void)
{
    return g_hw_systick_ms;
}

static uint32_t hw_get_tick_us(void)
{
    uint32_t load = SYST_RVR & 0x00FFFFFFUL;
    uint32_t ms1;
    uint32_t ms2;
    uint32_t val;
    uint32_t pend1;
    uint32_t pend2;

    do {
        ms1 = g_hw_systick_ms;
        pend1 = SCB_ICSR & SCB_ICSR_PENDSTSET;
        val = SYST_CVR & 0x00FFFFFFUL;
        ms2 = g_hw_systick_ms;
        pend2 = SCB_ICSR & SCB_ICSR_PENDSTSET;
    } while (ms1 != ms2 || pend1 != pend2);

    if (pend1 != 0U) {
        ms1++;
    }

    uint32_t elapsed_cycles = (val == 0U || val > load) ? 0U : (load - val);
    return (ms1 * 1000U) + (elapsed_cycles / MPS2_CYCLES_PER_US);
}

/* ── Shared Scheduler + Profiler Fixture ────────────────────────────────── */

static SYN_Task s_tasks[6];
static SYN_Sched s_sched;
static SYN_TaskProfileManager s_prof;
static uint32_t s_crit_runs = 0U;
static uint32_t s_contend_runs = 0U;

static SYN_PT_Status crit_sentinel_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        s_crit_runs++;
        PT_PASS(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status contend_peer_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        s_contend_runs++;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

void setUp(void)
{
    mock_port_reset();
    memset(s_tasks, 0, sizeof(s_tasks));
    memset(&s_sched, 0, sizeof(s_sched));
    s_crit_runs = 0U;
    s_contend_runs = 0U;
    (void)syn_task_profile_init(&s_prof);
    hw_systick_reset();
    mock_port_set_tick_hooks(hw_get_tick_ms, hw_get_tick_us);
}

void tearDown(void)
{
}

static void assert_budget_gate(const char *subsystem)
{
    size_t violations = 0U;
    SYN_TaskBudgetViolation worst;
    SYN_Status st = syn_task_profile_check_budgets(&s_prof, &s_sched, &violations, &worst);
    TEST_ASSERT_EQUAL_INT(SYN_OK, st);

    /* Report worst non-sentinel worker task row */
    size_t report_idx = 1U;
    uint32_t report_max = 0U;
    for (size_t i = 1U; i < s_sched.task_count && i < s_prof.count; i++) {
        const char *tname = s_prof.profiles[i].task_name;
        if (tname != NULL && strcmp(tname, "contend_peer") == 0) {
            continue;
        }
        if (s_prof.profiles[i].max_yield_us >= report_max && s_prof.profiles[i].step_count > 0U) {
            report_max = s_prof.profiles[i].max_yield_us;
            report_idx = i;
        }
    }

    const SYN_TaskProfile *wp = &s_prof.profiles[report_idx];
    uint32_t budget_us = syn_task_profile_task_budget_us(&s_prof, &s_sched, report_idx, NULL);
    printf("[BUDGET] %-22s | task=%-13s steps=%-5lu max_yield=%6lu us budget=%6lu us | %s\n",
           subsystem, wp->task_name ? wp->task_name : "?", (unsigned long)wp->step_count,
           (unsigned long)wp->max_yield_us, (unsigned long)budget_us,
           (violations == 0U) ? "PASS" : "VIOLATION");

    static char s_fail_msg[160];
    if (violations > 0U) {
        (void)snprintf(s_fail_msg, sizeof(s_fail_msg),
                       "%s violated cooperative budget: task '%s' (prio %u) ran %lu us > %lu us "
                       "(peer prio %u)",
                       subsystem, worst.task_name ? worst.task_name : "?",
                       (unsigned)worst.task_priority, (unsigned long)worst.max_yield_us,
                       (unsigned long)worst.budget_us, (unsigned)worst.peer_priority);
    } else {
        s_fail_msg[0] = '\0';
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, (uint32_t)violations, s_fail_msg);
}

/* ── 1. DSP 256-Point Q16 FFT (syn_dsp_fft_pt) ──────────────────────────── */

static q16_t s_fft_real[256];
static q16_t s_fft_imag[256];
static uint16_t s_fft_stage = 0U;
static SYN_PT s_fft_child;

static SYN_PT_Status fft_worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    s_fft_stage = 0U;
    PT_SPAWN_IF_NEEDED(pt, task, &s_fft_child,
                       syn_dsp_fft_pt(&s_fft_child, s_fft_real, s_fft_imag, 256U, &s_fft_stage));
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_dsp_fft_256_pt(void)
{
    for (uint16_t i = 0U; i < 256U; i++) {
        s_fft_real[i] = (q16_t)((i & 7U) << 12);
        s_fft_imag[i] = 0;
    }

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "fft_256", fft_worker_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 4096U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);
    assert_budget_gate("dsp_fft_256_pt");
}

/* ── 2. TinyML NN Pipeline (Conv1D + Attention + Dense PT) ──────────────── */

static q7_t s_nn_in[256];
static q7_t s_nn_weights[8192];
static q7_t s_nn_mid[512];
static q7_t s_nn_attn_out[256];
static q7_t s_nn_out[32];
static size_t s_nn_cursor = 0U;
static SYN_PT s_nn_child;

static SYN_PT_Status nn_worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    s_nn_cursor = 0U;
    PT_SPAWN_IF_NEEDED(pt, task, &s_nn_child,
                       syn_nn_conv1d_pt(&s_nn_child, s_nn_in, 32U, 8U, s_nn_weights, NULL, s_nn_mid,
                                        16U, 3U, 1U, SYN_NN_ACT_RELU, 4U, &s_nn_cursor, 4U));
    s_nn_cursor = 0U;
    PT_SPAWN_IF_NEEDED(pt, task, &s_nn_child,
                       syn_nn_attention_pt(&s_nn_child, s_nn_mid, s_nn_mid, s_nn_mid, 16U, 16U, 16U,
                                           s_nn_attn_out, 4U, &s_nn_cursor, 2U));
    s_nn_cursor = 0U;
    PT_SPAWN_IF_NEEDED(pt, task, &s_nn_child,
                       syn_nn_dense_pt(&s_nn_child, s_nn_attn_out, 256U, s_nn_weights, NULL,
                                       s_nn_out, 32U, SYN_NN_ACT_RELU, 5U, &s_nn_cursor, 8U));
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_nn_pipeline_pt(void)
{
    for (size_t i = 0U; i < sizeof(s_nn_in); i++) {
        s_nn_in[i] = (q7_t)((int)(i & 15U) - 8);
    }
    for (size_t i = 0U; i < sizeof(s_nn_weights); i++) {
        s_nn_weights[i] = (q7_t)((int)(i & 7U) - 3);
    }

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "nn_pipe", nn_worker_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 4096U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);
    assert_budget_gate("nn_pipeline_pt");
}

/* ── 3. WebAssembly VM Default Quantum (syn_wasm_task) ──────────────────── */

static const uint8_t k_wasm_loop_bin[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x60, 0x01, 0x7f,
    0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07, 0x0e, 0x01, 0x0a, 0x6c, 0x6f, 0x6f, 0x70,
    0x5f, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x00, 0x00, 0x0a, 0x20, 0x01, 0x1e, 0x01, 0x01,
    0x7f, 0x20, 0x00, 0x21, 0x01, 0x02, 0x40, 0x03, 0x40, 0x20, 0x01, 0x45, 0x0d, 0x01,
    0x20, 0x01, 0x41, 0x01, 0x6b, 0x21, 0x01, 0x0c, 0x00, 0x0b, 0x0b, 0x20, 0x01, 0x0b};

static uint8_t s_wasm_mem[512];
static SYN_WASM_Module s_wasm_mod;
static SYN_WASM_Context s_wasm_ctx;

static void test_budget_wasm_loop_task(void)
{
    TEST_ASSERT_TRUE(syn_wasm_module_load(&s_wasm_mod, k_wasm_loop_bin, sizeof(k_wasm_loop_bin)));
    TEST_ASSERT_TRUE(syn_wasm_init(&s_wasm_ctx, &s_wasm_mod, s_wasm_mem, sizeof(s_wasm_mem)));
    TEST_ASSERT_TRUE(syn_wasm_call(&s_wasm_ctx, 0U));
    s_wasm_ctx.locals[0] = 600U;

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "wasm_vm", syn_wasm_task, SYN_PRIO_LOW, &s_wasm_ctx);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 8192U && s_wasm_ctx.status != SYN_WASM_HALTED; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL(SYN_WASM_HALTED, s_wasm_ctx.status);
    assert_budget_gate("wasm_default_quantum");
}

/* ── 4. Sliced X25519 Default Chunk (syn_x25519_pt) ─────────────────────── */

static SYN_X25519_Ctx s_x25519_ctx;
static uint8_t s_x25519_priv[32];
static uint8_t s_x25519_pub[32];
static SYN_PT s_x25519_child;

static SYN_PT_Status x25519_worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_SPAWN_IF_NEEDED(pt, task, &s_x25519_child,
                       syn_x25519_pt(&s_x25519_child, &s_x25519_ctx, s_x25519_pub));
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_x25519_sliced_pt(void)
{
    memset(s_x25519_priv, 0x42, sizeof(s_x25519_priv));
    syn_x25519_clamp(s_x25519_priv);
    TEST_ASSERT_TRUE(syn_x25519_pubkey_init(&s_x25519_ctx, s_x25519_priv));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "x25519_pt", x25519_worker_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 4096U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);
    assert_budget_gate("x25519_default_chunk");
}

/* ── 4b. Sliced P-256 ECDH + Sign + Verify (syn_p256_*_pt) ──────────────── */

static SYN_P256_Ctx s_p256_ctx;
static uint8_t s_p256_priv_a[32];
static uint8_t s_p256_priv_b[32];
static uint8_t s_p256_pub_bx[32];
static uint8_t s_p256_pub_by[32];
static uint8_t s_p256_pub_ax[32];
static uint8_t s_p256_pub_ay[32];
static uint8_t s_p256_shared[32];
static uint8_t s_p256_hash[32];
static uint8_t s_p256_r[32];
static uint8_t s_p256_s[32];
static bool s_p256_ok = false;
static SYN_PT s_p256_child;

static SYN_PT_Status p256_worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    TEST_ASSERT_TRUE(
        syn_p256_point_mul_init(&s_p256_ctx, s_p256_priv_a, s_p256_pub_bx, s_p256_pub_by));
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_SPAWN_IF_NEEDED(pt, task, &s_p256_child,
                       syn_p256_mul_pt(&s_p256_child, &s_p256_ctx, s_p256_shared, NULL));
    PT_CONTINUE_IF_NEEDED(pt, task);

    TEST_ASSERT_TRUE(syn_p256_sign_deterministic_init(&s_p256_ctx, s_p256_priv_a, s_p256_hash));
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_SPAWN_IF_NEEDED(
        pt, task, &s_p256_child,
        syn_p256_sign_pt(&s_p256_child, &s_p256_ctx, s_p256_r, s_p256_s, &s_p256_ok));
    TEST_ASSERT_TRUE(s_p256_ok);
    PT_CONTINUE_IF_NEEDED(pt, task);

    TEST_ASSERT_TRUE(syn_p256_verify_init(&s_p256_ctx, s_p256_hash, s_p256_r, s_p256_s,
                                          s_p256_pub_ax, s_p256_pub_ay));
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_SPAWN_IF_NEEDED(pt, task, &s_p256_child,
                       syn_p256_verify_pt(&s_p256_child, &s_p256_ctx, &s_p256_ok));
    TEST_ASSERT_TRUE(s_p256_ok);
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_p256_sliced_pt(void)
{
    memset(s_p256_priv_a, 0x23, sizeof(s_p256_priv_a));
    memset(s_p256_priv_b, 0x45, sizeof(s_p256_priv_b));
    memset(s_p256_hash, 0xA5, sizeof(s_p256_hash));
    TEST_ASSERT_TRUE(syn_p256_base_mul(s_p256_priv_a, s_p256_pub_ax, s_p256_pub_ay));
    TEST_ASSERT_TRUE(syn_p256_base_mul(s_p256_priv_b, s_p256_pub_bx, s_p256_pub_by));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "p256_pt", p256_worker_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 16384U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);
    assert_budget_gate("p256_default_chunk");
}

/* ── 4c. Sliced Ed25519 Keygen + Sign + Verify (syn_ed25519_*_pt) ───────── */

static SYN_Ed25519_Ctx s_ed_ctx;
static uint8_t s_ed_seed[32];
static uint8_t s_ed_pub[32];
static uint8_t s_ed_msg[64];
static uint8_t s_ed_sig[64];
static bool s_ed_ok = false;
static SYN_PT s_ed_child;

static SYN_PT_Status ed25519_worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    TEST_ASSERT_TRUE(syn_ed25519_publickey_init(&s_ed_ctx, s_ed_seed));
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_SPAWN_IF_NEEDED(pt, task, &s_ed_child,
                       syn_ed25519_publickey_pt(&s_ed_child, &s_ed_ctx, s_ed_pub));
    PT_CONTINUE_IF_NEEDED(pt, task);

    TEST_ASSERT_TRUE(
        syn_ed25519_sign_init(&s_ed_ctx, s_ed_msg, sizeof(s_ed_msg), s_ed_seed, s_ed_pub));
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_SPAWN_IF_NEEDED(pt, task, &s_ed_child,
                       syn_ed25519_sign_pt(&s_ed_child, &s_ed_ctx, s_ed_sig));
    PT_CONTINUE_IF_NEEDED(pt, task);

    TEST_ASSERT_TRUE(
        syn_ed25519_verify_init(&s_ed_ctx, s_ed_sig, s_ed_msg, sizeof(s_ed_msg), s_ed_pub));
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_SPAWN_IF_NEEDED(pt, task, &s_ed_child,
                       syn_ed25519_verify_pt(&s_ed_child, &s_ed_ctx, &s_ed_ok));
    TEST_ASSERT_TRUE(s_ed_ok);
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_ed25519_sliced_pt(void)
{
    memset(s_ed_seed, 0x77, sizeof(s_ed_seed));
    memset(s_ed_msg, 0x3C, sizeof(s_ed_msg));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "ed25519_pt", ed25519_worker_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 8192U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);
    assert_budget_gate("ed25519_default_chunk");
}

/* ── 5. HTTP/1.1 Server Request Pipeline (syn_httpd_task) ───────────────── */

static uint8_t s_httpd_buf[1024];
static SYN_Httpd s_httpd;
static bool s_httpd_handled = false;

static void httpd_status_route(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req;
    (void)ctx;
    s_httpd_handled = true;
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_body_str(resp, "{\"ok\":true}");
}

static const SYN_HttpdRoute k_httpd_routes[] = {
    {SYN_HTTP_GET, "/api/status", httpd_status_route, NULL},
};

static void test_budget_httpd_request_task(void)
{
    static const char req_str[] = "GET /api/status HTTP/1.1\r\nHost: localhost\r\n\r\n";
    s_httpd_handled = false;
    mock_sock_set_response(req_str, sizeof(req_str) - 1U);
    mock_sock_connected = true;
    mock_sock_accept_ok = true;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_httpd_init(&s_httpd, 8080U, k_httpd_routes, 1U, s_httpd_buf,
                                                 sizeof(s_httpd_buf)));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "httpd", syn_httpd_task, SYN_PRIO_NORMAL, &s_httpd);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 256U && !s_httpd_handled; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    syn_httpd_stop(&s_httpd);
    TEST_ASSERT_TRUE(s_httpd_handled);
    assert_budget_gate("httpd_request_task");
}

/* ── 6. Active Object Event Burst (syn_ao) ──────────────────────────────── */

static SYN_ActiveObject s_ao;
static SYN_AO_Event s_ao_queue[16];
static uint32_t s_ao_count = 0U;

static void ao_on_work(void *ctx)
{
    (void)ctx;
    s_ao_count++;
}

static const SYN_FSM_Transition k_ao_transitions[] = {
    {0U, 1U, 0U, NULL, ao_on_work},
    SYN_FSM_END,
};

static void test_budget_active_object_burst(void)
{
    s_ao_count = 0U;
    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_ao_init(&s_ao, &s_tasks[1], "ao_worker", SYN_PRIO_NORMAL, k_ao_transitions, 0U, s_ao_queue,
                16U);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t i = 0U; i < 12U; i++) {
        TEST_ASSERT_TRUE(syn_ao_post(&s_ao, 1U, NULL));
    }
    for (uint32_t tick = 0U; tick < 256U && s_ao_count < 12U; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT32(12U, s_ao_count);
    assert_budget_gate("active_object_burst");
}

/* ── 7. WireGuard Noise_IKpsk2 Handshake Task (syn_wg_task) ─────────────── */

static void wg_test_mix_hash(uint8_t h[32], const uint8_t *data, size_t len)
{
    SYN_BLAKE2s bctx;
    syn_blake2s_init(&bctx, 32U);
    syn_blake2s_update(&bctx, h, 32U);
    syn_blake2s_update(&bctx, data, len);
    syn_blake2s_final(&bctx, h);
}

static void wg_test_hkdf(const uint8_t ck[32], const uint8_t *in, size_t in_len, uint8_t out1[32],
                         uint8_t out2[32], uint8_t out3[32])
{
    uint8_t prk[32];
    uint8_t buf[33];
    uint8_t b1 = 0x01U;
    syn_hmac_blake2s(ck, 32U, in, in_len, prk);
    syn_hmac_blake2s(prk, 32U, &b1, 1U, out1);
    if (out2 != NULL) {
        memcpy(buf, out1, 32U);
        buf[32] = 0x02U;
        syn_hmac_blake2s(prk, 32U, buf, 33U, out2);
    }
    if (out3 != NULL && out2 != NULL) {
        memcpy(buf, out2, 32U);
        buf[32] = 0x03U;
        syn_hmac_blake2s(prk, 32U, buf, 33U, out3);
    }
}

static void wg_build_server_response(const SYN_WgConfig *cfg, const uint8_t resp_s_priv[32],
                                     const uint8_t init_pkt[148], uint8_t out_resp[92])
{
    static const char k_cons[] = "Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s";
    static const char k_id[] = "WireGuard v1 zx2c4 Jason@zx2c4.com";
    static const char k_mac1[] = "mac1----";
    static uint8_t ck[32], h[32], k[32], dh[32], tau[32], dec_pub[32], dec_ts[12];
    static uint8_t e_priv_r[32], e_pub_r[32], mac_key[32];
    uint8_t zero_nonce[12] = {0};

    syn_blake2s((const uint8_t *)k_cons, sizeof(k_cons) - 1U, ck, 32U);
    memcpy(h, ck, 32U);
    wg_test_mix_hash(h, (const uint8_t *)k_id, sizeof(k_id) - 1U);
    wg_test_mix_hash(h, cfg->peer_public_key, 32U);

    const uint8_t *e_pub_i = init_pkt + 8U;
    wg_test_hkdf(ck, e_pub_i, 32U, ck, NULL, NULL);
    wg_test_mix_hash(h, e_pub_i, 32U);

    syn_x25519(dh, resp_s_priv, e_pub_i);
    wg_test_hkdf(ck, dh, 32U, ck, k, NULL);
    TEST_ASSERT_TRUE(
        syn_aead_decrypt(k, zero_nonce, h, 32U, init_pkt + 40U, 32U, init_pkt + 72U, dec_pub));
    wg_test_mix_hash(h, init_pkt + 40U, 48U);

    syn_x25519(dh, resp_s_priv, dec_pub);
    wg_test_hkdf(ck, dh, 32U, ck, k, NULL);
    TEST_ASSERT_TRUE(
        syn_aead_decrypt(k, zero_nonce, h, 32U, init_pkt + 88U, 12U, init_pkt + 100U, dec_ts));
    wg_test_mix_hash(h, init_pkt + 88U, 28U);

    memset(out_resp, 0, 92U);
    out_resp[0] = SYN_WG_MSG_RESPONSE;
    out_resp[4] = 0x78U;
    out_resp[5] = 0x56U;
    memcpy(out_resp + 8U, init_pkt + 4U, 4U);

    memset(e_priv_r, 0x33, 32U);
    syn_x25519_clamp(e_priv_r);
    syn_x25519_pubkey(e_pub_r, e_priv_r);
    memcpy(out_resp + 12U, e_pub_r, 32U);

    wg_test_hkdf(ck, e_pub_r, 32U, ck, NULL, NULL);
    wg_test_mix_hash(h, e_pub_r, 32U);
    syn_x25519(dh, e_priv_r, e_pub_i);
    wg_test_hkdf(ck, dh, 32U, ck, NULL, NULL);
    syn_x25519(dh, e_priv_r, dec_pub);
    wg_test_hkdf(ck, dh, 32U, ck, NULL, NULL);
    wg_test_hkdf(ck, cfg->preshared_key, 32U, ck, tau, k);
    wg_test_mix_hash(h, tau, 32U);
    syn_aead_encrypt(k, zero_nonce, h, 32U, NULL, 0U, NULL, out_resp + 44U);

    SYN_BLAKE2s mctx;
    syn_blake2s_init(&mctx, 32U);
    syn_blake2s_update(&mctx, (const uint8_t *)k_mac1, 8U);
    syn_blake2s_update(&mctx, dec_pub, 32U);
    syn_blake2s_final(&mctx, mac_key);
    syn_blake2s_mac(mac_key, 32U, out_resp, 60U, out_resp + 60U, 16U);
}

static SYN_SNTP s_wg_sntp;
static SYN_WgConfig s_wg_cfg;
static SYN_WG s_wg;
static uint8_t s_wg_rx[512];
static uint8_t s_wg_tx[512];
static uint8_t s_wg_srv_priv[32];
static uint8_t s_wg_resp_pkt[92];

static void test_budget_wireguard_handshake_task(void)
{
    memset(&s_wg_sntp, 0, sizeof(s_wg_sntp));
    s_wg_sntp.synced = true;
    s_wg_sntp.epoch_s = 1700001000U;

    memset(s_wg_srv_priv, 0x22, sizeof(s_wg_srv_priv));
    syn_x25519_clamp(s_wg_srv_priv);

    memset(&s_wg_cfg, 0, sizeof(s_wg_cfg));
    memset(s_wg_cfg.private_key, 0x21, 32U);
    syn_x25519_clamp(s_wg_cfg.private_key);
    syn_x25519_pubkey(s_wg_cfg.peer_public_key, s_wg_srv_priv);
    memset(s_wg_cfg.preshared_key, 0x29, 32U);
    s_wg_cfg.endpoint.ip[0] = 10U;
    s_wg_cfg.endpoint.ip[3] = 1U;
    s_wg_cfg.endpoint.port = 51820U;

    syn_wg_init(&s_wg, &s_wg_cfg, &s_wg_sntp, s_wg_rx, sizeof(s_wg_rx), s_wg_tx, sizeof(s_wg_tx));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "wg_vpn", syn_wg_task, SYN_PRIO_HIGH, &s_wg);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    bool resp_injected = false;
    for (uint32_t tick = 0U; tick < 16384U && s_wg.state != SYN_WG_ESTABLISHED; tick++) {
        (void)syn_sched_run(&s_sched);
        if (s_wg.state == SYN_WG_HANDSHAKE_INIT && !resp_injected && mock_udp_tx_len == 148U) {
            wg_build_server_response(&s_wg_cfg, s_wg_srv_priv, mock_udp_tx_buf, s_wg_resp_pkt);
            mock_udp_inject_packet(s_wg_resp_pkt, sizeof(s_wg_resp_pkt), &s_wg_cfg.endpoint);
            resp_injected = true;
        }
    }
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, s_wg.state);
    assert_budget_gate("wg_handshake_task");
}

/* ── Shared Loopback Wire & Cert Builders for TLS 1.3 / DTLS 1.3 ────────── */

typedef struct {
    uint8_t buf[2560];
    size_t len;
    size_t dgram_off[4];
    size_t dgram_len[4];
    uint8_t dgram_head;
    uint8_t dgram_count;
} MemChannel;

typedef struct {
    MemChannel *tx;
    MemChannel *rx;
    bool datagram;
} EndpointPair;

static MemChannel s_c2s;
static MemChannel s_s2c;
static EndpointPair s_cli_ep;
static EndpointPair s_srv_ep;
static SYN_Transport s_cli_tr;
static SYN_Transport s_srv_tr;

static uint8_t s_c_rx[2560];
static uint8_t s_c_tx[2560];
static uint8_t s_s_rx[2560];
static uint8_t s_s_tx[2560];

static bool mem_send_stream(const uint8_t *data, size_t len, void *ctx)
{
    EndpointPair *ep = (EndpointPair *)ctx;
    if (ep->tx->len + len > sizeof(ep->tx->buf)) {
        return false;
    }
    memcpy(ep->tx->buf + ep->tx->len, data, len);
    ep->tx->len += len;
    return true;
}

static bool mem_send_dgram(const uint8_t *data, size_t len, void *ctx)
{
    EndpointPair *ep = (EndpointPair *)ctx;
    MemChannel *ch = ep->tx;
    if (ch->dgram_count >= 4U) {
        return false;
    }
    if (ch->dgram_count == 0U) {
        ch->len = 0U;
        ch->dgram_head = 0U;
    }
    if (ch->len + len > sizeof(ch->buf)) {
        return false;
    }
    uint8_t slot = (uint8_t)((ch->dgram_head + ch->dgram_count) & 3U);
    ch->dgram_off[slot] = ch->len;
    ch->dgram_len[slot] = len;
    memcpy(ch->buf + ch->len, data, len);
    ch->len += len;
    ch->dgram_count++;
    return true;
}

static bool mem_recv(uint8_t *data, size_t max_len, size_t *out_len, void *ctx)
{
    EndpointPair *ep = (EndpointPair *)ctx;
    MemChannel *ch = ep->rx;
    if (ep->datagram) {
        if (ch->dgram_count == 0U) {
            return false;
        }
        size_t off = ch->dgram_off[ch->dgram_head];
        size_t pkt_len = ch->dgram_len[ch->dgram_head];
        size_t n = (pkt_len > max_len) ? max_len : pkt_len;
        memcpy(data, ch->buf + off, n);
        *out_len = n;
        ch->dgram_head = (uint8_t)((ch->dgram_head + 1U) & 3U);
        ch->dgram_count--;
        if (ch->dgram_count == 0U) {
            ch->len = 0U;
            ch->dgram_head = 0U;
        }
        return true;
    }
    if (ch->len == 0U) {
        return false;
    }
    size_t n = (ch->len > max_len) ? max_len : ch->len;
    memcpy(data, ch->buf, n);
    *out_len = n;
    ch->len = 0U;
    return true;
}

static void init_mem_transports(bool datagram)
{
    memset(&s_c2s, 0, sizeof(s_c2s));
    memset(&s_s2c, 0, sizeof(s_s2c));
    s_cli_ep.tx = &s_c2s;
    s_cli_ep.rx = &s_s2c;
    s_cli_ep.datagram = datagram;
    s_srv_ep.tx = &s_s2c;
    s_srv_ep.rx = &s_c2s;
    s_srv_ep.datagram = datagram;
    s_cli_tr.send = datagram ? mem_send_dgram : mem_send_stream;
    s_cli_tr.recv = mem_recv;
    s_cli_tr.has_data = NULL;
    s_cli_tr.ctx = &s_cli_ep;
    s_srv_tr.send = datagram ? mem_send_dgram : mem_send_stream;
    s_srv_tr.recv = mem_recv;
    s_srv_tr.has_data = NULL;
    s_srv_tr.ctx = &s_srv_ep;
}

static void build_ed25519_cert(const uint8_t leaf_pub[32], const uint8_t ca_sec[32],
                               const uint8_t ca_pub[32], uint8_t out_der[184])
{
    static const uint8_t hdr[78] = {
        0x30, 0x81, 0xB5, 0x30, 0x69, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01,
        0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E,
        0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',
        'A',  0x30, 0x00, 0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04,
        0x03, 0x13, 0x0B, 'T',  'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',
        'r',  0x30, 0x2A, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x21, 0x00};
    static const uint8_t sig_hdr[10] = {0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x41, 0x00};

    memcpy(out_der, hdr, sizeof(hdr));
    memcpy(out_der + 78U, leaf_pub, 32U);
    memcpy(out_der + 110U, sig_hdr, sizeof(sig_hdr));
    TEST_ASSERT_TRUE(syn_ed25519_sign(out_der + 3U, 107U, ca_sec, ca_pub, out_der + 120U));
}

static size_t build_p256_cert(const uint8_t leaf_pub65[65], const uint8_t ca_priv[32],
                              uint8_t out_der[256])
{
    static const uint8_t tbs_prefix[66] = {
        0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86,
        0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03,
        0x55, 0x04, 0x03, 0x13, 0x07, 'T',  'e',  's',  't',  ' ',  'C',  'A',  0x30, 0x00,
        0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0B, 'T',
        'e',  's',  't',  ' ',  'S',  'e',  'r',  'v',  'e',  'r'};
    static const uint8_t spki_alg[21] = {0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48,
                                         0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
                                         0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07};
    static const uint8_t sig_hdr[15] = {0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE,
                                        0x3D, 0x04, 0x03, 0x02, 0x03, 0x41, 0x00};

    size_t spki_body = sizeof(spki_alg) + 3U + 65U;
    size_t tbs_body = sizeof(tbs_prefix) + 2U + spki_body;
    size_t tbs_total = 3U + tbs_body;
    size_t outer_body = tbs_total + sizeof(sig_hdr) + 64U;

    size_t pos = 0U;
    out_der[pos++] = 0x30U;
    out_der[pos++] = 0x81U;
    out_der[pos++] = (uint8_t)outer_body;

    size_t tbs_start = pos;
    out_der[pos++] = 0x30U;
    out_der[pos++] = 0x81U;
    out_der[pos++] = (uint8_t)tbs_body;
    memcpy(out_der + pos, tbs_prefix, sizeof(tbs_prefix));
    pos += sizeof(tbs_prefix);
    out_der[pos++] = 0x30U;
    out_der[pos++] = (uint8_t)spki_body;
    memcpy(out_der + pos, spki_alg, sizeof(spki_alg));
    pos += sizeof(spki_alg);
    out_der[pos++] = 0x03U;
    out_der[pos++] = 66U;
    out_der[pos++] = 0x00U;
    memcpy(out_der + pos, leaf_pub65, 65U);
    pos += 65U;

    uint8_t tbs_hash[32];
    syn_sha256(out_der + tbs_start, tbs_total, tbs_hash);
    memcpy(out_der + pos, sig_hdr, sizeof(sig_hdr));
    pos += sizeof(sig_hdr);
    TEST_ASSERT_TRUE(
        syn_p256_sign_ecdsa_deterministic(ca_priv, tbs_hash, out_der + pos, out_der + pos + 32U));
    return pos + 64U;
}

/* ── 8. TLS 1.3 mTLS Ed25519 Handshake Task (syn_tls_task) ──────────────── */

static SYN_TLS_Context s_tls_cli;
static SYN_TLS_Context s_tls_srv;
static SYN_X509_Cert s_root_ca;
static uint8_t s_srv_cert[256];
static uint8_t s_cli_cert[256];
static uint8_t s_ca_pub[65];
static uint8_t s_ca_sec[32];
static uint8_t s_srv_pub[65];
static uint8_t s_srv_sec[32];
static uint8_t s_cli_pub[65];
static uint8_t s_cli_sec[32];

static void test_budget_tls13_mtls_ed25519_task(void)
{
    static const uint8_t ca_seed[32] = {0x11U, 0x22U, 0x33U, 0x44U};
    static const uint8_t srv_seed[32] = {0x51U, 0x52U, 0x53U, 0x54U};
    static const uint8_t cli_seed[32] = {0x61U, 0x62U, 0x63U, 0x64U};

    /* Pre-generate static credentials BEFORE attaching scheduler profiler */
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(s_ca_pub, s_ca_sec, ca_seed));
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(s_srv_pub, s_srv_sec, srv_seed));
    TEST_ASSERT_TRUE(syn_ed25519_create_keypair(s_cli_pub, s_cli_sec, cli_seed));
    build_ed25519_cert(s_srv_pub, s_ca_sec, s_ca_pub, s_srv_cert);
    build_ed25519_cert(s_cli_pub, s_ca_sec, s_ca_pub, s_cli_cert);

    memset(&s_root_ca, 0, sizeof(s_root_ca));
    memcpy(s_root_ca.pubkey, s_ca_pub, 32U);
    s_root_ca.pubkey_len = 32U;
    s_root_ca.pubkey_algo = SYN_X509_ALGO_ED25519;
    s_root_ca.is_ca = true;
    strcpy(s_root_ca.subject_cn, "Test CA");

    init_mem_transports(false);
    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                            .server_name = "Test Server",
                            .root_ca = &s_root_ca,
                            .client_cert_der = s_cli_cert,
                            .client_cert_len = 184U,
                            .client_privkey = s_cli_sec,
                            .peer_cert_der = s_srv_cert,
                            .peer_cert_len = 184U};
    SYN_TLS_Config s_cfg = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                            .is_server = true,
                            .root_ca = &s_root_ca,
                            .client_cert_der = s_srv_cert,
                            .client_cert_len = 184U,
                            .client_privkey = s_srv_sec,
                            .peer_cert_der = s_cli_cert,
                            .peer_cert_len = 184U};

    TEST_ASSERT_TRUE(syn_tls_init(&s_tls_cli, &c_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx), s_c_tx,
                                  sizeof(s_c_tx)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_tls_srv, &s_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                                  sizeof(s_s_tx)));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "tls_cli_ed", syn_tls_task, SYN_PRIO_NORMAL, &s_tls_cli);
    syn_task_create(&s_tasks[2], "tls_srv_ed", syn_tls_task, SYN_PRIO_NORMAL, &s_tls_srv);
    syn_task_create(&s_tasks[3], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 4U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 32768U && (s_tls_cli.state != SYN_TLS_STATE_ESTABLISHED ||
                                               s_tls_srv.state != SYN_TLS_STATE_ESTABLISHED);
         tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, s_tls_cli.state);
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, s_tls_srv.state);
    assert_budget_gate("tls13_mtls_ed25519");
}

/* ── 9. TLS 1.3 mTLS ECDSA P-256 Handshake Task (syn_tls_task) ──────────── */

static size_t s_p256_srv_cert_len = 0U;
static size_t s_p256_cli_cert_len = 0U;

static void prepare_p256_mtls_credentials(void)
{
    static const uint8_t ca_priv[32] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16,
                                        0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93,
                                        0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12,
                                        0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
    static const uint8_t srv_priv[32] = {0x7D, 0x40, 0xE0, 0x7E, 0x74, 0xF4, 0x9A, 0x71,
                                         0x48, 0xC3, 0x32, 0x10, 0xCA, 0x19, 0xBD, 0x9B,
                                         0x9D, 0x04, 0x50, 0x77, 0x92, 0xEC, 0xF7, 0x3E,
                                         0x87, 0x7E, 0x10, 0x64, 0x79, 0x80, 0x0C, 0x85};
    static const uint8_t cli_priv[32] = {0x3B, 0x91, 0x8F, 0xC2, 0x3A, 0xA4, 0x77, 0xD8,
                                         0xDE, 0x9B, 0x28, 0x12, 0xF3, 0xBE, 0x60, 0xCE,
                                         0x8B, 0x7E, 0x45, 0x26, 0xA3, 0x81, 0x25, 0x60,
                                         0xB9, 0x92, 0x21, 0x3F, 0x19, 0x33, 0xAE, 0x71};

    memcpy(s_ca_sec, ca_priv, 32U);
    memcpy(s_srv_sec, srv_priv, 32U);
    memcpy(s_cli_sec, cli_priv, 32U);
    s_ca_pub[0] = 0x04U;
    s_srv_pub[0] = 0x04U;
    s_cli_pub[0] = 0x04U;
    TEST_ASSERT_TRUE(syn_p256_base_mul(s_ca_sec, s_ca_pub + 1U, s_ca_pub + 33U));
    TEST_ASSERT_TRUE(syn_p256_base_mul(s_srv_sec, s_srv_pub + 1U, s_srv_pub + 33U));
    TEST_ASSERT_TRUE(syn_p256_base_mul(s_cli_sec, s_cli_pub + 1U, s_cli_pub + 33U));

    s_p256_srv_cert_len = build_p256_cert(s_srv_pub, s_ca_sec, s_srv_cert);
    s_p256_cli_cert_len = build_p256_cert(s_cli_pub, s_ca_sec, s_cli_cert);

    memset(&s_root_ca, 0, sizeof(s_root_ca));
    memcpy(s_root_ca.pubkey, s_ca_pub, 65U);
    s_root_ca.pubkey_len = 65U;
    s_root_ca.pubkey_algo = SYN_X509_ALGO_ECDSA_P256;
    s_root_ca.is_ca = true;
    strcpy(s_root_ca.subject_cn, "Test CA");
}

static void test_budget_tls13_mtls_p256_task(void)
{
    prepare_p256_mtls_credentials();
    init_mem_transports(false);

    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                            .server_name = "Test Server",
                            .root_ca = &s_root_ca,
                            .client_cert_der = s_cli_cert,
                            .client_cert_len = s_p256_cli_cert_len,
                            .client_privkey = s_cli_sec,
                            .peer_cert_der = s_srv_cert,
                            .peer_cert_len = s_p256_srv_cert_len};
    SYN_TLS_Config s_cfg = {.mode = SYN_TLS_AUTH_MODE_MTLS,
                            .is_server = true,
                            .root_ca = &s_root_ca,
                            .client_cert_der = s_srv_cert,
                            .client_cert_len = s_p256_srv_cert_len,
                            .client_privkey = s_srv_sec,
                            .peer_cert_der = s_cli_cert,
                            .peer_cert_len = s_p256_cli_cert_len};

    TEST_ASSERT_TRUE(syn_tls_init(&s_tls_cli, &c_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx), s_c_tx,
                                  sizeof(s_c_tx)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_tls_srv, &s_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                                  sizeof(s_s_tx)));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "tls_cli_p256", syn_tls_task, SYN_PRIO_NORMAL, &s_tls_cli);
    syn_task_create(&s_tasks[2], "tls_srv_p256", syn_tls_task, SYN_PRIO_NORMAL, &s_tls_srv);
    syn_task_create(&s_tasks[3], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 4U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 32768U && (s_tls_cli.state != SYN_TLS_STATE_ESTABLISHED ||
                                               s_tls_srv.state != SYN_TLS_STATE_ESTABLISHED);
         tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, s_tls_cli.state);
    TEST_ASSERT_EQUAL(SYN_TLS_STATE_ESTABLISHED, s_tls_srv.state);
    assert_budget_gate("tls13_mtls_p256");
}

/* ── 10. DTLS 1.3 mTLS ECDSA P-256 Handshake Task (syn_dtls_task) ───────── */

static SYN_DTLS_Context s_dtls_cli;
static SYN_DTLS_Context s_dtls_srv;

static void test_budget_dtls13_mtls_p256_task(void)
{
    prepare_p256_mtls_credentials();
    init_mem_transports(true);

    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                             .server_name = "Test Server",
                             .root_ca = &s_root_ca,
                             .client_cert_der = s_cli_cert,
                             .client_cert_len = s_p256_cli_cert_len,
                             .client_privkey = s_cli_sec,
                             .peer_cert_der = s_srv_cert,
                             .peer_cert_len = s_p256_srv_cert_len};
    SYN_DTLS_Config s_cfg = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                             .is_server = true,
                             .root_ca = &s_root_ca,
                             .client_cert_der = s_srv_cert,
                             .client_cert_len = s_p256_srv_cert_len,
                             .client_privkey = s_srv_sec,
                             .peer_cert_der = s_cli_cert,
                             .peer_cert_len = s_p256_cli_cert_len};

    TEST_ASSERT_TRUE(syn_dtls_init(&s_dtls_cli, &c_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx), s_c_tx,
                                   sizeof(s_c_tx)));
    TEST_ASSERT_TRUE(syn_dtls_init(&s_dtls_srv, &s_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                                   sizeof(s_s_tx)));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "dtls_cli_p256", syn_dtls_task, SYN_PRIO_NORMAL, &s_dtls_cli);
    syn_task_create(&s_tasks[2], "dtls_srv_p256", syn_dtls_task, SYN_PRIO_NORMAL, &s_dtls_srv);
    syn_task_create(&s_tasks[3], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 4U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 32768U && (s_dtls_cli.state != SYN_DTLS_STATE_ESTABLISHED ||
                                               s_dtls_srv.state != SYN_DTLS_STATE_ESTABLISHED);
         tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, s_dtls_cli.state);
    TEST_ASSERT_EQUAL(SYN_DTLS_STATE_ESTABLISHED, s_dtls_srv.state);
    assert_budget_gate("dtls13_mtls_p256");
}

/* ── 11. Sliced X.509 Chain Validation (syn_x509_validate_chain_pt) ─────── */

static SYN_X509_VerifyCtx s_x509_vctx;
static SYN_X509_Cert s_x509_parsed;
static bool s_x509_ok = false;
static SYN_PT s_x509_child;

static SYN_PT_Status x509_worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    TEST_ASSERT_TRUE(
        syn_x509_validate_chain_init(&s_x509_vctx, &s_x509_parsed, &s_root_ca, "Test Server"));
    PT_SPAWN_IF_NEEDED(pt, task, &s_x509_child,
                       syn_x509_verify_pt(&s_x509_child, &s_x509_vctx, &s_x509_ok));
    TEST_ASSERT_TRUE(s_x509_ok);
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_x509_validate_chain_pt(void)
{
    prepare_p256_mtls_credentials();
    TEST_ASSERT_TRUE(syn_x509_parse(s_srv_cert, s_p256_srv_cert_len, &s_x509_parsed));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "x509_p256_pt", x509_worker_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 8192U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);
    assert_budget_gate("x509_p256_chain_pt");
}

/* ── 12. TLS 1.3 AES-256-GCM-SHA384 + 1 KB Cooperative Record Stream ────── */

static uint8_t s_tls_app_tx[1024];
static uint8_t s_tls_app_rx[1024];
static size_t s_tls_tx_off = 0U;
static size_t s_tls_rx_off = 0U;
static SYN_PT s_tls_cli_hs_pt;
static SYN_PT s_tls_srv_hs_pt;

static SYN_PT_Status tls_aes256_cli_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_SPAWN_IF_NEEDED(pt, task, &s_tls_cli_hs_pt,
                       syn_tls_handshake_pt(&s_tls_cli_hs_pt, &s_tls_cli));
    PT_CONTINUE_IF_NEEDED(pt, task);
    while (s_tls_tx_off < sizeof(s_tls_app_tx)) {
        PT_WAIT_UNTIL(pt, s_c2s.len == 0U);
        TEST_ASSERT_TRUE(syn_tls_send(&s_tls_cli, s_tls_app_tx + s_tls_tx_off, 128U));
        s_tls_tx_off += 128U;
        PT_CONTINUE_IF_NEEDED(pt, task);
    }
    PT_EXIT(pt);
    PT_END(pt);
}

static SYN_PT_Status tls_aes256_srv_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_SPAWN_IF_NEEDED(pt, task, &s_tls_srv_hs_pt,
                       syn_tls_handshake_pt(&s_tls_srv_hs_pt, &s_tls_srv));
    PT_CONTINUE_IF_NEEDED(pt, task);
    while (s_tls_rx_off < sizeof(s_tls_app_rx)) {
        size_t got = 0U;
        if (syn_tls_recv(&s_tls_srv, s_tls_app_rx + s_tls_rx_off,
                         sizeof(s_tls_app_rx) - s_tls_rx_off, &got) &&
            got > 0U) {
            s_tls_rx_off += got;
            PT_CONTINUE_IF_NEEDED(pt, task);
        } else {
            PT_PASS(pt);
        }
    }
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_budget_tls13_aes256_gcm_record_task(void)
{
    static const uint8_t k_psk_id[] = "syn-aes256-node";
    static const uint8_t k_psk_sec[32] = {0xA1U, 0xB2U, 0xC3U, 0xD4U, 0xE5U, 0xF6U, 0x07U, 0x18U};

    mock_aes_hw_enabled = false;
    init_mem_transports(false);
    for (size_t i = 0U; i < sizeof(s_tls_app_tx); i++) {
        s_tls_app_tx[i] = (uint8_t)(i ^ 0x5AU);
    }
    memset(s_tls_app_rx, 0, sizeof(s_tls_app_rx));
    s_tls_tx_off = 0U;
    s_tls_rx_off = 0U;

    SYN_TLS_Config c_cfg = {.mode = SYN_TLS_AUTH_MODE_PSK,
                            .cipher_suite = SYN_TLS_CIPHER_SUITE_AES_256_GCM_SHA384,
                            .psk_identity = k_psk_id,
                            .psk_identity_len = sizeof(k_psk_id) - 1U,
                            .psk_secret = k_psk_sec,
                            .psk_secret_len = sizeof(k_psk_sec)};
    SYN_TLS_Config s_cfg = c_cfg;
    s_cfg.is_server = true;

    TEST_ASSERT_TRUE(syn_tls_init(&s_tls_cli, &c_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx), s_c_tx,
                                  sizeof(s_c_tx)));
    TEST_ASSERT_TRUE(syn_tls_init(&s_tls_srv, &s_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                                  sizeof(s_s_tx)));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "tls_a256_cli", tls_aes256_cli_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&s_tasks[2], "tls_a256_srv", tls_aes256_srv_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&s_tasks[3], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 4U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 16384U && s_tls_rx_off < sizeof(s_tls_app_rx); tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT32((uint32_t)sizeof(s_tls_app_rx), (uint32_t)s_tls_rx_off);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(s_tls_app_tx, s_tls_app_rx, sizeof(s_tls_app_tx));
    assert_budget_gate("tls13_aes256_1k_io");
}

/* ── 13. WireGuard 1420B Full-MTU Transport + Hitless Rekey ─────────────── */

static uint8_t s_wg_mtu_rx[1536];
static uint8_t s_wg_mtu_tx[1536];
static uint8_t s_wg_mtu_plain[SYN_WG_MTU];
static uint8_t s_wg_mtu_wire[SYN_WG_MTU + SYN_WG_TRANSPORT_OVERHEAD];
static size_t s_wg_mtu_got_len = 0U;

static void wg_on_mtu_recv(const uint8_t *ip_packet, size_t len, void *ctx)
{
    (void)ip_packet;
    (void)ctx;
    s_wg_mtu_got_len = len;
}

static void wg_build_mtu_transport(const uint8_t recv_key[32], const uint8_t cli_idx_le[4],
                                   const uint8_t plain[SYN_WG_MTU],
                                   uint8_t out_wire[SYN_WG_MTU + SYN_WG_TRANSPORT_OVERHEAD])
{
    uint8_t zero_nonce[12] = {0};
    memset(out_wire, 0, 16U);
    out_wire[0] = SYN_WG_MSG_TRANSPORT;
    memcpy(out_wire + 4U, cli_idx_le, 4U);
    syn_aead_encrypt(recv_key, zero_nonce, NULL, 0U, plain, SYN_WG_MTU, out_wire + 16U,
                     out_wire + 16U + SYN_WG_MTU);
}

static void test_budget_wireguard_mtu_rekey_task(void)
{
    uint8_t cli_idx_le[4] = {0};
    s_wg_mtu_got_len = 0U;
    for (size_t i = 0U; i < sizeof(s_wg_mtu_plain); i++) {
        s_wg_mtu_plain[i] = (uint8_t)(i & 0xFFU);
    }

    memset(&s_wg_sntp, 0, sizeof(s_wg_sntp));
    s_wg_sntp.synced = true;
    s_wg_sntp.epoch_s = 1700002000U;

    memset(s_wg_srv_priv, 0x24, sizeof(s_wg_srv_priv));
    syn_x25519_clamp(s_wg_srv_priv);

    memset(&s_wg_cfg, 0, sizeof(s_wg_cfg));
    memset(s_wg_cfg.private_key, 0x25, 32U);
    syn_x25519_clamp(s_wg_cfg.private_key);
    syn_x25519_pubkey(s_wg_cfg.peer_public_key, s_wg_srv_priv);
    memset(s_wg_cfg.preshared_key, 0x2BU, 32U);
    s_wg_cfg.endpoint.ip[0] = 10U;
    s_wg_cfg.endpoint.ip[3] = 2U;
    s_wg_cfg.endpoint.port = 51820U;

    syn_wg_init(&s_wg, &s_wg_cfg, &s_wg_sntp, s_wg_mtu_rx, sizeof(s_wg_mtu_rx), s_wg_mtu_tx,
                sizeof(s_wg_mtu_tx));
    s_wg.on_recv = wg_on_mtu_recv;

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "wg_rekey", syn_wg_task, SYN_PRIO_HIGH, &s_wg);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    bool init_replied = false;
    for (uint32_t tick = 0U; tick < 16384U && s_wg.state != SYN_WG_ESTABLISHED; tick++) {
        (void)syn_sched_run(&s_sched);
        if (s_wg.state == SYN_WG_HANDSHAKE_INIT && !init_replied && mock_udp_tx_len == 148U) {
            memcpy(cli_idx_le, mock_udp_tx_buf + 4U, 4U);
            wg_build_server_response(&s_wg_cfg, s_wg_srv_priv, mock_udp_tx_buf, s_wg_resp_pkt);
            mock_udp_inject_packet(s_wg_resp_pkt, sizeof(s_wg_resp_pkt), &s_wg_cfg.endpoint);
            init_replied = true;
        }
    }
    TEST_ASSERT_EQUAL(SYN_WG_ESTABLISHED, s_wg.state);

    /* Age session past SYN_WG_REKEY_AFTER_TIME to start hitless rekey + inject 1420B MTU RX */
    mock_udp_tx_len = 0U;
    s_wg.session.established_ms -= (SYN_WG_REKEY_AFTER_TIME + 2U) * 1000U;
    wg_build_mtu_transport(s_wg.session.recv_key, cli_idx_le, s_wg_mtu_plain, s_wg_mtu_wire);

    bool mtu_injected = false;
    bool rekey_replied = false;
    for (uint32_t tick = 0U; tick < 16384U && (!rekey_replied || s_wg.rekey_in_progress); tick++) {
        (void)syn_sched_run(&s_sched);
        if (s_wg.rekey_in_progress && !mtu_injected) {
            mock_udp_inject_packet(s_wg_mtu_wire, sizeof(s_wg_mtu_wire), &s_wg_cfg.endpoint);
            mtu_injected = true;
        }
        if (s_wg.rekey_in_progress && !rekey_replied && mock_udp_tx_len == 148U) {
            s_wg_sntp.epoch_s += 5U;
            wg_build_server_response(&s_wg_cfg, s_wg_srv_priv, mock_udp_tx_buf, s_wg_resp_pkt);
            mock_udp_inject_packet(s_wg_resp_pkt, sizeof(s_wg_resp_pkt), &s_wg_cfg.endpoint);
            rekey_replied = true;
        }
    }
    TEST_ASSERT_TRUE(mtu_injected);
    TEST_ASSERT_TRUE(rekey_replied);
    TEST_ASSERT_FALSE(s_wg.rekey_in_progress);
    TEST_ASSERT_EQUAL_UINT32((uint32_t)SYN_WG_MTU, (uint32_t)s_wg_mtu_got_len);
    assert_budget_gate("wg_mtu_hitless_rekey");
}

/* ── 14. MQTT Multi-Packet PUBLISH Burst (syn_mqtt_task) ────────────────── */

static SYN_MqttClient s_mqtt_cli;
static uint8_t s_mqtt_rx[256];
static uint8_t s_mqtt_tx[256];
static uint8_t s_mqtt_wire[640];
static uint32_t s_mqtt_msgs = 0U;

static void mqtt_budget_on_msg(const char *topic, const uint8_t *payload, size_t len, void *ctx)
{
    (void)topic;
    (void)payload;
    (void)len;
    (void)ctx;
    s_mqtt_msgs++;
}

static void test_budget_mqtt_publish_burst_task(void)
{
    static const char k_topic[] = "sensors/temp";
    size_t pos = 0U;
    s_mqtt_msgs = 0U;

    /* CONNACK followed by 6 back-to-back QoS-0 PUBLISH packets (64B payload each) */
    s_mqtt_wire[pos++] = 0x20U;
    s_mqtt_wire[pos++] = 0x02U;
    s_mqtt_wire[pos++] = 0x00U;
    s_mqtt_wire[pos++] = 0x00U;

    for (uint32_t i = 0U; i < 6U; i++) {
        s_mqtt_wire[pos++] = 0x30U;
        s_mqtt_wire[pos++] = 2U + 12U + 64U;
        s_mqtt_wire[pos++] = 0x00U;
        s_mqtt_wire[pos++] = 12U;
        memcpy(s_mqtt_wire + pos, k_topic, 12U);
        pos += 12U;
        memset(s_mqtt_wire + pos, (int)(0x30U + i), 64U);
        pos += 64U;
    }

    mock_sock_set_response(s_mqtt_wire, pos);
    mock_sock_connected = true;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mqtt_init(&s_mqtt_cli, "127.0.0.1", 1883U, "syn-node", NULL,
                                                NULL, 60U, s_mqtt_rx, sizeof(s_mqtt_rx), s_mqtt_tx,
                                                sizeof(s_mqtt_tx)));
    s_mqtt_cli.on_message = mqtt_budget_on_msg;

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "mqtt_burst", syn_mqtt_task, SYN_PRIO_NORMAL, &s_mqtt_cli);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 512U && s_mqtt_msgs < 6U; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT32(6U, s_mqtt_msgs);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(7U, (uint32_t)s_prof.profiles[1].step_count);
    assert_budget_gate("mqtt_publish_burst");
}

/* ── 15. Micro OPC UA Pipelined Frame Burst (syn_opcua_server_pt) ───────── */

static SYN_OPCUA_Server s_opcua_srv;
static uint8_t s_opcua_rx[512];
static uint8_t s_opcua_tx[512];
static uint32_t s_opcua_reads = 0U;

static SYN_Status opcua_budget_on_read(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node,
                                       SYN_OPCUA_DataValue *out_val, void *user_data)
{
    (void)srv;
    (void)node;
    (void)user_data;
    s_opcua_reads++;
    out_val->value.type = SYN_OPCUA_TYPE_INT32;
    out_val->value.val.int32 = (int32_t)s_opcua_reads;
    out_val->status_code = SYN_OPCUA_STATUS_GOOD;
    return SYN_OK;
}

static void test_budget_opcua_pipelined_burst_task(void)
{
    s_opcua_reads = 0U;
    init_mem_transports(false);

    SYN_OPCUA_Config cfg = {.transport = &s_srv_tr,
                            .rx_buf = s_opcua_rx,
                            .rx_buf_size = sizeof(s_opcua_rx),
                            .tx_buf = s_opcua_tx,
                            .tx_buf_size = sizeof(s_opcua_tx)};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_init(&s_opcua_srv, &cfg));

    SYN_OPCUA_Node sensor_node = {
        .node_id = {.ns_index = 1U, .id_type = SYN_OPCUA_NODEID_NUMERIC, .id = {.num = 2000U}},
        .browse_name = "Temp",
        .display_name = "Temp",
        .data_type = SYN_OPCUA_TYPE_INT32,
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .access_level = 0x01U,
        .on_read = opcua_budget_on_read,
    };
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_opcua_server_register_node(&s_opcua_srv, &sensor_node));
    s_opcua_srv.state = SYN_OPCUA_STATE_SESSION_ACTIVE;
    s_opcua_srv.secure_channel_id = 1U;
    s_opcua_srv.security_token_id = 1U;

    /* Concatenate 4 complete 56B MSGF ReadRequest frames into a single pipelined TCP burst */
    for (uint32_t f = 0U; f < 4U; f++) {
        uint8_t *m = s_c2s.buf + (f * 56U);
        uint32_t sz = 56U;
        uint32_t one = 1U;
        uint32_t seq = f + 1U;
        uint16_t svc_rd = 631U;
        uint16_t nid = 2000U;
        memset(m, 0, 56U);
        m[0] = (uint8_t)'M';
        m[1] = (uint8_t)'S';
        m[2] = (uint8_t)'G';
        m[3] = (uint8_t)'F';
        memcpy(m + 4U, &sz, 4U);
        memcpy(m + 8U, &one, 4U);
        memcpy(m + 12U, &one, 4U);
        memcpy(m + 16U, &seq, 4U);
        memcpy(m + 20U, &seq, 4U);
        m[24] = 0x01U;
        memcpy(m + 26U, &svc_rd, 2U);
        memcpy(m + 44U, &one, 4U);
        m[48] = 0x01U;
        m[49] = 0x01U;
        memcpy(m + 50U, &nid, 2U);
    }
    s_c2s.len = 4U * 56U;

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "opcua_srv", syn_opcua_server_pt, SYN_PRIO_NORMAL, &s_opcua_srv);
    syn_task_create(&s_tasks[2], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 3U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 512U && s_opcua_reads < 4U; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT32(4U, s_opcua_reads);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(4U, (uint32_t)s_prof.profiles[1].step_count);
    assert_budget_gate("opcua_pipelined_burst");
}

/* ── 16. LwM2M Client Supervisor over CoAPs / DTLS 1.3 mTLS P-256 ───────── */

static SYN_CoapsClient s_coaps_cli;
static SYN_LwM2M_Client s_lwm2m_cli;
static SYN_LwM2M_Task s_lwm2m_task;
static uint8_t s_lwm2m_rx[512];
static uint8_t s_lwm2m_tx[512];
static uint8_t s_lwm2m_srv_coap[256];
static uint8_t s_lwm2m_srv_resp[128];

static void reply_lwm2m_register_over_dtls(const uint8_t *req_buf, size_t req_len)
{
    SYN_CoapMsg tx_coap;
    SYN_CoapOption tx_opts[8];
    size_t tx_opt_cnt = 0U;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_coap_parse(&tx_coap, tx_opts, 8U, &tx_opt_cnt, req_buf, req_len));

    SYN_CoapMsg resp;
    memset(&resp, 0, sizeof(resp));
    resp.type = COAP_TYPE_ACK;
    resp.code = COAP_RESP_CREATED;
    resp.msg_id = tx_coap.msg_id;
    resp.token_len = tx_coap.token_len;
    memcpy(resp.token, tx_coap.token, tx_coap.token_len);

    SYN_CoapOption resp_opts[2];
    resp_opts[0].num = COAP_OPT_LOCATION_PATH;
    resp_opts[0].val = (const uint8_t *)"rd";
    resp_opts[0].len = 2U;
    resp_opts[1].num = COAP_OPT_LOCATION_PATH;
    resp_opts[1].val = (const uint8_t *)"edge01";
    resp_opts[1].len = 6U;

    size_t srv_len =
        syn_coap_serialize(&resp, resp_opts, 2U, s_lwm2m_srv_resp, sizeof(s_lwm2m_srv_resp));
    TEST_ASSERT_TRUE(srv_len > 0U);
    TEST_ASSERT_TRUE(syn_dtls_send(&s_dtls_srv, s_lwm2m_srv_resp, srv_len));
}

static void test_budget_lwm2m_coaps_dtls13_task(void)
{
    prepare_p256_mtls_credentials();
    init_mem_transports(true);

    SYN_DTLS_Config c_cfg = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                             .server_name = "Test Server",
                             .root_ca = &s_root_ca,
                             .client_cert_der = s_cli_cert,
                             .client_cert_len = s_p256_cli_cert_len,
                             .client_privkey = s_cli_sec,
                             .peer_cert_der = s_srv_cert,
                             .peer_cert_len = s_p256_srv_cert_len};
    SYN_DTLS_Config s_cfg = {.mode = SYN_DTLS_AUTH_MODE_MTLS,
                             .is_server = true,
                             .root_ca = &s_root_ca,
                             .client_cert_der = s_srv_cert,
                             .client_cert_len = s_p256_srv_cert_len,
                             .client_privkey = s_srv_sec,
                             .peer_cert_der = s_cli_cert,
                             .peer_cert_len = s_p256_cli_cert_len};

    TEST_ASSERT_TRUE(syn_coaps_client_init(&s_coaps_cli, &c_cfg, &s_cli_tr, s_c_rx, sizeof(s_c_rx),
                                           s_c_tx, sizeof(s_c_tx)));
    TEST_ASSERT_TRUE(syn_dtls_init(&s_dtls_srv, &s_cfg, &s_srv_tr, s_s_rx, sizeof(s_s_rx), s_s_tx,
                                   sizeof(s_s_tx)));
    TEST_ASSERT_TRUE(
        syn_lwm2m_client_init(&s_lwm2m_cli, "syn-edge-01", 300U, &s_coaps_cli.dtls_transport));
    static SYN_LwM2M_DeviceContext s_lwm2m_dev_ctx = {.manufacturer = "SyntropicOS",
                                                      .model_number = "Edge-M4",
                                                      .serial_number = "SN-0001",
                                                      .firmware_ver = "1.0.0"};
    static SYN_LwM2M_Object s_lwm2m_dev_obj;
    s_lwm2m_dev_obj = syn_lwm2m_make_device_object(&s_lwm2m_dev_ctx);
    TEST_ASSERT_TRUE(syn_lwm2m_register_object(&s_lwm2m_cli, &s_lwm2m_dev_obj));

    SYN_LwM2M_TaskConfig tcfg = {.client = &s_lwm2m_cli,
                                 .transport = &s_coaps_cli.dtls_transport,
                                 .coaps_client = &s_coaps_cli,
                                 .retry_backoff_ms = 5000U,
                                 .rx_buf = s_lwm2m_rx,
                                 .rx_buf_size = sizeof(s_lwm2m_rx),
                                 .tx_buf = s_lwm2m_tx,
                                 .tx_buf_size = sizeof(s_lwm2m_tx)};
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_lwm2m_task_init(&s_lwm2m_task, &tcfg));

    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "lwm2m_coaps", syn_lwm2m_task_pt, SYN_PRIO_NORMAL, &s_lwm2m_task);
    syn_task_create(&s_tasks[2], "dtls_srv", syn_dtls_task, SYN_PRIO_NORMAL, &s_dtls_srv);
    syn_task_create(&s_tasks[3], "contend_peer", contend_peer_task, SYN_PRIO_NORMAL, NULL);
    syn_sched_init(&s_sched, s_tasks, 4U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    bool reg_replied = false;
    for (uint32_t tick = 0U; tick < 32768U && s_lwm2m_cli.state != SYN_LWM2M_STATE_REGISTERED;
         tick++) {
        (void)syn_sched_run(&s_sched);
        if (!reg_replied && s_dtls_srv.state == SYN_DTLS_STATE_ESTABLISHED) {
            size_t coap_len = 0U;
            if (syn_dtls_recv(&s_dtls_srv, s_lwm2m_srv_coap, sizeof(s_lwm2m_srv_coap), &coap_len) &&
                coap_len > 0U) {
                reply_lwm2m_register_over_dtls(s_lwm2m_srv_coap, coap_len);
                reg_replied = true;
            }
        }
    }
    TEST_ASSERT_EQUAL(SYN_LWM2M_STATE_REGISTERED, s_lwm2m_cli.state);
    assert_budget_gate("lwm2m_coaps_p256");
}

/* ── 17. Uncontended PT_PASS Batching Slack (1 ms tick + 1 in-flight step) ─ */

static void test_budget_uncontended_batching_slack(void)
{
    prepare_p256_mtls_credentials();
    TEST_ASSERT_TRUE(syn_x509_parse(s_srv_cert, s_p256_srv_cert_len, &s_x509_parsed));

    /* Only PT_PASS crit_ctrl registered (no PT_CONTINUE peer) -> steps batch inside 1 ms tick */
    syn_task_create(&s_tasks[0], "crit_ctrl", crit_sentinel_task, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[1], "x509_batch", x509_worker_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&s_sched, s_tasks, 2U);
    syn_sched_set_profiler(&s_sched, &s_prof);

    for (uint32_t tick = 0U; tick < 1024U && s_tasks[1].state != (uint8_t)SYN_TASK_DEAD; tick++) {
        (void)syn_sched_run(&s_sched);
    }
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, s_tasks[1].state);

    const SYN_TaskProfile *wp = &s_prof.profiles[1];
    printf("[BATCH ] %-22s | task=%-13s steps=%-5lu max_yield=%6lu us slack<=1500 us | %s\n",
           "uncontended_pt_pass", wp->task_name ? wp->task_name : "?",
           (unsigned long)wp->step_count, (unsigned long)wp->max_yield_us,
           (wp->step_count < 350U && wp->max_yield_us <= 1500U) ? "PASS" : "FAIL");
    TEST_ASSERT_LESS_THAN_UINT32(350U, (uint32_t)wp->step_count);
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(1500U, wp->max_yield_us);
}

int main(void)
{
    printf("\n=== Cortex-M4 Cooperative Chunk-Latency Budget Gate (SysTick @ 25 MHz, -icount "
           "shift=4 / 62.5 MIPS) ===\n");
    UNITY_BEGIN();
    RUN_TEST(test_budget_dsp_fft_256_pt);
    RUN_TEST(test_budget_nn_pipeline_pt);
    RUN_TEST(test_budget_wasm_loop_task);
    RUN_TEST(test_budget_x25519_sliced_pt);
    RUN_TEST(test_budget_p256_sliced_pt);
    RUN_TEST(test_budget_ed25519_sliced_pt);
    RUN_TEST(test_budget_x509_validate_chain_pt);
    RUN_TEST(test_budget_httpd_request_task);
    RUN_TEST(test_budget_active_object_burst);
    RUN_TEST(test_budget_wireguard_handshake_task);
    RUN_TEST(test_budget_wireguard_mtu_rekey_task);
    RUN_TEST(test_budget_tls13_mtls_ed25519_task);
    RUN_TEST(test_budget_tls13_mtls_p256_task);
    RUN_TEST(test_budget_tls13_aes256_gcm_record_task);
    RUN_TEST(test_budget_dtls13_mtls_p256_task);
    RUN_TEST(test_budget_lwm2m_coaps_dtls13_task);
    RUN_TEST(test_budget_mqtt_publish_burst_task);
    RUN_TEST(test_budget_opcua_pipelined_burst_task);
    RUN_TEST(test_budget_uncontended_batching_slack);
    return UNITY_END();
}
