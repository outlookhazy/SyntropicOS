/**
 * @file test_task_profile.c
 * @brief Unit test suite for Opt-In Task Execution Profiler (syn_task_profile).
 */

#include "unity/unity.h"
#include "syntropic/debug/syn_task_profile.h"
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

void run_task_profile_tests(void)
{
    RUN_TEST(test_task_profile_init_and_step);
    RUN_TEST(test_task_profile_enable_disable);
}
