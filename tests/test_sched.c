/**
 * @file test_sched.c
 * @brief Unity tests for syn_sched.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/sched/syn_sched.h"

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

void run_sched_tests(void)
{
    RUN_TEST(test_scheduler);
    RUN_TEST(test_suspend_resume);
    RUN_TEST(test_sched_empty);
    RUN_TEST(test_sched_delayed_task);
    RUN_TEST(test_sched_alive_count);
    RUN_TEST(test_sched_run_forever);
}
