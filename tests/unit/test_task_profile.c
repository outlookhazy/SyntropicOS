/**
 * @file test_task_profile.c
 * @brief Unit test suite for Opt-In Task Execution & Cooperative Chunk-Budget Profiler.
 */

#include "mocks/mock_port.h"
#include "syntropic/debug/syn_task_profile.h"
#include "unity/unity.h"

#include <string.h>

typedef struct {
    uint32_t run_us;
    uint32_t max_runs;
    uint32_t runs;
    SYN_Sched *detach_sched;
} ProfileTaskCtx;

static SYN_PT_Status profile_timed_task(SYN_PT *pt, SYN_Task *task)
{
    ProfileTaskCtx *ctx = (ProfileTaskCtx *)task->user_data;
    PT_BEGIN(pt);
    while (ctx->runs < ctx->max_runs) {
        mock_tick_advance_us(ctx->run_us);
        ctx->runs++;
        if (ctx->detach_sched != NULL) {
            syn_sched_set_profiler(ctx->detach_sched, NULL);
        }
        if (ctx->runs < ctx->max_runs) {
            PT_PASS(pt);
        }
    }
    PT_END(pt);
}

void test_task_profile_init_and_step(void)
{
    SYN_TaskProfileManager mgr;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_init(&mgr));

    syn_task_profile_step_start(&mgr, 0, "blink_task", 1000);
    syn_task_profile_step_end(&mgr, 0, 1500); /* 500 us duration */

    syn_task_profile_step_start(&mgr, 0, "blink_task", 2000);
    syn_task_profile_step_end(&mgr, 0, 3200); /* 1200 us duration */

    SYN_TaskProfile prof;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0, &prof));
    TEST_ASSERT_EQUAL_STRING("blink_task", prof.task_name);
    TEST_ASSERT_EQUAL_UINT32(2, prof.step_count);
    TEST_ASSERT_EQUAL_UINT32(1700, prof.total_time_us);
    TEST_ASSERT_EQUAL_UINT32(1200, prof.max_yield_us);

    /* Test CPU % calculation */
    syn_task_profile_update(&mgr, 10000); /* 10000 us total window */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0, &prof));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 17.0f, prof.cpu_pct);

    /* Null checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_task_profile_get(&mgr, 99, &prof));
}

void test_task_profile_enable_disable(void)
{
    SYN_TaskProfileManager mgr;
    syn_task_profile_init(&mgr);

    syn_task_profile_enable(&mgr, false);
    syn_task_profile_step_start(&mgr, 0, "idle_task", 1000);
    syn_task_profile_step_end(&mgr, 0, 2000);

    SYN_TaskProfile prof;
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_task_profile_get(&mgr, 0, &prof));
}

void test_task_profile_unnamed_and_zero_window(void)
{
    SYN_TaskProfileManager mgr;
    syn_task_profile_init(&mgr);

    /* Null task name defaults to "unnamed", then updates on non-NULL rename, keeps on NULL */
    syn_task_profile_step_start(&mgr, 0, NULL, 500);
    syn_task_profile_step_end(&mgr, 0, 600);

    /* Zero window duration does not perform divide-by-zero */
    syn_task_profile_update(&mgr, 0);

    SYN_TaskProfile prof;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0, &prof));
    TEST_ASSERT_EQUAL_STRING("unnamed", prof.task_name);

    syn_task_profile_step_start(&mgr, 0, "renamed_task", 700);
    syn_task_profile_step_end(&mgr, 0, 800);
    syn_task_profile_step_start(&mgr, 0, NULL, 900);
    syn_task_profile_step_end(&mgr, 0, 950);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0, &prof));
    TEST_ASSERT_EQUAL_STRING("renamed_task", prof.task_name);

    /* Null parameter checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_task_profile_init(NULL));
    syn_task_profile_enable(NULL, true);
    syn_task_profile_update(NULL, 100);
}

static void test_task_profile_disable_clears_active_and_multi_window_clamp(void)
{
    SYN_TaskProfileManager mgr;
    SYN_TaskProfile prof;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_init(&mgr));

    /* Null & bounds guards on step_start / step_end / get */
    syn_task_profile_step_start(NULL, 0U, "t0", 10U);
    syn_task_profile_step_end(NULL, 0U, 20U);
    syn_task_profile_step_start(&mgr, SYN_TASK_PROFILE_MAX_TASKS, "oob", 10U);
    syn_task_profile_step_end(&mgr, SYN_TASK_PROFILE_MAX_TASKS, 20U);
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_task_profile_get(NULL, 0U, &prof));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_task_profile_get(&mgr, 0U, NULL));

    /* Register task 0 mid-step, then disable profiler -> clears step_active = false */
    syn_task_profile_step_start(&mgr, 0U, "t0", 100U);
    syn_task_profile_enable(&mgr, false);
    syn_task_profile_enable(&mgr, true);

    /* Calling step_end while step_active == false returns early without recording a step */
    syn_task_profile_step_end(&mgr, 0U, 200U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0U, &prof));
    TEST_ASSERT_EQUAL_UINT32(0U, prof.step_count);
    TEST_ASSERT_EQUAL_UINT32(0U, prof.total_time_us);

    /* Multi-window rollover + >100% clamp (K-09):
     * Window 1 (0 -> 1000 us): 500 us work -> 50.0% CPU */
    syn_task_profile_step_start(&mgr, 0U, "t0", 100U);
    syn_task_profile_step_end(&mgr, 0U, 600U);
    syn_task_profile_update(&mgr, 1000U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0U, &prof));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, prof.cpu_pct);
    TEST_ASSERT_EQUAL_UINT32(500U, prof.total_time_us);

    /* Window 2 (1000 -> 2000 us): 0 us work -> cpu_pct resets to 0.0% (total_time_us stays 500) */
    syn_task_profile_update(&mgr, 2000U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0U, &prof));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, prof.cpu_pct);
    TEST_ASSERT_EQUAL_UINT32(500U, prof.total_time_us);

    /* Window 3 (2000 -> 3000 us): 1300 us accumulated in 1000 us window -> clamps to 100.0% */
    syn_task_profile_step_start(&mgr, 0U, "t0", 2000U);
    syn_task_profile_step_end(&mgr, 0U, 3300U);
    syn_task_profile_update(&mgr, 3000U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0U, &prof));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, prof.cpu_pct);
    TEST_ASSERT_EQUAL_UINT32(1800U, prof.total_time_us);
}

static void test_task_profile_sched_hook_and_budget_pass(void)
{
    SYN_TaskProfileManager mgr;
    SYN_Task tasks[3];
    SYN_Sched sched;
    ProfileTaskCtx ctx0 = {.run_us = 250U, .max_runs = 2U, .runs = 0U, .detach_sched = NULL};
    ProfileTaskCtx ctx1 = {.run_us = 800U, .max_runs = 2U, .runs = 0U, .detach_sched = NULL};
    ProfileTaskCtx ctx2 = {.run_us = 400U, .max_runs = 1U, .runs = 0U, .detach_sched = NULL};

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_init(&mgr));
    syn_sched_set_profiler(NULL, &mgr);

    syn_task_create(&tasks[0], "crit_ctrl", profile_timed_task, SYN_PRIO_CRITICAL, &ctx0);
    syn_task_create(&tasks[1], "high_io", profile_timed_task, SYN_PRIO_HIGH, &ctx1);
    syn_task_create(&tasks[2], "low_bg", profile_timed_task, SYN_PRIO_LOW, &ctx2);
    syn_sched_init(&sched, tasks, 3U);
    syn_sched_set_profiler(&sched, &mgr);

    for (uint32_t step = 0U; step < 16U; step++) {
        (void)syn_sched_run(&sched);
        if (tasks[0].state == (uint8_t)SYN_TASK_DEAD && tasks[1].state == (uint8_t)SYN_TASK_DEAD &&
            tasks[2].state == (uint8_t)SYN_TASK_DEAD) {
            break;
        }
    }

    size_t violations = 99U;
    SYN_TaskBudgetViolation worst;
    memset(&worst, 0xAA, sizeof(worst));
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_task_profile_check_budgets(&mgr, &sched, &violations, &worst));
    TEST_ASSERT_EQUAL_size_t(0U, violations);
    TEST_ASSERT_NULL(worst.task_name);
    TEST_ASSERT_EQUAL_UINT32(0U, worst.max_yield_us);

    /* Reentrant mid-dispatch profiler detach safely skips step_end */
    ctx0.runs = 0U;
    ctx0.max_runs = 1U;
    ctx0.detach_sched = &sched;
    syn_task_create(&tasks[0], "detach_t", profile_timed_task, SYN_PRIO_CRITICAL, &ctx0);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_init(&mgr));
    syn_sched_set_profiler(&sched, &mgr);
    (void)syn_sched_run(&sched);
    SYN_TaskProfile prof;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0U, &prof));
    TEST_ASSERT_EQUAL_UINT32(0U, prof.step_count);
}

static void test_task_profile_budget_violations_and_edge_cases(void)
{
    SYN_TaskProfileManager mgr;
    SYN_Task tasks[5];
    SYN_Sched sched;
    ProfileTaskCtx c_crit = {.run_us = 200U, .max_runs = 1U, .runs = 0U, .detach_sched = NULL};
    ProfileTaskCtx c_high = {.run_us = 1800U, .max_runs = 1U, .runs = 0U, .detach_sched = NULL};
    ProfileTaskCtx c_low = {.run_us = 9500U, .max_runs = 1U, .runs = 0U, .detach_sched = NULL};
    ProfileTaskCtx c_susp = {.run_us = 50U, .max_runs = 1U, .runs = 0U, .detach_sched = NULL};

    memset(tasks, 0, sizeof(tasks));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_init(&mgr));

    syn_task_create(&tasks[0], "crit", profile_timed_task, SYN_PRIO_CRITICAL, &c_crit);
    syn_task_create(&tasks[1], "high_over", profile_timed_task, SYN_PRIO_HIGH, &c_high);
    syn_task_create(&tasks[2], "low_worst", profile_timed_task, SYN_PRIO_LOW, &c_low);
    syn_task_create(&tasks[3], "suspended", profile_timed_task, SYN_PRIO_CRITICAL, &c_susp);
    syn_task_suspend(&tasks[3]);
    /* tasks[4] intentionally left with func == NULL (marked DEAD so scheduler skips it) */
    tasks[4].state = (uint8_t)SYN_TASK_DEAD;

    syn_sched_init(&sched, tasks, 5U);
    syn_sched_set_profiler(&sched, &mgr);

    for (uint32_t step = 0U; step < 16U; step++) {
        (void)syn_sched_run(&sched);
        if (tasks[0].state == (uint8_t)SYN_TASK_DEAD && tasks[1].state == (uint8_t)SYN_TASK_DEAD &&
            tasks[2].state == (uint8_t)SYN_TASK_DEAD) {
            break;
        }
    }

    /* Both high_over (1800 us > 1000 us) and low_worst (9500 us > 1000 us) violate crit's 1 ms
     * budget; low_worst has larger overrun (8500 us vs 800 us). */
    size_t violations = 0U;
    SYN_TaskBudgetViolation worst;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_task_profile_check_budgets(&mgr, &sched, &violations, &worst));
    TEST_ASSERT_EQUAL_size_t(2U, violations);
    TEST_ASSERT_EQUAL_size_t(2U, worst.task_idx);
    TEST_ASSERT_EQUAL_STRING("low_worst", worst.task_name);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_LOW, worst.task_priority);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_CRITICAL, worst.peer_priority);
    TEST_ASSERT_EQUAL_UINT32(9500U, worst.max_yield_us);
    TEST_ASSERT_EQUAL_UINT32(1000U, worst.budget_us);

    /* Also exercise NULL worst_out, tie/smaller overrun after larger overrun, and bounds guards */
    syn_task_profile_step_start(&mgr, 3U, "late_mild", 0U);
    syn_task_profile_step_end(&mgr, 3U, 1200U); /* overrun 200 < 8500 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_check_budgets(&mgr, &sched, &violations, NULL));
    TEST_ASSERT_EQUAL_size_t(3U, violations);
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_task_profile_check_budgets(&mgr, &sched, &violations, &worst));
    TEST_ASSERT_EQUAL_size_t(2U, worst.task_idx);

    /* Parameter guards and uncontended / dead-peer / equal-wait tie-break branches */
    uint8_t peer_prio = 0U;
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX,
                             syn_task_profile_task_budget_us(&mgr, NULL, 0U, &peer_prio));
    TEST_ASSERT_EQUAL_UINT8(0xFFU, peer_prio);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_task_profile_task_budget_us(&mgr, &sched, 99U, NULL));

    SYN_Sched empty_sched;
    memset(&empty_sched, 0, sizeof(empty_sched));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX,
                             syn_task_profile_task_budget_us(&mgr, &empty_sched, 0U, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_task_profile_check_budgets(NULL, &sched, &violations, &worst));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_task_profile_check_budgets(&mgr, NULL, &violations, &worst));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_task_profile_check_budgets(&mgr, &empty_sched, &violations, &worst));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM,
                          syn_task_profile_check_budgets(&mgr, &sched, NULL, &worst));

    /* Equal wait_us tie-break selects higher priority (smaller numeric prio) + out-of-range clamp
     */
    SYN_Task tie_tasks[3];
    SYN_Sched tie_sched;
    syn_task_create(&tie_tasks[0], "worker", profile_timed_task, SYN_PRIO_LOW, &c_low);
    syn_task_create(&tie_tasks[1], "p_norm", profile_timed_task, SYN_PRIO_NORMAL, &c_crit);
    syn_task_create(&tie_tasks[2], "p_high", profile_timed_task, SYN_PRIO_HIGH, &c_crit);
    syn_sched_init(&tie_sched, tie_tasks, 3U);
    syn_sched_set_max_wait(&tie_sched, SYN_PRIO_HIGH, 10U);
    syn_sched_set_max_wait(&tie_sched, SYN_PRIO_NORMAL, 10U);
    TEST_ASSERT_EQUAL_UINT32(10000U,
                             syn_task_profile_task_budget_us(NULL, &tie_sched, 0U, &peer_prio));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_PRIO_HIGH, peer_prio);

    /* Out-of-range corrupted peer priority clamps cleanly; dead unprofiled peer is ignored */
    tie_tasks[1].state = (uint8_t)SYN_TASK_DEAD;
    tie_tasks[2].priority = 200U;
    TEST_ASSERT_EQUAL_UINT32(50000U,
                             syn_task_profile_task_budget_us(NULL, &tie_sched, 0U, &peer_prio));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)(SYN_SCHED_PRIO_LEVELS - 1U), peer_prio);

    /* Single uncontended task has UINT32_MAX budget and 0 violations */
    tie_tasks[2].state = (uint8_t)SYN_TASK_DEAD;
    SYN_TaskProfileManager solo_mgr;
    syn_task_profile_init(&solo_mgr);
    syn_task_profile_step_start(&solo_mgr, 0U, "worker", 0U);
    syn_task_profile_step_end(&solo_mgr, 0U, 999999U);
    solo_mgr.count = SYN_TASK_PROFILE_MAX_TASKS;
    TEST_ASSERT_EQUAL_INT(SYN_OK,
                          syn_task_profile_check_budgets(&solo_mgr, &tie_sched, &violations, NULL));
    TEST_ASSERT_EQUAL_size_t(0U, violations);
}

void run_task_profile_tests(void)
{
    RUN_TEST(test_task_profile_init_and_step);
    RUN_TEST(test_task_profile_enable_disable);
    RUN_TEST(test_task_profile_unnamed_and_zero_window);
    RUN_TEST(test_task_profile_disable_clears_active_and_multi_window_clamp);
    RUN_TEST(test_task_profile_sched_hook_and_budget_pass);
    RUN_TEST(test_task_profile_budget_violations_and_edge_cases);
}
