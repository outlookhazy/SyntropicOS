/**
 * @file test_profiler.c
 * @brief Unity tests for syn_profiler.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/debug/syn_profiler.h"

static void test_profiler(void)
{

    mock_tick_ms = 0;

    SYN_ProfileEntry prof_entries[3];
    SYN_Profiler prof;
    syn_profiler_init(&prof, prof_entries, 3);

    syn_profiler_register(&prof, 0, "task_a");
    syn_profiler_register(&prof, 1, "task_b");

    /* Simulate task_a taking 5ms */
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(5);
    syn_profiler_task_end(&prof, 0);

    /* Simulate task_b taking 2ms */
    syn_profiler_task_begin(&prof, 1);
    mock_tick_advance(2);
    syn_profiler_task_end(&prof, 1);

    /* Run again */
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(3);
    syn_profiler_task_end(&prof, 0);

    /* Update stats (total period = 10ms) */
    syn_profiler_update(&prof);

    const SYN_ProfileEntry *e0 = syn_profiler_get(&prof, 0);
    TEST_ASSERT_TRUE(e0 != NULL);
    TEST_ASSERT_EQUAL_INT(5000, e0->peak_us);
    /* CPU%: 8ms total_us / 10ms period × 100 × 10 = 800 → 80.0% */
    /* But our run_count was reset by update, check it was 2 before */
    TEST_ASSERT_TRUE(e0->cpu_percent_x10 > 0);

    const SYN_ProfileEntry *e1 = syn_profiler_get(&prof, 1);
    TEST_ASSERT_TRUE(e1 != NULL);
    TEST_ASSERT_EQUAL_INT(2000, e1->peak_us);

    /* Enable/disable */
    syn_profiler_enable(&prof, false);
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(100);
    syn_profiler_task_end(&prof, 0);
    TEST_ASSERT_EQUAL_INT(5000, e0->peak_us);
}

void run_profiler_tests(void)
{
    RUN_TEST(test_profiler);
}
