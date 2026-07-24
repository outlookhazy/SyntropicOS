/**
 * @file test_timer_wheel.c
 * @brief Unit test suite for Hashed Timing Wheel Scheduler (syn_timer_wheel).
 */

#include "unity/unity.h"
#include "syntropic/sched/syn_timer_wheel.h"
#include <string.h>

static int s_timer1_fired = 0;
static int s_timer2_fired = 0;

static void timer1_cb(void *arg)
{
    (void)arg;
    s_timer1_fired++;
}

static void timer2_cb(void *arg)
{
    (void)arg;
    s_timer2_fired++;
}

void test_timer_wheel_add_step_cancel(void)
{
    SYN_TimerWheel wheel;
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_init(&wheel));

    SYN_TimerWheelNode n1, n2;
    memset(&n1, 0, sizeof(n1));
    memset(&n2, 0, sizeof(n2));
    s_timer1_fired = 0;
    s_timer2_fired = 0;

    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &n1, 3, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_add(&wheel, &n2, 70, timer2_cb, NULL)); /* Multi-rotation timer */

    /* Step 1 & 2: no timers fire */
    TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_UINT32(0, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(0, s_timer1_fired);

    /* Step 3: timer 1 fires */
    TEST_ASSERT_EQUAL_UINT32(1, syn_timer_wheel_step(&wheel));
    TEST_ASSERT_EQUAL_INT(1, s_timer1_fired);

    /* Cancel timer 2 */
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_timer_wheel_cancel(&wheel, &n2));

    /* Advance wheel by 70 ticks */
    for (int i = 0; i < 70; i++) {
        syn_timer_wheel_step(&wheel);
    }
    TEST_ASSERT_EQUAL_INT(0, s_timer2_fired); /* Canceled, should not fire */

    /* Test collision: two timers in the same bucket */
    SYN_TimerWheelNode t_a, t_b, t_c;
    memset(&t_a, 0, sizeof(t_a));
    memset(&t_b, 0, sizeof(t_b));
    memset(&t_c, 0, sizeof(t_c));

    syn_timer_wheel_add(&wheel, &t_a, 5, timer1_cb, NULL);
    syn_timer_wheel_add(&wheel, &t_b, 5, timer2_cb, NULL);
    syn_timer_wheel_add(&wheel, &t_c, 5, timer1_cb, NULL);

    /* Re-add active timer t_b (triggers cancel before re-add) */
    syn_timer_wheel_add(&wheel, &t_b, 5, timer2_cb, NULL);

    /* Cancel middle element t_b */
    syn_timer_wheel_cancel(&wheel, &t_b);

    /* Step 5 ticks to fire t_a and t_c */
    for (int i = 0; i < 4; i++) syn_timer_wheel_step(&wheel);
    TEST_ASSERT_EQUAL_UINT32(2, syn_timer_wheel_step(&wheel));

    /* Null & param checks */
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_init(NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_add(NULL, &n1, 5, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_add(&wheel, &n1, 0, timer1_cb, NULL));
    TEST_ASSERT_EQUAL_INT(SYN_INVALID_PARAM, syn_timer_wheel_cancel(NULL, &n1));
}

void run_timer_wheel_tests(void)
{
    RUN_TEST(test_timer_wheel_add_step_cancel);
}
