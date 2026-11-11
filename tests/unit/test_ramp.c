/**
 * @file test_ramp.c
 * @brief Unity tests for syn_ramp.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_ramp.h"
#include "unity/unity.h"

static void test_ramp(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 0);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 0);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    syn_ramp_set_target(&r, 100, 10);
    TEST_ASSERT_TRUE(!syn_ramp_done(&r));
    int32_t val = 0;
    for (int i = 0; i < 10; i++)
        val = syn_ramp_update(&r);
    TEST_ASSERT_EQUAL_INT(100, val);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    syn_ramp_set_target(&r, 50, 25);
    syn_ramp_update(&r);
    syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 50);
    syn_ramp_jump(&r, 999);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 999);
    syn_ramp_set_target(&r, 1002, 10);
    syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 1002);
    syn_ramp_init(&r, 0);
    syn_ramp_set_target_trapezoid(&r, 1000, 100, 10);
    int iters = 0;
    while (!syn_ramp_done(&r) && iters < 500) {
        syn_ramp_update(&r);
        iters++;
    }
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == 1000);
    syn_ramp_init(&r, 100);
    syn_ramp_set_target(&r, -100, 50);
    for (int i = 0; i < 4; i++)
        syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_value(&r) == -100);
}

/** Linear ramp already at target — exercises lines 65-67 (diff==0, done=true) */
static void test_ramp_already_at_target(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 500);
    syn_ramp_set_target(&r, 500, 10); /* target == current */
    syn_ramp_update(&r);              /* diff == 0 → done = true */
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    TEST_ASSERT_EQUAL_INT(500, syn_ramp_value(&r));
}

/** S-curve with negative velocity during deceleration — exercises lines 113-115 */
static void test_ramp_scurve_negative_velocity_decel(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 1000);
    /* Set target below current to force negative velocity */
    syn_ramp_set_target_trapezoid(&r, 0, 50, 5);

    /* Run until done or max iterations */
    int iters = 0;
    while (!syn_ramp_done(&r) && iters < 2000) {
        syn_ramp_update(&r);
        iters++;
    }
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    TEST_ASSERT_EQUAL_INT(0, syn_ramp_value(&r));
}

/** S-curve already at target — exercises lines 89-91 (diff==0 && vel==0) */
static void test_ramp_scurve_at_target(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 200);
    syn_ramp_set_target_trapezoid(&r, 200, 50, 5); /* same as current */
    syn_ramp_update(&r);
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
}

/** update_linear: diff==0 path — exercises lines 65-67.
 *  Requires calling update when target==current but done is still false.
 *  This happens when the ramp steps exactly onto the target (line 76 sets done)
 *  then a second call hits the diff==0 path before done is checked by the caller.
 *  Trick: set target to current+1, rate=1 — first update steps to target,
 *  sets done=true via line 76. Lines 65-67 are reached when update_linear
 *  is called with diff already 0 on first entry. We force this by initializing
 *  current==target but done=false directly.
 */
static void test_ramp_linear_done_at_diff_zero(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 100);
    syn_ramp_set_target(&r, 101, 1); /* 1 step away */
    r.done = false;                  /* ensure not short-circuited */
    /* First update: diff=1, steps to 101, sets done=true via line 76 */
    syn_ramp_update(&r);
    TEST_ASSERT_EQUAL_INT(101, r.current);
    TEST_ASSERT_TRUE(r.done);

    /* Force another call with diff==0 but done=false to hit lines 65-67 */
    r.done = false;
    int32_t v = syn_ramp_update(&r); /* diff==0 → hits lines 65-67 */
    TEST_ASSERT_EQUAL_INT(101, v);
    TEST_ASSERT_TRUE(r.done);
}

/** update_scurve: diff==0&&vel==0 path — exercises lines 89-91. */
static void test_ramp_scurve_done_at_diff_zero(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 200);
    syn_ramp_set_target_trapezoid(&r, 200, 50, 5);
    /* Already at target, velocity=0, done=true. Force done=false to enter update_scurve */
    r.done = false;
    int32_t v = syn_ramp_update(&r); /* diff==0 && vel==0 → hits lines 89-91 */
    TEST_ASSERT_EQUAL_INT(200, v);
    TEST_ASSERT_TRUE(r.done);
}

static void test_ramp_fixed_point_and_negative_rate_clamp(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 0);

    /* Negative rate and accel clamped to 1 */
    syn_ramp_set_target(&r, 100, -5);
    TEST_ASSERT_EQUAL_INT(1, r.rate);

    syn_ramp_set_target_trapezoid(&r, 100, -10, -5);
    TEST_ASSERT_EQUAL_INT(1, r.rate);
    TEST_ASSERT_EQUAL_INT(1, r.accel);

    /* Fixed-point mode negative rate/accel and frac_bits > 16 clamp */
    syn_ramp_set_target_trapezoid_fp(&r, 10, -5, -2, 20);
    TEST_ASSERT_EQUAL_INT(1, r.rate);
    TEST_ASSERT_EQUAL_INT(1, r.accel);

    /* Fixed-point mode (frac_bits = 4 -> Q4.4) */
    syn_ramp_set_target_trapezoid_fp(&r, 10, 16, 1, 4);
    int iters = 0;
    while (!syn_ramp_done(&r) && iters < 500) {
        syn_ramp_update(&r);
        iters++;
    }
    TEST_ASSERT_TRUE(syn_ramp_done(&r));
    TEST_ASSERT_EQUAL_INT(10, syn_ramp_value(&r));

    /* Negative fixed-point trapezoid must match positive step count without signed shift UB */
    SYN_Ramp r_pos, r_neg;
    syn_ramp_init(&r_pos, 0);
    syn_ramp_init(&r_neg, 10);
    syn_ramp_set_target_trapezoid_fp(&r_pos, 10, 64, 4, 8);
    syn_ramp_set_target_trapezoid_fp(&r_neg, 0, 64, 4, 8);

    /* First sub-unit tick (-4/256) must NOT jump full integer unit from 10 to 9 */
    TEST_ASSERT_EQUAL_INT(0, syn_ramp_update(&r_pos));
    TEST_ASSERT_EQUAL_INT(10, syn_ramp_update(&r_neg));

    int pos_iters = 1;
    int neg_iters = 1;
    while (!syn_ramp_done(&r_pos) && pos_iters < 2000) {
        syn_ramp_update(&r_pos);
        pos_iters++;
    }
    while (!syn_ramp_done(&r_neg) && neg_iters < 2000) {
        syn_ramp_update(&r_neg);
        neg_iters++;
    }
    TEST_ASSERT_TRUE(syn_ramp_done(&r_neg));
    TEST_ASSERT_EQUAL_INT(0, syn_ramp_value(&r_neg));
    TEST_ASSERT_EQUAL_INT(pos_iters, neg_iters);
}

static void test_ramp_over_deceleration_clamp(void)
{
    SYN_Ramp r;
    syn_ramp_init(&r, 100);

    /* Positive velocity over-decelerating past zero (line 169) */
    syn_ramp_set_target_trapezoid(&r, 105, 50, 10);
    r.current = 104;
    r.velocity = 3;
    r.done = false;
    syn_ramp_update(&r);
    TEST_ASSERT_EQUAL_INT(0, r.velocity);

    /* Negative velocity over-decelerating past zero (line 173) */
    syn_ramp_set_target_trapezoid(&r, 95, 50, 10);
    r.current = 96;
    r.velocity = -3;
    r.done = false;
    syn_ramp_update(&r);
    TEST_ASSERT_EQUAL_INT(0, r.velocity);
}

void run_ramp_tests(void)
{
    RUN_TEST(test_ramp);
    RUN_TEST(test_ramp_already_at_target);
    RUN_TEST(test_ramp_scurve_negative_velocity_decel);
    RUN_TEST(test_ramp_scurve_at_target);
    RUN_TEST(test_ramp_linear_done_at_diff_zero);
    RUN_TEST(test_ramp_scurve_done_at_diff_zero);
    RUN_TEST(test_ramp_fixed_point_and_negative_rate_clamp);
    RUN_TEST(test_ramp_over_deceleration_clamp);
}
