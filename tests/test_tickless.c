/**
 * @file test_tickless.c
 * @brief Unity tests for tickless idle — syn_sched_next_wakeup().
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"

#include <limits.h>

/* ── Dummy task functions ───────────────────────────────────────────────── */

static SYN_PT_Status dummy_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    PT_YIELD(pt);
    PT_END(pt);
}

/* ── Tests ──────────────────────────────────────────────────────────────── */

/** No delayed tasks → returns current tick (tasks ready now). */
static void test_next_wakeup_no_delays(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    /* Both tasks have delay_until == 0 (ready now) */
    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(100, wake);  /* returns 'now' */
}

/** One task delayed to tick 500 → returns 500. */
static void test_next_wakeup_one_delayed(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    /* Task 0 ready now, task 1 delayed to 500 */
    tasks[1].delay_until = 500;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    /* Task 0 is ready now, so returns 'now' */
    TEST_ASSERT_EQUAL(100, wake);
}

/** All tasks delayed → returns the minimum delay_until. */
static void test_next_wakeup_all_delayed(void)
{
    SYN_Task tasks[3];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_task_create(&tasks[2], "c", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 3);

    tasks[0].delay_until = 300;
    tasks[1].delay_until = 200;
    tasks[2].delay_until = 500;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(200, wake);  /* minimum future deadline */
}

/** Dead tasks are ignored. */
static void test_next_wakeup_dead_ignored(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    tasks[0].state = (uint8_t)SYN_TASK_DEAD;
    tasks[0].delay_until = 200;  /* should be ignored */
    tasks[1].delay_until = 500;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(500, wake);
}

/** Suspended tasks are ignored. */
static void test_next_wakeup_suspended_ignored(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    tasks[0].state = (uint8_t)SYN_TASK_SUSPENDED;
    tasks[0].delay_until = 0;  /* ready but suspended */
    tasks[1].delay_until = 400;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(400, wake);
}

/** No alive tasks → returns UINT32_MAX. */
static void test_next_wakeup_all_dead(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    tasks[0].state = (uint8_t)SYN_TASK_DEAD;
    tasks[1].state = (uint8_t)SYN_TASK_DEAD;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, wake);
}

/** Expired delays (in the past) count as ready now. */
static void test_next_wakeup_expired_delay(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 500;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    tasks[0].delay_until = 300;  /* already expired at tick 500 */
    tasks[1].delay_until = 800;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    /* Task 0 is expired → ready now → returns 'now' */
    TEST_ASSERT_EQUAL(500, wake);
}

/* ── Sleep decision tests ───────────────────────────────────────────────── */
/*
 * syn_sched_run_tickless() is NORETURN, so we can't call it directly.
 * Instead, we exercise the same decision logic it uses:
 *   1. syn_sched_run() — dispatch ready tasks
 *   2. syn_sched_next_wakeup() — find next deadline
 *   3. If deadline, call syn_port_sleep_until()
 *   4. If no deadline, call syn_sleep_enter()
 *   5. If wakelock held, skip sleep entirely
 */

/** When all tasks are delayed, sleep_until is called with the earliest. */
static void test_tickless_sleeps_until_deadline(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_task_create(&tasks[1], "b", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    /* Run once to move tasks past their first yield */
    syn_sched_run(&sched);

    /* Delay both tasks into the future */
    tasks[0].delay_until = 500;
    tasks[1].delay_until = 300;

    mock_sleep_until_count = 0;
    mock_sleep_until_tick = 0;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(300, wake);

    /* Simulate what run_tickless does: call sleep_until */
    SYN_Sleep sleep;
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    if (wake != mock_tick_ms && !syn_sleep_any_locked(&sleep)) {
        syn_port_sleep_until(wake);
    }

    TEST_ASSERT_EQUAL(1, mock_sleep_until_count);
    TEST_ASSERT_EQUAL(300, mock_sleep_until_tick);
}

/** When no deadlines exist, syn_sleep_enter is used (light sleep). */
static void test_tickless_light_sleep_no_deadlines(void)
{
    SYN_Task tasks[1];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Kill the task so there are no alive tasks */
    tasks[0].state = (uint8_t)SYN_TASK_DEAD;

    mock_sleep_count = 0;

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, wake);

    /* Simulate run_tickless: no deadline → syn_sleep_enter */
    SYN_Sleep sleep;
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    if (wake == UINT32_MAX) {
        syn_sleep_enter(&sleep);
    }

    TEST_ASSERT_EQUAL(1, mock_sleep_count);
}

/** Wakelock prevents sleeping even with deadlines. */
static void test_tickless_wakelock_prevents_sleep(void)
{
    SYN_Task tasks[1];
    SYN_Sched sched;

    mock_tick_ms = 100;
    syn_task_create(&tasks[0], "a", dummy_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    syn_sched_run(&sched);
    tasks[0].delay_until = 500;

    mock_sleep_until_count = 0;
    mock_sleep_count = 0;

    SYN_Sleep sleep;
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_sleep_lock(&sleep, 1);  /* hold a wakelock (bit 0) */

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(500, wake);

    /* Simulate run_tickless: wakelock held → skip sleep */
    if (wake != mock_tick_ms && !syn_sleep_any_locked(&sleep)) {
        syn_port_sleep_until(wake);
    }

    TEST_ASSERT_EQUAL(0, mock_sleep_until_count);  /* did NOT sleep */
    TEST_ASSERT_EQUAL(0, mock_sleep_count);

    syn_sleep_unlock(&sleep, 1);
}

/* ── NORETURN loop coverage via longjmp ─────────────────────────────────── */

#include <setjmp.h>

static jmp_buf g_tickless_jmp;
static int g_tickless_call_count;

/**
 * First call: delay the task so the tickless sleep-decision code executes.
 * Second call: longjmp out to escape the NORETURN loop.
 */
static SYN_PT_Status task_tickless_longjmp(SYN_PT *pt, SYN_Task *task)
{
    (void)pt;
    g_tickless_call_count++;
    if (g_tickless_call_count >= 2) {
        longjmp(g_tickless_jmp, 1);
    }
    /* First call: delay so next_wakeup finds a future deadline */
    task->delay_until = mock_tick_ms + 500;
    return PT_YIELDED;
}

/**
 * Call syn_sched_run_tickless and let one full loop iteration complete
 * (including sleep-decision code) before escaping via longjmp.
 */
static void test_run_tickless_longjmp(void)
{
    SYN_Task tasks[1];
    SYN_Sched sched;
    SYN_Sleep sleep;

    mock_tick_ms = 100;
    g_tickless_call_count = 0;
    mock_sleep_until_count = 0;

    syn_task_create(&tasks[0], "jmp", task_tickless_longjmp, 0, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    if (setjmp(g_tickless_jmp) == 0) {
        syn_sched_run_tickless(&sched, &sleep);
        TEST_FAIL_MESSAGE("syn_sched_run_tickless should not return normally");
    } else {
        /* Verify the sleep-decision code ran: it should have called
         * syn_port_sleep_until with the task's deadline. */
        TEST_ASSERT_GREATER_OR_EQUAL(2, g_tickless_call_count);
        TEST_ASSERT_EQUAL(1, mock_sleep_until_count);
    }
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_tickless_tests(void)
{
    /* syn_sched_next_wakeup() */
    RUN_TEST(test_next_wakeup_no_delays);
    RUN_TEST(test_next_wakeup_one_delayed);
    RUN_TEST(test_next_wakeup_all_delayed);
    RUN_TEST(test_next_wakeup_dead_ignored);
    RUN_TEST(test_next_wakeup_suspended_ignored);
    RUN_TEST(test_next_wakeup_all_dead);
    RUN_TEST(test_next_wakeup_expired_delay);

    /* Sleep decision logic (run_tickless body) */
    RUN_TEST(test_tickless_sleeps_until_deadline);
    RUN_TEST(test_tickless_light_sleep_no_deadlines);
    RUN_TEST(test_tickless_wakelock_prevents_sleep);

    /* NORETURN loop coverage */
    RUN_TEST(test_run_tickless_longjmp);
}
