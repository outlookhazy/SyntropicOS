/**
 * @file test_task_profile.c
 * @brief Unit test suite for Opt-In Task Execution Profiler (syn_task_profile).
 */

#include "syntropic/debug/syn_task_profile.h"
#include "unity/unity.h"

#include <string.h>

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

    /* Null task name defaults to "unnamed" */
    syn_task_profile_step_start(&mgr, 0, NULL, 500);
    syn_task_profile_step_end(&mgr, 0, 600);

    /* Zero window duration does not perform divide-by-zero */
    syn_task_profile_update(&mgr, 0);

    SYN_TaskProfile prof;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_task_profile_get(&mgr, 0, &prof));
    TEST_ASSERT_EQUAL_STRING("unnamed", prof.task_name);

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

void run_task_profile_tests(void)
{
    RUN_TEST(test_task_profile_init_and_step);
    RUN_TEST(test_task_profile_enable_disable);
    RUN_TEST(test_task_profile_unnamed_and_zero_window);
    RUN_TEST(test_task_profile_disable_clears_active_and_multi_window_clamp);
}
