#include "unity/unity.h"
#include "syntropic/motor/syn_interpolator.h"

#include <math.h>

void test_interpolator_linear_planning(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F start  = { 0.0f, 0.0f, 0.0f };
    SYN_Vector3F target = { 30.0f, 40.0f, 0.0f }; /* 3-4-5 triangle -> length 50 */

    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_linear(&interp, start, target, 100.0f, 500.0f, 2000.0f, 1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, interp.total_length);
    TEST_ASSERT_EQUAL(50, interp.total_steps);

    SYN_Vector3F pos;
    TEST_ASSERT_TRUE(syn_interpolator_step(&interp, &pos));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.6f, pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.8f, pos.y);

    /* Step to end */
    for (int i = 0; i < 50; i++) {
        syn_interpolator_step(&interp, &pos);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 40.0f, pos.y);
    TEST_ASSERT_EQUAL(SYN_INTERP_MODE_IDLE, interp.mode);
}

void test_interpolator_circular_arc(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F start = { 10.0f, 0.0f, 0.0f };
    SYN_Vector3F target = { 0.0f, 10.0f, 0.0f };
    SYN_Vector3F center_off = { -10.0f, 0.0f, 0.0f }; /* Center at (0, 0) */

    /* CCW quarter circle from (10, 0) to (0, 10) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_circular(&interp, start, target, center_off, false, 50.0f, 200.0f, 1000.0f, 0.1f));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, interp.radius);

    SYN_Vector3F pos;
    for (int i = 0; i < (int)interp.total_steps; i++) {
        syn_interpolator_step(&interp, &pos);
    }

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, pos.y);
}

void test_interpolator_eval_time(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F start  = { 0.0f, 0.0f, 0.0f };
    SYN_Vector3F target = { 100.0f, 0.0f, 0.0f };

    syn_interpolator_plan_linear(&interp, start, target, 50.0f, 100.0f, 500.0f, 0.1f);

    SYN_Vector3F pos, vel;
    TEST_ASSERT_TRUE(syn_interpolator_eval_at_time(&interp, 0.0f, &pos, &vel));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, pos.x);
}

void test_interpolator_edge_cases(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F p = { 0, 0, 0 };
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_linear(&interp, p, p, 0.0f, 100.0f, 500.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_linear(&interp, p, p, 100.0f, 100.0f, 500.0f, 0.0f));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_circular(&interp, p, p, p, true, 0.0f, 100.0f, 500.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_circular(&interp, p, p, p, true, 100.0f, 100.0f, 500.0f, 0.1f)); /* center_offset 0 -> radius 0 */

    SYN_Vector3F out;
    TEST_ASSERT_FALSE(syn_interpolator_step(&interp, &out));
}

void test_interpolator_additional_coverage(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F p = { 10.0f, 10.0f, 0.0f };

    /* Zero length linear plan */
    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_linear(&interp, p, p, 100.0f, 500.0f, 2000.0f, 1.0f));
    TEST_ASSERT_EQUAL(SYN_INTERP_MODE_IDLE, interp.mode);

    /* Clockwise circular arc (start (0, 10), target (10, 0), center_off (0, -10)) */
    SYN_Vector3F start = { 0.0f, 10.0f, 0.0f };
    SYN_Vector3F target = { 10.0f, 0.0f, 0.0f };
    SYN_Vector3F center_off = { 0.0f, -10.0f, 0.0f };
    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_circular(&interp, start, target, center_off, true, 50.0f, 200.0f, 1000.0f, 0.1f));

    SYN_Vector3F pos, vel;
    TEST_ASSERT_TRUE(syn_interpolator_eval_at_time(&interp, 0.5f, &pos, &vel));
    TEST_ASSERT_TRUE(pos.x >= 0.0f && pos.y >= 0.0f);

    /* Eval when idle */
    syn_interpolator_init(&interp);
    TEST_ASSERT_FALSE(syn_interpolator_eval_at_time(&interp, 0.0f, &pos, &vel));
}

void run_interpolator_tests(void)
{
    RUN_TEST(test_interpolator_linear_planning);
    RUN_TEST(test_interpolator_circular_arc);
    RUN_TEST(test_interpolator_eval_time);
    RUN_TEST(test_interpolator_edge_cases);
    RUN_TEST(test_interpolator_additional_coverage);
}
