/**
 * @file test_timer.c
 * @brief Unity tests for syn_timer.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_timer.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static int timer_fire_count = 0;

static void timer_callback(SYN_Timer *t, void *ctx)
{
    (void)t;
    (void)ctx;
    timer_fire_count++;
}

static void test_software_timer(void)
{
    SYN_Timer tmr;
    mock_tick_ms = 0;
    timer_fire_count = 0;

    syn_timer_init(&tmr, 100, true, timer_callback, NULL);
    syn_timer_start(&tmr);

    /* Service at t=50 — should not fire */
    mock_tick_advance(50);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(0, timer_fire_count);

    /* Service at t=100 — should fire */
    mock_tick_advance(50);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(1, timer_fire_count);

    /* Service at t=200 — should fire again (periodic) */
    mock_tick_advance(100);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(2, timer_fire_count);

    /* Stop and verify no more fires */
    syn_timer_stop(&tmr);
    mock_tick_advance(200);
    syn_timer_service(&tmr, 1);
    TEST_ASSERT_EQUAL_INT(2, timer_fire_count);
}

static void test_timeout(void)
{
    mock_tick_ms = 0;
    SYN_Timeout to;

    syn_timeout_start(&to, 100);
    TEST_ASSERT_FALSE(syn_timeout_expired(&to));
    TEST_ASSERT_EQUAL_INT(100, syn_timeout_remaining(&to));

    mock_tick_advance(50);
    TEST_ASSERT_FALSE(syn_timeout_expired(&to));
    TEST_ASSERT_EQUAL_INT(50, syn_timeout_elapsed(&to));
    TEST_ASSERT_EQUAL_INT(50, syn_timeout_remaining(&to));

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(syn_timeout_expired(&to));
    TEST_ASSERT_EQUAL_INT(0, syn_timeout_remaining(&to));

    /* Restart */
    syn_timeout_restart(&to);
    TEST_ASSERT_FALSE(syn_timeout_expired(&to));

    /* Periodic */
    mock_tick_ms = 0;
    syn_timeout_start(&to, 50);
    TEST_ASSERT_FALSE(syn_timeout_periodic(&to));

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(syn_timeout_periodic(&to));
    TEST_ASSERT_FALSE(syn_timeout_periodic(&to));

    mock_tick_advance(50);
    TEST_ASSERT_TRUE(syn_timeout_periodic(&to));
}

static void test_timer_extensions(void)
{
    SYN_Timer tmr;
    mock_tick_ms = 0;
    timer_fire_count = 0;

    /* 1. Init inactive timer and check syn_timer_expired */
    syn_timer_init(&tmr, 100, false, timer_callback, NULL);
    TEST_ASSERT_FALSE(syn_timer_expired(&tmr));

    /* 2. Set period & remaining */
    TEST_ASSERT_EQUAL_INT(0, syn_timer_remaining(&tmr));
    syn_timer_set_period(&tmr, 200);
    TEST_ASSERT_EQUAL_INT(200, tmr.period_ms);
    syn_timer_start(&tmr);
    TEST_ASSERT_EQUAL_INT(200, syn_timer_remaining(&tmr));

    mock_tick_advance(50);
    TEST_ASSERT_EQUAL_INT(150, syn_timer_remaining(&tmr));

    /* 3. Expiration of one-shot timer */
    mock_tick_advance(150); /* t=200 */
    TEST_ASSERT_TRUE(syn_timer_expired(&tmr));
    TEST_ASSERT_FALSE(tmr.active);
    TEST_ASSERT_EQUAL_INT(0, syn_timer_remaining(&tmr));

    /* 4. Timer service with NULL callback */
    SYN_Timer null_cb_tmr;
    syn_timer_init(&null_cb_tmr, 100, false, NULL, NULL);
    syn_timer_start(&null_cb_tmr);
    mock_tick_advance(100);
    syn_timer_service(&null_cb_tmr, 1);
    TEST_ASSERT_FALSE(null_cb_tmr.active);

    /* 5. Periodic drift and snap forward */
    SYN_Timer periodic_tmr;
    syn_timer_init(&periodic_tmr, 100, true, timer_callback, NULL);
    syn_timer_start(&periodic_tmr); /* target = 100 */

    mock_tick_advance(250); /* t = 350 */
    TEST_ASSERT_TRUE(syn_timer_expired(&periodic_tmr));
    TEST_ASSERT_EQUAL_INT(mock_tick_ms + periodic_tmr.period_ms,
                          periodic_tmr.target_tick); /* snapped forward to now + period */

    syn_timer_stop(&periodic_tmr);
}

static void test_timer_tick_wraparound(void)
{
    SYN_Timer tmr;
    SYN_Timeout to;

    /* Start tick near 32-bit overflow */
    mock_tick_ms = 0xFFFFFFE0u;

    syn_timer_init(&tmr, 50, false, timer_callback, NULL);
    syn_timer_start(&tmr); /* target_tick = 0xFFFFFFE0 + 50 = 18 */
    TEST_ASSERT_FALSE(syn_timer_expired(&tmr));

    syn_timeout_start(&to, 50); /* target_tick = 18 */
    TEST_ASSERT_FALSE(syn_timeout_expired(&to));

    /* Advance across 0xFFFFFFFF overflow boundary by 60ms (tick becomes 28) */
    mock_tick_advance(60);

    TEST_ASSERT_TRUE(syn_timer_expired(&tmr));
    TEST_ASSERT_TRUE(syn_timeout_expired(&to));
    TEST_ASSERT_EQUAL_UINT32(0, syn_timeout_remaining(&to));
}

static void test_timer_next_expiry(void)
{
    SYN_Timer timers[3];
    mock_tick_ms = 1000;

    syn_timer_init(&timers[0], 500, false, NULL, NULL);
    syn_timer_init(&timers[1], 200, false, NULL, NULL);
    syn_timer_init(&timers[2], 1000, false, NULL, NULL);

    /* Inactive timers return UINT32_MAX */
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_timer_next_expiry(timers, 3));

    /* Start timers: 0 at t=1500, 1 at t=1200, 2 at t=2000 */
    syn_timer_start(&timers[0]);
    syn_timer_start(&timers[1]);
    syn_timer_start(&timers[2]);

    /* Earliest is timer 1 at t=1200 */
    TEST_ASSERT_EQUAL_UINT32(1200, syn_timer_next_expiry(timers, 3));

    /* Advance tick to 1250 (timer 1 expired -> any_ready_now == true) */
    mock_tick_advance(250);
    TEST_ASSERT_EQUAL_UINT32(1250, syn_timer_next_expiry(timers, 3));
}

static void test_timer_next_expiry_uint32_max_clamp_and_edges(void)
{
    SYN_Timer timers[3];

    /* Empty timer list */
    mock_tick_ms = 500U;
    syn_timer_service(NULL, 0U);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_timer_next_expiry(NULL, 0U));

    /* target_tick == UINT32_MAX (0xFFFFFFF0 + 15) must clamp to UINT32_MAX - 1 */
    mock_tick_ms = 0xFFFFFFF0U;
    syn_timer_init(&timers[0], 15U, false, NULL, NULL);
    TEST_ASSERT_FALSE(syn_timer_is_active(&timers[0]));
    syn_timer_start(&timers[0]);
    TEST_ASSERT_TRUE(syn_timer_is_active(&timers[0]));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, timers[0].target_tick);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX - 1U, syn_timer_next_expiry(&timers[0], 1U));

    /* Active but already-past target_tick returns 0 remaining before syn_timer_expired() */
    mock_tick_ms = 100U;
    syn_timer_init(&timers[0], 50U, false, NULL, NULL);
    syn_timer_start(&timers[0]);
    mock_tick_ms = 160U;
    TEST_ASSERT_EQUAL_UINT32(0U, syn_timer_remaining(&timers[0]));

    /* Mixed inactive + active future timers */
    mock_tick_ms = 1000U;
    syn_timer_init(&timers[0], 50U, false, NULL, NULL); /* inactive */
    syn_timer_init(&timers[1], 300U, false, NULL, NULL);
    syn_timer_init(&timers[2], 120U, false, NULL, NULL);
    syn_timer_start(&timers[1]);
    syn_timer_start(&timers[2]);
    TEST_ASSERT_EQUAL_UINT32(1120U, syn_timer_next_expiry(timers, 3U));
}

/** R3-K09: period_ms > INT32_MAX clamps in syn_timer_init() and syn_timer_set_period(). */
static void test_timer_period_int32_max_clamp(void)
{
    SYN_Timer tmr;
    mock_tick_ms = 1000U;

    syn_timer_init(&tmr, 0x80000005U, false, NULL, NULL);
    TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, tmr.period_ms);

    syn_timer_start(&tmr);
    TEST_ASSERT_FALSE(syn_timer_expired(&tmr));
    TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, syn_timer_remaining(&tmr));

    mock_tick_ms = 1000U + 0x7FFFFFFEU;
    TEST_ASSERT_FALSE(syn_timer_expired(&tmr));

    mock_tick_ms = 1000U + 0x7FFFFFFFU;
    TEST_ASSERT_TRUE(syn_timer_expired(&tmr));

    mock_tick_ms = 250U;
    syn_timer_set_period(&tmr, UINT32_MAX);
    TEST_ASSERT_EQUAL_UINT32(0x7FFFFFFFU, tmr.period_ms);
    syn_timer_start(&tmr);
    TEST_ASSERT_FALSE(syn_timer_expired(&tmr));
}

/** R4-K10: Zero period_ms clamps to 1ms and public NULL guards return safely. */
static void test_r4_timer_zero_period_clamp_and_null_guards(void)
{
    SYN_Timer tmr;

    /* 1. Public NULL guards */
    syn_timer_init(NULL, 10U, false, NULL, NULL);
    syn_timer_start(NULL);
    syn_timer_stop(NULL);
    syn_timer_set_period(NULL, 10U);
    TEST_ASSERT_FALSE(syn_timer_expired(NULL));
    syn_timer_service(NULL, 2U);
    TEST_ASSERT_EQUAL_UINT32(0U, syn_timer_remaining(NULL));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_timer_next_expiry(NULL, 2U));

    /* 2. period_ms == 0U clamps to 1U in both init and set_period */
    mock_tick_ms = 500U;
    syn_timer_init(&tmr, 0U, true, NULL, NULL);
    TEST_ASSERT_EQUAL_UINT32(1U, tmr.period_ms);
    syn_timer_start(&tmr);
    TEST_ASSERT_FALSE(syn_timer_expired(&tmr));
    TEST_ASSERT_EQUAL_UINT32(501U, syn_timer_next_expiry(&tmr, 1U));

    mock_tick_ms = 501U;
    TEST_ASSERT_TRUE(syn_timer_expired(&tmr));

    syn_timer_set_period(&tmr, 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, tmr.period_ms);
}

static int g_cb_crit_depth = -1;

static void timer_self_stop_cb(SYN_Timer *t, void *ctx)
{
    int *cnt = (int *)ctx;
    g_cb_crit_depth = mock_critical_depth;
    (*cnt)++;
    syn_timer_stop(t);
}

static void test_tier_a_timer_critical_section_and_self_stop(void)
{
    /* SCHED-5: Callback executes outside critical section and self-stop is not clobbered */
    SYN_Timer tmr;
    int fires = 0;
    mock_port_reset();
    mock_tick_ms = 1000U;
    g_cb_crit_depth = -1;

    int cs_before = mock_critical_enter_count;
    syn_timer_init(&tmr, 25U, true, timer_self_stop_cb, &fires);
    syn_timer_start(&tmr);
    TEST_ASSERT_GREATER_THAN(cs_before, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    mock_tick_ms = 1025U;
    syn_timer_service(&tmr, 1U);
    TEST_ASSERT_EQUAL_INT(1, fires);
    TEST_ASSERT_EQUAL_INT(0, g_cb_crit_depth);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    TEST_ASSERT_FALSE(syn_timer_is_active(&tmr));

    mock_tick_ms = 1050U;
    syn_timer_service(&tmr, 1U);
    TEST_ASSERT_EQUAL_INT(1, fires);
}

void run_timer_tests(void)
{
    RUN_TEST(test_software_timer);
    RUN_TEST(test_timeout);
    RUN_TEST(test_timer_extensions);
    RUN_TEST(test_timer_tick_wraparound);
    RUN_TEST(test_timer_next_expiry);
    RUN_TEST(test_timer_next_expiry_uint32_max_clamp_and_edges);
    RUN_TEST(test_timer_period_int32_max_clamp);
    RUN_TEST(test_r4_timer_zero_period_clamp_and_null_guards);
    RUN_TEST(test_tier_a_timer_critical_section_and_self_stop);
}
