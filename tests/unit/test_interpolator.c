#include "syntropic/motor/syn_interpolator.h"
#include "unity/unity.h"

#include <math.h>

void test_interpolator_linear_planning(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F start = {0.0f, 0.0f, 0.0f};
    SYN_Vector3F target = {30.0f, 40.0f, 0.0f}; /* 3-4-5 triangle -> length 50 */

    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_linear(&interp, start, target, 100.0f, 500.0f,
                                                           2000.0f, 1.0f));
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

    SYN_Vector3F start = {10.0f, 0.0f, 0.0f};
    SYN_Vector3F target = {0.0f, 10.0f, 0.0f};
    SYN_Vector3F center_off = {-10.0f, 0.0f, 0.0f}; /* Center at (0, 0) */

    /* CCW quarter circle from (10, 0) to (0, 10) */
    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_circular(&interp, start, target, center_off,
                                                             false, 50.0f, 200.0f, 1000.0f, 0.1f));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, interp.radius);

    SYN_Vector3F pos = {0.0f, 0.0f, 0.0f};
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

    SYN_Vector3F start = {0.0f, 0.0f, 0.0f};
    SYN_Vector3F target = {100.0f, 0.0f, 0.0f};

    syn_interpolator_plan_linear(&interp, start, target, 50.0f, 100.0f, 500.0f, 0.1f);

    SYN_Vector3F pos, vel;
    TEST_ASSERT_TRUE(syn_interpolator_eval_at_time(&interp, 0.0f, &pos, &vel));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, pos.x);
}

void test_interpolator_edge_cases(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F p = {0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_interpolator_plan_linear(&interp, p, p, 0.0f, 100.0f, 500.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_interpolator_plan_linear(&interp, p, p, 100.0f, 100.0f, 500.0f, 0.0f));

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_circular(
                                             &interp, p, p, p, true, 0.0f, 100.0f, 500.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_interpolator_plan_circular(&interp, p, p, p, true, 100.0f, 100.0f, 500.0f,
                                                     0.1f)); /* center_offset 0 -> radius 0 */

    SYN_Vector3F out;
    TEST_ASSERT_FALSE(syn_interpolator_step(&interp, &out));
}

void test_interpolator_out_of_bounds_extrapolation(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F p = {10.0f, 10.0f, 0.0f};

    /* Zero length linear plan */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_interpolator_plan_linear(&interp, p, p, 100.0f, 500.0f, 2000.0f, 1.0f));
    TEST_ASSERT_EQUAL(SYN_INTERP_MODE_IDLE, interp.mode);

    /* Tiny movement linear plan total_steps == 0 (line 57) */
    SYN_Vector3F p_tiny = {10.00001f, 10.0f, 0.0f};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_interpolator_plan_linear(&interp, p, p_tiny, 100.0f, 500.0f, 2000.0f, 1.0f));

    /* Clockwise circular arc sweep >= 0 (line 105) */
    SYN_Vector3F start_cw = {10.0f, 0.0f, 0.0f};
    SYN_Vector3F target_cw = {0.0f, 10.0f, 0.0f};
    SYN_Vector3F center_off_cw = {-10.0f, 0.0f, 0.0f};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_interpolator_plan_circular(&interp, start_cw, target_cw, center_off_cw,
                                                     true, 50.0f, 200.0f, 1000.0f, 0.1f));

    /* Counter-Clockwise circular arc sweep <= 0 (line 110) */
    SYN_Vector3F start_ccw = {0.0f, 10.0f, 0.0f};
    SYN_Vector3F target_ccw = {10.0f, 0.0f, 0.0f};
    SYN_Vector3F center_off_ccw = {0.0f, -10.0f, 0.0f};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_interpolator_plan_circular(&interp, start_ccw, target_ccw, center_off_ccw,
                                                     false, 50.0f, 200.0f, 1000.0f, 0.1f));

    /* Tiny circular movement total_steps == 0 (line 120) */
    SYN_Vector3F p_circ_tiny = {10.00001f, 0.0f, 0.0f};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_interpolator_plan_circular(&interp, start_cw, p_circ_tiny, center_off_cw,
                                                     true, 50.0f, 200.0f, 1000.0f, 10.0f));

    SYN_Vector3F pos, vel;
    TEST_ASSERT_TRUE(syn_interpolator_eval_at_time(&interp, 0.5f, &pos, &vel));

    /* Eval when idle */
    syn_interpolator_init(&interp);
    TEST_ASSERT_FALSE(syn_interpolator_eval_at_time(&interp, 0.0f, &pos, &vel));
}

static void test_scurve3d(void)
{
    SYN_SCurve3D sc3d;
    syn_scurve3d_plan(&sc3d, 0, 0, 0, 300, 400, 0, 100, 50, 10);
    TEST_ASSERT_EQUAL(500, sc3d.total_dist);

    int32_t x = 0, y = 0, z = 0;
    int iters = 10000;
    while (syn_scurve3d_update(&sc3d, &x, &y, &z) && --iters > 0) {
        TEST_ASSERT_TRUE(x >= 0 && x <= 300);
        TEST_ASSERT_TRUE(y >= 0 && y <= 400);
    }
    TEST_ASSERT_EQUAL(300, x);
    TEST_ASSERT_EQUAL(400, y);
}

static void test_interpolator_bezier(void)
{
    SYN_Interpolator interp;
    syn_interpolator_init(&interp);

    SYN_Vector3F p0 = {0.0f, 0.0f, 0.0f};
    SYN_Vector3F p1 = {10.0f, 20.0f, 0.0f};
    SYN_Vector3F p2 = {30.0f, 20.0f, 0.0f};
    SYN_Vector3F p3 = {40.0f, 0.0f, 10.0f};

    /* Parameter validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_interpolator_plan_bezier(NULL, p0, p1, p2, p3, 10.0f, 5.0f, 2.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_bezier(&interp, p0, p1, p2, p3, 0.0f,
                                                                      5.0f, 2.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_bezier(&interp, p0, p1, p2, p3,
                                                                      10.0f, 0.0f, 2.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_bezier(&interp, p0, p1, p2, p3,
                                                                      10.0f, 5.0f, 0.0f, 0.1f));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_interpolator_plan_bezier(&interp, p0, p1, p2, p3,
                                                                      10.0f, 5.0f, 2.0f, 0.0f));

    SYN_Status st = syn_interpolator_plan_bezier(&interp, p0, p1, p2, p3, 10.0f, 5.0f, 2.0f, 1.0f);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL(SYN_INTERP_MODE_BEZIER, interp.mode);
    TEST_ASSERT_TRUE(interp.total_length > 45.0f);

    /* Evaluate curved Bezier with non-zero tangent via eval_at_time */
    SYN_Vector3F eval_pos = {0.0f, 0.0f, 0.0f};
    SYN_Vector3F eval_vel = {0.0f, 0.0f, 0.0f};
    (void)syn_interpolator_eval_at_time(&interp, 0.1f, &eval_pos, &eval_vel);
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_interpolator_plan_bezier(&interp, p0, p1, p2, p3, 10.0f, 5.0f, 2.0f, 1.0f));

    /* Step to midpoint u = 0.5 -> B(0.5) = (20.0, 15.0, 5.0), proving true curved Bezier */
    uint32_t mid_step = interp.total_steps / 2U;
    SYN_Vector3F pos = {0.0f, 0.0f, 0.0f};
    for (uint32_t i = 0U; i < mid_step; i++) {
        TEST_ASSERT_TRUE(syn_interpolator_step(&interp, &pos));
    }
    float expected_u = (float)mid_step / (float)interp.total_steps;
    float ui = 1.0f - expected_u;
    float exp_y = 3.0f * ui * ui * expected_u * 20.0f + 3.0f * ui * expected_u * expected_u * 20.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.05f, exp_y, pos.y);
    TEST_ASSERT_TRUE(pos.y > 14.0f);

    /* Step to completion */
    while (syn_interpolator_step(&interp, &pos)) {
    }
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 40.0f, pos.x);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, pos.y);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, pos.z);

    /* Degenerate zero-length Bezier, tiny-step Bezier, and zero-endpoint-tangent eval_at_time */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_interpolator_plan_bezier(&interp, p0, p0, p0, p0, 10.0f, 5.0f, 2.0f, 1.0f));
    TEST_ASSERT_EQUAL(SYN_INTERP_MODE_IDLE, interp.mode);

    SYN_Vector3F p_tiny = {0.01f, 0.0f, 0.0f};
    TEST_ASSERT_EQUAL(SYN_OK, syn_interpolator_plan_bezier(&interp, p0, p0, p_tiny, p_tiny, 1.0f,
                                                           1.0f, 1.0f, 10.0f));
    TEST_ASSERT_EQUAL_UINT32(1U, interp.total_steps);
    SYN_Vector3F vel = {0.0f, 0.0f, 0.0f};
    /* First scurve tick at ratio=0 with p0==p1 has B'(0)=0, exercising tang_norm <= 1e-6f */
    (void)syn_interpolator_eval_at_time(&interp, 0.0f, &pos, &vel);
}

void run_interpolator_tests(void)
{
    RUN_TEST(test_interpolator_linear_planning);
    RUN_TEST(test_interpolator_circular_arc);
    RUN_TEST(test_interpolator_eval_time);
    RUN_TEST(test_interpolator_edge_cases);
    RUN_TEST(test_interpolator_out_of_bounds_extrapolation);
    RUN_TEST(test_scurve3d);
    RUN_TEST(test_interpolator_bezier);
}
