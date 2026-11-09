/**
 * @file test_sched.c
 * @brief Unity tests for syn_sched.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_event.h"
#include "unity/unity.h"

static int sched_order[10];
static int sched_order_idx = 0;

static SYN_PT_Status sched_task_a(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    sched_order[sched_order_idx++] = 1;
    PT_CONTINUE(pt);
    sched_order[sched_order_idx++] = 1;

    PT_END(pt);
}

static SYN_PT_Status sched_task_b(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    sched_order[sched_order_idx++] = 2;
    PT_CONTINUE(pt);
    sched_order[sched_order_idx++] = 2;

    PT_END(pt);
}

static void test_scheduler(void)
{
    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "a", sched_task_a, 0, NULL);
    syn_task_create(&tasks[1], "b", sched_task_b, 0, NULL);
    syn_sched_init(&sched, tasks, 2);

    sched_order_idx = 0;
    memset(sched_order, 0, sizeof(sched_order));

    bool alive;

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive);
    TEST_ASSERT_EQUAL_INT(1, sched_order_idx);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive);
    TEST_ASSERT_EQUAL_INT(2, sched_order_idx);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive);
    TEST_ASSERT_EQUAL_INT(3, sched_order_idx);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive);
    TEST_ASSERT_EQUAL_INT(4, sched_order_idx);
    TEST_ASSERT_EQUAL_INT(1, sched_order[0]);
    TEST_ASSERT_EQUAL_INT(2, sched_order[1]);
    TEST_ASSERT_EQUAL_INT(1, sched_order[2]);
    TEST_ASSERT_EQUAL_INT(2, sched_order[3]);

    alive = syn_sched_run(&sched);
    TEST_ASSERT_FALSE(alive);
    TEST_ASSERT_EQUAL_INT(0, syn_sched_alive_count(&sched));
}

static int suspend_counter = 0;

static SYN_PT_Status suspend_task_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    for (;;) {
        suspend_counter++;
        PT_CONTINUE(pt);
    }

    PT_END(pt);
}

static void test_suspend_resume(void)
{
    SYN_Task tasks[1];
    SYN_Sched sched;
    suspend_counter = 0;

    syn_task_create(&tasks[0], "cnt", suspend_task_func, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, suspend_counter);

    syn_task_suspend(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, suspend_counter);

    syn_task_resume(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, suspend_counter);

    syn_task_restart(&tasks[0]);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, suspend_counter);
}

/** syn_sched_run with 0 tasks — exercises line 68: return false */
static void test_sched_empty(void)
{
    mock_tick_ms = 0;
    SYN_Task tasks[2];
    SYN_Sched sched;
    syn_sched_init(&sched, tasks, 0);
    bool alive = syn_sched_run(&sched);
    TEST_ASSERT_FALSE(alive);
}

/** Task with delay_until in future — exercises line 103: continue (still waiting) */
static void test_sched_delayed_task(void)
{
    mock_tick_ms = 0;
    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "a", sched_task_a, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Set the task delay to 100ms in the future */
    tasks[0].delay_until = mock_tick_ms + 100;

    /* Run before delay expires — task should not execute */
    sched_order_idx = 0;
    syn_sched_run(&sched); /* still waiting — line 103 hit */
    TEST_ASSERT_EQUAL_INT(0, sched_order_idx);

    /* Advance past delay */
    mock_tick_advance(150);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, sched_order_idx);
}

/** syn_sched_alive_count — exercises line 167 */
static void test_sched_alive_count(void)
{
    mock_tick_ms = 0;
    SYN_Task tasks[4];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "a", sched_task_a, 0, NULL);
    syn_task_create(&tasks[1], "b", sched_task_b, 0, NULL);
    syn_sched_init(&sched, tasks, 2);
    TEST_ASSERT_EQUAL_size_t(2, syn_sched_alive_count(&sched));

    /* Run task_a to completion (2 steps + final dead check) */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    syn_sched_run(&sched);

    /* task_a should be DEAD now, task_b still alive */
    size_t alive = syn_sched_alive_count(&sched);
    TEST_ASSERT_EQUAL_size_t(1U, alive);
}

#include <setjmp.h>
static jmp_buf g_sched_jmp;

static SYN_PT_Status task_longjmp(SYN_PT *pt, SYN_Task *task)
{
    (void)pt;
    (void)task;
    longjmp(g_sched_jmp, 1);
    return PT_ENDED;
}

static void test_sched_run_forever(void)
{
    SYN_Sched sched;
    SYN_Task tasks[1];
    syn_task_create(&tasks[0], "jmp", task_longjmp, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    if (setjmp(g_sched_jmp) == 0) {
        syn_sched_run_forever(&sched);
        TEST_FAIL_MESSAGE("syn_sched_run_forever should not return normally");
    } else {
        TEST_ASSERT_TRUE(true);
    }
}

/* ── PT_PASS, PT_CONTINUE, and per-priority round-robin tests ───────────── */

static int run_log[64];
static int run_log_idx;

static void log_reset(void)
{
    run_log_idx = 0;
    memset(run_log, 0, sizeof(run_log));
}

/* Task that logs its ID and passes (PT_IDLE) every call */
static SYN_PT_Status pass_task(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        run_log[run_log_idx++] = id;
        PT_PASS(pt);
    }
    PT_END(pt);
}

/* Task that logs its ID and continues (PT_MORE) every call */
static SYN_PT_Status continue_task(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        run_log[run_log_idx++] = id;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

/**
 * Basic pass: A (CRITICAL, passes) lets B (HIGH, passes) run every round.
 * Expected pattern: A, B, A, B, A, B.
 */
static void test_pass_basic(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", pass_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", pass_task, SYN_PRIO_HIGH, &id_b);
    syn_sched_init(&sched, tasks, 2);

    for (int i = 0; i < 6; i++) {
        syn_sched_run(&sched);
    }

    /* A, B, A, B, A, B */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
    TEST_ASSERT_EQUAL_INT(2, run_log[3]);
    TEST_ASSERT_EQUAL_INT(1, run_log[4]);
    TEST_ASSERT_EQUAL_INT(2, run_log[5]);
}

/**
 * Per-priority round-robin: A (CRITICAL, passes) with B1, B2 (HIGH, continue).
 * While A has passed for the round, B1 and B2 alternate fairly; when 1 ms
 * elapses, A wakes as overdue, polls once, and B1/B2 resume alternating.
 */
static void test_pass_rr_fairness(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b1 = 2, id_b2 = 3;

    syn_task_create(&tasks[0], "a", pass_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b1", continue_task, SYN_PRIO_HIGH, &id_b1);
    syn_task_create(&tasks[2], "b2", continue_task, SYN_PRIO_HIGH, &id_b2);
    syn_sched_init(&sched, tasks, 3);

    /* A passes, then B1 and B2 alternate for 4 dispatches within 0 ms */
    for (int i = 0; i < 5; i++) {
        syn_sched_run(&sched);
    }
    mock_tick_advance(1); /* A (1 ms max wait) becomes overdue */
    for (int i = 0; i < 3; i++) {
        syn_sched_run(&sched);
    }

    /* A, B1, B2, B1, B2, A, B1, B2 */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
    TEST_ASSERT_EQUAL_INT(3, run_log[2]);
    TEST_ASSERT_EQUAL_INT(2, run_log[3]);
    TEST_ASSERT_EQUAL_INT(3, run_log[4]);
    TEST_ASSERT_EQUAL_INT(1, run_log[5]);
    TEST_ASSERT_EQUAL_INT(2, run_log[6]);
    TEST_ASSERT_EQUAL_INT(3, run_log[7]);
}

/**
 * Same-priority round-robin: two CRITICAL tasks that continue alternate A, B, A, B.
 */
static void test_rr_same_priority(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", continue_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", continue_task, SYN_PRIO_CRITICAL, &id_b);
    syn_sched_init(&sched, tasks, 2);

    for (int i = 0; i < 6; i++) {
        syn_sched_run(&sched);
    }

    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
    TEST_ASSERT_EQUAL_INT(2, run_log[3]);
    TEST_ASSERT_EQUAL_INT(1, run_log[4]);
    TEST_ASSERT_EQUAL_INT(2, run_log[5]);
}

/**
 * Strict priority within work window: A (CRITICAL, continues) runs ahead of
 * B (HIGH, continues) while B's starvation deadline has not elapsed.
 */
static void test_strict_priority_within_window(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", continue_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", continue_task, SYN_PRIO_HIGH, &id_b);
    syn_sched_init(&sched, tasks, 2);

    for (int i = 0; i < 4; i++) {
        syn_sched_run(&sched);
    }

    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(1, run_log[1]);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
    TEST_ASSERT_EQUAL_INT(1, run_log[3]);
}

/**
 * Pass with 3 lower-priority workers: A (CRITICAL, passes), B1/B2/B3 (HIGH, continue).
 * All three get fair rotation while A is passed, and resume where RR left off after A polls.
 */
static void test_pass_rr_three_lower(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[4];
    SYN_Sched sched;
    static int id_a = 1, id_b1 = 2, id_b2 = 3, id_b3 = 4;

    syn_task_create(&tasks[0], "a", pass_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b1", continue_task, SYN_PRIO_HIGH, &id_b1);
    syn_task_create(&tasks[2], "b2", continue_task, SYN_PRIO_HIGH, &id_b2);
    syn_task_create(&tasks[3], "b3", continue_task, SYN_PRIO_HIGH, &id_b3);
    syn_sched_init(&sched, tasks, 4);

    for (int i = 0; i < 5; i++) {
        syn_sched_run(&sched);
    }
    mock_tick_advance(1);
    for (int i = 0; i < 3; i++) {
        syn_sched_run(&sched);
    }

    /* A, B1, B2, B3, B1, A, B2, B3 */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
    TEST_ASSERT_EQUAL_INT(3, run_log[2]);
    TEST_ASSERT_EQUAL_INT(4, run_log[3]);
    TEST_ASSERT_EQUAL_INT(2, run_log[4]);
    TEST_ASSERT_EQUAL_INT(1, run_log[5]);
    TEST_ASSERT_EQUAL_INT(3, run_log[6]);
    TEST_ASSERT_EQUAL_INT(4, run_log[7]);
}

/**
 * PASSED round-flag lifecycle: task passes -> stays READY with PASSED flag ->
 * once all pollers pass, next round clears PASSED.
 */
static void test_pass_flag_lifecycle(void)
{
    mock_tick_ms = 0;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", pass_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", pass_task, SYN_PRIO_HIGH, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Dispatch 1: A runs and passes */
    log_reset();
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[1].state);

    /* Dispatch 2: B runs and passes */
    syn_sched_run(&sched);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[1].sched_flags & SYN_SCHED_FLAG_PASSED);

    /* Dispatch 3: new round clears PASSED on all tasks, then A runs and passes */
    syn_sched_run(&sched);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[1].sched_flags & SYN_SCHED_FLAG_PASSED);
}

/**
 * Pass is compatible with suspend: a passed task can be suspended,
 * and when resumed it wakes as READY with MORE work rank.
 */
static void test_pass_then_suspend(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", pass_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", continue_task, SYN_PRIO_HIGH, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* A runs and passes */
    syn_sched_run(&sched);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);

    /* Suspend A while it is passed */
    syn_task_suspend(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_SUSPENDED, tasks[0].state);

    /* B runs while A is suspended */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);

    /* Resume A — comes back READY and preempts B immediately as waking Work */
    syn_task_resume(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
}

/* ── PT_BLOCK_EVENT tests ────────────────────────────────────────────────── */

#define EVT_DATA SYN_BIT(0)
#define EVT_DONE SYN_BIT(1)

static int block_counter;

/* Task that blocks on an event, increments counter when woken */
static SYN_PT_Status block_task_fn(SYN_PT *pt, SYN_Task *task)
{
    SYN_EventGroup *evt = (SYN_EventGroup *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        PT_BLOCK_EVENT(pt, task, evt, EVT_DATA);
        block_counter++;
    }
    PT_END(pt);
}

/**
 * Basic: task blocks, event set, task wakes and runs.
 */
static void test_block_event_basic(void)
{
    mock_tick_ms = 0;
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "blk", block_task_fn, 0, &evt);
    syn_sched_init(&sched, tasks, 1);

    /* Pass 1: task runs, hits PT_BLOCK_EVENT, goes BLOCKED */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);
    TEST_ASSERT_EQUAL_INT(0, block_counter);

    /* Pass 2: no event — task stays blocked */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);
    TEST_ASSERT_EQUAL_INT(0, block_counter);

    /* Set event flag */
    syn_event_set(&evt, EVT_DATA);

    /* Pass 3: scheduler sees event, wakes task, task increments counter */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_counter);

    /* Flag should be auto-cleared */
    TEST_ASSERT_FALSE(syn_event_check_any(&evt, EVT_DATA));
}

/**
 * Blocked task is skipped — lower-priority task runs.
 */
static void test_block_event_skips_scan(void)
{
    mock_tick_ms = 0;
    log_reset();
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_b = 2;

    syn_task_create(&tasks[0], "blk", block_task_fn, 0, &evt);
    syn_task_create(&tasks[1], "low", continue_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Pass 1: high-pri task runs, blocks */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    /* Pass 2: high-pri blocked, low-pri runs */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);    /* Low-pri ran */
    TEST_ASSERT_EQUAL_INT(0, block_counter); /* High-pri didn't */
}

/**
 * Auto-clear: matched event flags are cleared after task resumes.
 */
static void test_block_event_auto_clear(void)
{
    mock_tick_ms = 0;
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "blk", block_task_fn, 0, &evt);
    syn_sched_init(&sched, tasks, 1);

    /* Block the task */
    syn_sched_run(&sched);

    /* Set both DATA and DONE flags */
    syn_event_set(&evt, EVT_DATA | EVT_DONE);

    /* Wake — task waits on EVT_DATA only */
    syn_sched_run(&sched);

    /* EVT_DATA should be cleared (auto-clear), EVT_DONE should remain */
    TEST_ASSERT_FALSE(syn_event_check_any(&evt, EVT_DATA));
    TEST_ASSERT_TRUE(syn_event_check_any(&evt, EVT_DONE));
}

/**
 * Priority interaction: blocked high-pri task wakes and preempts.
 */
static void test_block_event_priority(void)
{
    mock_tick_ms = 0;
    log_reset();
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_b = 2;

    syn_task_create(&tasks[0], "blk", block_task_fn, 0, &evt);
    syn_task_create(&tasks[1], "low", continue_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Block high-pri */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    /* Low-pri runs while high-pri is blocked */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);

    /* Set event — high-pri should wake and run next */
    syn_event_set(&evt, EVT_DATA);
    log_reset();
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_counter); /* High-pri ran */
}

/**
 * Multiple tasks blocking on different events.
 */
static void test_block_event_multiple_tasks(void)
{
    mock_tick_ms = 0;
    SYN_EventGroup evt_a, evt_b;
    syn_event_init(&evt_a);
    syn_event_init(&evt_b);

    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "a", block_task_fn, 0, &evt_a);
    syn_task_create(&tasks[1], "b", block_task_fn, 0, &evt_b);
    syn_sched_init(&sched, tasks, 2);

    block_counter = 0;

    /* Both tasks run once, then block */
    syn_sched_run(&sched); /* Task A blocks */
    syn_sched_run(&sched); /* Task B blocks */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[1].state);

    /* Only fire event A */
    syn_event_set(&evt_a, EVT_DATA);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_counter);                   /* Only A woke */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[1].state); /* B still blocked */

    /* Fire event B */
    syn_event_set(&evt_b, EVT_DATA);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, block_counter); /* Now B woke too */
}

/**
 * Suspend overrides block: suspending a blocked task changes state.
 */
static void test_block_event_suspend(void)
{
    mock_tick_ms = 0;
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "blk", block_task_fn, 0, &evt);
    syn_sched_init(&sched, tasks, 1);

    /* Block the task */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    /* Suspend while blocked */
    syn_task_suspend(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_SUSPENDED, tasks[0].state);

    /* Set event — task should NOT wake (it's suspended) */
    syn_event_set(&evt, EVT_DATA);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(0, block_counter);

    /* Resume — task goes to READY, not BLOCKED */
    syn_task_resume(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
}

/* ── PT_IDLE (PT_WAIT_UNTIL) round-pass tests ────────────────────────────── */

static bool wait_cond_a;

/* Task that waits on a condition, logs ID when condition is true */
static SYN_PT_Status wait_task(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        PT_WAIT_UNTIL(pt, wait_cond_a);
        run_log[run_log_idx++] = id;
        wait_cond_a = false; /* consume the event */
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

/**
 * Waiting high-pri task doesn't starve lower-pri task.
 * A (CRITICAL) waits on false condition (returns PT_IDLE -> PASSED).
 * B (HIGH) runs on subsequent dispatches until A's poll window elapses.
 */
static void test_waiting_doesnt_starve(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "w", wait_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", continue_task, SYN_PRIO_HIGH, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Dispatch 1: A polls wait_cond_a (false) -> returns PT_IDLE (PASSED) */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(0, run_log_idx);

    /* Dispatches 2 & 3: A is PASSED, so B runs continuously */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log_idx);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);

    /* 1 ms elapses and condition becomes true — A wakes as overdue and runs */
    wait_cond_a = true;
    mock_tick_advance(1);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log_idx);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
}

/**
 * PT_PASS + PT_WAIT_UNTIL compose cleanly across priorities.
 * A (CRITICAL, passes) -> B (HIGH, waits false) -> C (NORMAL, continues).
 */
static void test_waiting_with_defer_no_inversion(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2, id_c = 3;

    syn_task_create(&tasks[0], "a", pass_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "w", wait_task, SYN_PRIO_HIGH, &id_b);
    syn_task_create(&tasks[2], "c", continue_task, SYN_PRIO_NORMAL, &id_c);
    syn_sched_init(&sched, tasks, 3);

    /* Dispatch 1: A runs and passes */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);

    /* Dispatch 2: B polls false and passes (no log) */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);

    /* Dispatches 3 & 4: C has MORE work and runs */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log[1]);
    TEST_ASSERT_EQUAL_INT(3, run_log[2]);

    /* 1 ms elapses: A (1 ms window) polls again, then C resumes (B's 5 ms not yet due) */
    mock_tick_advance(1);
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[3]);
    TEST_ASSERT_EQUAL_INT(3, run_log[4]);
}

/**
 * All tasks waiting — round resets cleanly, no hang, all stay READY.
 */
static void test_all_tasks_waiting(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2, id_c = 3;

    syn_task_create(&tasks[0], "a", wait_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", wait_task, SYN_PRIO_HIGH, &id_b);
    syn_task_create(&tasks[2], "c", wait_task, SYN_PRIO_NORMAL, &id_c);
    syn_sched_init(&sched, tasks, 3);

    for (int i = 0; i < 4; i++) {
        bool alive = syn_sched_run(&sched);
        TEST_ASSERT_TRUE(alive);
    }
    TEST_ASSERT_EQUAL_INT(0, run_log_idx);

    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[1].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[2].state);
}

/**
 * PT_IDLE state lifecycle: task stays READY with PASSED flag set.
 */
static void test_waiting_state_lifecycle(void)
{
    mock_tick_ms = 0;
    wait_cond_a = false;

    SYN_Task tasks[1];
    SYN_Sched sched;
    static int id_a = 1;

    syn_task_create(&tasks[0], "w", wait_task, SYN_PRIO_CRITICAL, &id_a);
    syn_sched_init(&sched, tasks, 1);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
}

/**
 * Same-priority: one waits, the other works. Once the poll window elapses
 * and condition clears, A gets its turn.
 */
static void test_waiting_same_priority_fairness(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "w", wait_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "b", continue_task, SYN_PRIO_CRITICAL, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Dispatch 1: A polls false (PASSED). Dispatches 2 & 3: B runs */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);

    /* Condition true + 1 ms window elapsed: A runs */
    wait_cond_a = true;
    mock_tick_advance(1);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
}

/**
 * Delayed CRITICAL + waiting HIGH: no priority inversion.
 * A (CRITICAL) is delayed. B (HIGH) waits false. C (NORMAL) runs.
 * When A's delay expires within B/C windows, A runs immediately.
 */
static void test_waiting_delayed_no_inversion(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2, id_c = 3;

    syn_task_create(&tasks[0], "a", continue_task, SYN_PRIO_CRITICAL, &id_a);
    syn_task_create(&tasks[1], "w", wait_task, SYN_PRIO_HIGH, &id_b);
    syn_task_create(&tasks[2], "c", continue_task, SYN_PRIO_NORMAL, &id_c);
    syn_sched_init(&sched, tasks, 3);

    /* Delay A for 3 ms */
    tasks[0].delay_until = mock_tick_ms + 3;

    /* Dispatch 1: A delayed, B polls false (PASSED). Dispatch 2: C runs */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log[0]);

    /* Advance past A's 3 ms delay (within B's 5 ms and C's 15 ms windows) */
    mock_tick_advance(4);

    /* A wakes as Work at CRITICAL priority -> runs first */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[1]);
}

static void test_task_priority_boost_and_restore(void)
{
    mock_tick_ms = 0;
    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", continue_task, SYN_PRIO_LOW, &id_a);
    syn_task_create(&tasks[1], "b", continue_task, SYN_PRIO_HIGH, &id_b);
    syn_sched_init(&sched, tasks, 2);

    TEST_ASSERT_EQUAL_UINT8(3, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(3, tasks[0].base_priority);
    TEST_ASSERT_EQUAL_UINT8(1, tasks[1].priority);

    log_reset();
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]); /* Task B (prio 1) runs first */

    /* Attempt demotion within valid range (temp_prio 2 > base_priority 1) — rejected */
    syn_task_boost_priority(&tasks[1], SYN_PRIO_NORMAL);
    TEST_ASSERT_EQUAL_UINT8(1, tasks[1].priority);

    /* Boost before first dispatch (DUE_VALID == 0) and on unbound task (sched == NULL) */
    SYN_Task unbound;
    syn_task_create(&unbound, "unbound", continue_task, SYN_PRIO_LOW, &id_a);
    syn_task_boost_priority(&unbound, SYN_PRIO_HIGH);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, unbound.priority);

    /* Boost Task A from prio 3 (due=50) to prio 1 (max_wait=5): tightens due_ms to 5 */
    TEST_ASSERT_EQUAL_UINT32(50U, tasks[0].due_ms);
    syn_task_boost_priority(&tasks[0], SYN_PRIO_HIGH);
    TEST_ASSERT_EQUAL_UINT8(1, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT32(5U, tasks[0].due_ms);

    /* Advance tick to 10 so tasks[0].due_ms (5) is already earlier than boosted_due (10 + 1 = 11)
     */
    mock_tick_ms = 10U;
    syn_task_boost_priority(&tasks[0], SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(0, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT32(5U, tasks[0].due_ms);      /* Earlier deadline preserved */
    TEST_ASSERT_EQUAL_UINT8(3, tasks[0].base_priority); /* Base priority unchanged */
    mock_tick_ms = 0U;

    /* Re-boosting to same active priority 0 is a harmless no-op */
    syn_task_boost_priority(&tasks[0], SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(0, tasks[0].priority);

    log_reset();
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[0]); /* Task A now preempts Task B */

    /* Attempt out-of-range boost (prio 5 >= SYN_SCHED_PRIO_LEVELS) — rejected */
    syn_task_boost_priority(&tasks[0], (SYN_Prio)5);
    TEST_ASSERT_EQUAL_UINT8(0, tasks[0].priority); /* Stays at boosted 0 */

    /* Restore Task A back to base priority 3 */
    syn_task_restore_priority(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(3, tasks[0].priority);

    log_reset();
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]); /* Task B runs again */

    /* Dynamically change base priority */
    syn_task_set_base_priority(&tasks[0], SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(0, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(0, tasks[0].base_priority);
}

/**
 * Two CRITICAL pollers (A1, A2 passing) plus a LOW worker:
 * round-pass semantics prevent ping-pong starvation, so LOW runs on dispatch 3!
 */
static void test_two_passing_high_pri_tasks_do_not_starve_lower(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a1 = 1, id_a2 = 2, id_b = 3;

    syn_task_create(&tasks[0], "a1", pass_task, SYN_PRIO_CRITICAL, &id_a1);
    syn_task_create(&tasks[1], "a2", pass_task, SYN_PRIO_CRITICAL, &id_a2);
    syn_task_create(&tasks[2], "b", continue_task, SYN_PRIO_LOW, &id_b);
    syn_sched_init(&sched, tasks, 3);

    for (int i = 0; i < 5; i++) {
        syn_sched_run(&sched);
    }

    /* A1 passes, A2 passes, then LOW worker B runs without starving! */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
    TEST_ASSERT_EQUAL_INT(3, run_log[2]);
    TEST_ASSERT_EQUAL_INT(3, run_log[3]);
    TEST_ASSERT_EQUAL_INT(3, run_log[4]);
}

static SYN_PT_Status us_delay_task_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    static uint32_t us_target;
    PT_BEGIN(pt);
    run_log[run_log_idx++] = 1;
    PT_DELAY_US(pt, &us_target, 50); /* 50 microsecond delay */
    run_log[run_log_idx++] = 2;
    PT_END(pt);
}

static void test_pt_delay_us(void)
{
    mock_tick_us = 0;
    log_reset();

    SYN_Task tasks[1];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "us_task", us_delay_task_func, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Step 1: Task runs, logs 1, sets us_target = 50us, returns PT_IDLE */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);

    /* Step 2: Advance by 20us (less than 50us) — task returns PT_IDLE */
    mock_tick_advance_us(20);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);

    /* Step 3: Advance by 35us more (total 55us >= 50us) — task completes and logs 2 */
    mock_tick_advance_us(35);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log_idx);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
}

static bool block_cond_flag = false;
static int block_cond_runs = 0;

static SYN_PT_Status task_block_cond_entry(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    block_cond_runs++;
    PT_BLOCK_CONDITION(pt, task, block_cond_flag);
    block_cond_runs++;

    PT_END(pt);
}

static void test_block_condition_and_primitives(void)
{
    SYN_Task task;
    SYN_Sched sched;

    block_cond_flag = false;
    block_cond_runs = 0;

    syn_task_create(&task, "block_cond", task_block_cond_entry, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* Run 1: Task runs up to PT_BLOCK_CONDITION, increments to 1, sets state to BLOCKED */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_cond_runs);
    TEST_ASSERT_EQUAL_INT((uint8_t)SYN_TASK_BLOCKED, task.state);

    /* Run 2: Task is BLOCKED, condition still false — scheduler skips task */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_cond_runs);
    TEST_ASSERT_EQUAL_INT((uint8_t)SYN_TASK_BLOCKED, task.state);

    /* Resume task by setting condition true and unblocking */
    block_cond_flag = true;
    syn_task_resume(&task);
    TEST_ASSERT_EQUAL_INT((uint8_t)SYN_TASK_READY, task.state);

    /* Run 3: Task unblocks, evaluates condition true, and completes */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, block_cond_runs);
    TEST_ASSERT_EQUAL_INT((uint8_t)SYN_TASK_DEAD, task.state);
}

/* ── PT_BLOCK_CONDITION lost-wakeup regression ──────────────────────────── */

static SYN_Task *g_lw_task;
static volatile bool g_lw_cond;
static bool g_lw_fire_isr_once;
static uint8_t g_lw_state_at_eval;
static int g_lw_done;

/**
 * Condition expression with an "ISR" side effect. Records the task state
 * observed at evaluation time, then — one-shot — simulates an interrupt
 * that produces the data and resumes the task, exactly in the window
 * between the condition check and the yield.
 */
static bool lw_cond_eval(void)
{
    bool result = g_lw_cond;

    g_lw_state_at_eval = g_lw_task->state;
    if (g_lw_fire_isr_once) {
        g_lw_fire_isr_once = false;
        g_lw_cond = true;           /* ISR produces the data...       */
        syn_task_resume(g_lw_task); /* ...and wakes the consumer task */
    }
    return result;
}

static SYN_PT_Status lw_task_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_BLOCK_CONDITION(pt, task, lw_cond_eval());
    g_lw_done = 1;
    PT_END(pt);
}

/**
 * Regression: PT_BLOCK_CONDITION must set BLOCKED before evaluating the
 * condition. With the old order (evaluate, then block), an ISR firing in
 * between saw READY, its syn_task_resume() was a no-op, and the task then
 * blocked forever on a condition that was already true (lost wakeup).
 */
static void test_block_condition_isr_wakeup_not_lost(void)
{
    SYN_Task task;
    SYN_Sched sched;

    g_lw_task = &task;
    g_lw_cond = false;
    g_lw_fire_isr_once = true;
    g_lw_state_at_eval = 0xFF;
    g_lw_done = 0;

    syn_task_create(&task, "lost_wake", lw_task_fn, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* Run 1: condition false, "ISR" fires at the evaluation boundary.
     * The task must already be BLOCKED so the resume takes effect. */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_BLOCKED, g_lw_state_at_eval);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_READY, task.state);
    TEST_ASSERT_EQUAL_INT(0, g_lw_done);

    /* Run 2: resumed task re-evaluates, condition now true, completes.
     * Pre-fix this deadlocked: BLOCKED with wait_event == NULL. */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, g_lw_done);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)SYN_TASK_DEAD, task.state);
}

/**
 * Regression: PT_BLOCK_EVENT's auto-clear must go through the
 * critical-section-protected syn_event_flags_clear(), not a raw RMW that
 * an ISR flag-set could interleave with.
 */
static void test_block_event_auto_clear_is_atomic(void)
{
    mock_tick_ms = 0;
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "blk", block_task_fn, 0, &evt);
    syn_sched_init(&sched, tasks, 1);

    /* Block the task, then fire the event plus an unrelated bit */
    syn_sched_run(&sched);
    syn_event_set(&evt, EVT_DATA | SYN_BIT(5));

    int enters_before = mock_critical_enter_count;

    /* Wake — the auto-clear inside PT_BLOCK_EVENT runs on this pass */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_counter);

    /* Clear ran inside a critical section (raw RMW would not enter one) */
    TEST_ASSERT_GREATER_THAN_INT(enters_before, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* Only the waited-on mask was cleared; the unrelated bit survives */
    TEST_ASSERT_FALSE(syn_event_check_any(&evt, EVT_DATA));
    TEST_ASSERT_TRUE(syn_event_check_any(&evt, SYN_BIT(5)));
}

/* ── PT_TASK_DELAY_MS deadline-lifecycle regressions ─────────────────────── */

static int stale_runs;

/* Task that delays once at startup, then yields forever (event-driven
 * long-lived task shape) */
static SYN_PT_Status delay_once_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_TASK_DELAY_MS(pt, task, 10);
    for (;;) {
        stale_runs++;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

/**
 * Regression: a consumed delay deadline must be cleared. A stale
 * delay_until reads as "in the future" again once the tick advances
 * 2^31 ms (~24.8 days) past it, and the scheduler then freezes the task
 * for up to another 24.8 days.
 */
static void test_delay_deadline_consumed_no_wrap_freeze(void)
{
    mock_tick_ms = 1000;
    stale_runs = 0;

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "d", delay_once_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Task starts its 10ms delay */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(0, stale_runs);

    /* Delay expires — task runs and the deadline is consumed */
    mock_tick_advance(20);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, stale_runs);
    TEST_ASSERT_EQUAL_UINT32(0, tasks[0].delay_until);

    /* ~25 days later: pre-fix the stale deadline (1010) looked "future"
     * and the task was silently skipped. It must still be scheduled. */
    mock_tick_advance(0x80000000u);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, stale_runs);
}

/* Task that runs, delays 5ms, runs again — logs each phase */
static SYN_PT_Status wrap_delay_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    run_log[run_log_idx++] = 1;
    PT_TASK_DELAY_MS(pt, task, 5);
    run_log[run_log_idx++] = 2;
    PT_END(pt);
}

/**
 * Regression: a delay whose computed deadline lands exactly on tick 0
 * (49.7-day counter wrap) must not alias the scheduler's "no deadline"
 * sentinel. Pre-fix, delay_until == 0 meant the scheduler polled the
 * task every pass instead of skipping it, and tickless mode saw it as
 * "ready now" (no sleep) for the whole delay window.
 */
static void test_delay_deadline_wrap_zero_alias(void)
{
    mock_tick_ms = UINT32_MAX - 4; /* deadline: (UINT32_MAX - 4) + 5 == 0 */
    log_reset();

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "z", wrap_delay_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Task starts a 5ms delay spanning the wrap — the deadline must be
     * nudged off the sentinel (0 → 1), not disappear */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_UINT32(1, tasks[0].delay_until);

    /* Mid-delay: task is skipped, and the tickless wakeup is the real
     * deadline, not "ready now" */
    mock_tick_advance(2);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);
    TEST_ASSERT_EQUAL_UINT32(1, syn_sched_next_wakeup(&sched));

    /* Past the deadline (tick wrapped to 1): completes */
    mock_tick_advance(4);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
}

static void test_sched_next_wakeup_blocked_event_fired(void)
{
    SYN_Task task;
    SYN_Sched sched;
    syn_task_create(&task, "evt_task", suspend_task_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    SYN_EventFlags flags;
    syn_event_flags_init(&flags);

    /* Set task to BLOCKED state with wait_event and wait_mask */
    task.state = (uint8_t)SYN_TASK_BLOCKED;
    task.wait_event = &flags;
    task.wait_mask = 0x01;
    task.delay_until = syn_port_get_tick_ms() + 1000;

    /* Fire event flags */
    syn_event_flags_set(&flags, 0x01);

    uint32_t now = syn_port_get_tick_ms();
    uint32_t wakeup = syn_sched_next_wakeup(&sched);

    /* Since wait_event fired, next_wakeup returns 'now' (no sleep) */
    TEST_ASSERT_EQUAL_UINT32(now, wakeup);
}

static void test_sched_next_wakeup_uint32_max_and_prio_bounds(void)
{
    SYN_Task task;
    syn_task_create(&task, "dummy", sched_task_a, 0, NULL);
    SYN_Sched sched;
    mock_tick_ms = UINT32_MAX - 500;
    task.delay_until = UINT32_MAX;
    syn_sched_init(&sched, &task, 1);

    /* delay_until == UINT32_MAX target cap branch (line 276) */
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX - 1, syn_sched_next_wakeup(&sched));
    mock_tick_ms = 0;

    /* Round-robin start index clamp branch (line 165) */
    task.delay_until = 0;
    sched.rr_per_prio[0] = 10;
    syn_sched_run(&sched);
}

static SYN_EventFlags test_evt_flags;
static uint32_t test_evt_matched_flags = 0;

static SYN_PT_Status block_evt_timeout_task_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, &test_evt_flags, 0x01, 100, &test_evt_matched_flags);
    PT_END(pt);
}

static void test_block_event_with_timeout_event_fired(void)
{
    SYN_Task task;
    SYN_Sched sched;
    mock_tick_ms = 1000;
    syn_event_flags_init(&test_evt_flags);
    syn_task_create(&task, "evt_to_task", block_evt_timeout_task_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* First pass: enters BLOCKED state with delay_until = 1100 */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);
    TEST_ASSERT_EQUAL_UINT32(1100, task.delay_until);

    /* Fire event at t = 1050 (before timeout 1100) */
    mock_tick_ms = 1050;
    syn_event_flags_set(&test_evt_flags, 0x01);

    /* Second pass: unblocks on event */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(0, task.delay_until);
    TEST_ASSERT_NULL(task.wait_event);
    TEST_ASSERT_EQUAL_UINT32(0x01, test_evt_matched_flags);
    /* Matched flag auto-cleared */
    TEST_ASSERT_EQUAL_UINT32(0, syn_event_flags_get(&test_evt_flags));
}

static void test_block_event_with_timeout_expired(void)
{
    SYN_Task task;
    SYN_Sched sched;
    mock_tick_ms = 1000;
    syn_event_flags_init(&test_evt_flags);
    syn_task_create(&task, "evt_to_task", block_evt_timeout_task_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* First pass: enters BLOCKED state with delay_until = 1100 */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);

    /* Mid-timeout pass (t = 1050 < 1100): stays BLOCKED */
    mock_tick_ms = 1050;
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);

    /* Advance time past timeout without firing event */
    mock_tick_ms = 1105;

    /* Second pass: unblocks on timeout */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(0, task.delay_until);
    TEST_ASSERT_NULL(task.wait_event);
    TEST_ASSERT_EQUAL_UINT32(0, test_evt_matched_flags);
}

static void test_sched_next_wakeup_blocked_timeout(void)
{
    SYN_Task task;
    SYN_Sched sched;
    mock_tick_ms = 1000;
    syn_event_flags_init(&test_evt_flags);
    syn_task_create(&task, "evt_to_task", block_evt_timeout_task_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* Enter BLOCKED state with delay_until = 1100 */
    syn_sched_run(&sched);

    /* next_wakeup returns earliest deadline = 1100 */
    uint32_t wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL_UINT32(1100, wake);

    /* Test UINT32_MAX cap branch for BLOCKED task */
    mock_tick_ms = UINT32_MAX - 500;
    task.delay_until = UINT32_MAX;
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX - 1, syn_sched_next_wakeup(&sched));

    /* Advance time to 1100: next_wakeup returns current tick (now) */
    task.delay_until = 1100;
    mock_tick_ms = 1100;
    wake = syn_sched_next_wakeup(&sched);
    TEST_ASSERT_EQUAL_UINT32(1100, wake);
    mock_tick_ms = 0;
}

static void test_block_event_with_timeout_isr_race_post_timeout(void)
{
    SYN_Task task;
    SYN_Sched sched;
    mock_tick_ms = 1000;
    syn_event_flags_init(&test_evt_flags);
    test_evt_matched_flags = 0xAA; /* sentinel */
    syn_task_create(&task, "evt_to_task", block_evt_timeout_task_func, 0, NULL);
    syn_sched_init(&sched, &task, 1);

    /* First pass: enters BLOCKED state with delay_until = 1100 */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);

    /* Advance time past timeout */
    mock_tick_ms = 1105;

    /* Second pass: unblocks on timeout */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(0, test_evt_matched_flags); /* reported timeout (0) */

    /* Now set an event flag to simulate an ISR setting a flag after timeout */
    syn_event_flags_set(&test_evt_flags, 0x01);
    TEST_ASSERT_EQUAL_UINT32(0x01, syn_event_flags_get(&test_evt_flags)); /* flag remains intact */
}

static SYN_PT_Status block_task_two_flags_fn(SYN_PT *pt, SYN_Task *task)
{
    SYN_EventGroup *evt = (SYN_EventGroup *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, evt, EVT_DATA | EVT_DONE);
    block_counter++;
    PT_END(pt);
}

static void test_block_event_clears_only_matched_flags(void)
{
    mock_tick_ms = 0;
    block_counter = 0;

    SYN_EventGroup evt;
    syn_event_init(&evt);

    SYN_Task tasks[1];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "blk_two", block_task_two_flags_fn, 0, &evt);
    syn_sched_init(&sched, tasks, 1);

    /* Pass 1: task blocks waiting on EVT_DATA | EVT_DONE */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    /* Set only EVT_DATA (0x01) */
    syn_event_set(&evt, EVT_DATA);

    /* Scheduler sees EVT_DATA, sets task->wait_mask = EVT_DATA and marks READY */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_counter);

    /* EVT_DATA is cleared */
    TEST_ASSERT_FALSE(syn_event_check_any(&evt, EVT_DATA));

    /* Now set EVT_DONE (0x02) to verify it is preserved if set subsequently */
    syn_event_set(&evt, EVT_DONE);
    TEST_ASSERT_TRUE(syn_event_check_any(&evt, EVT_DONE));
}

static void test_sched_rr_uint16_rotation(void)
{
    SYN_Sched sched;
    syn_sched_init(&sched, NULL, 0);

    /* Verify rr_per_prio holds uint16_t without truncation */
    for (size_t i = 0; i < SYN_SCHED_PRIO_LEVELS; i++) {
        sched.rr_per_prio[i] = (uint16_t)(300 + i);
        TEST_ASSERT_EQUAL_UINT16((uint16_t)(300 + i), sched.rr_per_prio[i]);
    }
}

/* ── Intent & starvation-bound redesign tests ───────────────────────────── */

/**
 * 1. Two CRITICAL pollers plus a LOW worker: LOW runs on dispatch 3.
 */
static void test_two_critical_pollers_plus_low_worker(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_c1 = 1, id_c2 = 2, id_low = 3;

    syn_task_create(&tasks[0], "c1", pass_task, SYN_PRIO_CRITICAL, &id_c1);
    syn_task_create(&tasks[1], "c2", pass_task, SYN_PRIO_CRITICAL, &id_c2);
    syn_task_create(&tasks[2], "low", continue_task, SYN_PRIO_LOW, &id_low);
    syn_sched_init(&sched, tasks, 3);

    syn_sched_run(&sched);
    syn_sched_run(&sched);
    syn_sched_run(&sched);

    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
    TEST_ASSERT_EQUAL_INT(3, run_log[2]);
}

/**
 * 2. HIGH MORE flood plus CRITICAL poller: poller runs at least once per 1 ms.
 */
static void test_high_more_flood_vs_critical_poller(void)
{
    mock_tick_ms = 0;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_crit = 1, id_high = 2;

    syn_task_create(&tasks[0], "crit", pass_task, SYN_PRIO_CRITICAL, &id_crit);
    syn_task_create(&tasks[1], "high", continue_task, SYN_PRIO_HIGH, &id_high);
    syn_sched_init(&sched, tasks, 2);

    for (int ms = 0; ms < 10; ms++) {
        log_reset();
        for (int step = 0; step < 12; step++) {
            syn_sched_run(&sched);
        }

        int crit_runs = 0;
        int high_runs = 0;
        for (int k = 0; k < run_log_idx; k++) {
            if (run_log[k] == 1) {
                crit_runs++;
            } else if (run_log[k] == 2) {
                high_runs++;
            }
        }
        /* CRITICAL poller ran once this ms, HIGH worker got remaining dispatches */
        TEST_ASSERT_EQUAL_INT(1, crit_runs);
        TEST_ASSERT_EQUAL_INT(11, high_runs);

        mock_tick_advance(1);
    }
}

/**
 * 3. HIGH MORE flood plus LOW MORE: LOW runs within 50 ms (SYN_SCHED_MAX_WAIT_LOW_MS).
 */
static int g_flood_high_cnt;
static int g_flood_low_cnt;
static uint32_t g_first_low_ms;

static SYN_PT_Status flood_high_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        g_flood_high_cnt++;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status flood_low_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        if (g_flood_low_cnt == 0) {
            g_first_low_ms = syn_port_get_tick_ms();
        }
        g_flood_low_cnt++;
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void test_high_more_flood_vs_low_more(void)
{
    mock_tick_ms = 0;
    g_flood_high_cnt = 0;
    g_flood_low_cnt = 0;
    g_first_low_ms = UINT32_MAX;

    SYN_Task tasks[2];
    SYN_Sched sched;

    syn_task_create(&tasks[0], "high", flood_high_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[1], "low", flood_low_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 2);

    /* Run 4 dispatches per ms across 200 ms */
    for (uint32_t ms = 0; ms <= 200U; ms++) {
        mock_tick_ms = ms;
        for (int step = 0; step < 4; step++) {
            syn_sched_run(&sched);
        }
        if (ms < SYN_SCHED_MAX_WAIT_LOW_MS) {
            TEST_ASSERT_EQUAL_INT(0, g_flood_low_cnt);
        }
    }

    TEST_ASSERT_EQUAL_UINT32(SYN_SCHED_MAX_WAIT_LOW_MS, g_first_low_ms);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(4, g_flood_low_cnt);
}

/**
 * 4. Simultaneous overdue tasks (CRITICAL and LOW both overdue after a long HIGH chunk,
 * even when LOW's deadline is older than CRITICAL's): CRITICAL runs first, then LOW runs
 * immediately on the next dispatch ahead of non-overdue HIGH work, then HIGH resumes.
 */
static uint32_t g_high_chunk_advance_ms;

static SYN_PT_Status variable_chunk_high_fn(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        run_log[run_log_idx++] = id;
        if (g_high_chunk_advance_ms > 0U) {
            mock_tick_advance(g_high_chunk_advance_ms);
            g_high_chunk_advance_ms = 0U;
        }
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static void test_simultaneous_overdue_priority_order(void)
{
    mock_tick_ms = 0;
    g_high_chunk_advance_ms = 0U;
    log_reset();

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_crit = 1, id_high = 2, id_low = 3;

    /* Place LOW before CRITICAL in task array so priority comparison (< and >) is exercised */
    syn_task_create(&tasks[0], "low", continue_task, SYN_PRIO_LOW, &id_low);
    syn_task_create(&tasks[1], "crit", pass_task, SYN_PRIO_CRITICAL, &id_crit);
    syn_task_create(&tasks[2], "high", variable_chunk_high_fn, SYN_PRIO_HIGH, &id_high);
    syn_sched_init(&sched, tasks, 3);

    syn_sched_set_max_wait(&sched, SYN_PRIO_CRITICAL, 4U);
    syn_sched_set_max_wait(&sched, SYN_PRIO_HIGH, 20U);
    syn_sched_set_max_wait(&sched, SYN_PRIO_LOW, 10U);

    /* t = 0: crit runs (due = 4), low stamped due = 10, high runs (due = 20) */
    syn_sched_run(&sched);
    syn_sched_run(&sched);

    /* t = 4: crit overdue -> runs (due = 8) */
    mock_tick_ms = 4U;
    syn_sched_run(&sched);

    /* t = 8: crit overdue -> runs (due = 12); then high runs a 7 ms chunk -> t = 15 */
    mock_tick_ms = 8U;
    syn_sched_run(&sched);
    g_high_chunk_advance_ms = 7U;
    syn_sched_run(&sched);

    /* At t = 15: low.due_ms == 10 (older!), crit.due_ms == 12 (newer), high.due_ms == 35 */
    TEST_ASSERT_EQUAL_UINT32(15U, mock_tick_ms);
    TEST_ASSERT_LESS_THAN_UINT32(tasks[1].due_ms, tasks[0].due_ms);

    log_reset();
    syn_sched_run(&sched); /* Overdue rank: CRITICAL wins over older LOW */
    syn_sched_run(&sched); /* Overdue rank: LOW wins over non-overdue HIGH */
    syn_sched_run(&sched); /* Work rank: HIGH resumes ahead of LOW */

    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(3, run_log[1]);
    TEST_ASSERT_EQUAL_INT(2, run_log[2]);
}

/**
 * 5. N pollers across priorities: each runs exactly once per round.
 */
static void test_n_pollers_once_per_round(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[5];
    SYN_Sched sched;
    static int ids[5] = {1, 2, 3, 4, 5};

    syn_task_create(&tasks[0], "p_low", pass_task, SYN_PRIO_LOW, &ids[0]);
    syn_task_create(&tasks[1], "p_crit", pass_task, SYN_PRIO_CRITICAL, &ids[1]);
    syn_task_create(&tasks[2], "p_high1", pass_task, SYN_PRIO_HIGH, &ids[2]);
    syn_task_create(&tasks[3], "p_norm", pass_task, SYN_PRIO_NORMAL, &ids[3]);
    syn_task_create(&tasks[4], "p_high2", pass_task, SYN_PRIO_HIGH, &ids[4]);
    syn_sched_init(&sched, tasks, 5);

    /* Run 4 rounds of 5 pollers = 20 dispatches */
    for (int round = 0; round < 4; round++) {
        log_reset();
        for (int i = 0; i < 5; i++) {
            syn_sched_run(&sched);
        }

        /* Priority order within every round: crit(2), high1(3), high2(5), norm(4), low(1) */
        TEST_ASSERT_EQUAL_INT(2, run_log[0]);
        TEST_ASSERT_EQUAL_INT(3, run_log[1]);
        TEST_ASSERT_EQUAL_INT(5, run_log[2]);
        TEST_ASSERT_EQUAL_INT(4, run_log[3]);
        TEST_ASSERT_EQUAL_INT(1, run_log[4]);
    }
}

/**
 * 6. syn_task_should_share and PT_CONTINUE_IF_NEEDED semantics.
 */
static bool g_share_observed;
static bool g_share_null_observed;
static bool g_share_peer_observed;
static const SYN_Task *g_peer_probe;

static SYN_PT_Status share_probe_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        g_share_null_observed = syn_task_should_share(NULL);
        g_share_peer_observed = syn_task_should_share(g_peer_probe);
        g_share_observed = syn_task_should_share(task);
        PT_CONTINUE(pt);
    }
    PT_END(pt);
}

static int g_drain_iters;
static uint32_t g_drain_advance_on_iter;

static SYN_PT_Status drain_loop_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        g_drain_iters++;
        if ((uint32_t)g_drain_iters == g_drain_advance_on_iter) {
            mock_tick_advance(1);
        }
        PT_CONTINUE_IF_NEEDED(pt, task);
    }
    PT_END(pt);
}

static void test_task_should_share_and_continue_if_needed(void)
{
    mock_tick_ms = 0;

    SYN_Task sole_task;
    SYN_Task dummy_peer;
    SYN_Sched sole_sched;
    syn_task_create(&sole_task, "sole", share_probe_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&dummy_peer, "peer", pass_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sole_sched, &sole_task, 1);
    g_peer_probe = &dummy_peer;

    /* Outside dispatch: false for NULL and valid task */
    TEST_ASSERT_FALSE(syn_task_should_share(NULL));
    TEST_ASSERT_FALSE(syn_task_should_share(&sole_task));

    /* Sole ready task: false (no peer deadline, no contention) */
    g_share_observed = true;
    g_share_null_observed = true;
    g_share_peer_observed = true;
    syn_sched_run(&sole_sched);
    TEST_ASSERT_FALSE(g_share_null_observed);
    TEST_ASSERT_FALSE(g_share_peer_observed);
    TEST_ASSERT_FALSE(g_share_observed);

    /* Multi-peer scenario with PASSED, POLL, BLOCKED, SUSPENDED, DEAD, delayed READY,
     * and lower-priority MORE peers (none overdue): should_share must be false. */
    SYN_EventGroup evt;
    syn_event_init(&evt);
    SYN_Task peers[7];
    SYN_Sched peer_sched;
    static int ids[7] = {1, 2, 3, 4, 5, 6, 7};

    syn_task_create(&peers[0], "crit_pass", pass_task, SYN_PRIO_CRITICAL, &ids[0]);
    syn_task_create(&peers[1], "runner_hi", share_probe_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&peers[2], "low_more", continue_task, SYN_PRIO_LOW, &ids[2]);
    syn_task_create(&peers[3], "norm_more", continue_task, SYN_PRIO_NORMAL, &ids[3]);
    syn_task_create(&peers[4], "blocked", block_task_fn, SYN_PRIO_CRITICAL, &evt);
    syn_task_create(&peers[5], "susp", continue_task, SYN_PRIO_CRITICAL, &ids[5]);
    syn_task_create(&peers[6], "delayed", continue_task, SYN_PRIO_CRITICAL, &ids[6]);
    syn_sched_init(&peer_sched, peers, 7);
    syn_sched_set_max_wait(&peer_sched, SYN_PRIO_CRITICAL, 10U);

    syn_task_suspend(&peers[5]);
    peers[6].delay_until = 50U;
    g_peer_probe = &peers[0];

    syn_sched_run(&peer_sched); /* peers[0] (crit_pass) runs and passes (due = 10) */
    syn_sched_run(&peer_sched); /* peers[4] (blocked) runs and blocks */

    /* Next dispatch runs peers[1] (runner_hi): peers[0] is PASSED (due=10),
     * peers[2] is LOW MORE (due=50), peers[3] is NORMAL MORE (due=15 < 50),
     * peers[4] is BLOCKED, peers[5] is SUSPENDED, peers[6] is READY+delayed (SLEEPING). */
    g_share_observed = true;
    syn_sched_run(&peer_sched);
    TEST_ASSERT_FALSE(g_share_observed);

    /* Equal-priority MORE peer -> should_share is true */
    SYN_Task eq_tasks[2];
    SYN_Sched eq_sched;
    syn_task_create(&eq_tasks[0], "hi_probe", share_probe_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&eq_tasks[1], "hi_more", continue_task, SYN_PRIO_HIGH, &ids[1]);
    syn_sched_init(&eq_sched, eq_tasks, 2);
    g_share_observed = false;
    syn_sched_run(&eq_sched);
    TEST_ASSERT_TRUE(g_share_observed);

    /* Higher-priority MORE peer while overdue LOW runs -> should_share is true */
    mock_tick_ms = 0;
    SYN_Task hi_tasks[2];
    SYN_Sched hi_sched;
    syn_task_create(&hi_tasks[0], "hi_more", continue_task, SYN_PRIO_HIGH, &ids[0]);
    syn_task_create(&hi_tasks[1], "low_probe", share_probe_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&hi_sched, hi_tasks, 2);
    syn_sched_set_max_wait(&hi_sched, SYN_PRIO_LOW, 2U);
    syn_sched_run(&hi_sched); /* hi_more runs at t=0 (due=5), low_probe stamped due=2 */
    mock_tick_ms = 2U;        /* low_probe overdue (2 >= 2), hi_more not overdue (2 < 5, MORE) */
    g_share_observed = false;
    syn_sched_run(&hi_sched);
    TEST_ASSERT_TRUE(g_share_observed);

    /* Mid-loop overdue transition via PT_CONTINUE_IF_NEEDED:
     * CRITICAL poller passes at t=0 (due=1), HIGH drain loop runs 3 iterations
     * until iteration 3 advances tick to 1 ms -> yields immediately, CRITICAL runs next! */
    mock_tick_ms = 0;
    log_reset();
    g_drain_iters = 0;
    g_drain_advance_on_iter = 3U;

    SYN_Task drain_tasks[2];
    SYN_Sched drain_sched;
    syn_task_create(&drain_tasks[0], "crit_poll", pass_task, SYN_PRIO_CRITICAL, &ids[0]);
    syn_task_create(&drain_tasks[1], "hi_drain", drain_loop_fn, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&drain_sched, drain_tasks, 2);

    syn_sched_run(&drain_sched); /* crit_poll runs (logs 1) and passes (due = 1) */
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);

    syn_sched_run(&drain_sched); /* hi_drain runs 3 loop iterations, yields on iter 3 */
    TEST_ASSERT_EQUAL_INT(3, g_drain_iters);

    syn_sched_run(&drain_sched); /* crit_poll is now overdue and runs immediately! */
    TEST_ASSERT_EQUAL_INT(2, run_log_idx);
    TEST_ASSERT_EQUAL_INT(1, run_log[1]);
}

/**
 * 7. PT_SPAWN passes child's PT_MORE and PT_IDLE up unchanged to syn_sched_run.
 */
static SYN_PT g_sched_child_pt;

static SYN_PT_Status sched_child_coroutine(SYN_PT *pt)
{
    PT_BEGIN(pt);
    run_log[run_log_idx++] = 10;
    PT_CONTINUE(pt); /* child reports PT_MORE */
    run_log[run_log_idx++] = 11;
    PT_PASS(pt); /* child reports PT_IDLE */
    run_log[run_log_idx++] = 12;
    PT_END(pt);
}

static SYN_PT_Status sched_parent_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    PT_SPAWN(pt, &g_sched_child_pt, sched_child_coroutine(&g_sched_child_pt));
    run_log[run_log_idx++] = 99;
    PT_END(pt);
}

static void test_pt_spawn_propagates_child_intent(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_low = 2;

    syn_task_create(&tasks[0], "parent", sched_parent_task, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[1], "low_work", continue_task, SYN_PRIO_LOW, &id_low);
    syn_sched_init(&sched, tasks, 2);

    /* Dispatch 1: child yields PT_MORE -> parent keeps MORE flag and stays ahead of LOW */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(10, run_log[0]);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MORE);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);

    /* Dispatch 2: parent runs again immediately (ahead of LOW), child yields PT_IDLE -> PASSED */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(11, run_log[1]);
    TEST_ASSERT_NOT_EQUAL(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MORE);

    /* Dispatch 3: because PT_IDLE propagated, LOW worker now gets to run! */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[2]);

    /* Advance past HIGH max_wait (5 ms): parent wakes, child finishes, parent exits */
    mock_tick_advance(5);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(12, run_log[3]);
    TEST_ASSERT_EQUAL_INT(99, run_log[4]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, tasks[0].state);
}

/**
 * 8. syn_sched_set_max_wait(..., 0) raises value to 1, and custom windows apply.
 */
static void test_sched_set_max_wait(void)
{
    mock_tick_ms = 0;
    g_flood_high_cnt = 0;
    g_flood_low_cnt = 0;
    g_first_low_ms = UINT32_MAX;

    SYN_Task tasks[2];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "high", flood_high_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[1], "low", flood_low_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 2);

    syn_sched_set_max_wait(&sched, SYN_PRIO_CRITICAL, 0U);
    TEST_ASSERT_EQUAL_UINT16(1U, sched.max_wait_ms[SYN_PRIO_CRITICAL]);

    syn_sched_set_max_wait(&sched, SYN_PRIO_LOW, 25U);
    TEST_ASSERT_EQUAL_UINT16(25U, sched.max_wait_ms[SYN_PRIO_LOW]);

    for (uint32_t ms = 0; ms <= 30U; ms++) {
        mock_tick_ms = ms;
        syn_sched_run(&sched);
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_UINT32(25U, g_first_low_ms);
}

/**
 * 9. A task waking from a delay ranks as Work (SCHED_RANK_WORK) and runs ahead
 * of a higher-priority unpolled Poll task (SCHED_RANK_POLL, non-overdue).
 */
static SYN_PT_Status delayed_low_worker_fn(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    PT_TASK_DELAY_MS(pt, task, 5);
    run_log[run_log_idx++] = id;
    PT_END(pt);
}

static void test_waking_delayed_task_ranks_as_work(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_p1 = 1, id_p2 = 2, id_w = 3;

    syn_task_create(&tasks[0], "p1", pass_task, SYN_PRIO_HIGH, &id_p1);
    syn_task_create(&tasks[1], "p2", pass_task, SYN_PRIO_HIGH, &id_p2);
    syn_task_create(&tasks[2], "w", delayed_low_worker_fn, SYN_PRIO_LOW, &id_w);
    syn_sched_init(&sched, tasks, 3);
    syn_sched_set_max_wait(&sched, SYN_PRIO_HIGH, 20U);

    /* t = 0: P1 passes, P2 passes, W enters 5 ms delay */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log_idx);

    /* Dispatch 4 at t = 0: P1 and P2 both PASSED -> round resets, P1 runs & passes,
     * leaving P2 (SYN_PRIO_HIGH) in SCHED_RANK_POLL (!MORE && !PASSED, due = 20). */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log_idx);
    TEST_ASSERT_EQUAL_UINT8(SYN_SCHED_FLAG_DUE_VALID, tasks[1].sched_flags);

    /* Advance to t = 5: W (LOW) wakes from delay as SCHED_RANK_WORK (MORE),
     * beating P2 (HIGH) in SCHED_RANK_POLL! */
    mock_tick_ms = 5U;
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log[3]);

    /* Next dispatch runs P2 (HIGH, SCHED_RANK_POLL) */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[4]);
}

/* ── Regression tests: Multi-scheduler isolation & mid-chunk wakeups ───── */

static SYN_Sched g_nested_sched;
static SYN_Task g_nested_tasks[1];
static bool g_nested_inner_share = true;
static bool g_outer_share_before = false;
static bool g_outer_share_after = false;
static bool g_cross_sched_query = true;

static SYN_PT_Status nested_inner_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    g_nested_inner_share = syn_task_should_share(task);
    PT_PASS(pt);
    PT_END(pt);
}

static SYN_PT_Status nested_outer_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    g_outer_share_before = syn_task_should_share(task);
    syn_task_create(&g_nested_tasks[0], "inner", nested_inner_fn, SYN_PRIO_HIGH, NULL);
    syn_sched_init(&g_nested_sched, g_nested_tasks, 1);
    syn_sched_run(&g_nested_sched);
    g_cross_sched_query = syn_task_should_share(&g_nested_tasks[0]);
    g_outer_share_after = syn_task_should_share(task);
    PT_PASS(pt);
    PT_END(pt);
}

static void test_dual_sched_isolation_and_unbound_share(void)
{
    mock_tick_ms = 0;

    SYN_Task unbound;
    static int dummy_id = 1;
    syn_task_create(&unbound, "unbound", pass_task, SYN_PRIO_LOW, &dummy_id);
    TEST_ASSERT_FALSE(syn_task_should_share(&unbound));

    SYN_Task outer_tasks[2];
    SYN_Sched outer_sched;
    syn_task_create(&outer_tasks[0], "outer_probe", nested_outer_fn, SYN_PRIO_HIGH, NULL);
    syn_task_create(&outer_tasks[1], "outer_peer", continue_task, SYN_PRIO_HIGH, &dummy_id);
    syn_sched_init(&outer_sched, outer_tasks, 2);

    g_nested_inner_share = true;
    g_outer_share_before = false;
    g_outer_share_after = false;
    g_cross_sched_query = true;

    syn_sched_run(&outer_sched);

    TEST_ASSERT_TRUE(g_outer_share_before);
    TEST_ASSERT_FALSE(g_nested_inner_share);
    TEST_ASSERT_FALSE(g_cross_sched_query);
    TEST_ASSERT_TRUE(g_outer_share_after);
}

typedef struct {
    uint32_t iters;
    uint32_t max_iters;
    uint32_t tick_step;
    SYN_EventGroup *wake_evt;
    uint32_t wake_on_iter;
    SYN_Task *resume_target;
    uint32_t resume_on_iter;
} BurstWakeCtx;

static SYN_PT_Status burst_wake_driver_fn(SYN_PT *pt, SYN_Task *task)
{
    BurstWakeCtx *ctx = (BurstWakeCtx *)task->user_data;
    PT_BEGIN(pt);
    while (ctx->iters < ctx->max_iters) {
        ctx->iters++;
        mock_tick_ms += ctx->tick_step;
        if (ctx->wake_evt != NULL && ctx->iters == ctx->wake_on_iter) {
            syn_event_set(ctx->wake_evt, 0x01U);
        }
        if (ctx->resume_target != NULL && ctx->iters == ctx->resume_on_iter) {
            syn_task_resume(ctx->resume_target);
        }
        PT_CONTINUE_IF_NEEDED(pt, task);
    }
    PT_PASS(pt);
    PT_END(pt);
}

static SYN_PT_Status one_shot_exit_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    PT_END(pt);
}

static SYN_PT_Status block_cond_never_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_BLOCK_CONDITION(pt, task, false);
    PT_END(pt);
}

static void test_should_share_delayed_and_isr_wakeups(void)
{
    static int id_crit = 1;

    /* (a) LOW burst task yields as soon as delayed CRITICAL peer's delay_until (5 ms) expires */
    mock_tick_ms = 0;
    log_reset();
    BurstWakeCtx ctx_a = {0, 20U, 1U, NULL, 0U, NULL, 0U};
    SYN_Task tasks_a[2];
    SYN_Sched sched_a;
    syn_task_create(&tasks_a[0], "crit_dly", delayed_low_worker_fn, SYN_PRIO_CRITICAL, &id_crit);
    syn_task_create(&tasks_a[1], "low_burst", burst_wake_driver_fn, SYN_PRIO_LOW, &ctx_a);
    syn_sched_init(&sched_a, tasks_a, 2);

    syn_sched_run(&sched_a); /* crit_dly enters PT_TASK_DELAY_MS(5) */
    syn_sched_run(&sched_a); /* low_burst advances 1 ms/iter, yields on iter 5 (t = 5) */
    TEST_ASSERT_EQUAL_UINT32(5U, ctx_a.iters);
    syn_sched_run(&sched_a); /* crit_dly runs immediately at t = 5! */
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);

    /* (b) HIGH burst task with DEAD peer, BLOCKED-condition peer, BLOCKED-timeout peer,
     * and delayed LOW peer (delay=5, max_wait=50 -> overdue at 55):
     * must NOT yield at t=5 when LOW wakes, only when earliest peer deadline expires! */
    mock_tick_ms = 0;
    syn_event_flags_init(&test_evt_flags);
    BurstWakeCtx ctx_b = {0, 100U, 5U, NULL, 0U, NULL, 0U};
    SYN_Task tasks_b[5];
    SYN_Sched sched_b;
    syn_task_create(&tasks_b[0], "dead_peer", one_shot_exit_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&tasks_b[1], "cond_blk", block_cond_never_fn, SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&tasks_b[2], "tmo_blk", block_evt_timeout_task_func, SYN_PRIO_LOW, NULL);
    syn_task_create(&tasks_b[3], "low_dly", delayed_low_worker_fn, SYN_PRIO_LOW, &id_crit);
    syn_task_create(&tasks_b[4], "hi_burst", burst_wake_driver_fn, SYN_PRIO_HIGH, &ctx_b);
    syn_sched_init(&sched_b, tasks_b, 5);

    syn_task_suspend(&tasks_b[4]);
    syn_sched_run(&sched_b); /* dead_peer exits -> SYN_TASK_DEAD */
    syn_sched_run(&sched_b); /* cond_blk blocks with wait_event == NULL */
    syn_sched_run(&sched_b); /* tmo_blk (LOW) blocks with delay_until = 100 (wake_due = 150) */
    syn_sched_run(&sched_b); /* low_dly (LOW) delays with delay_until = 5 (wake_due = 55) */
    syn_task_resume(&tasks_b[4]);
    syn_sched_run(&sched_b); /* hi_burst runs 5 ms/iter: skips t=5, yields at t=55 (iter 11) */
    TEST_ASSERT_EQUAL_UINT32(11U, ctx_b.iters);
    TEST_ASSERT_EQUAL_UINT32(55U, mock_tick_ms);

    /* (c) Mid-burst ISR syn_event_set wakes BLOCKED CRITICAL peer at same tick (t = 0) */
    mock_tick_ms = 0;
    log_reset();
    SYN_EventGroup evt_c;
    syn_event_init(&evt_c);
    BurstWakeCtx ctx_c = {0, 20U, 0U, &evt_c, 3U, NULL, 0U};
    SYN_Task tasks_c[2];
    SYN_Sched sched_c;
    syn_task_create(&tasks_c[0], "crit_evt", block_task_fn, SYN_PRIO_CRITICAL, &evt_c);
    syn_task_create(&tasks_c[1], "low_burst", burst_wake_driver_fn, SYN_PRIO_LOW, &ctx_c);
    syn_sched_init(&sched_c, tasks_c, 2);

    syn_sched_run(&sched_c); /* crit_evt blocks on evt_c */
    syn_sched_run(&sched_c); /* low_burst fires evt_c on iter 3 -> yields immediately on iter 3! */
    TEST_ASSERT_EQUAL_UINT32(3U, ctx_c.iters);

    /* (d) Mid-burst ISR syn_task_resume on SUSPENDED peer (unexpired delay vs immediate ready) */
    mock_tick_ms = 0;
    BurstWakeCtx ctx_d = {0, 10U, 1U, NULL, 0U, NULL, 2U};
    SYN_Task tasks_d[2];
    SYN_Sched sched_d;
    syn_task_create(&tasks_d[0], "crit_susp", continue_task, SYN_PRIO_CRITICAL, &id_crit);
    syn_task_create(&tasks_d[1], "low_burst", burst_wake_driver_fn, SYN_PRIO_LOW, &ctx_d);
    syn_sched_init(&sched_d, tasks_d, 2);
    ctx_d.resume_target = &tasks_d[0];

    /* Suspend crit_susp while it also has delay_until = 5: resume on iter 2 (t=2) stays asleep
     * until t=5! */
    tasks_d[0].delay_until = 5U;
    syn_task_suspend(&tasks_d[0]);
    syn_sched_run(
        &sched_d); /* low_burst resumes crit_susp at t=2, keeps running until t=5 (iter 5)! */
    TEST_ASSERT_EQUAL_UINT32(5U, ctx_d.iters);

    /* Suspend crit_susp with delay_until = 0: resume on iter 2 yields immediately on iter 2! */
    mock_tick_ms = 0;
    tasks_d[0].delay_until = 0U;
    syn_task_suspend(&tasks_d[0]);
    syn_task_restart(&tasks_d[1]);
    ctx_d.iters = 0;
    syn_sched_run(&sched_d);
    TEST_ASSERT_EQUAL_UINT32(2U, ctx_d.iters);
}

static void test_overdue_rr_preservation_and_bounds(void)
{
    static int ids[4] = {10, 20, 30, 40};

    /* Part 1: Out-of-bounds rr_per_prio[prio] >= task_count clamps to 0 during OVERDUE dispatch */
    mock_tick_ms = 0U;
    log_reset();
    SYN_Task bounds_tasks[2];
    SYN_Sched bounds_sched;
    syn_task_create(&bounds_tasks[0], "hi_pass", pass_task, SYN_PRIO_HIGH, &ids[0]);
    syn_task_create(&bounds_tasks[1], "hi_work", continue_task, SYN_PRIO_HIGH, &ids[1]);
    syn_sched_init(&bounds_sched, bounds_tasks, 2);
    syn_sched_set_max_wait(&bounds_sched, SYN_PRIO_HIGH, 10U);

    syn_sched_run(&bounds_sched); /* hi_pass runs at t=0 (due=10), hi_work stamped due=10 */
    mock_tick_ms = 8U;
    bounds_tasks[0].due_ms = 20U; /* keep hi_pass non-overdue (PASSED) while hi_work hits overdue */
    mock_tick_ms = 10U;
    bounds_sched.rr_per_prio[SYN_PRIO_HIGH] = 99U;

    log_reset();
    syn_sched_run(&bounds_sched); /* hi_work (idx 1) wins OVERDUE, rr clamps 99->0 and advances */
    TEST_ASSERT_EQUAL_INT(20, run_log[0]);
    TEST_ASSERT_EQUAL_UINT16(0U, bounds_sched.rr_per_prio[SYN_PRIO_HIGH]);

    /* Part 2: OVERDUE dispatch does NOT advance rr_per_prio past an earlier non-overdue MORE peer
     */
    mock_tick_ms = 0U;
    log_reset();
    SYN_Task tasks[4];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "hi0", continue_task, SYN_PRIO_HIGH, &ids[0]);
    syn_task_create(&tasks[1], "hi1", continue_task, SYN_PRIO_HIGH, &ids[1]);
    syn_task_create(&tasks[2], "hi2", continue_task, SYN_PRIO_HIGH, &ids[2]);
    syn_task_create(&tasks[3], "crit", continue_task, SYN_PRIO_CRITICAL, &ids[3]);
    syn_sched_init(&sched, tasks, 4);
    syn_sched_set_max_wait(&sched, SYN_PRIO_HIGH, 10U);

    syn_task_suspend(&tasks[3]);
    tasks[0].delay_until = 6U;

    /* t = 0: hi1 and hi2 run (due_ms = 10), rr_per_prio[HIGH] advances to 3 */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT16(3U, sched.rr_per_prio[SYN_PRIO_HIGH]);

    /* t = 6: wake crit so sched_prepare_task stamps hi0 awake (MORE, due_ms = 16) without running
     * hi0 */
    mock_tick_ms = 6U;
    syn_task_resume(&tasks[3]);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(16U, tasks[0].due_ms);
    TEST_ASSERT_EQUAL_UINT16(3U, sched.rr_per_prio[SYN_PRIO_HIGH]);

    /* t = 10: suspend crit; hi1 and hi2 are OVERDUE while hi0 (at wrap pos 0) is READY+MORE */
    syn_task_suspend(&tasks[3]);
    mock_tick_ms = 10U;
    log_reset();

    syn_sched_run(&sched); /* hi1 wins OVERDUE; rr_per_prio stays 3 to protect hi0 */
    TEST_ASSERT_EQUAL_INT(20, run_log[0]);
    TEST_ASSERT_EQUAL_UINT16(3U, sched.rr_per_prio[SYN_PRIO_HIGH]);

    syn_sched_run(&sched); /* hi2 wins OVERDUE; rr_per_prio still stays 3 */
    TEST_ASSERT_EQUAL_INT(30, run_log[1]);
    TEST_ASSERT_EQUAL_UINT16(3U, sched.rr_per_prio[SYN_PRIO_HIGH]);

    syn_sched_run(&sched); /* Work rank: hi0 runs next instead of being skipped! */
    TEST_ASSERT_EQUAL_INT(10, run_log[2]);
}

static void test_task_restart_releases_recursive_contended_mutexes(void)
{
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task owner = {0};
    SYN_Task w_norm = {0};
    SYN_Task w_crit = {0};
    SYN_Task dead_task = {0};
    SYN_Sched dead_sched;

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    syn_task_create(&owner, "owner", pass_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&w_norm, "w_norm", pass_task, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&w_crit, "w_crit", pass_task, SYN_PRIO_CRITICAL, NULL);

    /* Recursive acquisitions (lock_count > 1) on two mutexes */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m1, &owner));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m1, &owner));
    TEST_ASSERT_EQUAL_UINT16(2U, m1.lock_count);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m2, &owner));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m2, &owner));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m2, &owner));
    TEST_ASSERT_EQUAL_UINT16(3U, m2.lock_count);

    /* Contend both mutexes from PASSED state, boosting owner to SYN_PRIO_CRITICAL */
    w_norm.sched_flags = SYN_SCHED_FLAG_PASSED;
    w_crit.sched_flags = SYN_SCHED_FLAG_PASSED;
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m1, &w_norm));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m2, &w_crit));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, owner.priority);

    /* Restart owner (K-06): both recursive mutexes unlock, waiters wake, priority resets */
    syn_task_restart(&owner);

    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m1));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m2));
    TEST_ASSERT_NULL(owner.held_mutexes);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, owner.priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, owner.state);
    TEST_ASSERT_TRUE((w_norm.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_TRUE((w_crit.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);

    /* Suspending a DEAD task is a no-op */
    syn_task_create(&dead_task, "dead", one_shot_exit_fn, SYN_PRIO_LOW, NULL);
    syn_sched_init(&dead_sched, &dead_task, 1);
    syn_sched_run(&dead_sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, dead_task.state);
    syn_task_suspend(&dead_task);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, dead_task.state);
}

static void test_priority_restore_and_set_base_preserve_mutex_boost(void)
{
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task owner = {0};
    SYN_Task w_high = {0};
    SYN_Task w_susp_crit = {0};

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    syn_task_create(&owner, "owner", pass_task, SYN_PRIO_LOW, NULL);
    syn_task_create(&w_high, "w_high", pass_task, SYN_PRIO_HIGH, NULL);
    syn_task_create(&w_susp_crit, "w_crit", pass_task, SYN_PRIO_CRITICAL, NULL);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m1, &owner));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m2, &owner));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m1, &w_high));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m2, &w_susp_crit));

    /* Suspend CRITICAL waiter so only READY HIGH waiter contributes active inheritance */
    syn_task_suspend(&w_susp_crit);

    /* Manual boost to CRITICAL, then restore: must drop only to HIGH (not LOW!) */
    syn_task_boost_priority(&owner, SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, owner.priority);
    syn_task_restore_priority(&owner);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, owner.priority);

    /* Changing base_priority to NORMAL while contended by HIGH keeps effective priority HIGH */
    syn_task_set_base_priority(&owner, SYN_PRIO_NORMAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, owner.base_priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, owner.priority);

    /* Unlocking m1 drops effective priority to the updated base_priority (NORMAL) */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_unlock(&m1, &owner));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, owner.priority);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_unlock(&m2, &owner));
}

typedef struct {
    SYN_EventFlags *evt;
    uint32_t mask;
    uint32_t timeout_ms;
    uint32_t *out_flags;
    int completions;
} EvtTimeoutCfg;

static SYN_PT_Status evt_timeout_cfg_task_fn(SYN_PT *pt, SYN_Task *task)
{
    EvtTimeoutCfg *cfg = (EvtTimeoutCfg *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, cfg->evt, cfg->mask, cfg->timeout_ms, cfg->out_flags);
    cfg->completions++;
    PT_END(pt);
}

static SYN_PT_Status shared_evt_block_task_fn(SYN_PT *pt, SYN_Task *task)
{
    EvtTimeoutCfg *cfg = (EvtTimeoutCfg *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, cfg->evt, cfg->mask);
    cfg->completions++;
    PT_END(pt);
}

static void test_k02_event_blocking_edge_cases(void)
{
    mock_tick_ms = 100U;

    /* 1. Spurious syn_task_resume on PT_BLOCK_EVENT task leaves state SYN_TASK_BLOCKED,
     * and suspend -> resume -> syn_sched_run with event == 0 re-blocks without advancing! */
    SYN_EventGroup evt1;
    syn_event_init(&evt1);
    EvtTimeoutCfg cfg1 = {&evt1, EVT_DATA, 0U, NULL, 0};
    SYN_Task t1 = {0};
    SYN_Sched s1;
    syn_task_create(&t1, "blk1", shared_evt_block_task_fn, SYN_PRIO_HIGH, &cfg1);
    syn_sched_init(&s1, &t1, 1);

    syn_sched_run(&s1);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, t1.state);
    TEST_ASSERT_EQUAL_INT(0, cfg1.completions);

    /* Spurious resume ignored while wait_event != NULL */
    syn_task_resume(&t1);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, t1.state);

    /* Suspend then resume sets state READY while wait_event != NULL */
    syn_task_suspend(&t1);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_SUSPENDED, t1.state);
    syn_task_resume(&t1);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, t1.state);

    /* Running scheduler with event == 0 transitions cleanly back to SYN_TASK_BLOCKED */
    syn_sched_run(&s1);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, t1.state);
    TEST_ASSERT_EQUAL_INT(0, cfg1.completions);

    /* Direct coroutine step after another suspend/resume also re-blocks inside PT_BLOCK_EVENT */
    syn_task_suspend(&t1);
    syn_task_resume(&t1);
    TEST_ASSERT_EQUAL_INT(PT_IDLE, t1.func(&t1.pt, &t1));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, t1.state);
    TEST_ASSERT_EQUAL_INT(0, cfg1.completions);

    /* Setting the event now wakes t1 and delivers completion */
    syn_event_set(&evt1, EVT_DATA);
    syn_sched_run(&s1);
    TEST_ASSERT_EQUAL_INT(1, cfg1.completions);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, t1.state);
    TEST_ASSERT_FALSE(syn_event_check_any(&evt1, EVT_DATA));

    /* 2. Two tasks blocked on same event bit (0x01): single set wakes only first/higher-prio task
     */
    SYN_EventGroup shared_evt;
    syn_event_init(&shared_evt);
    EvtTimeoutCfg hi_cfg = {&shared_evt, 0x01U, 0U, NULL, 0};
    EvtTimeoutCfg lo_cfg = {&shared_evt, 0x01U, 0U, NULL, 0};
    SYN_Task pair[2];
    SYN_Sched pair_sched;
    memset(pair, 0, sizeof(pair));
    syn_task_create(&pair[0], "hi_wait", shared_evt_block_task_fn, SYN_PRIO_HIGH, &hi_cfg);
    syn_task_create(&pair[1], "lo_wait", shared_evt_block_task_fn, SYN_PRIO_LOW, &lo_cfg);
    syn_sched_init(&pair_sched, pair, 2);

    syn_sched_run(&pair_sched);
    syn_sched_run(&pair_sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, pair[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, pair[1].state);

    syn_event_set(&shared_evt, 0x01U);
    syn_sched_run(&pair_sched); /* hi_wait consumes & clears 0x01 */
    syn_sched_run(&pair_sched); /* lo_wait sees 0x01 cleared and stays SYN_TASK_BLOCKED */
    TEST_ASSERT_EQUAL_INT(1, hi_cfg.completions);
    TEST_ASSERT_EQUAL_INT(0, lo_cfg.completions);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, pair[1].state);

    /* 3. PT_BLOCK_EVENT_WITH_TIMEOUT with timeout_ms == 0U (matched, unmatched, NULL out_flags) */
    SYN_EventGroup imm_evt;
    syn_event_init(&imm_evt);
    uint32_t out_flags = 0xDEADBEEFU;
    EvtTimeoutCfg imm_cfg = {&imm_evt, 0x03U, 0U, &out_flags, 0};
    SYN_Task imm_task = {0};
    SYN_Sched imm_sched;
    syn_task_create(&imm_task, "imm", evt_timeout_cfg_task_fn, SYN_PRIO_NORMAL, &imm_cfg);
    syn_sched_init(&imm_sched, &imm_task, 1);

    /* Unmatched immediate poll -> out_flags == 0, completes without blocking */
    syn_sched_run(&imm_sched);
    TEST_ASSERT_EQUAL_INT(1, imm_cfg.completions);
    TEST_ASSERT_EQUAL_UINT32(0U, out_flags);

    /* Matched immediate poll -> out_flags receives matched bit, preserves unrelated bit */
    syn_event_set(&imm_evt, 0x05U);
    out_flags = 0U;
    syn_task_restart(&imm_task);
    syn_sched_run(&imm_sched);
    TEST_ASSERT_EQUAL_INT(2, imm_cfg.completions);
    TEST_ASSERT_EQUAL_UINT32(0x01U, out_flags);
    TEST_ASSERT_EQUAL_UINT32(0x04U, syn_event_flags_get(&imm_evt));

    /* NULL out_flags variants (timeout_ms == 0U and non-zero timeout_ms) */
    imm_cfg.out_flags = NULL;
    syn_task_restart(&imm_task);
    syn_sched_run(&imm_sched);
    TEST_ASSERT_EQUAL_INT(3, imm_cfg.completions);

    imm_cfg.timeout_ms = 10U;
    syn_task_restart(&imm_task);
    syn_sched_run(&imm_sched); /* blocks until 110 */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, imm_task.state);
    mock_tick_ms = 110U;
    syn_sched_run(&imm_sched); /* expires cleanly with out_flags == NULL */
    TEST_ASSERT_EQUAL_INT(4, imm_cfg.completions);
}

typedef struct {
    SYN_Task *target;
    bool isr_fired;
    int woke_count;
} IsrSelfWakeCtx;

static bool isr_self_wake_cond(IsrSelfWakeCtx *ctx)
{
    if (!ctx->isr_fired) {
        ctx->isr_fired = true;
        syn_task_resume(ctx->target);
        return false;
    }
    return true;
}

static SYN_PT_Status isr_self_wake_task_fn(SYN_PT *pt, SYN_Task *task)
{
    IsrSelfWakeCtx *ctx = (IsrSelfWakeCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_CONDITION(pt, task, isr_self_wake_cond(ctx));
    ctx->woke_count++;
    PT_PASS(pt);
    PT_END(pt);
}

static void test_k03_isr_wake_and_critical_burst_share(void)
{
    static int id_low = 7;
    static int id_crit = 8;

    /* 1. Task simulates ISR calling syn_task_resume(self) before returning PT_IDLE:
     * SYN_SCHED_FLAG_ISR_WAKE must preserve SYN_SCHED_FLAG_MORE instead of PASSED! */
    mock_tick_ms = 0U;
    log_reset();
    SYN_Task wake_tasks[2];
    SYN_Sched wake_sched;
    memset(wake_tasks, 0, sizeof(wake_tasks));
    IsrSelfWakeCtx wake_ctx = {&wake_tasks[0], false, 0};

    syn_task_create(&wake_tasks[0], "hi_self", isr_self_wake_task_fn, SYN_PRIO_HIGH, &wake_ctx);
    syn_task_create(&wake_tasks[1], "lo_work", continue_task, SYN_PRIO_LOW, &id_low);
    syn_sched_init(&wake_sched, wake_tasks, 2);

    syn_sched_run(&wake_sched); /* hi_self returns PT_IDLE after simulated ISR syn_task_resume */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, wake_tasks[0].state);
    TEST_ASSERT_TRUE((wake_tasks[0].sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
    TEST_ASSERT_TRUE((wake_tasks[0].sched_flags & SYN_SCHED_FLAG_PASSED) == 0U);

    syn_sched_run(&wake_sched); /* hi_self runs immediately ahead of lo_work! */
    TEST_ASSERT_EQUAL_INT(1, wake_ctx.woke_count);
    TEST_ASSERT_EQUAL_INT(0, run_log_idx);

    /* 2. CRITICAL burst task with PT_CONTINUE_IF_NEEDED:
     * waking SYN_PRIO_LOW mid-slice does NOT preempt CRITICAL, whereas waking CRITICAL DOES yield!
     */
    mock_tick_ms = 0U;
    BurstWakeCtx burst_ctx = {0U, 10U, 0U, NULL, 0U, NULL, 3U};
    SYN_Task burst_tasks[4];
    SYN_Sched burst_sched;
    memset(burst_tasks, 0, sizeof(burst_tasks));

    syn_task_create(&burst_tasks[0], "crit_idle", continue_task, SYN_PRIO_CRITICAL, &id_crit);
    syn_task_create(&burst_tasks[1], "crit_burst", burst_wake_driver_fn, SYN_PRIO_CRITICAL,
                    &burst_ctx);
    syn_task_create(&burst_tasks[2], "low_wake", continue_task, SYN_PRIO_LOW, &id_low);
    syn_task_create(&burst_tasks[3], "crit_wake", continue_task, SYN_PRIO_CRITICAL, &id_crit);
    syn_sched_init(&burst_sched, burst_tasks, 4);

    syn_task_suspend(&burst_tasks[0]); /* ensures share_watch_wakeups == true */
    syn_task_suspend(&burst_tasks[2]);
    syn_task_suspend(&burst_tasks[3]);

    /* Waking LOW task on iter 3 does NOT cause CRITICAL burst to yield */
    burst_ctx.resume_target = &burst_tasks[2];
    syn_sched_run(&burst_sched);
    TEST_ASSERT_EQUAL_UINT32(10U, burst_ctx.iters);

    /* Waking equal-priority CRITICAL task on iter 4 DOES cause CRITICAL burst to yield */
    syn_task_suspend(&burst_tasks[2]);
    syn_task_suspend(&burst_tasks[3]);
    syn_task_restart(&burst_tasks[1]);
    burst_ctx.iters = 0U;
    burst_ctx.resume_target = &burst_tasks[3];
    burst_ctx.resume_on_iter = 4U;
    syn_sched_run(&burst_sched);
    TEST_ASSERT_EQUAL_UINT32(4U, burst_ctx.iters);
}

static SYN_PT_Status wrap_delay_task_fn(SYN_PT *pt, SYN_Task *task)
{
    int *done = (int *)task->user_data;
    PT_BEGIN(pt);
    PT_TASK_DELAY_MS(pt, task, 16U);
    (*done)++;
    PT_END(pt);
}

static void test_t03_tick_wraparound_sched_and_timeouts(void)
{
    /* 1. PT_TASK_DELAY_MS wraparound across 0xFFFFFFFAU -> 0x0000000AU */
    mock_tick_ms = 0xFFFFFFFAU;
    int delay_done = 0;
    SYN_Task dly_task = {0};
    SYN_Sched dly_sched;
    syn_task_create(&dly_task, "wrap_dly", wrap_delay_task_fn, SYN_PRIO_HIGH, &delay_done);
    syn_sched_init(&dly_sched, &dly_task, 1);

    syn_sched_run(&dly_sched); /* arms delay_until = 0x0000000AU */
    TEST_ASSERT_EQUAL_UINT32(0x0000000AU, dly_task.delay_until);

    mock_tick_ms = 0xFFFFFFFEU;
    syn_sched_run(&dly_sched);
    TEST_ASSERT_EQUAL_INT(0, delay_done);

    mock_tick_ms = 0x00000009U;
    syn_sched_run(&dly_sched);
    TEST_ASSERT_EQUAL_INT(0, delay_done);

    mock_tick_ms = 0x0000000AU;
    syn_sched_run(&dly_sched);
    TEST_ASSERT_EQUAL_INT(1, delay_done);

    /* 2. PT_BLOCK_EVENT_WITH_TIMEOUT wraparound across 0xFFFFFFFAU -> 0x00000010U */
    mock_tick_ms = 0xFFFFFFFAU;
    SYN_EventGroup evt;
    syn_event_init(&evt);
    uint32_t matched = 0xFFU;
    EvtTimeoutCfg cfg = {&evt, 0x01U, 20U, &matched, 0};
    SYN_Task tmo_task = {0};
    SYN_Sched tmo_sched;
    syn_task_create(&tmo_task, "wrap_tmo", evt_timeout_cfg_task_fn, SYN_PRIO_HIGH, &cfg);
    syn_sched_init(&tmo_sched, &tmo_task, 1);

    syn_sched_run(&tmo_sched); /* delay_until = 0x0000000EU */
    TEST_ASSERT_EQUAL_UINT32(0x0000000EU, tmo_task.delay_until);

    mock_tick_ms = 0x00000005U;
    syn_sched_run(&tmo_sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tmo_task.state);

    mock_tick_ms = 0x00000010U;
    syn_sched_run(&tmo_sched);
    TEST_ASSERT_EQUAL_INT(1, cfg.completions);
    TEST_ASSERT_EQUAL_UINT32(0U, matched);

    /* 3. Starvation due_ms wraparound across 0xFFFFFFFAU -> 0x00000010U (+ max_rem sleep fold) */
    mock_tick_ms = 0xFFFFFFFAU;
    log_reset();
    static int id_hi = 1;
    static int id_lo = 2;
    SYN_Task tasks[3];
    SYN_Sched sched;
    memset(tasks, 0, sizeof(tasks));
    syn_task_create(&tasks[0], "hi", continue_task, SYN_PRIO_HIGH, &id_hi);
    syn_task_create(&tasks[1], "lo", continue_task, SYN_PRIO_LOW, &id_lo);
    syn_task_create(&tasks[2], "far_sleep", continue_task, SYN_PRIO_LOW, &id_lo);
    syn_sched_init(&sched, tasks, 3);
    syn_sched_set_max_wait(&sched, SYN_PRIO_HIGH, 5U);
    syn_sched_set_max_wait(&sched, SYN_PRIO_LOW, 16U);
    tasks[2].delay_until = mock_tick_ms + (uint32_t)INT32_MAX;

    syn_sched_run(&sched); /* hi runs, lo stamped due_ms = 0x0000000AU */
    TEST_ASSERT_EQUAL_UINT32(0x0000000AU, tasks[1].due_ms);

    mock_tick_ms = 0x00000005U;
    log_reset();
    syn_sched_run(&sched); /* hi overdue (due=0xFFFFFFFF), lo not yet overdue -> hi runs */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);

    mock_tick_ms = 0x00000010U;
    log_reset();
    syn_sched_run(&sched); /* both overdue at 0x10: hi runs first */
    syn_sched_run(&sched); /* lo still overdue at 0x10: lo preempts hi! */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);
}

/** R3-K01: Overdue lower-prio ready peer + max-clamped sleeper must not wrap share_deadline_ms. */
static void test_r3_k01_overdue_peer_with_max_clamped_sleeper(void)
{
    static int id_low = 3;
    BurstWakeCtx burst_ctx = {0U, 10U, 0U, NULL, 0U, NULL, 0U};
    SYN_Task tasks[3];
    SYN_Sched sched;

    memset(tasks, 0, sizeof(tasks));
    mock_tick_ms = 1000U;

    syn_task_create(&tasks[0], "hi_burst", burst_wake_driver_fn, SYN_PRIO_HIGH, &burst_ctx);
    syn_task_create(&tasks[1], "lo_overdue", continue_task, SYN_PRIO_LOW, &id_low);
    syn_task_create(&tasks[2], "lo_max_sleep", continue_task, SYN_PRIO_LOW, &id_low);
    syn_sched_init(&sched, tasks, 3);

    /* Both tasks[0] (HIGH) and tasks[1] (LOW) are overdue; HIGH wins selection by priority.
     * tasks[2] sleeps for 0x7FFFFFFF ms (candidate = 1000 + 0x7FFFFFFF = 0x800003E7).
     * Clamping tasks[1].due_ms (999) to now (1000) prevents signed subtraction wraparound! */
    tasks[0].due_ms = 998U;
    tasks[0].sched_flags = (uint8_t)(SYN_SCHED_FLAG_MORE | SYN_SCHED_FLAG_DUE_VALID);
    tasks[1].due_ms = 999U;
    tasks[1].sched_flags = (uint8_t)(SYN_SCHED_FLAG_MORE | SYN_SCHED_FLAG_DUE_VALID);
    tasks[2].delay_until = 1000U + (uint32_t)INT32_MAX;

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT32(1000U, sched.share_deadline_ms);
    TEST_ASSERT_EQUAL_UINT32(1U, burst_ctx.iters);
}

/** R3-K12: Post-init syn_task_create() binds task->sched in sched_prepare_task before dispatch. */
static void test_r3_k12_post_init_task_create_binds_sched_before_dispatch(void)
{
    static int id_hi = 1;
    static int id_lo = 2;
    SYN_Task tasks[2];
    SYN_Sched sched;

    memset(tasks, 0, sizeof(tasks));
    mock_tick_ms = 100U;

    syn_task_create(&tasks[0], "hi_runner", continue_task, SYN_PRIO_HIGH, &id_hi);
    tasks[1].state = (uint8_t)SYN_TASK_DEAD;
    syn_sched_init(&sched, tasks, 2);

    /* Slot 1 starts DEAD; sched_prepare_task still binds tasks[1].sched */
    tasks[1].sched = NULL;
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_PTR(&sched, tasks[1].sched);

    /* Post-init syn_task_create resets tasks[1].sched = NULL */
    syn_task_create(&tasks[1], "late_low", continue_task, SYN_PRIO_LOW, &id_lo);
    TEST_ASSERT_NULL(tasks[1].sched);

    /* Tick at t=100: tasks[0] (HIGH) wins dispatch, while sched_prepare_task binds tasks[1].sched
     * and stamps tasks[1].due_ms = 100 + 50 = 150 before tasks[1] ever dispatches! */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_PTR(&sched, tasks[1].sched);
    TEST_ASSERT_EQUAL_UINT32(150U, tasks[1].due_ms);

    /* Boosting tasks[1] before its first dispatch tightens due_ms to CRITICAL window (100 + 1) */
    syn_task_boost_priority(&tasks[1], SYN_PRIO_CRITICAL);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT32(101U, tasks[1].due_ms);

    /* Monotonic ratchet: boosting to lower priority (HIGH > CRITICAL) is ignored */
    syn_task_boost_priority(&tasks[1], SYN_PRIO_HIGH);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[1].priority);
}

typedef struct {
    SYN_Mutex *m1;
    SYN_Mutex *m2;
    SYN_Task *peer_target;
    int action;
    int yielded_early;
} R4SchedMutexCtx;

static SYN_PT_Status r4_trylock_then_delay_fn(SYN_PT *pt, SYN_Task *task)
{
    R4SchedMutexCtx *ctx = (R4SchedMutexCtx *)task->user_data;
    PT_BEGIN(pt);
    (void)syn_mutex_try_lock(ctx->m1, task);
    PT_TASK_DELAY_MS(pt, task, 25U);
    PT_END(pt);
}

static SYN_PT_Status r4_share_mutation_runner_fn(SYN_PT *pt, SYN_Task *task)
{
    R4SchedMutexCtx *ctx = (R4SchedMutexCtx *)task->user_data;
    PT_BEGIN(pt);
    if (ctx->action == 1) {
        syn_task_boost_priority(ctx->peer_target, SYN_PRIO_CRITICAL);
    } else if (ctx->action == 2) {
        syn_task_set_base_priority(ctx->peer_target, SYN_PRIO_HIGH);
    } else if (ctx->action == 3) {
        syn_task_restart(ctx->peer_target);
    } else if (ctx->action == 4) {
        syn_task_restore_priority(task);
    }
    PT_CONTINUE_IF_NEEDED(pt, task);
    ctx->yielded_early = 0;
    PT_END(pt);
}

/** R4-K01, R4-K03, R4-K04: Task size invariant, stale waiter drop, and share refresh. */
static void test_r4_sched_mutex_wait_lifecycle_and_share_refresh(void)
{
    const size_t expected_task_size = (sizeof(void *) == 8U) ? 80U : 44U;
    TEST_ASSERT_EQUAL_UINT32((uint32_t)expected_task_size, (uint32_t)sizeof(SYN_Task));

    /* 1. Bound CRITICAL task fails PT_MUTEX_TRYLOCK then calls PT_TASK_DELAY_MS:
     * sched_finish_dispatch clears MUTEX_WAIT and drops stale boost back to HIGH. */
    SYN_Mutex m1;
    SYN_Mutex m2;
    SYN_Task tasks[3];
    SYN_Sched sched;
    R4SchedMutexCtx ctx = {&m1, &m2, NULL, 0, 1};

    PT_MUTEX_INIT(&m1);
    PT_MUTEX_INIT(&m2);
    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "crit_dly", r4_trylock_then_delay_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "high_w", pass_task, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[2], "low_own", pass_task, SYN_PRIO_LOW, NULL);
    syn_sched_init(&sched, tasks, 3);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m1, &tasks[2]));
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[2].priority);

    /* Run crit_dly: fails trylock then delays -> drops owner boost back to HIGH */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[2].priority);
    TEST_ASSERT_NULL(m1.waiters[1]);

    /* Bound suspend and restart on contended waiters immediately recompute owner */
    syn_task_restart(&tasks[0]);
    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[2].priority);
    syn_task_restart(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[2].priority);

    TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(&m1, &tasks[0]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_CRITICAL, tasks[2].priority);
    syn_task_suspend(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_HIGH, tasks[2].priority);

    /* Acquiring unlocked m2 while tasks[1] had MUTEX_WAIT on m1 drops low_own to LOW */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m2, &tasks[1]));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[2].priority);
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_unlock(&m2, &tasks[1]));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_unlock(&m1, &tasks[2]));

    /* 2. Mid-dispatch boost, set_base_priority, restart, and restore refresh share */
    for (int act = 1; act <= 4; act++) {
        SYN_Task s_tasks[2];
        SYN_Sched s_sched;
        R4SchedMutexCtx s_ctx = {NULL, NULL, &s_tasks[1], act, 1};
        SYN_Prio peer_prio = (act == 4) ? SYN_PRIO_HIGH : SYN_PRIO_LOW;
        mock_tick_ms = 0U;
        syn_task_create(&s_tasks[0], "runner", r4_share_mutation_runner_fn, SYN_PRIO_NORMAL,
                        &s_ctx);
        syn_task_create(&s_tasks[1], "peer", pass_task, peer_prio, NULL);
        syn_sched_init(&s_sched, s_tasks, 2);
        if (act == 3) {
            syn_task_suspend(&s_tasks[1]);
            s_tasks[1].base_priority = SYN_PRIO_HIGH;
        } else if (act == 4) {
            syn_task_boost_priority(&s_tasks[0], SYN_PRIO_CRITICAL);
        }
        syn_sched_run(&s_sched);
        TEST_ASSERT_EQUAL_INT(1, s_ctx.yielded_early);
    }
}

typedef struct {
    SYN_Task *low_cond_peer;
    SYN_EventGroup *low_evt;
    volatile int cond_ready;
    uint32_t crit_iters;
    int low_cond_runs;
    int low_evt_runs;
} LowIsrWakeCtx;

static SYN_PT_Status low_cond_woken_fn(SYN_PT *pt, SYN_Task *task)
{
    LowIsrWakeCtx *ctx = (LowIsrWakeCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_CONDITION(pt, task, ctx->cond_ready != 0);
    ctx->low_cond_runs++;
    PT_END(pt);
}

static SYN_PT_Status low_evt_woken_fn(SYN_PT *pt, SYN_Task *task)
{
    LowIsrWakeCtx *ctx = (LowIsrWakeCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, ctx->low_evt, 0x01U);
    ctx->low_evt_runs++;
    PT_END(pt);
}

static SYN_PT_Status crit_share_vs_low_isr_fn(SYN_PT *pt, SYN_Task *task)
{
    LowIsrWakeCtx *ctx = (LowIsrWakeCtx *)task->user_data;
    PT_BEGIN(pt);
    while (ctx->low_cond_runs == 0 || ctx->low_evt_runs == 0) {
        ctx->crit_iters++;
        mock_tick_ms += 1U;
        if (ctx->crit_iters == 3U) {
            ctx->cond_ready = 1;
            syn_task_resume(ctx->low_cond_peer);
        } else if (ctx->crit_iters == 60U) {
            syn_event_set(ctx->low_evt, 0x01U);
        }
        TEST_ASSERT_TRUE(ctx->crit_iters < 200U);
        PT_CONTINUE_IF_NEEDED(pt, task);
    }
    PT_END(pt);
}

/** Verify lower-priority ISR/event wakeups and boosted round-robin fairness. */
static void test_low_prio_isr_wakeup_and_boosted_rr_fairness(void)
{
    /* 1. CRITICAL PT_CONTINUE_IF_NEEDED task notices ISR-woken and event-woken LOW peers,
     * stamps their due_ms, and dispatches each within max_wait_ms[LOW] (50 ms). */
    SYN_EventGroup evt;
    syn_event_init(&evt);
    LowIsrWakeCtx ctx = {NULL, &evt, 0, 0U, 0, 0};
    SYN_Task tasks[3];
    SYN_Sched sched;

    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "crit_loop", crit_share_vs_low_isr_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "low_cond", low_cond_woken_fn, SYN_PRIO_LOW, &ctx);
    syn_task_create(&tasks[2], "low_evt", low_evt_woken_fn, SYN_PRIO_LOW, &ctx);
    ctx.low_cond_peer = &tasks[1];
    syn_sched_init(&sched, tasks, 3);

    syn_task_suspend(&tasks[0]);
    syn_sched_run(&sched); /* low_cond parks in SYN_TASK_BLOCKED */
    syn_sched_run(&sched); /* low_evt parks in SYN_TASK_BLOCKED */
    syn_task_resume(&tasks[0]);

    int guard = 0;
    while ((ctx.low_cond_runs == 0 || ctx.low_evt_runs == 0) && guard++ < 20) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_INT(1, ctx.low_cond_runs);
    TEST_ASSERT_EQUAL_INT(1, ctx.low_evt_runs);
    TEST_ASSERT_TRUE(mock_tick_ms <= 115U);

    /* 2. Boosted LOW task running at CRITICAL advances rr_per_prio[CRITICAL] fairly */
    static int id0 = 100, id1 = 200, id2 = 300;
    SYN_Task rr_tasks[3];
    SYN_Sched rr_sched;
    mock_tick_ms = 0U;
    log_reset();
    syn_task_create(&rr_tasks[0], "c0", continue_task, SYN_PRIO_CRITICAL, &id0);
    syn_task_create(&rr_tasks[1], "c1", continue_task, SYN_PRIO_CRITICAL, &id1);
    syn_task_create(&rr_tasks[2], "boosted", continue_task, SYN_PRIO_LOW, &id2);
    syn_sched_init(&rr_sched, rr_tasks, 3);
    syn_task_boost_priority(&rr_tasks[2], SYN_PRIO_CRITICAL);

    for (int step = 0; step < 6; step++) {
        syn_sched_run(&rr_sched);
    }
    TEST_ASSERT_EQUAL_INT(100, run_log[0]);
    TEST_ASSERT_EQUAL_INT(200, run_log[1]);
    TEST_ASSERT_EQUAL_INT(300, run_log[2]);
    TEST_ASSERT_EQUAL_INT(100, run_log[3]);
    TEST_ASSERT_EQUAL_INT(200, run_log[4]);
    TEST_ASSERT_EQUAL_INT(300, run_log[5]);
}

typedef struct {
    SYN_Task *noise_peer;
    SYN_EventGroup *evt;
    uint32_t steps;
    int low_ran;
} RefreshDueCtx;

static SYN_PT_Status low_due_preserve_fn(SYN_PT *pt, SYN_Task *task)
{
    RefreshDueCtx *ctx = (RefreshDueCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, ctx->evt, 0x01U);
    ctx->low_ran++;
    PT_END(pt);
}

static SYN_PT_Status crit_refresh_noise_fn(SYN_PT *pt, SYN_Task *task)
{
    RefreshDueCtx *ctx = (RefreshDueCtx *)task->user_data;
    PT_BEGIN(pt);
    while (ctx->low_ran == 0) {
        ctx->steps++;
        mock_tick_ms += 5U;
        if (ctx->steps == 2U) {
            syn_event_set(ctx->evt, 0x01U); /* Wakes low peer at t=10 */
        }
        /* Repeatedly trigger sched_refresh_share_locked every 5 ms */
        syn_task_boost_priority(ctx->noise_peer, SYN_PRIO_HIGH);
        syn_task_restore_priority(ctx->noise_peer);
        TEST_ASSERT_TRUE(ctx->steps <= 20U);
        PT_CONTINUE_IF_NEEDED(pt, task);
    }
    PT_END(pt);
}

static SYN_PT_Status delay_20ms_record_fn(SYN_PT *pt, SYN_Task *task)
{
    int *id = (int *)task->user_data;
    PT_BEGIN(pt);
    PT_TASK_DELAY_MS(pt, task, 20U);
    run_log[run_log_idx++] = *id;
    PT_END(pt);
}

static void test_mid_dispatch_refresh_preserves_low_peer_due_and_delay_base(void)
{
    /* 1. Repeated mid-dispatch share refreshes must not push woken LOW peer's due_ms forward */
    SYN_EventGroup evt;
    syn_event_init(&evt);
    RefreshDueCtx ctx = {NULL, &evt, 0U, 0};
    SYN_Task tasks[3];
    SYN_Sched sched;
    int noise_id = 99;

    mock_tick_ms = 0U;
    log_reset();
    syn_task_create(&tasks[0], "crit_noise", crit_refresh_noise_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "low_target", low_due_preserve_fn, SYN_PRIO_LOW, &ctx);
    syn_task_create(&tasks[2], "idle_peer", pass_task, SYN_PRIO_NORMAL, &noise_id);
    ctx.noise_peer = &tasks[2];
    syn_sched_init(&sched, tasks, 3U);

    syn_task_suspend(&tasks[0]);
    syn_sched_run(&sched); /* low_target parks on evt */
    syn_sched_run(&sched); /* idle_peer passes */
    syn_task_suspend(&tasks[2]);
    syn_task_resume(&tasks[0]);

    int guard = 0;
    while (ctx.low_ran == 0 && guard++ < 10) {
        syn_sched_run(&sched);
    }
    TEST_ASSERT_EQUAL_INT(1, ctx.low_ran);
    /* Woken at t=10 with LOW max_wait=50 -> must run at t=60, not starve past t=60 */
    TEST_ASSERT_EQUAL_UINT32(60U, mock_tick_ms);

    /* 2. Expired PT_TASK_DELAY_MS computes due_ms from delay_until, preempting HIGH when overdue */
    SYN_Task d_tasks[2];
    SYN_Sched d_sched;
    int high_id = 11;
    int low_id = 22;

    mock_tick_ms = 0U;
    log_reset();
    syn_task_create(&d_tasks[0], "high_work", continue_task, SYN_PRIO_HIGH, &high_id);
    syn_task_create(&d_tasks[1], "low_dly", delay_20ms_record_fn, SYN_PRIO_LOW, &low_id);
    syn_sched_init(&d_sched, d_tasks, 2U);

    syn_task_suspend(&d_tasks[0]);
    syn_sched_run(&d_sched); /* low_dly arms delay_until = 20 */

    /* Resume high_work at t=74 (due_ms = 74 + 5 = 79, WORK rank), then step at t=75:
     * low_dly (20 + 50 = 70 <= 75) is OVERDUE relative to wakeup tick 20 and preempts high_work! */
    mock_tick_ms = 74U;
    syn_task_resume(&d_tasks[0]);
    mock_tick_ms = 75U;
    syn_sched_run(&d_sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);
    TEST_ASSERT_EQUAL_INT(22, run_log[0]);
}

typedef struct {
    SYN_EventGroup *evt;
    bool share_before_isr;
    bool share_after_isr;
    int w0_done;
    int w1_done;
} SharedEvtResleepCtx;

static SYN_PT_Status shared_evt_w0_fn(SYN_PT *pt, SYN_Task *task)
{
    SharedEvtResleepCtx *ctx = (SharedEvtResleepCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, ctx->evt, 0x01U);
    ctx->share_before_isr = syn_task_should_share(task);
    syn_event_set(ctx->evt, 0x01U); /* Simulate ISR re-setting auto-cleared flag mid-dispatch */
    ctx->share_after_isr = syn_task_should_share(task);
    ctx->w0_done = 1;
    PT_END(pt);
}

static SYN_PT_Status shared_evt_w1_fn(SYN_PT *pt, SYN_Task *task)
{
    SharedEvtResleepCtx *ctx = (SharedEvtResleepCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT(pt, task, ctx->evt, 0x01U);
    ctx->w1_done = 1;
    PT_END(pt);
}

static void test_shared_event_flags_autoclear_resleeps_loser_for_isr_rewake(void)
{
    SYN_EventGroup evt;
    syn_event_init(&evt);
    SharedEvtResleepCtx ctx = {&evt, true, false, 0, 0};
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "w0", shared_evt_w0_fn, SYN_PRIO_HIGH, &ctx);
    syn_task_create(&tasks[1], "w1", shared_evt_w1_fn, SYN_PRIO_HIGH, &ctx);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched); /* w0 parks on evt */
    syn_sched_run(&sched); /* w1 parks on evt */

    syn_event_set(&evt, 0x01U);
    syn_sched_run(&sched); /* w0 wins, auto-clears 0x01U, w1 re-sleeps; ISR sets 0x01U again */
    TEST_ASSERT_EQUAL_INT(1, ctx.w0_done);
    TEST_ASSERT_FALSE(ctx.share_before_isr);
    TEST_ASSERT_TRUE(ctx.share_after_isr);

    syn_sched_run(&sched); /* w1 consumes second event */
    TEST_ASSERT_EQUAL_INT(1, ctx.w1_done);
}

typedef struct {
    SYN_EventGroup *evt;
    bool share_when_unready;
    bool share_when_set;
} ResumeEvtShareCtx;

static SYN_PT_Status crit_evt_timeout_waiter_fn(SYN_PT *pt, SYN_Task *task)
{
    ResumeEvtShareCtx *ctx = (ResumeEvtShareCtx *)task->user_data;
    PT_BEGIN(pt);
    PT_BLOCK_EVENT_WITH_TIMEOUT(pt, task, ctx->evt, 0x01U, 100U, NULL);
    PT_END(pt);
}

static SYN_PT_Status high_check_resumed_evt_share_fn(SYN_PT *pt, SYN_Task *task)
{
    ResumeEvtShareCtx *ctx = (ResumeEvtShareCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->share_when_unready = syn_task_should_share(task);
    syn_event_set(ctx->evt, 0x01U);
    ctx->share_when_set = syn_task_should_share(task);
    PT_END(pt);
}

static void test_suspend_resume_on_event_wait_and_expired_delay(void)
{
    /* 1. Suspended + resumed PT_BLOCK_EVENT_TIMEOUT task stays READY per API contract,
     * does not cause false should_share() while event == 0, and triggers should_share() on ISR set
     */
    SYN_EventGroup evt;
    syn_event_init(&evt);
    ResumeEvtShareCtx ctx = {&evt, true, false};
    SYN_Task tasks[2];
    SYN_Sched sched;

    mock_tick_ms = 10U;
    syn_task_create(&tasks[0], "crit_evt", crit_evt_timeout_waiter_fn, SYN_PRIO_CRITICAL, &ctx);
    syn_task_create(&tasks[1], "high_chk", high_check_resumed_evt_share_fn, SYN_PRIO_HIGH, &ctx);
    syn_sched_init(&sched, tasks, 2U);

    syn_sched_run(&sched); /* crit_evt arms wait_event + delay_until=110 and blocks */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    syn_task_suspend(&tasks[0]);
    mock_tick_ms = 20U;
    syn_task_resume(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_DUE_VALID);

    syn_sched_run(&sched); /* crit_evt is unready (re-parks), high_chk runs */
    TEST_ASSERT_FALSE(ctx.share_when_unready);
    TEST_ASSERT_TRUE(ctx.share_when_set);

    /* 2. Suspend + resume after PT_TASK_DELAY_MS deadline expired stamps DUE_VALID immediately */
    SYN_Task d_task;
    SYN_Sched d_sched;
    int id = 7;
    mock_tick_ms = 0U;
    syn_task_create(&d_task, "dly", delay_20ms_record_fn, SYN_PRIO_LOW, &id);
    syn_sched_init(&d_sched, &d_task, 1U);
    syn_sched_run(&d_sched); /* delay_until = 20 */
    syn_task_suspend(&d_task);
    mock_tick_ms = 30U;
    syn_task_resume(&d_task);
    TEST_ASSERT_TRUE((d_task.sched_flags & SYN_SCHED_FLAG_DUE_VALID) != 0U);
    TEST_ASSERT_EQUAL_UINT32(80U, d_task.due_ms);
}

typedef struct {
    SYN_Mutex *mtx;
    int mode;
    int runs;
} SelfRestartCtx;

static SYN_PT_Status self_restart_task_fn(SYN_PT *pt, SYN_Task *task)
{
    SelfRestartCtx *ctx = (SelfRestartCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->runs++;
    if (ctx->runs == 1) {
        if (ctx->mtx != NULL) {
            TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(ctx->mtx, task));
        }
        syn_task_restart(task);
        if (ctx->mode == 0) {
            PT_EXIT(pt);
        } else if (ctx->mode == 1) {
            PT_PASS(pt);
        }
    }
    PT_END(pt);
}

static void test_task_self_restart_preserves_ready_more_state(void)
{
    for (int mode = 0; mode < 3; mode++) {
        SYN_Mutex mtx;
        PT_MUTEX_INIT(&mtx);
        SelfRestartCtx ctx = {&mtx, mode, 0};
        SYN_Task task;
        SYN_Sched sched;

        mock_tick_ms = 0U;
        syn_task_create(&task, "self_rst", self_restart_task_fn, SYN_PRIO_NORMAL, &ctx);
        syn_sched_init(&sched, &task, 1U);

        /* First run: locks mtx, calls syn_task_restart(task), returns EXITED/IDLE/ENDED */
        TEST_ASSERT_TRUE(syn_sched_run(&sched));
        TEST_ASSERT_EQUAL_INT(1, ctx.runs);
        TEST_ASSERT_FALSE(syn_mutex_is_locked(&mtx));
        TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, task.state);
        TEST_ASSERT_TRUE((task.sched_flags & SYN_SCHED_FLAG_MORE) != 0U);
        TEST_ASSERT_EQUAL_UINT8(0U, task.sched_flags & SYN_SCHED_FLAG_RESTARTED);
        TEST_ASSERT_EQUAL_UINT16(0U, task.pt.lc);

        /* Second run: starts cleanly from PT_BEGIN and runs to PT_END -> DEAD */
        TEST_ASSERT_TRUE(syn_sched_run(&sched));
        TEST_ASSERT_EQUAL_INT(2, ctx.runs);
        TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, task.state);
    }
}

typedef struct {
    SYN_Mutex *m_owned;
    SYN_Mutex *m_busy;
    int runs;
} SelfRestartPostOpsCtx;

static SYN_PT_Status self_restart_post_ops_fn(SYN_PT *pt, SYN_Task *task)
{
    SelfRestartPostOpsCtx *ctx = (SelfRestartPostOpsCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->runs++;
    if (ctx->runs == 1) {
        syn_task_restart(task);
        /* Operations performed after syn_task_restart(task) before yielding must be cleaned up */
        TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(ctx->m_owned, task));
        syn_task_boost_priority(task, SYN_PRIO_CRITICAL);
        TEST_ASSERT_EQUAL_INT(SYN_BUSY, syn_mutex_try_lock(ctx->m_busy, task));
        PT_PASS(pt);
    }
    PT_END(pt);
}

static void test_boost_priority_ignores_non_higher_and_self_restart_post_ops(void)
{
    /* 1. syn_task_boost_priority ignores equal/lower temp_prio without latching manual_priority */
    SYN_Task t;
    SYN_Sched s;
    int id = 1;
    mock_tick_ms = 0U;
    syn_task_create(&t, "hi", pass_task, SYN_PRIO_HIGH, &id);
    syn_sched_init(&s, &t, 1U);

    syn_task_boost_priority(&t, SYN_PRIO_NORMAL);
    syn_task_boost_priority(&t, SYN_PRIO_HIGH);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_NO_MANUAL_PRIO, t.manual_priority);

    syn_task_set_base_priority(&t, SYN_PRIO_LOW);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, t.priority);

    /* 2. Post-restart mutex lock, contention, and manual boost in same step are cleaned up */
    SYN_Mutex m_owned;
    SYN_Mutex m_busy;
    PT_MUTEX_INIT(&m_owned);
    PT_MUTEX_INIT(&m_busy);

    SelfRestartPostOpsCtx ctx = {&m_owned, &m_busy, 0};
    SYN_Task tasks[2];
    SYN_Sched sched;
    syn_task_create(&tasks[0], "rst", self_restart_post_ops_fn, SYN_PRIO_NORMAL, &ctx);
    syn_task_create(&tasks[1], "holder", pass_task, SYN_PRIO_LOW, &id);
    syn_sched_init(&sched, tasks, 2U);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_try_lock(&m_busy, &tasks[1]));

    TEST_ASSERT_TRUE(syn_sched_run(&sched));
    TEST_ASSERT_EQUAL_INT(1, ctx.runs);
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&m_owned));
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_LOW, tasks[1].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_PRIO_NORMAL, tasks[0].priority);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_NO_MANUAL_PRIO, tasks[0].manual_priority);
    TEST_ASSERT_EQUAL_UINT8(0U, tasks[0].sched_flags & SYN_SCHED_FLAG_MUTEX_WAIT);
    TEST_ASSERT_EQUAL_UINT16(0U, tasks[0].pt.lc);

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_mutex_unlock(&m_busy, &tasks[1]));
}

static SYN_PT_Status exit_with_dirty_wait_fields_fn(SYN_PT *pt, SYN_Task *task)
{
    SYN_EventGroup *evt = (SYN_EventGroup *)task->user_data;
    PT_BEGIN(pt);
    task->wait_event = evt;
    task->wait_mask = 0x01U;
    task->delay_until = 50U;
    PT_EXIT(pt);
    PT_END(pt);
}

typedef struct {
    SYN_Task *low_dly;
    bool share_at_65;
    bool share_at_70;
} ResumedDelayPeerCtx;

static SYN_PT_Status crit_check_resumed_delay_fn(SYN_PT *pt, SYN_Task *task)
{
    ResumedDelayPeerCtx *ctx = (ResumedDelayPeerCtx *)task->user_data;
    PT_BEGIN(pt);
    mock_tick_ms = 10U;
    syn_task_resume(ctx->low_dly);
    mock_tick_ms = 65U;
    ctx->share_at_65 = syn_task_should_share(task);
    mock_tick_ms = 70U;
    ctx->share_at_70 = syn_task_should_share(task);
    PT_END(pt);
}

static void test_resleep_skips_dead_suspended_and_resumed_delay_wake_base(void)
{
    /* 1. PT_EXIT clears wait fields; sched_resleep_event_peers skips DEAD & SUSPENDED peers */
    SYN_EventGroup evt;
    syn_event_init(&evt);
    SharedEvtResleepCtx ectx = {&evt, false, false, 0, 0};
    SYN_Task tasks[3];
    SYN_Sched sched;

    mock_tick_ms = 0U;
    syn_task_create(&tasks[0], "exiter", exit_with_dirty_wait_fields_fn, SYN_PRIO_CRITICAL, &evt);
    syn_task_create(&tasks[1], "winner", shared_evt_w1_fn, SYN_PRIO_HIGH, &ectx);
    syn_task_create(&tasks[2], "susp", shared_evt_w1_fn, SYN_PRIO_HIGH, &ectx);
    syn_sched_init(&sched, tasks, 3U);

    syn_sched_run(&sched); /* exiter runs PT_EXIT -> clears wait_event/wait_mask/delay_until */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, tasks[0].state);
    TEST_ASSERT_NULL(tasks[0].wait_event);
    TEST_ASSERT_EQUAL_UINT32(0U, tasks[0].wait_mask);
    TEST_ASSERT_EQUAL_UINT32(0U, tasks[0].delay_until);

    syn_sched_run(&sched); /* winner parks on evt */
    syn_sched_run(&sched); /* susp parks on evt */
    syn_task_suspend(&tasks[2]);

    /* Defensively ensure DEAD and SUSPENDED peers with non-SLEEPING flags stay untouched */
    tasks[0].wait_event = &evt;
    tasks[0].wait_mask = 0x01U;
    tasks[2].sched_flags = 0U;

    syn_event_set(&evt, 0x01U);
    syn_sched_run(&sched); /* winner consumes evt and runs sched_resleep_event_peers */
    TEST_ASSERT_EQUAL_INT(1, ectx.w1_done);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_SUSPENDED, tasks[2].state);
    tasks[0].wait_event = NULL;

    /* 2. Resumed delayed LOW peer computes sched_peer_woke due_ms from delay_until (20+50=70) */
    ResumedDelayPeerCtx rctx = {NULL, true, false};
    SYN_Task d_tasks[2];
    SYN_Sched d_sched;
    int low_id = 42;

    mock_tick_ms = 0U;
    log_reset();
    syn_task_create(&d_tasks[0], "crit", crit_check_resumed_delay_fn, SYN_PRIO_CRITICAL, &rctx);
    syn_task_create(&d_tasks[1], "low_dly", delay_20ms_record_fn, SYN_PRIO_LOW, &low_id);
    rctx.low_dly = &d_tasks[1];
    syn_sched_init(&d_sched, d_tasks, 2U);

    syn_task_suspend(&d_tasks[0]);
    syn_sched_run(&d_sched); /* low_dly arms delay_until = 20 */
    syn_task_suspend(&d_tasks[1]);
    syn_task_resume(&d_tasks[0]);

    syn_sched_run(&d_sched);
    TEST_ASSERT_FALSE(rctx.share_at_65);
    TEST_ASSERT_TRUE(rctx.share_at_70);
}

typedef struct {
    SYN_Task *dead_slot;
    bool share_before;
    bool share_after;
    int respawn_runs;
} DeadSlotRespawnCtx;

static SYN_PT_Status respawned_peer_fn(SYN_PT *pt, SYN_Task *task)
{
    DeadSlotRespawnCtx *ctx = (DeadSlotRespawnCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->respawn_runs++;
    PT_END(pt);
}

static SYN_PT_Status spawner_low_fn(SYN_PT *pt, SYN_Task *task)
{
    DeadSlotRespawnCtx *ctx = (DeadSlotRespawnCtx *)task->user_data;
    PT_BEGIN(pt);
    ctx->share_before = syn_task_should_share(task);
    syn_task_create(ctx->dead_slot, "respawned", respawned_peer_fn, SYN_PRIO_CRITICAL, ctx);
    ctx->share_after = syn_task_should_share(task);
    PT_CONTINUE_IF_NEEDED(pt, task);
    PT_END(pt);
}

typedef struct {
    SYN_Sched *sched;
    SYN_Task *new_tasks;
    size_t new_count;
    SYN_Mutex *mtx;
    bool exit_task;
} ReinitSchedCtx;

static SYN_PT_Status reinit_sched_mid_dispatch_fn(SYN_PT *pt, SYN_Task *task)
{
    ReinitSchedCtx *ctx = (ReinitSchedCtx *)task->user_data;
    PT_BEGIN(pt);
    if (ctx->mtx != NULL) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_mutex_try_lock(ctx->mtx, task));
    }
    syn_sched_init(ctx->sched, ctx->new_tasks, ctx->new_count);
    if (!ctx->exit_task) {
        PT_PASS(pt);
    }
    PT_END(pt);
}

static SYN_PT_Status idle_keep_fn(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        PT_PASS(pt);
    }
    PT_END(pt);
}

static void test_dead_slot_respawn_suspended_wrap_and_mid_reinit(void)
{
    /* 1. Respawning a SYN_TASK_DEAD slot mid-dispatch triggers syn_task_should_share() */
    SYN_Task r_tasks[2];
    SYN_Sched r_sched;
    DeadSlotRespawnCtx rctx = {&r_tasks[1], true, false, 0};

    mock_tick_ms = 0U;
    syn_task_create(&r_tasks[0], "spawner", spawner_low_fn, SYN_PRIO_LOW, &rctx);
    syn_task_create(&r_tasks[1], "initial_dead", respawned_peer_fn, SYN_PRIO_CRITICAL, &rctx);
    syn_sched_init(&r_sched, r_tasks, 2U);

    /* Run 1: initial_dead runs to PT_END -> r_tasks[1].state == SYN_TASK_DEAD */
    TEST_ASSERT_TRUE(syn_sched_run(&r_sched));
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, r_tasks[1].state);
    TEST_ASSERT_EQUAL_INT(1, rctx.respawn_runs);

    /* Run 2: spawner runs while r_tasks[1] is DEAD, calls syn_task_create(&r_tasks[1], ...)
     * and sees syn_task_should_share() flip from false -> true immediately! */
    TEST_ASSERT_TRUE(syn_sched_run(&r_sched));
    TEST_ASSERT_FALSE(rctx.share_before);
    TEST_ASSERT_TRUE(rctx.share_after);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, r_tasks[0].state);

    /* Run 3: newly respawned CRITICAL peer preempts spawner cleanly */
    TEST_ASSERT_TRUE(syn_sched_run(&r_sched));
    TEST_ASSERT_EQUAL_INT(2, rctx.respawn_runs);

    /* 2. Suspended task with expired delay_until anchors across >24.85-day wraparound
     *    (covering both now == 0U -> 0xFFFFFFFFU and now != 0U branches) */
    SYN_Task w_task;
    SYN_Sched w_sched;
    int wake_id = 77;
    log_reset();

    mock_tick_ms = 0xFFFFFFF0U;
    syn_task_create(&w_task, "dly_susp", delayed_low_worker_fn, SYN_PRIO_HIGH, &wake_id);
    syn_sched_init(&w_sched, &w_task, 1U);

    syn_sched_run(&w_sched); /* w_task arms delay_until = 0xFFFFFFF5U */
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFF5U, w_task.delay_until);
    syn_task_suspend(&w_task);

    /* Tick wraps to exact 0: expired delay anchors to 0xFFFFFFFFU (avoiding 0 sentinel) */
    mock_tick_ms = 0U;
    syn_sched_run(&w_sched);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, w_task.delay_until);

    /* Advance past 2^31 ms in two 0x50000000U steps while suspended -> stays anchored! */
    mock_tick_ms = 0x50000000U;
    syn_sched_run(&w_sched);
    TEST_ASSERT_EQUAL_UINT32(0x50000000U, w_task.delay_until);

    mock_tick_ms = 0xA0000000U; /* > 2^31 ms past original 0xFFFFFFF5U! */
    syn_task_resume(&w_task);
    syn_sched_run(&w_sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);
    TEST_ASSERT_EQUAL_INT(77, run_log[0]);

    /* 3. Calling syn_sched_init(sched, ...) inside a running task exits post-dispatch safely */
    SYN_Task old_tasks[2];
    SYN_Task replacement_task;
    SYN_Sched m_sched;
    SYN_Mutex held_m;
    syn_mutex_init(&held_m);

    syn_task_create(&replacement_task, "repl", idle_keep_fn, SYN_PRIO_NORMAL, NULL);
    ReinitSchedCtx ictx1 = {&m_sched, &replacement_task, 1U, &held_m, true};
    syn_task_create(&old_tasks[0], "peer0", idle_keep_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&old_tasks[1], "reinit_exit", reinit_sched_mid_dispatch_fn, SYN_PRIO_CRITICAL,
                    &ictx1);
    syn_sched_init(&m_sched, old_tasks, 2U);

    TEST_ASSERT_TRUE(syn_sched_run(&m_sched));
    TEST_ASSERT_FALSE(syn_mutex_is_locked(&held_m));
    TEST_ASSERT_NULL(m_sched.running);
    TEST_ASSERT_EQUAL_size_t(1U, m_sched.task_count);

    /* Subcase 3b: reinit to empty scheduler (count = 0) while yielding PT_IDLE returns false */
    ReinitSchedCtx ictx2 = {&m_sched, NULL, 0U, NULL, false};
    syn_task_create(&old_tasks[0], "reinit_empty", reinit_sched_mid_dispatch_fn, SYN_PRIO_CRITICAL,
                    &ictx2);
    syn_sched_init(&m_sched, old_tasks, 1U);
    TEST_ASSERT_FALSE(syn_sched_run(&m_sched));
    TEST_ASSERT_NULL(m_sched.running);
    TEST_ASSERT_EQUAL_size_t(0U, m_sched.task_count);
}

void run_sched_tests(void)
{
    RUN_TEST(test_scheduler);
    RUN_TEST(test_suspend_resume);
    RUN_TEST(test_sched_empty);
    RUN_TEST(test_sched_delayed_task);
    RUN_TEST(test_sched_alive_count);
    RUN_TEST(test_sched_run_forever);
    RUN_TEST(test_pass_basic);
    RUN_TEST(test_pass_rr_fairness);
    RUN_TEST(test_rr_same_priority);
    RUN_TEST(test_strict_priority_within_window);
    RUN_TEST(test_pass_rr_three_lower);
    RUN_TEST(test_pass_flag_lifecycle);
    RUN_TEST(test_pass_then_suspend);
    RUN_TEST(test_block_event_basic);
    RUN_TEST(test_block_event_skips_scan);
    RUN_TEST(test_block_event_auto_clear);
    RUN_TEST(test_block_event_priority);
    RUN_TEST(test_block_event_multiple_tasks);
    RUN_TEST(test_block_event_suspend);
    RUN_TEST(test_waiting_doesnt_starve);
    RUN_TEST(test_waiting_with_defer_no_inversion);
    RUN_TEST(test_all_tasks_waiting);
    RUN_TEST(test_waiting_state_lifecycle);
    RUN_TEST(test_waiting_same_priority_fairness);
    RUN_TEST(test_waiting_delayed_no_inversion);
    RUN_TEST(test_task_priority_boost_and_restore);
    RUN_TEST(test_two_passing_high_pri_tasks_do_not_starve_lower);
    RUN_TEST(test_pt_delay_us);
    RUN_TEST(test_block_condition_and_primitives);
    RUN_TEST(test_block_condition_isr_wakeup_not_lost);
    RUN_TEST(test_block_event_auto_clear_is_atomic);
    RUN_TEST(test_block_event_clears_only_matched_flags);
    RUN_TEST(test_delay_deadline_consumed_no_wrap_freeze);
    RUN_TEST(test_delay_deadline_wrap_zero_alias);
    RUN_TEST(test_sched_next_wakeup_blocked_event_fired);
    RUN_TEST(test_sched_next_wakeup_uint32_max_and_prio_bounds);
    RUN_TEST(test_block_event_with_timeout_event_fired);
    RUN_TEST(test_block_event_with_timeout_expired);
    RUN_TEST(test_sched_next_wakeup_blocked_timeout);
    RUN_TEST(test_block_event_with_timeout_isr_race_post_timeout);
    RUN_TEST(test_sched_rr_uint16_rotation);
    RUN_TEST(test_two_critical_pollers_plus_low_worker);
    RUN_TEST(test_high_more_flood_vs_critical_poller);
    RUN_TEST(test_high_more_flood_vs_low_more);
    RUN_TEST(test_simultaneous_overdue_priority_order);
    RUN_TEST(test_n_pollers_once_per_round);
    RUN_TEST(test_task_should_share_and_continue_if_needed);
    RUN_TEST(test_pt_spawn_propagates_child_intent);
    RUN_TEST(test_sched_set_max_wait);
    RUN_TEST(test_waking_delayed_task_ranks_as_work);
    RUN_TEST(test_dual_sched_isolation_and_unbound_share);
    RUN_TEST(test_should_share_delayed_and_isr_wakeups);
    RUN_TEST(test_overdue_rr_preservation_and_bounds);
    RUN_TEST(test_task_restart_releases_recursive_contended_mutexes);
    RUN_TEST(test_priority_restore_and_set_base_preserve_mutex_boost);
    RUN_TEST(test_k02_event_blocking_edge_cases);
    RUN_TEST(test_k03_isr_wake_and_critical_burst_share);
    RUN_TEST(test_t03_tick_wraparound_sched_and_timeouts);
    RUN_TEST(test_r3_k01_overdue_peer_with_max_clamped_sleeper);
    RUN_TEST(test_r3_k12_post_init_task_create_binds_sched_before_dispatch);
    RUN_TEST(test_r4_sched_mutex_wait_lifecycle_and_share_refresh);
    RUN_TEST(test_low_prio_isr_wakeup_and_boosted_rr_fairness);
    RUN_TEST(test_mid_dispatch_refresh_preserves_low_peer_due_and_delay_base);
    RUN_TEST(test_shared_event_flags_autoclear_resleeps_loser_for_isr_rewake);
    RUN_TEST(test_suspend_resume_on_event_wait_and_expired_delay);
    RUN_TEST(test_task_self_restart_preserves_ready_more_state);
    RUN_TEST(test_boost_priority_ignores_non_higher_and_self_restart_post_ops);
    RUN_TEST(test_resleep_skips_dead_suspended_and_resumed_delay_wake_base);
    RUN_TEST(test_dead_slot_respawn_suspended_wrap_and_mid_reinit);
}
