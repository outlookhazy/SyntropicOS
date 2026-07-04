/**
 * @file test_sched.c
 * @brief Unity tests for syn_sched.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/util/syn_event.h"

static int sched_order[10];
static int sched_order_idx = 0;

static SYN_PT_Status sched_task_a(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    sched_order[sched_order_idx++] = 1;
    PT_YIELD(pt);
    sched_order[sched_order_idx++] = 1;

    PT_END(pt);
}

static SYN_PT_Status sched_task_b(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    sched_order[sched_order_idx++] = 2;
    PT_YIELD(pt);
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
        PT_YIELD(pt);
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
    TEST_ASSERT_TRUE(alive >= 1);
}

#include <setjmp.h>
static jmp_buf g_sched_jmp;

static SYN_PT_Status task_longjmp(SYN_PT *pt, SYN_Task *task)
{
    (void)pt; (void)task;
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

/* ── PT_DEFER and per-priority round-robin tests ─────────────────────── */

static int run_log[32];
static int run_log_idx;

static void log_reset(void) {
    run_log_idx = 0;
    memset(run_log, 0, sizeof(run_log));
}

/* Task that logs its ID and defers every call */
static SYN_PT_Status defer_task(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        run_log[run_log_idx++] = id;
        PT_DEFER(pt, task);
    }
    PT_END(pt);
}

/* Task that logs its ID and yields every call */
static SYN_PT_Status yield_task(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        run_log[run_log_idx++] = id;
        PT_YIELD(pt);
    }
    PT_END(pt);
}

/**
 * Basic defer: A (pri 0, defers) should let B (pri 1) run every other pass.
 * Expected pattern: A, B, A, B, ...
 */
static void test_defer_basic(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", defer_task, 0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 1, &id_b);
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
 * Per-priority round-robin: A (pri 0, defers) with B1, B2 (pri 1, yield).
 * B1 and B2 should alternate fairly: A, B1, A, B2, A, B1, ...
 */
static void test_defer_rr_fairness(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b1 = 2, id_b2 = 3;

    syn_task_create(&tasks[0], "a",  defer_task, 0, &id_a);
    syn_task_create(&tasks[1], "b1", yield_task, 1, &id_b1);
    syn_task_create(&tasks[2], "b2", yield_task, 1, &id_b2);
    syn_sched_init(&sched, tasks, 3);

    for (int i = 0; i < 8; i++) {
        syn_sched_run(&sched);
    }

    /* A, B1, A, B2, A, B1, A, B2 */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);  /* A */
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);  /* B1 */
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);  /* A */
    TEST_ASSERT_EQUAL_INT(3, run_log[3]);  /* B2 */
    TEST_ASSERT_EQUAL_INT(1, run_log[4]);  /* A */
    TEST_ASSERT_EQUAL_INT(2, run_log[5]);  /* B1 */
    TEST_ASSERT_EQUAL_INT(1, run_log[6]);  /* A */
    TEST_ASSERT_EQUAL_INT(3, run_log[7]);  /* B2 */
}

/**
 * Same-priority round-robin still works without defer.
 * Two pri-0 tasks that yield should alternate: A, B, A, B, ...
 */
static void test_rr_same_priority(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", yield_task, 0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 0, &id_b);
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
 * Strict priority without defer: A (pri 0, yields) starves B (pri 1).
 * This verifies that defer is needed and that strict priority is preserved.
 */
static void test_strict_priority_no_defer(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", yield_task, 0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    for (int i = 0; i < 4; i++) {
        syn_sched_run(&sched);
    }

    /* Only A runs — B starves (strict priority, no defer) */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);
    TEST_ASSERT_EQUAL_INT(1, run_log[1]);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);
    TEST_ASSERT_EQUAL_INT(1, run_log[3]);
}

/**
 * Defer with 3 lower-priority tasks: A (pri 0, defers), B1/B2/B3 (pri 1).
 * All three should get fair rotation: A, B1, A, B2, A, B3, A, B1, ...
 */
static void test_defer_rr_three_lower(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[4];
    SYN_Sched sched;
    static int id_a = 1, id_b1 = 2, id_b2 = 3, id_b3 = 4;

    syn_task_create(&tasks[0], "a",  defer_task, 0, &id_a);
    syn_task_create(&tasks[1], "b1", yield_task, 1, &id_b1);
    syn_task_create(&tasks[2], "b2", yield_task, 1, &id_b2);
    syn_task_create(&tasks[3], "b3", yield_task, 1, &id_b3);
    syn_sched_init(&sched, tasks, 4);

    for (int i = 0; i < 8; i++) {
        syn_sched_run(&sched);
    }

    /* A, B1, A, B2, A, B3, A, B1 */
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);  /* A */
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);  /* B1 */
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);  /* A */
    TEST_ASSERT_EQUAL_INT(3, run_log[3]);  /* B2 */
    TEST_ASSERT_EQUAL_INT(1, run_log[4]);  /* A */
    TEST_ASSERT_EQUAL_INT(4, run_log[5]);  /* B3 */
    TEST_ASSERT_EQUAL_INT(1, run_log[6]);  /* A */
    TEST_ASSERT_EQUAL_INT(2, run_log[7]);  /* B1 */
}

/**
 * DEFERRED state lifecycle: task defers → state = DEFERRED → skipped
 * one pass → cleared back to READY.
 */
static void test_defer_state_lifecycle(void)
{
    mock_tick_ms = 0;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", defer_task, 0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Pass 1: A runs and defers */
    log_reset();
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEFERRED, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY,    tasks[1].state);

    /* Pass 2: A skipped (DEFERRED), B runs, A cleared to READY */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[1].state);
}

/**
 * Defer is compatible with suspend: a deferred task can be suspended,
 * and when resumed it resumes as READY, not DEFERRED.
 */
static void test_defer_then_suspend(void)
{
    mock_tick_ms = 0;
    log_reset();

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "a", defer_task, 0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* A runs and defers */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEFERRED, tasks[0].state);

    /* Suspend A while it's deferred */
    syn_task_suspend(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_SUSPENDED, tasks[0].state);

    /* B should run while A is suspended */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);

    /* Resume A — should come back as READY */
    syn_task_resume(&tasks[0]);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
}

/* ── PT_BLOCK_EVENT tests ────────────────────────────────────────────────── */

#define EVT_DATA  SYN_BIT(0)
#define EVT_DONE  SYN_BIT(1)

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
    syn_task_create(&tasks[1], "low", yield_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Pass 1: high-pri task runs, blocks */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);

    /* Pass 2: high-pri blocked, low-pri runs */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);  /* Low-pri ran */
    TEST_ASSERT_EQUAL_INT(0, block_counter);  /* High-pri didn't */
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
    syn_task_create(&tasks[1], "low", yield_task, 1, &id_b);
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
    TEST_ASSERT_EQUAL_INT(1, block_counter);  /* High-pri ran */
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
    syn_sched_run(&sched);  /* Task A blocks */
    syn_sched_run(&sched);  /* Task B blocks */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[1].state);

    /* Only fire event A */
    syn_event_set(&evt_a, EVT_DATA);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, block_counter);  /* Only A woke */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, tasks[1].state);  /* B still blocked */

    /* Fire event B */
    syn_event_set(&evt_b, EVT_DATA);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, block_counter);  /* Now B woke too */
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

/* ── PT_WAITING retry tests ──────────────────────────────────────────────── */

static bool wait_cond_a;

/* Task that waits on a condition, logs ID when condition is true */
static SYN_PT_Status wait_task(SYN_PT *pt, SYN_Task *task)
{
    int id = *(int *)task->user_data;
    PT_BEGIN(pt);
    for (;;) {
        PT_WAIT_UNTIL(pt, wait_cond_a);
        run_log[run_log_idx++] = id;
        wait_cond_a = false;  /* consume the event */
        PT_YIELD(pt);
    }
    PT_END(pt);
}

/**
 * Waiting high-pri task doesn't starve lower-pri task.
 * A (pri 0) waits on false condition. B (pri 1) should run.
 */
static void test_waiting_doesnt_starve(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "w", wait_task,  0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 1, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* Tick 1: A runs, hits PT_WAIT_UNTIL (false) → WAITING.
     * Scheduler retries → B runs. */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log_idx);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);  /* B ran, not A */

    /* Tick 2: same pattern — B keeps running */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log_idx);
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);  /* B again */

    /* Tick 3: condition becomes true — A runs */
    wait_cond_a = true;
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log_idx);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);  /* A ran */
}

/**
 * PT_DEFER + PT_WAITING don't interfere.
 * A (pri 0, defers) → B (pri 1, waits) → C (pri 2, yields).
 * Expected: A, C, A, C, ...  (B never does useful work)
 */
static void test_waiting_with_defer_no_inversion(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2, id_c = 3;

    syn_task_create(&tasks[0], "a", defer_task, 0, &id_a);
    syn_task_create(&tasks[1], "w", wait_task,  1, &id_b);
    syn_task_create(&tasks[2], "c", yield_task, 2, &id_c);
    syn_sched_init(&sched, tasks, 3);

    /* Tick 1: A runs, defers */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[0]);  /* A */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEFERRED, tasks[0].state);

    /* Tick 2: A deferred → skipped. B waits → WAITING. C runs. 
     * A cleared to READY at end of tick. */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log[1]);  /* C */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);

    /* Tick 3: A is back — runs and defers again */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);  /* A */

    /* Tick 4: A deferred, B waits, C runs */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log[3]);  /* C */
}

/**
 * All tasks waiting — no infinite loop, all cleared to READY.
 */
static void test_all_tasks_waiting(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2, id_c = 3;

    syn_task_create(&tasks[0], "a", wait_task, 0, &id_a);
    syn_task_create(&tasks[1], "b", wait_task, 1, &id_b);
    syn_task_create(&tasks[2], "c", wait_task, 2, &id_c);
    syn_sched_init(&sched, tasks, 3);

    /* All tasks wait — should not hang, no log entries */
    bool alive = syn_sched_run(&sched);
    TEST_ASSERT_TRUE(alive);
    TEST_ASSERT_EQUAL_INT(0, run_log_idx);

    /* All should be READY again for next tick */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[1].state);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[2].state);
}

/**
 * WAITING state lifecycle: set during tick, cleared at end.
 */
static void test_waiting_state_lifecycle(void)
{
    mock_tick_ms = 0;
    wait_cond_a = false;

    SYN_Task tasks[1];
    SYN_Sched sched;
    static int id_a = 1;

    syn_task_create(&tasks[0], "w", wait_task, 0, &id_a);
    syn_sched_init(&sched, tasks, 1);

    /* After tick: task was WAITING during tick but cleared to READY */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, tasks[0].state);
}

/**
 * Same-priority: one waits, the other works. They should alternate
 * fairly once the waiting condition clears.
 */
static void test_waiting_same_priority_fairness(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[2];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2;

    syn_task_create(&tasks[0], "w", wait_task,  0, &id_a);
    syn_task_create(&tasks[1], "b", yield_task, 0, &id_b);
    syn_sched_init(&sched, tasks, 2);

    /* While A waits, B should run every tick */
    syn_sched_run(&sched);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(2, run_log[0]);  /* B */
    TEST_ASSERT_EQUAL_INT(2, run_log[1]);  /* B */

    /* Condition true: A runs (RR should give it a turn) */
    wait_cond_a = true;
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[2]);  /* A */
}

/**
 * Delayed prio-0 + waiting prio-1: no priority inversion.
 * A (pri 0) is delayed. B (pri 1) waits. C (pri 2) should run.
 * When A's delay expires, A runs first.
 */
static void test_waiting_delayed_no_inversion(void)
{
    mock_tick_ms = 0;
    log_reset();
    wait_cond_a = false;

    SYN_Task tasks[3];
    SYN_Sched sched;
    static int id_a = 1, id_b = 2, id_c = 3;

    syn_task_create(&tasks[0], "a", yield_task, 0, &id_a);
    syn_task_create(&tasks[1], "w", wait_task,  1, &id_b);
    syn_task_create(&tasks[2], "c", yield_task, 2, &id_c);
    syn_sched_init(&sched, tasks, 3);

    /* Delay A for 100ms */
    tasks[0].delay_until = mock_tick_ms + 100;

    /* Tick: A delayed, B waits, C runs */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(3, run_log[0]);  /* C, not B */

    /* Advance past A's delay */
    mock_tick_advance(150);

    /* Tick: A is ready (pri 0) → runs first */
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_INT(1, run_log[1]);  /* A */
}

void run_sched_tests(void)
{
    RUN_TEST(test_scheduler);
    RUN_TEST(test_suspend_resume);
    RUN_TEST(test_sched_empty);
    RUN_TEST(test_sched_delayed_task);
    RUN_TEST(test_sched_alive_count);
    RUN_TEST(test_sched_run_forever);
    RUN_TEST(test_defer_basic);
    RUN_TEST(test_defer_rr_fairness);
    RUN_TEST(test_rr_same_priority);
    RUN_TEST(test_strict_priority_no_defer);
    RUN_TEST(test_defer_rr_three_lower);
    RUN_TEST(test_defer_state_lifecycle);
    RUN_TEST(test_defer_then_suspend);
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
}

