/**
 * @file test_protothread.c
 * @brief Unity tests for syn_protothread.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_sched.h"

static int pt_basic_counter = 0;

static SYN_PT_Status pt_basic_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    pt_basic_counter = 1;
    PT_YIELD(pt);

    pt_basic_counter = 2;
    PT_YIELD(pt);

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
    TEST_ASSERT_EQUAL(PT_YIELDED, s);
    TEST_ASSERT_EQUAL_INT(1, pt_basic_counter);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_YIELDED, s);
    TEST_ASSERT_EQUAL_INT(2, pt_basic_counter);

    s = pt_basic_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
    TEST_ASSERT_EQUAL_INT(3, pt_basic_counter);
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
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    wait_condition = 1;
    s = pt_wait_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
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

    SYN_Task task;
    syn_task_create(&task, "delay_test", pt_delay_func, 0, NULL);
    delay_done = 0;
    mock_tick_ms = 0;

    SYN_PT_Status s;

    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_WAITING, s);
    TEST_ASSERT_EQUAL_INT(0, delay_done);

    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
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
    TEST_ASSERT_EQUAL(PT_WAITING, s);
    TEST_ASSERT_EQUAL_INT(0, sem_acquired);

    PT_SEM_SIGNAL(&test_sem);
    s = pt_sem_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
    TEST_ASSERT_EQUAL_INT(1, sem_acquired);
    TEST_ASSERT_EQUAL_INT(0, PT_SEM_COUNT(&test_sem));
}

#define EVT_A  SYN_BIT(0)
#define EVT_B  SYN_BIT(1)

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
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    syn_event_set(&test_events, EVT_A);
    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_WAITING, s);

    syn_event_set(&test_events, EVT_B);
    s = pt_event_func(&pt, NULL);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
    TEST_ASSERT_EQUAL_INT(1, events_received);
    TEST_ASSERT_EQUAL_INT(0, syn_event_get(&test_events));
}

static int spawn_child_ran = 0;
static int spawn_parent_done = 0;

static SYN_PT_Status spawn_child(SYN_PT *pt)
{
    PT_BEGIN(pt);
    spawn_child_ran = 1;
    PT_YIELD(pt);
    spawn_child_ran = 2;
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
    TEST_ASSERT_EQUAL(PT_WAITING, s);
    TEST_ASSERT_EQUAL_INT(1, spawn_child_ran);

    s = spawn_parent(&pt, NULL);
    TEST_ASSERT_EQUAL_INT(2, spawn_child_ran);
    TEST_ASSERT_EQUAL_INT(1, spawn_parent_done);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
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
    SYN_Task task;
    syn_task_create(&task, "roll_test", pt_rollover_func, 0, NULL);
    rollover_done = 0;

    /* Start 50 ms before the 32-bit tick wraps */
    mock_tick_ms = UINT32_MAX - 50;

    SYN_PT_Status s;

    /* First call: sets deadline = (UINT32_MAX - 50) + 100 = 49 (wrapped) */
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_WAITING, s);
    TEST_ASSERT_EQUAL_INT(0, rollover_done);

    /* Advance 60 ms → tick wraps to 9.  Deadline is 49 → not reached yet */
    mock_tick_advance(60);
    TEST_ASSERT_TRUE(mock_tick_ms < 100); /* sanity: we wrapped */
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_WAITING, s);
    TEST_ASSERT_EQUAL_INT(0, rollover_done);

    /* Advance 50 more ms → tick = 59.  Total elapsed = 110 > 100 ms */
    mock_tick_advance(50);
    s = task.func(&task.pt, &task);
    TEST_ASSERT_EQUAL(PT_EXITED, s);
    TEST_ASSERT_EQUAL_INT(1, rollover_done);
}

void run_protothread_tests(void)
{
    RUN_TEST(test_basic_protothread);
    RUN_TEST(test_wait_until);
    RUN_TEST(test_delay_ms);
    RUN_TEST(test_delay_ms_rollover);
    RUN_TEST(test_semaphore);
    RUN_TEST(test_event_flags);
    RUN_TEST(test_spawn);
}
