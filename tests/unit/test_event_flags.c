/**
 * @file test_event_flags.c
 * @brief Unit test suite for Event Flag Groups (syn_event_flags).
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_event_flags.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

void test_event_flags_set_clear_wait(void)
{
    SYN_EventFlags ef;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_init(&ef));
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(&ef));

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_set(&ef, 0x05U)); /* Bits 0 & 2 */
    TEST_ASSERT_EQUAL_UINT32(0x05U, syn_event_flags_get(&ef));

    uint32_t matched = 0;

    /* WAIT_ALL - unsatisfiable (requires 0x07U) */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY,
                          syn_event_flags_wait(&ef, 0x07U, SYN_EVENT_FLAGS_WAIT_ALL, &matched));

    /* WAIT_ANY - satisfied (bit 0 or 2 present) */
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_event_flags_wait(&ef, 0x07U, SYN_EVENT_FLAGS_WAIT_ANY, &matched));
    TEST_ASSERT_EQUAL_UINT32(0x05U, matched);

    /* WAIT_ALL with AUTO_CLEAR */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_set(&ef, 0x02U)); /* Bit 1 set -> now 0x07U */
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_event_flags_wait(
                    &ef, 0x07U, SYN_EVENT_FLAGS_WAIT_ALL | SYN_EVENT_FLAGS_AUTO_CLEAR, &matched));
    TEST_ASSERT_EQUAL_UINT32(0x07U, matched);
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(&ef)); /* Flags auto-cleared */

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_clear(&ef, 0xFFFFFFFFU));
}

void test_event_flags_null_and_invalid_params(void)
{
    SYN_EventFlags ef;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_init(&ef));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_set(&ef, 0x01U));

    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_event_flags_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_event_flags_set(NULL, 0x01U));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_event_flags_clear(NULL, 0x01U));
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(NULL));

    /* Wait with NULL context or wait_mask == 0 */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_event_flags_wait(NULL, 0x01U, SYN_EVENT_FLAGS_WAIT_ANY, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_event_flags_wait(&ef, 0, SYN_EVENT_FLAGS_WAIT_ANY, NULL));

    /* Wait with out_flags == NULL (valid call) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_wait(&ef, 0x01U, SYN_EVENT_FLAGS_WAIT_ANY, NULL));
}

static void test_event_flags_wait_any_auto_clear(void)
{
    SYN_EventFlags ef;
    syn_event_flags_init(&ef);
    syn_event_flags_set(&ef, 0x0CU);
    uint32_t matched = 0;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_event_flags_wait(
                    &ef, 0x0EU, SYN_EVENT_FLAGS_WAIT_ANY | SYN_EVENT_FLAGS_AUTO_CLEAR, &matched));
    TEST_ASSERT_EQUAL_UINT32(0x0CU, matched);
    TEST_ASSERT_EQUAL_UINT32(0x00U, syn_event_flags_get(&ef));
}

/**
 * Regression: set/clear/wait are ISR-callable read-modify-writes and must
 * run inside critical sections (volatile alone is not atomic). Verifies
 * every path — including the wait() early-return — enters and fully exits
 * the critical section.
 */
static void test_event_flags_rmw_critical_sections(void)
{
    SYN_EventFlags ef;
    syn_event_flags_init(&ef);

    int enters = mock_critical_enter_count;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_set(&ef, 0x01U));
    TEST_ASSERT_EQUAL_INT(enters + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_event_flags_clear(&ef, 0x01U));
    TEST_ASSERT_EQUAL_INT(enters + 2, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* wait(): satisfied path with AUTO_CLEAR (RMW) must be protected and
     * must not clobber bits outside the matched set */
    syn_event_flags_set(&ef, 0x03U);
    uint32_t matched = 0;
    TEST_ASSERT_EQUAL_INT(
        SYN_OK, syn_event_flags_wait(
                    &ef, 0x01U, SYN_EVENT_FLAGS_WAIT_ANY | SYN_EVENT_FLAGS_AUTO_CLEAR, &matched));
    TEST_ASSERT_EQUAL_UINT32(0x01U, matched);
    TEST_ASSERT_EQUAL_UINT32(0x02U, syn_event_flags_get(&ef));
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* wait(): unsatisfied path must not leak the critical section */
    TEST_ASSERT_EQUAL_INT(SYN_BUSY,
                          syn_event_flags_wait(&ef, 0x10U, SYN_EVENT_FLAGS_WAIT_ANY, NULL));
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
}

static SYN_EventFlags g_block_ef;

static SYN_PT_Status pt_block_event_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, &g_block_ef, 0x01U);
    PT_END(pt);
}

static void test_pt_block_event_critical_section(void)
{
    SYN_Task task;
    SYN_Sched sched;
    syn_task_create(&task, "evt_block", pt_block_event_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);
    syn_event_flags_init(&g_block_ef);
    syn_event_flags_set(&g_block_ef, 0x01U);

    int enters_before = mock_critical_enter_count;
    syn_sched_run(&sched);
    syn_sched_run(&sched);

    TEST_ASSERT_GREATER_THAN(enters_before, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(&g_block_ef));
}

static uint32_t g_timeout_step;

static SYN_PT_Status pt_block_event_timeout_null_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    /* 1. Immediate timeout_ms == 0 with literal NULL out_flags */
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, &g_block_ef, 0x02U, 0U, NULL);
    g_timeout_step = 1U;

    /* 2. Blocking timeout_ms > 0 signaled before deadline, literal NULL out_flags */
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, &g_block_ef, 0x04U, 20U, NULL);
    g_timeout_step = 2U;

    /* 3. Blocking timeout_ms > 0 expiring on timeout, literal NULL out_flags */
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, &g_block_ef, 0x08U, 15U, NULL);
    g_timeout_step = 3U;
    PT_END(pt);
}

static void test_r5_pt_block_event_with_timeout_null_out(void)
{
    SYN_Task task;
    SYN_Sched sched;
    mock_port_reset();
    g_timeout_step = 0U;

    syn_event_flags_init(&g_block_ef);
    syn_event_flags_set(&g_block_ef, 0x02U);

    syn_task_create(&task, "evt_to_null", pt_block_event_timeout_null_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* Step 1 completes immediately and blocks on Step 2 */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1U, g_timeout_step);
    TEST_ASSERT_EQUAL_UINT32(0U, syn_event_flags_get(&g_block_ef));

    /* Signal 0x04U before 20 ms deadline -> wakes Step 2, blocks on Step 3 */
    mock_tick_advance(5U);
    syn_event_flags_set(&g_block_ef, 0x04U);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(2U, g_timeout_step);
    TEST_ASSERT_EQUAL_UINT32(0U, syn_event_flags_get(&g_block_ef));

    /* Advance past 15 ms deadline without setting 0x08U -> Step 3 times out */
    mock_tick_advance(20U);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(3U, g_timeout_step);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
}

static uint32_t g_tier_a_evt_step;
static uint32_t g_tier_a_evt_out;

static SYN_PT_Status pt_tier_a_event_edge_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    /* mask == 0 and grp == NULL must not wedge task in SYN_TASK_BLOCKED */
    PT_BLOCK_EVENT(pt, task, &g_block_ef, 0U);
    PT_BLOCK_EVENT(pt, task, NULL, 0x01U);
    g_tier_a_evt_out = 0xDEADBEEFU;
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, NULL, 0x04U, 25U, &g_tier_a_evt_out);
    TEST_ASSERT_EQUAL_UINT32(0U, g_tier_a_evt_out);
    TEST_ASSERT_EQUAL_UINT32(0U, task->delay_until);
    g_tier_a_evt_step = 1U;

    /* Immediate-consume fast path when flags are already set */
    PT_BLOCK_EVENT(pt, task, &g_block_ef, 0x01U);
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, &g_block_ef, 0x02U, 50U, &g_tier_a_evt_out);
    g_tier_a_evt_step = 2U;
    PT_END(pt);
}

static void test_tier_a_pt_sem_and_event_guards(void)
{
    /* SCHED-1: pt_sem_signal NULL guard and INT16_MAX saturation */
    SYN_PT_Sem sem;
    PT_SEM_INIT(&sem, 0);
    pt_sem_signal(NULL);
    PT_SEM_SIGNAL(&sem);
    TEST_ASSERT_EQUAL_INT16(1, PT_SEM_COUNT(&sem));
    sem.count = INT16_MAX;
    PT_SEM_SIGNAL(&sem);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, PT_SEM_COUNT(&sem));
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* Direct NULL task guard coverage on syn_task_arm_event_wait */
    uint32_t direct_out = 0xFFU;
    TEST_ASSERT_FALSE(syn_task_arm_event_wait(NULL, &g_block_ef, 0x01U, false, 0U, &direct_out));
    TEST_ASSERT_EQUAL_UINT32(0U, direct_out);

    /* SCHED-2: mask==0 / grp==NULL guards + single-dispatch immediate consume */
    SYN_Task task;
    SYN_Sched sched;
    mock_port_reset();
    g_tier_a_evt_step = 0U;
    g_tier_a_evt_out = 0U;
    syn_event_flags_init(&g_block_ef);
    syn_event_flags_set(&g_block_ef, 0x03U);

    syn_task_create(&task, "tier_a_evt", pt_tier_a_event_edge_fn, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* Single dispatch runs straight through all guards + pre-set flags to PT_END */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(2U, g_tier_a_evt_step);
    TEST_ASSERT_EQUAL_UINT32(0x02U, g_tier_a_evt_out);
    TEST_ASSERT_EQUAL_UINT32(0U, syn_event_flags_get(&g_block_ef));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, task.state);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
}

void run_event_flags_tests(void)
{
    RUN_TEST(test_event_flags_set_clear_wait);
    RUN_TEST(test_event_flags_null_and_invalid_params);
    RUN_TEST(test_event_flags_wait_any_auto_clear);
    RUN_TEST(test_event_flags_rmw_critical_sections);
    RUN_TEST(test_pt_block_event_critical_section);
    RUN_TEST(test_r5_pt_block_event_with_timeout_null_out);
    RUN_TEST(test_tier_a_pt_sem_and_event_guards);
}
