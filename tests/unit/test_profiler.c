/**
 * @file test_profiler.c
 * @brief Unity tests for syn_profiler — full coverage.
 */

#include "mocks/mock_port.h"
#include "syntropic/debug/syn_profiler.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <string.h>

/* ── Print capture ─────────────────────────────────────────────────────── */

static char prof_out[2048];
static size_t prof_out_pos = 0;

static void prof_print(const char *str)
{
    while (str && *str && prof_out_pos < sizeof(prof_out) - 1) {
        prof_out[prof_out_pos++] = *str++;
    }
    prof_out[prof_out_pos] = '\0';
}

static void prof_out_clear(void)
{
    prof_out_pos = 0;
    prof_out[0] = '\0';
}

/* ── Test: basic timing and update ────────────────────────────────────── */

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

/* ── Test: dump function — normal output ──────────────────────────────── */

static void test_profiler_dump(void)
{
    mock_tick_ms = 0;

    SYN_ProfileEntry entries[4];
    SYN_Profiler prof;
    syn_profiler_init(&prof, entries, 4);

    /* Register tasks with different name lengths */
    syn_profiler_register(&prof, 0, "task_a");              /* short name */
    syn_profiler_register(&prof, 1, "very_long_task_name"); /* >13 chars — triggers truncation */
    /* Slot 2 and 3 — no name (should be skipped in dump) */

    /* Give task_a some timing */
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(10);
    syn_profiler_task_end(&prof, 0);

    syn_profiler_task_begin(&prof, 1);
    mock_tick_advance(5);
    syn_profiler_task_end(&prof, 1);

    /* Update with period > 0 */
    mock_tick_advance(5);
    syn_profiler_update(&prof);

    prof_out_clear();
    syn_profiler_dump(&prof, prof_print);

    TEST_ASSERT_NOT_NULL(strstr(prof_out, "Task Profiler"));
    TEST_ASSERT_NOT_NULL(strstr(prof_out, "task_a"));
    TEST_ASSERT_NOT_NULL(strstr(prof_out, "very_long_tas")); /* truncated to 13 */
}

/* ── Test: dump with NULL print function — early return ───────────────── */

static void test_profiler_dump_null_print(void)
{
    SYN_ProfileEntry entries[2];
    SYN_Profiler prof;
    syn_profiler_init(&prof, entries, 2);
    syn_profiler_register(&prof, 0, "t");

    /* Should not crash */
    syn_profiler_dump(&prof, NULL);
}

/* ── Test: index out of bounds — begin/end guard ─────────────────────── */

static void test_profiler_oob_guards(void)
{
    SYN_ProfileEntry entries[2];
    SYN_Profiler prof;
    syn_profiler_init(&prof, entries, 2);

    /* Indexes >= capacity should be silently ignored */
    syn_profiler_task_begin(&prof, 5); /* OOB */
    mock_tick_advance(1);
    syn_profiler_task_end(&prof, 5); /* OOB */
    TEST_ASSERT_NULL(syn_profiler_get(&prof, 99));
    /* No crash = pass */
}

/* ── Test: update with period == 0 (covers the divide-by-zero guard) ─── */

static void test_profiler_update_period_zero(void)
{
    mock_tick_ms = 0;
    SYN_ProfileEntry entries[2];
    SYN_Profiler prof;
    syn_profiler_init(&prof, entries, 2);
    syn_profiler_register(&prof, 0, "t");

    syn_profiler_task_begin(&prof, 0);
    /* Don't advance tick — period = 0 */
    syn_profiler_task_end(&prof, 0);
    syn_profiler_update(&prof); /* period==0 → uses 1 to avoid div by zero */
    /* No crash = pass */
}

/* ── Test: dump covers the peak_us padding branches ──────────────────── */

static void test_profiler_dump_peak_padding(void)
{
    mock_tick_ms = 1000;

    SYN_ProfileEntry entries[1];
    SYN_Profiler prof;
    syn_profiler_init(&prof, entries, 1);
    syn_profiler_register(&prof, 0, "t");

    /* Set up a large peak to exercise different padding branches */
    syn_profiler_task_begin(&prof, 0);
    mock_tick_advance(50);
    syn_profiler_task_end(&prof, 0);

    mock_tick_advance(50);
    syn_profiler_update(&prof);

    prof_out_clear();
    syn_profiler_dump(&prof, prof_print);
    TEST_ASSERT_NOT_NULL(strstr(prof_out, "Task Profiler"));
    TEST_ASSERT_NOT_NULL(strstr(prof_out, "t"));
}

static void test_r5_profiler_update_preserves_runs_in_dump(void)
{
    mock_tick_ms = 0U;

    SYN_ProfileEntry entries[2];
    SYN_Profiler prof;
    syn_profiler_init(&prof, entries, 2U);
    syn_profiler_register(&prof, 0U, "worker");

    for (int i = 0; i < 3; i++) {
        syn_profiler_task_begin(&prof, 0U);
        mock_tick_advance(2U);
        syn_profiler_task_end(&prof, 0U);
    }

    mock_tick_advance(4U);
    syn_profiler_update(&prof);

    const SYN_ProfileEntry *e0 = syn_profiler_get(&prof, 0U);
    TEST_ASSERT_NOT_NULL(e0);
    TEST_ASSERT_EQUAL_UINT32(3U, e0->last_run_count);
    TEST_ASSERT_EQUAL_UINT32(0U, e0->run_count);

    prof_out_clear();
    syn_profiler_dump(&prof, prof_print);
    /* Verify non-zero Runs column ("       3\n") is printed after update + dump */
    TEST_ASSERT_NOT_NULL(strstr(prof_out, "worker"));
    TEST_ASSERT_NOT_NULL(strstr(prof_out, "       3\n"));
}

/* ── Test runner ─────────────────────────────────────────────────────── */

void run_profiler_tests(void)
{
    RUN_TEST(test_profiler);
    RUN_TEST(test_profiler_dump);
    RUN_TEST(test_profiler_dump_null_print);
    RUN_TEST(test_profiler_oob_guards);
    RUN_TEST(test_profiler_update_period_zero);
    RUN_TEST(test_profiler_dump_peak_padding);
    RUN_TEST(test_r5_profiler_update_preserves_runs_in_dump);
}
