/**
 * @file test_watchdog.c
 * @brief Unity tests for syn_watchdog.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_watchdog.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static int wdt_timeout_count = 0;
static const char *wdt_timeout_name = NULL;

static void wdt_on_timeout(SYN_Watchdog *wdt, const SYN_WDT_Entry *entry, void *ctx)
{
    (void)wdt;
    (void)ctx;
    wdt_timeout_count++;
    wdt_timeout_name = entry->name;
}

static void test_watchdog(void)
{
    mock_tick_ms = 0;
    wdt_timeout_count = 0;

    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[4];
    syn_watchdog_init(&wdt, entries, 4, wdt_on_timeout, NULL);

    int8_t id0 = syn_watchdog_register(&wdt, "task_a", 100);
    int8_t id1 = syn_watchdog_register(&wdt, "task_b", 200);
    TEST_ASSERT_TRUE(id0 >= 0);
    TEST_ASSERT_TRUE(id1 >= 0);

    /* No timeout yet */
    mock_tick_advance(50);
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);

    /* Check in task_a */
    syn_watchdog_checkin(&wdt, id0);

    /* task_a deadline at 150ms, task_b at 200ms */
    mock_tick_advance(60); /* now 110ms */
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);

    mock_tick_advance(100); /* now 210ms — task_b should time out */
    syn_watchdog_update(&wdt);
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
    TEST_ASSERT_EQUAL_INT(0, strcmp(wdt_timeout_name, "task_b"));

    /* Unregister */
    syn_watchdog_unregister(&wdt, id1);
    wdt_timeout_count = 0;
    mock_tick_advance(500);
    syn_watchdog_update(&wdt);
    /* Only id0 should fire (id1 unregistered) */
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
}

/** table full → register returns -1 (line 55); errlog on timeout (lines 88-89) */
static void test_watchdog_table_full_and_errlog(void)
{
    mock_tick_ms = 0;
    wdt_timeout_count = 0;

    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[2]; /* tiny 2-slot table */
    syn_watchdog_init(&wdt, entries, 2, wdt_on_timeout, NULL);

    /* Attach errlog */
    static SYN_ErrEntry errlog_buf[8];
    static SYN_ErrLog errlog;
    syn_errlog_init(&errlog, errlog_buf, 8, 1);
    wdt.errlog = &errlog;

    int8_t id0 = syn_watchdog_register(&wdt, "a", 100);
    int8_t id1 = syn_watchdog_register(&wdt, "b", 100);
    TEST_ASSERT_TRUE(id0 >= 0);
    TEST_ASSERT_TRUE(id1 >= 0);

    /* Table full — returns -1 (line 55) */
    int8_t id2 = syn_watchdog_register(&wdt, "c", 100);
    TEST_ASSERT_EQUAL_INT(-1, id2);

    /* Trigger timeout — errlog records it (lines 88-89) */
    mock_tick_advance(200);
    syn_watchdog_update(&wdt);
    TEST_ASSERT_TRUE(wdt_timeout_count >= 1);
    TEST_ASSERT_TRUE(syn_errlog_count(&errlog) > 0);
}

/** Regression: double-unregister must not underflow wdt->count. */
static void test_watchdog_double_unregister(void)
{
    mock_tick_ms = 0;

    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[2];
    syn_watchdog_init(&wdt, entries, 2, NULL, NULL);

    int8_t id = syn_watchdog_register(&wdt, "a", 100);
    TEST_ASSERT_TRUE(id >= 0);
    TEST_ASSERT_EQUAL_UINT8(1, wdt.count);

    syn_watchdog_unregister(&wdt, id);
    TEST_ASSERT_EQUAL_UINT8(0, wdt.count);

    /* Second call on the same (now inactive) id must be a no-op */
    syn_watchdog_unregister(&wdt, id);
    TEST_ASSERT_EQUAL_UINT8(0, wdt.count); /* Must NOT wrap to 255 */

    /* Re-register into freed slot */
    int8_t new_id = syn_watchdog_register(&wdt, "b", 200);
    TEST_ASSERT_EQUAL_INT(id, new_id);
    TEST_ASSERT_EQUAL_UINT8(1, wdt.count);
}

static void test_watchdog_timeout_without_callback_or_errlog(void)
{
    mock_tick_ms = 0;
    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[2];
    /* Init with NULL callback and NULL errlog */
    syn_watchdog_init(&wdt, entries, 2, NULL, NULL);

    int8_t id = syn_watchdog_register(&wdt, "nocb", 50);
    TEST_ASSERT_TRUE(id >= 0);

    /* Advance time beyond timeout */
    mock_tick_advance(100);
    syn_watchdog_update(&wdt);

    /* Last checkin should have been reset to now (100) */
    TEST_ASSERT_EQUAL_UINT32(100, entries[id].last_checkin);
}

static void test_watchdog_capacity_clamp_null_entries_and_isr_forward_checkin(void)
{
    static SYN_WDT_Entry big_entries[200];
    SYN_Watchdog wdt;
    mock_tick_ms = 100U;
    wdt_timeout_count = 0;

    /* 1. Capacity > SYN_WATCHDOG_MAX_CAPACITY (127) clamps cleanly */
    syn_watchdog_init(&wdt, big_entries, 200U, wdt_on_timeout, NULL);
    TEST_ASSERT_EQUAL_UINT8(SYN_WATCHDOG_MAX_CAPACITY, wdt.capacity);

    /* 2. Check-in after unregister returns early cleanly without reactivating or updating */
    int8_t id = syn_watchdog_register(&wdt, "temp", 50U);
    TEST_ASSERT_TRUE(id >= 0);
    syn_watchdog_unregister(&wdt, id);
    mock_tick_ms = 120U;
    syn_watchdog_checkin(&wdt, id);
    TEST_ASSERT_FALSE(wdt.entries[id].active);
    TEST_ASSERT_EQUAL_UINT32(100U, wdt.entries[id].last_checkin);

    /* 3. Uninitialized watchdog (entries == NULL) early returns */
    SYN_Watchdog zero_wdt = {0};
    syn_watchdog_update(&zero_wdt);
    TEST_ASSERT_EQUAL_INT8(-1, syn_watchdog_register(&zero_wdt, "none", 50U));

    /* 4. ISR forward check-in (K-07): last_checkin slightly ahead of captured now must NOT fire */
    mock_tick_ms = 200U;
    int8_t isr_id = syn_watchdog_register(&wdt, "isr_task", 50U);
    TEST_ASSERT_TRUE(isr_id >= 0);
    wdt.entries[isr_id].last_checkin = mock_tick_ms + 5U;

    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);
    TEST_ASSERT_EQUAL_UINT32(205U, wdt.entries[isr_id].last_checkin);

    /* Once time genuinely advances past 205 + 50 = 255, timeout fires normally */
    mock_tick_ms = 256U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(1, wdt_timeout_count);
}

typedef struct {
    int8_t id;
    uint32_t advance_ms;
    int fires;
} WdtCbCheckinCtx;

static void wdt_checkin_inside_cb(SYN_Watchdog *wdt, const SYN_WDT_Entry *entry, void *ctx)
{
    (void)entry;
    WdtCbCheckinCtx *c = (WdtCbCheckinCtx *)ctx;
    c->fires++;
    mock_tick_advance(c->advance_ms);
    syn_watchdog_checkin(wdt, c->id);
}

/** R3-T04 & R3-K09: Callback check-in timestamp preservation + INT32_MAX timeout clamp. */
static void test_watchdog_callback_checkin_preserved_and_timeout_clamp(void)
{
    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[3];
    WdtCbCheckinCtx ctx = {-1, 15U, 0};

    mock_tick_ms = 100U;
    syn_watchdog_init(&wdt, entries, 3U, wdt_checkin_inside_cb, &ctx);

    ctx.id = syn_watchdog_register(&wdt, "cb_kick", 50U);
    TEST_ASSERT_TRUE(ctx.id >= 0);

    /* Timeout fires at t=160; callback advances tick to 175 and calls syn_watchdog_checkin() */
    mock_tick_ms = 160U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(1, ctx.fires);
    TEST_ASSERT_EQUAL_UINT32(175U, entries[ctx.id].last_checkin);

    /* At t=220 (60 ms after 160, but only 45 ms after callback's 175 check-in): must NOT fire */
    mock_tick_ms = 220U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(1, ctx.fires);

    /* At t=225 (50 ms after 175): fires cleanly */
    mock_tick_ms = 225U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(2, ctx.fires);

    /* R3-K09: timeout_ms > 0x7FFFFFFFU clamps to 0x7FFFFFFFU so signed diff can expire */
    wdt_timeout_count = 0;
    mock_tick_ms = 500U;
    syn_watchdog_init(&wdt, entries, 3U, wdt_on_timeout, NULL);
    int8_t big0 = syn_watchdog_register(&wdt, "big0", 0x80000000U);
    int8_t big1 = syn_watchdog_register(&wdt, "big1", UINT32_MAX);
    TEST_ASSERT_TRUE(big0 >= 0);
    TEST_ASSERT_TRUE(big1 >= 0);
    TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, entries[big0].timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, entries[big1].timeout_ms);

    mock_tick_ms = 500U + 0x7FFFFFFEU;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);

    mock_tick_ms = 500U + 0x7FFFFFFFU;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(2, wdt_timeout_count);
}

static SYN_Watchdog *s_r4_isr_wdt = NULL;
static int8_t s_r4_isr_id = -1;
static int s_r4_cb_critical_depth = -1;

static void r4_wdt_isr_checkin_hook(void)
{
    mock_critical_enter_cb = NULL;
    syn_watchdog_checkin(s_r4_isr_wdt, s_r4_isr_id);
}

static void r4_wdt_timeout_verify_outside_critical(SYN_Watchdog *wdt, const SYN_WDT_Entry *entry,
                                                   void *ctx)
{
    (void)wdt;
    (void)entry;
    (void)ctx;
    s_r4_cb_critical_depth = mock_critical_depth;
    wdt_timeout_count++;
}

/** R4-K09: Critical-section TOCTOU protection, callback outside critical, and 0ms clamp. */
static void test_r4_watchdog_isr_toctou_and_zero_timeout_clamp(void)
{
    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[2];

    /* 1. NULL / invalid ID guards */
    TEST_ASSERT_EQUAL_INT8(-1, syn_watchdog_register(NULL, "x", 10U));
    syn_watchdog_checkin(NULL, 0);
    syn_watchdog_unregister(NULL, 0);
    syn_watchdog_update(NULL);

    mock_tick_ms = 100U;
    wdt_timeout_count = 0;
    s_r4_cb_critical_depth = -1;
    syn_watchdog_init(&wdt, entries, 2U, r4_wdt_timeout_verify_outside_critical, NULL);
    syn_watchdog_checkin(&wdt, -1);
    syn_watchdog_unregister(&wdt, -1);

    /* 2. timeout_ms == 0U clamps to 1U: does not fire on same tick, fires after 1ms */
    int8_t zero_id = syn_watchdog_register(&wdt, "zero_clamp", 0U);
    TEST_ASSERT_TRUE(zero_id >= 0);
    TEST_ASSERT_EQUAL_UINT32(1U, entries[zero_id].timeout_ms);
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);

    mock_tick_ms = 101U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(1, wdt_timeout_count);
    TEST_ASSERT_EQUAL_INT(0, s_r4_cb_critical_depth); /* Callback ran outside critical section */
    syn_watchdog_unregister(&wdt, zero_id);

    /* 3. Interleaved ISR check-in right as syn_watchdog_update enters critical section */
    mock_tick_ms = 200U;
    wdt_timeout_count = 0;
    int8_t isr_id = syn_watchdog_register(&wdt, "isr_race", 50U);
    TEST_ASSERT_TRUE(isr_id >= 0);

    mock_tick_ms = 260U; /* Past 250ms deadline unless ISR just checked in */
    s_r4_isr_wdt = &wdt;
    s_r4_isr_id = isr_id;
    mock_critical_enter_cb = r4_wdt_isr_checkin_hook;

    syn_watchdog_update(&wdt);
    mock_critical_enter_cb = NULL;
    TEST_ASSERT_EQUAL_INT(0, wdt_timeout_count);
    TEST_ASSERT_EQUAL_UINT32(260U, entries[isr_id].last_checkin);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
}

static void test_watchdog_max_timeout_fires_past_int32_max_elapsed(void)
{
    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[1];

    mock_tick_ms = 100U;
    wdt_timeout_count = 0;
    syn_watchdog_init(&wdt, entries, 1U, wdt_on_timeout, NULL);

    int8_t id = syn_watchdog_register(&wdt, "max_tmo", 0x7FFFFFFFU);
    TEST_ASSERT_TRUE(id >= 0);
    TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, entries[id].timeout_ms);

    /* Poll 1 ms past 0x7FFFFFFFU deadline (elapsed == 0x80000000U): must fire, not wrap negative */
    mock_tick_ms = 100U + 0x80000000U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(1, wdt_timeout_count);

    /* Poll 4097 ms past the reset deadline (elapsed == 0x80001000U): also fires */
    mock_tick_ms += 0x80001000U;
    syn_watchdog_update(&wdt);
    TEST_ASSERT_EQUAL_INT(2, wdt_timeout_count);
}

typedef struct {
    SYN_Watchdog *wdt;
    SYN_WDT_Entry new_entries[2];
    int enter_Goal;
    int enter_seen;
    int cb_fires;
    const char *observed_name;
    bool reinit_in_cb;
} WdtSnapReinitCtx;

static WdtSnapReinitCtx *s_wdt_snap_ctx = NULL;

static void wdt_isr_hijack_slot0_during_errlog(void)
{
    WdtSnapReinitCtx *c = s_wdt_snap_ctx;
    if (c == NULL) {
        return;
    }
    c->enter_seen++;
    if (c->enter_seen == c->enter_Goal) {
        mock_critical_enter_cb = NULL;
        syn_watchdog_unregister(c->wdt, 0);
        (void)syn_watchdog_register(c->wdt, "hijacked_comms", 2000U);
    }
}

static void wdt_snap_reinit_cb(SYN_Watchdog *wdt, const SYN_WDT_Entry *entry, void *ctx)
{
    WdtSnapReinitCtx *c = (WdtSnapReinitCtx *)ctx;
    c->cb_fires++;
    c->observed_name = entry->name;
    /* Reentrant syn_watchdog_update inside callback is a safe no-op */
    syn_watchdog_update(wdt);
    if (c->reinit_in_cb) {
        c->reinit_in_cb = false;
        syn_watchdog_init(wdt, c->new_entries, 2U, wdt_snap_reinit_cb, c);
        (void)syn_watchdog_register(wdt, "new_slot0", 500U);
        (void)syn_watchdog_register(wdt, "new_slot1", 10U);
        mock_tick_advance(25U);
    }
}

static void test_watchdog_snapshot_across_isr_and_reinit_in_cb(void)
{
    static SYN_ErrEntry err_buf[4];
    static SYN_ErrLog errlog;
    SYN_Watchdog wdt;
    SYN_WDT_Entry entries[2];
    WdtSnapReinitCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.wdt = &wdt;
    ctx.enter_Goal = 2; /* 1st critical = slot 0 check, 2nd critical = syn_errlog_record */

    /* 1. ISR replacing slot 0 during syn_errlog_record must not corrupt entry passed to callback */
    mock_tick_ms = 100U;
    syn_errlog_init(&errlog, err_buf, 4U, 1U);
    syn_watchdog_init(&wdt, entries, 2U, wdt_snap_reinit_cb, &ctx);
    wdt.errlog = &errlog;
    TEST_ASSERT_EQUAL_INT8(0, syn_watchdog_register(&wdt, "sensor_orig", 50U));

    mock_tick_ms = 160U;
    s_wdt_snap_ctx = &ctx;
    mock_critical_enter_cb = wdt_isr_hijack_slot0_during_errlog;
    syn_watchdog_update(&wdt);
    mock_critical_enter_cb = NULL;
    s_wdt_snap_ctx = NULL;

    TEST_ASSERT_EQUAL_INT(1, ctx.cb_fires);
    TEST_ASSERT_EQUAL_STRING("sensor_orig", ctx.observed_name);

    /* 2. Callback calling syn_watchdog_init() aborts remaining outer loop slots cleanly */
    memset(&ctx, 0, sizeof(ctx));
    ctx.wdt = &wdt;
    ctx.reinit_in_cb = true;
    mock_tick_ms = 200U;
    syn_watchdog_init(&wdt, entries, 2U, wdt_snap_reinit_cb, &ctx);
    TEST_ASSERT_EQUAL_INT8(0, syn_watchdog_register(&wdt, "old0", 20U));
    TEST_ASSERT_EQUAL_INT8(1, syn_watchdog_register(&wdt, "old1", 20U));

    mock_tick_ms = 230U;
    syn_watchdog_update(&wdt);
    /* Only old0's timeout fires on this update; new_slot1 must NOT be scanned in stale loop */
    TEST_ASSERT_EQUAL_INT(1, ctx.cb_fires);
    TEST_ASSERT_EQUAL_STRING("old0", ctx.observed_name);
}

void run_watchdog_tests(void)
{
    RUN_TEST(test_watchdog);
    RUN_TEST(test_watchdog_table_full_and_errlog);
    RUN_TEST(test_watchdog_double_unregister);
    RUN_TEST(test_watchdog_timeout_without_callback_or_errlog);
    RUN_TEST(test_watchdog_capacity_clamp_null_entries_and_isr_forward_checkin);
    RUN_TEST(test_watchdog_callback_checkin_preserved_and_timeout_clamp);
    RUN_TEST(test_r4_watchdog_isr_toctou_and_zero_timeout_clamp);
    RUN_TEST(test_watchdog_max_timeout_fires_past_int32_max_elapsed);
    RUN_TEST(test_watchdog_snapshot_across_isr_and_reinit_in_cb);
}
