/**
 * @file test_protothread.c
 * @brief Unity tests for syn_protothread.
 */

#include "mocks/mock_port.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

static int pt_basic_counter = 0;

static SYN_PT_Status pt_basic_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    pt_basic_counter = 1;
    PT_CONTINUE(pt);

    pt_basic_counter = 2;
    PT_CONTINUE(pt);

    pt_basic_counter = 3;

    PT_END(pt);
}

static void test_basic_protothread(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    pt_basic_counter = 0;

    SYN_PT_Status s;

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_MORE, s);
    TEST_ASSERT_EQUAL_INT(1, pt_basic_counter);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_MORE, s);
    TEST_ASSERT_EQUAL_INT(2, pt_basic_counter);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
    TEST_ASSERT_EQUAL_INT(3, pt_basic_counter);
}

static SYN_PT_Status pt_exit_func(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_EXIT(pt);
    PT_END(pt);
}

static void test_pt_exit_vs_end(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, pt_exit_func(&pt));
}

static int wait_condition = 0;

static SYN_PT_Status pt_wait_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_WAIT_UNTIL(pt, wait_condition);

    PT_END(pt);
}

static void test_wait_until(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    wait_condition = 0;

    SYN_PT_Status s;

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_IDLE, s);

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_IDLE, s);

    wait_condition = 1;
    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
}

static SYN_Task *g_block_cond_task = NULL;
static int g_cond_eval_count = 0;

static bool eval_cond_with_simulated_isr(void)
{
    g_cond_eval_count++;
    if (g_block_cond_task != NULL) {
        /* Verify task state is set to BLOCKED before condition evaluation */
        TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, g_block_cond_task->state);
        /* Simulate an ISR firing during condition check and calling syn_task_resume */
        syn_task_resume(g_block_cond_task);
        /* Resume must transition state to READY */
        TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, g_block_cond_task->state);
    }
    return false;
}

static SYN_PT_Status pt_block_cond_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    PT_BLOCK_CONDITION(pt, task, eval_cond_with_simulated_isr());
    PT_END(pt);
}

static void test_pt_block_condition_isr_race(void)
{
    SYN_Task task = {0};
    syn_task_create(&task, "block_race", pt_block_cond_func, SYN_PRIO_CRITICAL, NULL);
    g_block_cond_task = &task;
    g_cond_eval_count = 0;

    SYN_PT_Status s = task.func(&task.pt, &task);
    g_block_cond_task = NULL;
    TEST_ASSERT_EQUAL(PT_IDLE, s);
    TEST_ASSERT_EQUAL_INT(1, g_cond_eval_count);

    /* Verify state remains READY (wakeup not lost) */
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_READY, task.state);
}

static int delay_done = 0;

static SYN_PT_Status pt_delay_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    PT_TASK_DELAY_MS(pt, task, 100);
    delay_done = 1;

    PT_END(pt);
}

static void test_delay_ms(void)
{
    SYN_Task task = {0};
    syn_task_create(&task, "delay_test", pt_delay_func, SYN_PRIO_CRITICAL, NULL);
    delay_done = 0;
    mock_tick_ms = 0;

    SYN_PT_Status s;

    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, s);
    TEST_ASSERT_EQUAL_INT(0, delay_done);

    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, s);

    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
    TEST_ASSERT_EQUAL_INT(1, delay_done);
}

static SYN_PT_Sem test_sem;
static int sem_acquired = 0;

static SYN_PT_Status pt_sem_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_SEM_WAIT(pt, &test_sem);
    sem_acquired = 1;

    PT_END(pt);
}

static void test_semaphore(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 0);
    sem_acquired = 0;

    SYN_PT_Status s;

    s = pt_sem_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_IDLE, s);
    TEST_ASSERT_EQUAL_INT(0, sem_acquired);

    PT_SEM_SIGNAL(&test_sem);
    s = pt_sem_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
    TEST_ASSERT_EQUAL_INT(1, sem_acquired);
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));
}

/**
 * Regression: PT_SEM_SIGNAL and all count decrements are load-modify-store
 * on a volatile int16_t and must run inside critical sections — a
 * preempting ISR signal would otherwise be overwritten (lost signal).
 */
static void test_semaphore_ops_critical_sections(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 0);
    sem_acquired = 0;

    int enters = mock_critical_enter_count;

    PT_SEM_SIGNAL(&test_sem);
    TEST_ASSERT_EQUAL_INT(1, PT_SEM_COUNT(&test_sem));
    TEST_ASSERT_EQUAL_INT(enters + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* PT_SEM_WAIT decrement path */
    enters = mock_critical_enter_count;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_sem_func(&pt, NULL));
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));
    TEST_ASSERT_EQUAL_INT(enters + 1, mock_critical_enter_count);
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);

    /* trywait: both acquire and fail paths balanced */
    PT_SEM_SIGNAL(&test_sem);
    TEST_ASSERT_EQUAL_INT(1, PT_SEM_TRYWAIT(&test_sem));
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_TRYWAIT(&test_sem));
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));
    TEST_ASSERT_EQUAL_INT(0, mock_critical_depth);
}

#define EVT_A SYN_BIT(0)
#define EVT_B SYN_BIT(1)

static SYN_EventGroup test_events;
static int events_received = 0;

static SYN_PT_Status pt_event_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_WAIT_EVENT(pt, &test_events, EVT_A | EVT_B);
    events_received = 1;

    PT_END(pt);
}

static void test_event_flags(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    syn_event_init(&test_events);
    events_received = 0;

    SYN_PT_Status s;

    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_IDLE, s);

    syn_event_set(&test_events, EVT_A);
    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_IDLE, s);

    syn_event_set(&test_events, EVT_B);
    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
    TEST_ASSERT_EQUAL_INT(1, events_received);
    TEST_ASSERT_EQUAL_INT(0, syn_event_get(&test_events));
}

static int spawn_child_ran = 0;
static int spawn_parent_done = 0;

static SYN_PT_Status spawn_child(SYN_PT *pt)
{
    PT_BEGIN(pt);
    spawn_child_ran = 1;
    PT_CONTINUE(pt);
    spawn_child_ran = 2;
    PT_PASS(pt);
    spawn_child_ran = 3;
    PT_END(pt);
}

static SYN_PT child_pt;

static SYN_PT_Status spawn_parent(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    PT_SPAWN(pt, &child_pt, spawn_child(&child_pt));
    spawn_parent_done = 1;

    PT_END(pt);
}

static void test_spawn(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    PT_INIT(&child_pt);
    spawn_child_ran = 0;
    spawn_parent_done = 0;

    SYN_PT_Status s;

    s = spawn_parent(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_MORE, s);
    TEST_ASSERT_EQUAL_INT(1, spawn_child_ran);

    s = spawn_parent(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_IDLE, s);
    TEST_ASSERT_EQUAL_INT(2, spawn_child_ran);

    s = spawn_parent(&pt, NULL);
    TEST_ASSERT_EQUAL_INT(3, spawn_child_ran);
    TEST_ASSERT_EQUAL_INT(1, spawn_parent_done);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
}

static int rollover_done = 0;

static SYN_PT_Status pt_rollover_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    PT_TASK_DELAY_MS(pt, task, 100);
    rollover_done = 1;

    PT_END(pt);
}

/**
 * Validates the tick-rollover fix in PT_DELAY_MS.
 *
 * Starting near UINT32_MAX, the deadline wraps past 0.
 * With the old unsigned '>=' comparison this would stall forever.
 * With the signed-difference check it should fire after 100 ms.
 */
static void test_delay_ms_rollover(void)
{
    SYN_Task task = {0};
    syn_task_create(&task, "roll_test", pt_rollover_func, SYN_PRIO_CRITICAL, NULL);
    rollover_done = 0;

    /* Start 50 ms before the 32-bit tick wraps */
    mock_tick_ms = UINT32_MAX - 50;

    SYN_PT_Status s;

    /* First call: sets deadline = (UINT32_MAX - 50) + 100 = 49 (wrapped) */
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, s);
    TEST_ASSERT_EQUAL_INT(0, rollover_done);

    /* Advance 60 ms → tick wraps to 9.  Deadline is 49 → not reached yet */
    mock_tick_advance(60);
    TEST_ASSERT_TRUE(mock_tick_ms < 100); /* sanity: we wrapped */
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_IDLE, s);
    TEST_ASSERT_EQUAL_INT(0, rollover_done);

    /* Advance 50 more ms → tick = 59.  Total elapsed = 110 > 100 ms */
    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, s);
    TEST_ASSERT_EQUAL_INT(1, rollover_done);
}

static uint32_t g_pt_delay_target;
static uint32_t g_pt_delay_arg_ms;
static int g_pt_delay_eval_count;

static uint32_t next_delay_counter(void)
{
    g_pt_delay_eval_count++;
    return 25U;
}

static SYN_PT_Status pt_delay_ms_param_func(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_DELAY_MS(pt, &g_pt_delay_target, g_pt_delay_arg_ms);
    PT_END(pt);
}

static SYN_PT_Status pt_delay_ms_single_eval_func(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_DELAY_MS(pt, &g_pt_delay_target, next_delay_counter());
    PT_END(pt);
}

static void test_pt_delay_ms_zero_clamp_and_single_eval(void)
{
    SYN_PT pt;

    /* 1. ms == 0 completes immediately without yielding */
    PT_INIT(&pt);
    mock_tick_ms = 100U;
    g_pt_delay_target = 0U;
    g_pt_delay_arg_ms = 0U;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0U, g_pt_delay_target);

    /* 2. ms > 0x7FFFFFFF clamps to 0x7FFFFFFF */
    PT_INIT(&pt);
    mock_tick_ms = 10U;
    g_pt_delay_target = 0U;
    g_pt_delay_arg_ms = 0x80000000U;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(10U + 0x7FFFFFFFU, g_pt_delay_target);

    mock_tick_ms = 10U + 0x7FFFFFFEU;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_param_func(&pt));

    mock_tick_ms = 10U + 0x7FFFFFFFU;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0U, g_pt_delay_target);

    /* 3. Single evaluation of ms argument */
    PT_INIT(&pt);
    mock_tick_ms = 0U;
    g_pt_delay_target = 0U;
    g_pt_delay_eval_count = 0;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_single_eval_func(&pt));
    TEST_ASSERT_EQUAL_INT(1, g_pt_delay_eval_count);

    mock_tick_ms = 10U;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_single_eval_func(&pt));
    TEST_ASSERT_EQUAL_INT(1, g_pt_delay_eval_count);

    mock_tick_ms = 25U;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_ms_single_eval_func(&pt));
    TEST_ASSERT_EQUAL_INT(1, g_pt_delay_eval_count);
}

static uint32_t g_pt_us_target;
static uint32_t g_pt_us_arg;

static SYN_PT_Status pt_delay_us_param_func(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_DELAY_US(pt, &g_pt_us_target, g_pt_us_arg);
    PT_END(pt);
}

static void test_pt_delay_us_zero_clamp_and_rollover(void)
{
    SYN_PT pt;

    /* 1. us == 0 returns without yielding */
    PT_INIT(&pt);
    mock_tick_us = 500U;
    g_pt_us_target = 0U;
    g_pt_us_arg = 0U;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_us_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0U, g_pt_us_target);

    /* 2. us > 0x7FFFFFFF (0x90000000) clamped to 0x7FFFFFFF */
    PT_INIT(&pt);
    mock_tick_us = 20U;
    g_pt_us_target = 0U;
    g_pt_us_arg = 0x90000000U;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_us_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(20U + 0x7FFFFFFFU, g_pt_us_target);

    mock_tick_us = 20U + 0x7FFFFFFEU;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_us_param_func(&pt));

    mock_tick_us = 20U + 0x7FFFFFFFU;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_us_param_func(&pt));

    /* 3. mock_tick_us wraparound across 0xFFFFFFF0 -> 0x00000020 */
    PT_INIT(&pt);
    mock_tick_us = 0xFFFFFFF0U;
    g_pt_us_target = 0U;
    g_pt_us_arg = 40U; /* deadline wraps to 0x00000018 */
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_us_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0x00000018U, g_pt_us_target);

    mock_tick_us = 0xFFFFFFFCU;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_us_param_func(&pt));

    mock_tick_us = 0x00000010U;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_us_param_func(&pt));

    mock_tick_us = 0x00000020U;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_us_param_func(&pt));
}

static SYN_PT_Status pt_event_any_func(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_WAIT_EVENT_ANY(pt, &test_events, EVT_A | EVT_B);
    events_received++;
    PT_END(pt);
}

static void test_pt_wait_event_any_atomic_preserve(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    syn_event_init(&test_events);
    events_received = 0;

    TEST_ASSERT_EQUAL(PT_IDLE, pt_event_any_func(&pt));

    /* Unrelated bit does not satisfy wait and stays intact */
    syn_event_set(&test_events, SYN_BIT(4));
    TEST_ASSERT_EQUAL(PT_IDLE, pt_event_any_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(SYN_BIT(4), syn_event_get(&test_events));

    /* Setting one watched bit + another unrelated bit wakes and clears only matched bit */
    syn_event_set(&test_events, EVT_B | SYN_BIT(5));
    TEST_ASSERT_TRUE(syn_event_check_any(&test_events, EVT_A | EVT_B));
    TEST_ASSERT_FALSE(syn_event_check_all(&test_events, EVT_A | EVT_B));
    TEST_ASSERT_EQUAL(PT_ENDED, pt_event_any_func(&pt));
    TEST_ASSERT_EQUAL_INT(1, events_received);
    TEST_ASSERT_EQUAL_UINT32(SYN_BIT(4) | SYN_BIT(5), syn_event_get(&test_events));
    TEST_ASSERT_TRUE(syn_event_check_all(&test_events, SYN_BIT(4) | SYN_BIT(5)));
    syn_event_clear(&test_events, SYN_BIT(4));
    TEST_ASSERT_EQUAL_UINT32(SYN_BIT(5), syn_event_get(&test_events));
}

/** R3-K04: syn_pt_arm_ms_deadline() zero-wrap nudge keeps positive delta at INT32_MAX. */
static void test_pt_delay_ms_int32_max_zero_wrap_nudge(void)
{
    SYN_PT pt;

    /* 1. now=0x80000001, ms=0xFFFFFFFF -> clamped=0x7FFFFFFF -> dl==0 -> nudged to 0xFFFFFFFF */
    PT_INIT(&pt);
    mock_tick_ms = 0x80000001U;
    g_pt_delay_target = 0U;
    g_pt_delay_arg_ms = 0xFFFFFFFFU;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFU, g_pt_delay_target);
    TEST_ASSERT_TRUE((int32_t)(g_pt_delay_target - mock_tick_ms) > 0);

    mock_tick_ms = 0xFFFFFFFEU;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_param_func(&pt));

    mock_tick_ms = 0xFFFFFFFFU;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0U, g_pt_delay_target);

    /* 2. Short delay wrapping onto 0 (clamped != 0x7FFFFFFF) nudges forward to 1U */
    PT_INIT(&pt);
    mock_tick_ms = 0xFFFFFFF0U;
    g_pt_delay_target = 0U;
    g_pt_delay_arg_ms = 0x10U;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(1U, g_pt_delay_target);

    mock_tick_ms = 0U;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_delay_ms_param_func(&pt));

    mock_tick_ms = 1U;
    TEST_ASSERT_EQUAL(PT_ENDED, pt_delay_ms_param_func(&pt));
    TEST_ASSERT_EQUAL_UINT32(0U, g_pt_delay_target);
}

static int g_sem_ptr_eval_count = 0;

static SYN_PT_Sem *next_sem_ptr(void)
{
    g_sem_ptr_eval_count++;
    return &test_sem;
}

static SYN_PT_Status pt_sem_single_eval_func(SYN_PT *pt)
{
    PT_BEGIN(pt);
    PT_SEM_WAIT(pt, next_sem_ptr());
    PT_END(pt);
}

static SYN_PT_Status pt_sem_block_task_fn(SYN_PT *pt, SYN_Task *task)
{
    int *done = (int *)task->user_data;
    PT_BEGIN(pt);
    PT_SEM_BLOCK(pt, task, &test_sem);
    (*done)++;
    PT_END(pt);
}

/** R3-K06: PT_SEM_WAIT / PT_SEM_BLOCK atomic trywait, single evaluation, and NULL safety. */
static void test_pt_sem_atomic_single_eval_and_block(void)
{
    TEST_ASSERT_EQUAL_INT(0, pt_sem_trywait(NULL));

    /* Single evaluation of sem expression in PT_SEM_WAIT */
    SYN_PT pt;
    PT_INIT(&pt);
    PT_SEM_INIT(&test_sem, 0);
    g_sem_ptr_eval_count = 0;
    TEST_ASSERT_EQUAL(PT_IDLE, pt_sem_single_eval_func(&pt));
    TEST_ASSERT_EQUAL_INT(1, g_sem_ptr_eval_count);

    PT_SEM_SIGNAL(&test_sem);
    TEST_ASSERT_EQUAL(PT_ENDED, pt_sem_single_eval_func(&pt));
    TEST_ASSERT_EQUAL_INT(2, g_sem_ptr_eval_count);
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));

    /* PT_SEM_BLOCK under scheduler + stolen token race without underflow */
    int done = 0;
    SYN_Task task = {0};
    SYN_Sched sched;
    PT_SEM_INIT(&test_sem, 0);
    syn_task_create(&task, "sem_blk", pt_sem_block_task_fn, SYN_PRIO_HIGH, &done);
    syn_sched_init(&sched, &task, 1);

    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);
    TEST_ASSERT_EQUAL_INT(0, done);

    /* Signal + resume, then steal token via pt_sem_trywait before task runs: re-blocks cleanly */
    PT_SEM_SIGNAL(&test_sem);
    syn_task_resume(&task);
    TEST_ASSERT_EQUAL_INT(1, pt_sem_trywait(&test_sem));
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_BLOCKED, task.state);
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));
    TEST_ASSERT_EQUAL_INT(0, done);

    /* Signal + resume without contention: acquires token atomically and finishes */
    PT_SEM_SIGNAL(&test_sem);
    syn_task_resume(&task);
    syn_sched_run(&sched);
    TEST_ASSERT_EQUAL_UINT8(SYN_TASK_DEAD, task.state);
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));
    TEST_ASSERT_EQUAL_INT(1, done);
}

void run_protothread_tests(void)
{
    RUN_TEST(test_basic_protothread);
    RUN_TEST(test_pt_exit_vs_end);
    RUN_TEST(test_wait_until);
    RUN_TEST(test_pt_block_condition_isr_race);
    RUN_TEST(test_delay_ms);
    RUN_TEST(test_delay_ms_rollover);
    RUN_TEST(test_pt_delay_ms_zero_clamp_and_single_eval);
    RUN_TEST(test_pt_delay_ms_int32_max_zero_wrap_nudge);
    RUN_TEST(test_pt_delay_us_zero_clamp_and_rollover);
    RUN_TEST(test_semaphore);
    RUN_TEST(test_semaphore_ops_critical_sections);
    RUN_TEST(test_pt_sem_atomic_single_eval_and_block);
    RUN_TEST(test_event_flags);
    RUN_TEST(test_pt_wait_event_any_atomic_preserve);
    RUN_TEST(test_spawn);
}
