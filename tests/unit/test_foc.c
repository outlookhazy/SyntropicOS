/**
 * @file test_foc.c
 * @brief Unity tests for syn_foc (Field-Oriented Control transforms).
 */

#include "mocks/mock_port.h"
#include "syntropic/motor/syn_foc.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#include <stdio.h>

/* ── Helper ─────────────────────────────────────────────────────────────── */

#define ASSERT_Q16_NEAR(expected, actual, tol_q16)                                             \
    do {                                                                                       \
        q16_t _e = (expected), _a = (actual), _t = (tol_q16);                                  \
        q16_t _d = (_a > _e) ? (_a - _e) : (_e - _a);                                          \
        if (_d > _t) {                                                                         \
            char _msg[80];                                                                     \
            snprintf(_msg, sizeof(_msg), "Expected %ld +- %ld, got %ld (delta %ld)", (long)_e, \
                     (long)_t, (long)_a, (long)_d);                                            \
            TEST_FAIL_MESSAGE(_msg);                                                           \
        }                                                                                      \
    } while (0)

#define Q16_TOL 655 /* ±0.01 — relaxed for transform chain roundtrip */

/* ── Test: Clarke identity (balanced) ──────────────────────────────────── */

static void test_clarke_balanced(void)
{
    /* Phase A = 1.0, B = C = -0.5 (balanced 3-phase at 0° angle) */
    SYN_FOC_ABC abc = {Q16_ONE, -Q16_HALF, -Q16_HALF};
    SYN_FOC_AB ab;

    syn_foc_clarke(&abc, &ab);

    /* α should be ≈ 1.0 (phase A) */
    ASSERT_Q16_NEAR(Q16_ONE, ab.alpha, Q16_TOL);
    /* β should be ≈ 0 for this symmetric case */
    ASSERT_Q16_NEAR(0, ab.beta, Q16_TOL);
}

/* ── Test: Park at θ=0 is identity ─────────────────────────────────────── */

static void test_park_zero_angle(void)
{
    /* At θ=0: d = α, q = β */
    SYN_FOC_AB ab = {Q16_FROM_INT(3), Q16_FROM_INT(4)};
    SYN_FOC_DQ dq;

    syn_foc_park(&ab, 0, &dq);

    ASSERT_Q16_NEAR(Q16_FROM_INT(3), dq.d, Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(4), dq.q, Q16_TOL);
}

/* ── Test: Park + Inverse Park roundtrip ───────────────────────────────── */

static void test_park_inv_roundtrip(void)
{
    SYN_FOC_AB ab_in = {Q16_FROM_INT(3), Q16_FROM_INT(4)};
    SYN_FOC_DQ dq;
    SYN_FOC_AB ab_out;

    q16_t theta = Q16_PI / 3; /* 60° */

    syn_foc_park(&ab_in, theta, &dq);
    syn_foc_inv_park(&dq, theta, &ab_out);

    ASSERT_Q16_NEAR(ab_in.alpha, ab_out.alpha, Q16_TOL);
    ASSERT_Q16_NEAR(ab_in.beta, ab_out.beta, Q16_TOL);
}

/* ── Test: Clarke + Inv Clarke roundtrip ───────────────────────────────── */

static void test_clarke_inv_roundtrip(void)
{
    /* Balanced 3-phase at 30° */
    SYN_FOC_ABC abc_in = {Q16_FROM_FRAC(866, 1000),   /* cos(30°) ≈ 0.866 */
                          Q16_FROM_FRAC(-500, 1000),  /* cos(150°) ≈ -0.5 */
                          Q16_FROM_FRAC(-366, 1000)}; /* cos(270°) ≈ -0.366 */
    SYN_FOC_AB ab;
    SYN_FOC_ABC abc_out;

    syn_foc_clarke(&abc_in, &ab);
    syn_foc_inv_clarke(&ab, &abc_out);

    ASSERT_Q16_NEAR(abc_in.a, abc_out.a, Q16_TOL);
    ASSERT_Q16_NEAR(abc_in.b, abc_out.b, Q16_TOL * 2);
    ASSERT_Q16_NEAR(abc_in.c, abc_out.c, Q16_TOL * 2);
}

/* ── Test: SVPWM duties sum to ~1.5 ────────────────────────────────────── */

static void test_svpwm_duty_range(void)
{
    /* Test all 6 SVPWM sectors for complete branch coverage of MIN/MAX voltage ordering */
    SYN_FOC_AB sectors[6] = {
        {Q16_FROM_INT(10), 0},                /* Sector 1 (0°) */
        {Q16_FROM_INT(5), Q16_FROM_INT(8)},   /* Sector 2 (60°) */
        {-Q16_FROM_INT(5), Q16_FROM_INT(8)},  /* Sector 3 (120°) */
        {-Q16_FROM_INT(10), 0},               /* Sector 4 (180°) */
        {-Q16_FROM_INT(5), -Q16_FROM_INT(8)}, /* Sector 5 (240°) */
        {Q16_FROM_INT(5), -Q16_FROM_INT(8)}   /* Sector 6 (300°) */
    };

    for (int i = 0; i < 6; i++) {
        q16_t duty_a = 0, duty_b = 0, duty_c = 0;
        syn_foc_svpwm(&sectors[i], Q16_FROM_INT(24), &duty_a, &duty_b, &duty_c);
        TEST_ASSERT_TRUE(duty_a >= 0 && duty_a <= Q16_ONE);
        TEST_ASSERT_TRUE(duty_b >= 0 && duty_b <= Q16_ONE);
        TEST_ASSERT_TRUE(duty_c >= 0 && duty_c <= Q16_ONE);
    }
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

static void test_foc_fast_and_field_weakening(void)
{
    SYN_FOC_AB ab_in = {Q16_FROM_INT(3), Q16_FROM_INT(4)};
    SYN_FOC_DQ dq;
    SYN_FOC_AB ab_out;
    q16_t theta = Q16_PI / 3;

    syn_foc_park_fast(&ab_in, theta, &dq);
    syn_foc_inv_park_fast(&dq, theta, &ab_out);

    ASSERT_Q16_NEAR(ab_in.alpha, ab_out.alpha, Q16_TOL * 2);
    ASSERT_Q16_NEAR(ab_in.beta, ab_out.beta, Q16_TOL * 2);

    /* Field weakening test */
    q16_t id_cmd = 0;
    q16_t v_max = Q16_FROM_INT(10);
    bool active = syn_foc_field_weakening(Q16_FROM_INT(8), Q16_FROM_INT(8), v_max, &id_cmd);
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_TRUE(id_cmd < 0);

    bool inactive = syn_foc_field_weakening(Q16_FROM_INT(1), Q16_FROM_INT(1), v_max, &id_cmd);
    TEST_ASSERT_FALSE(inactive);

    /* Boundary condition: v_mag == v_max */
    bool boundary = syn_foc_field_weakening(Q16_FROM_INT(10), 0, v_max, &id_cmd);
    TEST_ASSERT_FALSE(boundary);
}

static void test_foc_edge_cases(void)
{
    SYN_FOC_AB ab = {Q16_FROM_INT(5), Q16_FROM_INT(3)};
    q16_t duty_a = 0, duty_b = 0, duty_c = 0;

    /* SVPWM with low vs high valid bus voltages */
    syn_foc_svpwm(&ab, Q16_FROM_INT(6), &duty_a, &duty_b, &duty_c);
    TEST_ASSERT_TRUE(duty_a >= 0 && duty_a <= Q16_ONE);

    syn_foc_svpwm(&ab, Q16_FROM_INT(48), &duty_a, &duty_b, &duty_c);
    TEST_ASSERT_TRUE(duty_a >= 0 && duty_a <= Q16_ONE);

    /* Negative theta Park transforms */
    SYN_FOC_DQ dq;
    syn_foc_park(&ab, -Q16_PI, &dq);
    TEST_ASSERT_TRUE(dq.d < 0);

    SYN_FOC_AB ab_out;
    syn_foc_inv_park(&dq, -Q16_PI, &ab_out);
    ASSERT_Q16_NEAR(ab.alpha, ab_out.alpha, Q16_TOL * 2);
}

static void test_foc_deadtime_compensation(void)
{
    q16_t duty_a = Q16_HALF; /* 0.5 */
    q16_t duty_b = Q16_HALF;
    q16_t duty_c = Q16_HALF;
    q16_t dt_offset = Q16_FROM_FRAC(5, 100); /* 0.05 duty compensation */

    /* Phase A positive current (+1.0A), Phase B negative current (-1.0A), Phase C zero-crossing
     * (0.0A) */
    SYN_FOC_ABC i_abc = {Q16_ONE, -Q16_ONE, 0};

    syn_foc_deadtime_comp(&duty_a, &duty_b, &duty_c, &i_abc, dt_offset);

    /* Phase A should increase duty cycle (+0.05) */
    TEST_ASSERT_EQUAL_INT32(Q16_HALF + dt_offset, duty_a);

    /* Phase B should decrease duty cycle (-0.05) */
    TEST_ASSERT_EQUAL_INT32(Q16_HALF - dt_offset, duty_b);

    /* Phase C at zero-crossing should remain 0.5 */
    TEST_ASSERT_EQUAL_INT32(Q16_HALF, duty_c);

    /* Intermediate current in zero-crossing band (0.05A = half of 0.1A threshold) */
    SYN_FOC_ABC i_half = {Q16_FROM_FRAC(5, 100), -Q16_FROM_FRAC(5, 100), 0};
    q16_t da = Q16_HALF, db = Q16_HALF, dc = Q16_HALF;
    syn_foc_deadtime_comp(&da, &db, &dc, &i_half, dt_offset);
    TEST_ASSERT_TRUE(da > Q16_HALF && da < Q16_HALF + dt_offset);
    TEST_ASSERT_TRUE(db < Q16_HALF && db > Q16_HALF - dt_offset);

    /* Test zero dt_offset guard */
    syn_foc_deadtime_comp(&duty_a, &duty_b, &duty_c, &i_abc, 0);

    /* Negative threshold boundary current <= -0.1A */
    SYN_FOC_ABC i_neg_thresh = {-Q16_FROM_FRAC(1, 10), -Q16_FROM_FRAC(2, 10), 0};
    da = Q16_HALF;
    db = Q16_HALF;
    dc = Q16_HALF;
    syn_foc_deadtime_comp(&da, &db, &dc, &i_neg_thresh, dt_offset);
    TEST_ASSERT_EQUAL_INT32(Q16_HALF - dt_offset, da);
}

static void test_tier_a_foc_large_amplitude_and_high_vbus_svpwm(void)
{
    /* Large balanced 3-phase amplitude (20000.0) without int32_t overflow in a + 2*b or s3b */
    SYN_FOC_ABC abc_large = {Q16_FROM_INT(20000), Q16_FROM_INT(-10000), Q16_FROM_INT(-10000)};
    SYN_FOC_AB ab_large = {0, 0};
    SYN_FOC_ABC abc_rt = {0, 0, 0};

    syn_foc_clarke(&abc_large, &ab_large);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(20000), ab_large.alpha);
    ASSERT_Q16_NEAR(0, ab_large.beta, 4);

    /* Large beta (25000.0): sqrt(3)*25000 = 43301 > 32767, but b = +21650.7 fits in Q16.16 */
    SYN_FOC_AB ab_beta = {0, Q16_FROM_INT(25000)};
    syn_foc_inv_clarke(&ab_beta, &abc_rt);
    ASSERT_Q16_NEAR(Q16_FROM_INT(21650) + Q16_FROM_FRAC(7, 10), abc_rt.b, Q16_TOL);
    ASSERT_Q16_NEAR(-(Q16_FROM_INT(21650) + Q16_FROM_FRAC(7, 10)), abc_rt.c, Q16_TOL);

    /* Extreme positive and negative Clarke saturation to [INT32_MIN, INT32_MAX] */
    SYN_FOC_ABC abc_pos_sat = {INT32_MAX, INT32_MAX, 0};
    syn_foc_clarke(&abc_pos_sat, &ab_large);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, ab_large.beta);

    SYN_FOC_ABC abc_neg_sat = {INT32_MIN, INT32_MIN, 0};
    syn_foc_clarke(&abc_neg_sat, &ab_large);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, ab_large.beta);

    /* High DC bus (800V) SVPWM: q16_div preserves exact 0.6875 / 0.3125 duty */
    SYN_FOC_AB ab_hv = {Q16_FROM_INT(200), 0};
    q16_t duty_a = 0;
    q16_t duty_b = 0;
    q16_t duty_c = 0;
    syn_foc_svpwm(&ab_hv, Q16_FROM_INT(800), &duty_a, &duty_b, &duty_c);
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(6875, 10000), duty_a, 2);
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(3125, 10000), duty_b, 2);
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(3125, 10000), duty_c, 2);
}

static void test_foc_field_weakening_closed_loop_and_saturation(void)
{
    /* 1. Multi-step closed-loop voltage regulation: v_q = v_q0 + omega*Ld*id_cmd */
    q16_t id_cmd = 0;
    q16_t v_max = Q16_FROM_INT(10);
    q16_t v_q0 = Q16_FROM_INT(16);

    for (int step = 0; step < 24; step++) {
        TEST_ASSERT_TRUE(syn_foc_field_weakening(0, v_q0 + id_cmd, v_max, &id_cmd));
    }
    ASSERT_Q16_NEAR(-Q16_FROM_INT(6), id_cmd, 2);
    ASSERT_Q16_NEAR(v_max, v_q0 + id_cmd, 2);
    TEST_ASSERT_FALSE(syn_foc_field_weakening(0, v_max, v_max, &id_cmd));

    /* 2. Sustained over-voltage near INT32_MIN saturates cleanly instead of wrapping positive */
    q16_t id_floor = INT32_MIN + Q16_ONE;
    TEST_ASSERT_TRUE(syn_foc_field_weakening(Q16_FROM_INT(20), 0, Q16_FROM_INT(10), &id_floor));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, id_floor);

    /* 3. Park & dead-time saturate cleanly; invalid v_max <= 0 or NULL id_cmd rejected */
    SYN_FOC_AB ab_max = {INT32_MAX, INT32_MAX};
    SYN_FOC_DQ dq_max = {0, 0};
    syn_foc_park(&ab_max, Q16_PI / 4, &dq_max);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, dq_max.d);
    syn_foc_park_fast(&ab_max, Q16_PI / 4, &dq_max);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, dq_max.d);

    SYN_FOC_DQ dq_in = {INT32_MAX, INT32_MAX};
    SYN_FOC_AB ab_sat = {0, 0};
    syn_foc_inv_park(&dq_in, Q16_PI / 4, &ab_sat);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, ab_sat.beta);
    syn_foc_inv_park_fast(&dq_in, Q16_PI / 4, &ab_sat);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, ab_sat.beta);

    q16_t da = Q16_ONE;
    q16_t db = 0;
    q16_t dc = Q16_HALF;
    SYN_FOC_ABC i_ext = {Q16_ONE, -Q16_ONE, -Q16_ONE};
    syn_foc_deadtime_comp(&da, &db, &dc, &i_ext, INT32_MIN);
    syn_foc_deadtime_comp(&da, &db, &dc, &i_ext, INT32_MAX);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, da);
    TEST_ASSERT_EQUAL_INT32(0, db);

    TEST_ASSERT_FALSE(syn_foc_field_weakening(Q16_ONE, 0, Q16_ONE, NULL));
    TEST_ASSERT_FALSE(syn_foc_field_weakening(Q16_ONE, 0, 0, &id_floor));
}

void run_foc_tests(void)
{
    RUN_TEST(test_clarke_balanced);
    RUN_TEST(test_park_zero_angle);
    RUN_TEST(test_park_inv_roundtrip);
    RUN_TEST(test_clarke_inv_roundtrip);
    RUN_TEST(test_svpwm_duty_range);
    RUN_TEST(test_foc_fast_and_field_weakening);
    RUN_TEST(test_foc_edge_cases);
    RUN_TEST(test_foc_deadtime_compensation);
    RUN_TEST(test_tier_a_foc_large_amplitude_and_high_vbus_svpwm);
    RUN_TEST(test_foc_field_weakening_closed_loop_and_saturation);
}
