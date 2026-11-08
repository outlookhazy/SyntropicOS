/**
 * @file test_tickless.c
 * @brief Unity tests for tickless idle — syn_sched_next_wakeup().
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <limits.h>
#include <setjmp.h>
#include <string.h>

/* ── Dummy task functions ───────────────────────────────────────────────── */

static SYN_PT_Status dummy_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    PT_CONTINUE(pt);
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
    TEST_ASSERT_EQUAL(100, wake); /* returns 'now' */
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
    TEST_ASSERT_EQUAL(200, wake); /* minimum future deadline */
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
    tasks[0].delay_until = 200; /* should be ignored */
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
    tasks[0].delay_until = 0; /* ready but suspended */
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

    tasks[0].delay_until = 300; /* already expired at tick 500 */
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
    syn_sleep_lock(&sleep, 1); /* hold a wakelock (bit 0) */

    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL(500, wake);

    /* Simulate run_tickless: wakelock held → skip sleep */
    if (wake != mock_tick_ms && !syn_sleep_any_locked(&sleep)) {
        syn_port_sleep_until(wake);
    }

    TEST_ASSERT_EQUAL(0, mock_sleep_until_count); /* did NOT sleep */
    TEST_ASSERT_EQUAL(0, mock_sleep_count);

    syn_sleep_unlock(&sleep, 1);
}

/* ── NORETURN loop coverage via longjmp ─────────────────────────────────── */

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
    return PT_MORE;
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
    mock_sleep_until_critical_depth = -1;
    mock_critical_depth = 0;

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

        /* TOCTOU regression: sleep entry must happen with interrupts
         * masked (inside the critical section), otherwise an ISR firing
         * between next_wakeup() and sleep_until() is slept through. */
        TEST_ASSERT_EQUAL(1, mock_sleep_until_critical_depth);

        /* The longjmp escapes from the task function, which runs outside
         * the critical section — depth must be balanced back to 0. */
        TEST_ASSERT_EQUAL(0, mock_critical_depth);
    }
}

static SYN_PT_Status task_delay_1ms_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_TASK_DELAY_MS(pt, task, 1U);
    PT_END(pt);
}

/** K-05: UINT32_MAX sentinel non-aliasing in syn_sched_next_wakeup(). */
static void test_next_wakeup_uint32_max_non_aliasing(void)
{
    SYN_Task tasks[1];
    SYN_Sched sched;
    memset(tasks, 0, sizeof(tasks));

    /* mock_tick_ms = 0xFFFFFFFE, 1 ms delay -> delay_until == UINT32_MAX clamped to 0xFFFFFFFE */
    mock_tick_ms = 0xFFFFFFFEU;
    syn_task_create(&tasks[0], "d1", task_delay_1ms_fn, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, tasks[0].delay_until);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFEU, syn_sched_next_wakeup(&sched));

    /* mock_tick_ms == UINT32_MAX with ready task returns now (UINT32_MAX) */
    mock_tick_ms = UINT32_MAX;
    tasks[0].delay_until = 0U;
    tasks[0].state = (uint8_t)SYN_TASK_READY;
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_sched_next_wakeup(&sched));

    /* Idle scheduler at non-UINT32_MAX tick returns UINT32_MAX only when nothing is pending */
    mock_tick_ms = 500U;
    tasks[0].state = (uint8_t)SYN_TASK_DEAD;
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, syn_sched_next_wakeup(&sched));
}

static int g_hint_sleep_before_second;

static SYN_PT_Status task_hint_then_sleep(SYN_PT *pt, SYN_Task *task)
{
    (void)pt;
    g_tickless_call_count++;
    if (g_tickless_call_count == 1) {
        /* Leave delay_until == 0 so active-work hint takes continue branch */
        return PT_MORE;
    }
    if (g_tickless_call_count == 2) {
        g_hint_sleep_before_second = mock_sleep_until_count + mock_sleep_count;
        task->delay_until = mock_tick_ms + 250U;
        return PT_MORE;
    }
    longjmp(g_tickless_jmp, 1);
    return PT_ENDED;
}

/** Active-work hint continue branch in syn_sched_run_tickless(). */
static void test_run_tickless_active_hint_continue(void)
{
    static SYN_Task tasks[1];
    static SYN_Sched sched;
    static SYN_Sleep sleep;

    memset(tasks, 0, sizeof(tasks));
    mock_tick_ms = 100U;
    g_tickless_call_count = 0;
    g_hint_sleep_before_second = -1;
    mock_sleep_until_count = 0;
    mock_sleep_count = 0;
    mock_critical_depth = 0;

    syn_task_create(&tasks[0], "hint", task_hint_then_sleep, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    if (setjmp(g_tickless_jmp) == 0) {
        syn_sched_run_tickless(&sched, &sleep);
        TEST_FAIL_MESSAGE("syn_sched_run_tickless should not return normally");
    } else {
        TEST_ASSERT_EQUAL_INT(3, g_tickless_call_count);
        TEST_ASSERT_EQUAL_INT(0, g_hint_sleep_before_second);
        TEST_ASSERT_EQUAL_INT(1, mock_sleep_until_count);
        TEST_ASSERT_EQUAL_UINT32(350U, mock_sleep_until_tick);
        TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    }
}

static SYN_EventFlags *g_async_wake_evt = NULL;
static const SYN_Sleep *g_async_wake_sleep = NULL;

static void tickless_async_wake_cb(void)
{
    if (g_async_wake_evt != NULL && g_async_wake_sleep != NULL &&
        (g_async_wake_sleep->enter_count + g_async_wake_sleep->veto_count) >= 1U) {
        g_async_wake_evt->flags = 0x01U;
    }
}

static SYN_PT_Status task_evt_longjmp(SYN_PT *pt, SYN_Task *task)
{
    SYN_EventFlags *evt = (SYN_EventFlags *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, evt, 0x01U);
    longjmp(g_tickless_jmp, 1);
    PT_END(pt);
}

static void run_tickless_with_async_wake(SYN_Sched *sched, SYN_Sleep *sleep, SYN_EventFlags *evt,
                                         bool use_ex, SYN_Timer *timers, size_t timer_count)
{
    mock_critical_depth = 0;
    mock_critical_enter_count = 0;
    g_async_wake_evt = evt;
    g_async_wake_sleep = sleep;
    mock_critical_enter_cb = tickless_async_wake_cb;

    if (setjmp(g_tickless_jmp) == 0) {
        if (use_ex) {
            syn_sched_run_tickless_ex(sched, sleep, timers, timer_count);
        } else {
            syn_sched_run_tickless(sched, sleep);
        }
    }

    mock_critical_enter_cb = NULL;
    g_async_wake_evt = NULL;
    g_async_wake_sleep = NULL;
}

/** Cover wake == UINT32_MAX and all sleep veto conditions (including R3-K05). */
static void test_run_tickless_no_deadlines_and_veto(void)
{
    static SYN_Task tasks[2];
    static SYN_Sched sched;
    static SYN_Sleep sleep;
    static SYN_EventFlags evt;

    memset(tasks, 0, sizeof(tasks));
    /* 1. wake == UINT32_MAX: blocked without timeout + dead task -> syn_port_sleep() */
    mock_tick_ms = 100U;
    mock_sleep_count = 0;
    mock_sleep_until_count = 0;
    mock_sleep_critical_depth = -1;
    syn_event_flags_init(&evt);
    syn_task_create(&tasks[0], "evt_wake", task_evt_longjmp, SYN_PRIO_CRITICAL, &evt);
    syn_task_create(&tasks[1], "dead", dummy_task, SYN_PRIO_LOW, NULL);
    tasks[1].state = (uint8_t)SYN_TASK_DEAD;
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    run_tickless_with_async_wake(&sched, &sleep, &evt, false, NULL, 0U);
    TEST_ASSERT_EQUAL_INT(1, mock_sleep_count);
    TEST_ASSERT_EQUAL_UINT32(1U, sleep.enter_count);
    TEST_ASSERT_EQUAL_UINT32(0U, sleep.veto_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
    TEST_ASSERT_EQUAL_INT(1, mock_sleep_critical_depth);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* 2. R3-K05: wake == UINT32_MAX with SYN_SLEEP_NONE increments veto_count, not enter_count */
    mock_tick_ms = 100U;
    mock_sleep_count = 0;
    mock_sleep_until_count = 0;
    syn_event_flags_init(&evt);
    syn_task_create(&tasks[0], "evt_wake", task_evt_longjmp, SYN_PRIO_CRITICAL, &evt);
    tasks[1].state = (uint8_t)SYN_TASK_DEAD;
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep, SYN_SLEEP_NONE);

    run_tickless_with_async_wake(&sched, &sleep, &evt, false, NULL, 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, sleep.veto_count);
    TEST_ASSERT_EQUAL_UINT32(0U, sleep.enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* 3. Sleep veto: sleep.enabled == false */
    mock_tick_ms = 100U;
    mock_sleep_count = 0;
    mock_sleep_until_count = 0;
    syn_event_flags_init(&evt);
    syn_task_create(&tasks[0], "delayed", dummy_task, SYN_PRIO_LOW, NULL);
    tasks[0].delay_until = 500U;
    syn_task_create(&tasks[1], "evt_wake", task_evt_longjmp, SYN_PRIO_CRITICAL, &evt);
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_sleep_enable(&sleep, false);

    run_tickless_with_async_wake(&sched, &sleep, &evt, false, NULL, 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, sleep.veto_count);
    TEST_ASSERT_EQUAL_UINT32(0U, sleep.enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* 4. Sleep veto: sleep.max_depth == SYN_SLEEP_NONE with finite wake */
    syn_event_flags_init(&evt);
    syn_task_create(&tasks[1], "evt_wake", task_evt_longjmp, SYN_PRIO_CRITICAL, &evt);
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep, SYN_SLEEP_NONE);

    run_tickless_with_async_wake(&sched, &sleep, &evt, false, NULL, 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, sleep.veto_count);
    TEST_ASSERT_EQUAL_UINT32(0U, sleep.enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* 5. Sleep veto: wake lock held via syn_sleep_lock() */
    syn_event_flags_init(&evt);
    syn_task_create(&tasks[1], "evt_wake", task_evt_longjmp, SYN_PRIO_CRITICAL, &evt);
    syn_sched_init(&sched, tasks, 2);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_sleep_lock(&sleep, SYN_SLEEP_LOCK_UART);

    run_tickless_with_async_wake(&sched, &sleep, &evt, false, NULL, 0U);
    TEST_ASSERT_EQUAL_UINT32(1U, sleep.veto_count);
    TEST_ASSERT_EQUAL_UINT32(0U, sleep.enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    syn_sleep_unlock(&sleep, SYN_SLEEP_LOCK_UART);
}

static int g_timer_cb_count;

static void timer_longjmp_cb(SYN_Timer *timer, void *ctx)
{
    (void)timer;
    (void)ctx;
    g_timer_cb_count++;
    longjmp(g_tickless_jmp, 1);
}

static void timer_advance_20ms_cb(SYN_Timer *timer, void *ctx)
{
    (void)timer;
    (void)ctx;
    mock_tick_ms += 20U;
}

static SYN_PT_Status task_ex_hint_then_delay(SYN_PT *pt, SYN_Task *task)
{
    (void)pt;
    g_tickless_call_count++;
    if (g_tickless_call_count == 1) {
        return PT_MORE;
    }
    task->delay_until = 500U;
    return PT_MORE;
}

/** Cover syn_sched_run_tickless_ex() timer/task wakeup arbitration and ready_now race. */
static void test_run_tickless_ex_timers_and_tasks(void)
{
    static SYN_Task tasks[1];
    static SYN_Sched sched;
    static SYN_Sleep sleep;
    static SYN_Timer timers[3];

    memset(tasks, 0, sizeof(tasks));
    /* 1. Active hint continue + inactive timer skip + timer_wake < task_wake */
    mock_tick_ms = 100U;
    g_tickless_call_count = 0;
    g_timer_cb_count = 0;
    mock_sleep_until_count = 0;
    mock_sleep_count = 0;
    mock_critical_depth = 0;

    syn_task_create(&tasks[0], "ex_hint", task_ex_hint_then_delay, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    syn_timer_init(&timers[0], 100U, false, NULL, NULL); /* stays inactive */
    syn_timer_init(&timers[1], 150U, false, timer_longjmp_cb, NULL);
    syn_timer_init(&timers[2], 300U, false, NULL, NULL);
    syn_timer_start(&timers[1]); /* target = 250 */
    syn_timer_start(&timers[2]); /* target = 400 */

    if (setjmp(g_tickless_jmp) == 0) {
        syn_sched_run_tickless_ex(&sched, &sleep, timers, 3U);
        TEST_FAIL_MESSAGE("syn_sched_run_tickless_ex should not return normally");
    } else {
        TEST_ASSERT_EQUAL_INT(2, g_tickless_call_count);
        TEST_ASSERT_EQUAL_INT(1, g_timer_cb_count);
        TEST_ASSERT_EQUAL_INT(1, mock_sleep_until_count);
        TEST_ASSERT_EQUAL_UINT32(250U, mock_sleep_until_tick);
        TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    }

    /* 2. Future task earlier than future timer (task_wake < timer_wake) */
    mock_tick_ms = 100U;
    g_tickless_call_count = 0;
    g_timer_cb_count = 0;
    mock_sleep_until_count = 0;
    mock_critical_depth = 0;

    syn_task_create(&tasks[0], "task_first", task_tickless_longjmp, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_timer_init(&timers[0], 900U, false, timer_longjmp_cb, NULL);
    syn_timer_start(&timers[0]); /* target = 1000 > task 600 */

    if (setjmp(g_tickless_jmp) == 0) {
        syn_sched_run_tickless_ex(&sched, &sleep, timers, 1U);
        TEST_FAIL_MESSAGE("syn_sched_run_tickless_ex should not return normally");
    } else {
        TEST_ASSERT_EQUAL_INT(0, g_timer_cb_count);
        TEST_ASSERT_EQUAL_INT(1, mock_sleep_until_count);
        TEST_ASSERT_EQUAL_UINT32(600U, mock_sleep_until_tick);
        TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    }

    /* 3. Timer only (task_wake == UINT32_MAX, timer_wake != UINT32_MAX) */
    mock_tick_ms = 100U;
    g_timer_cb_count = 0;
    mock_sleep_until_count = 0;
    mock_critical_depth = 0;

    syn_task_create(&tasks[0], "dead_t", dummy_task, SYN_PRIO_CRITICAL, NULL);
    tasks[0].state = (uint8_t)SYN_TASK_DEAD;
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_timer_init(&timers[0], 180U, false, timer_longjmp_cb, NULL);
    syn_timer_start(&timers[0]); /* target = 280 */

    if (setjmp(g_tickless_jmp) == 0) {
        syn_sched_run_tickless_ex(&sched, &sleep, timers, 1U);
        TEST_FAIL_MESSAGE("syn_sched_run_tickless_ex should not return normally");
    } else {
        TEST_ASSERT_EQUAL_INT(1, g_timer_cb_count);
        TEST_ASSERT_EQUAL_INT(1, mock_sleep_until_count);
        TEST_ASSERT_EQUAL_UINT32(280U, mock_sleep_until_tick);
        TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    }

    /* 4. Already-expired timer in sched_timer_wakeup_at -> ready_now == true (no sleep) */
    mock_tick_ms = 100U;
    g_timer_cb_count = 0;
    mock_sleep_until_count = 0;
    mock_sleep_count = 0;
    mock_critical_depth = 0;

    syn_task_create(&tasks[0], "delayed_t", dummy_task, SYN_PRIO_CRITICAL, NULL);
    tasks[0].delay_until = 500U;
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);

    syn_timer_init(&timers[0], 10U, false, timer_longjmp_cb, NULL);
    syn_timer_start(&timers[0]); /* target = 110 */
    syn_timer_init(&timers[1], 1U, false, timer_advance_20ms_cb, NULL);
    timers[1].target_tick = 100U;
    timers[1].active = true;

    if (setjmp(g_tickless_jmp) == 0) {
        syn_sched_run_tickless_ex(&sched, &sleep, timers, 2U);
        TEST_FAIL_MESSAGE("syn_sched_run_tickless_ex should not return normally");
    } else {
        TEST_ASSERT_EQUAL_INT(1, g_timer_cb_count);
        TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
        TEST_ASSERT_EQUAL_INT(0, mock_sleep_count);
        TEST_ASSERT_EQUAL_UINT32(0U, sleep.enter_count);
        TEST_ASSERT_EQUAL_UINT32(0U, sleep.veto_count);
        TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
    }

    /* 5. No active timers + blocked task -> wake == UINT32_MAX -> syn_port_sleep() */
    static SYN_EventFlags evt;
    mock_tick_ms = 100U;
    mock_sleep_count = 0;
    mock_sleep_until_count = 0;
    syn_event_flags_init(&evt);
    syn_task_create(&tasks[0], "evt_ex", task_evt_longjmp, SYN_PRIO_CRITICAL, &evt);
    syn_sched_init(&sched, tasks, 1);
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_timer_init(&timers[0], 100U, false, NULL, NULL);

    run_tickless_with_async_wake(&sched, &sleep, &evt, true, timers, 1U);
    TEST_ASSERT_EQUAL_INT(1, mock_sleep_count);
    TEST_ASSERT_EQUAL_UINT32(1U, sleep.enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_sleep_until_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* 6. NULL timers pointer with non-zero timer_count is safely ignored */
    mock_sleep_count = 0;
    syn_sleep_init(&sleep, SYN_SLEEP_LIGHT);
    syn_event_flags_init(&evt);
    syn_task_restart(&tasks[0]);
    run_tickless_with_async_wake(&sched, &sleep, &evt, true, NULL, 2U);
    TEST_ASSERT_EQUAL_INT(1, mock_sleep_count);
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
    RUN_TEST(test_next_wakeup_uint32_max_non_aliasing);

    /* Sleep decision logic (run_tickless body) */
    RUN_TEST(test_tickless_sleeps_until_deadline);
    RUN_TEST(test_tickless_light_sleep_no_deadlines);
    RUN_TEST(test_tickless_wakelock_prevents_sleep);

    /* NORETURN loop coverage */
    RUN_TEST(test_run_tickless_longjmp);
    RUN_TEST(test_run_tickless_active_hint_continue);
    RUN_TEST(test_run_tickless_no_deadlines_and_veto);
    RUN_TEST(test_run_tickless_ex_timers_and_tasks);
}
