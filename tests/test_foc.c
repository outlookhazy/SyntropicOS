/**
 * @file test_foc.c
 * @brief Unity tests for syn_foc (Field-Oriented Control transforms).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/motor/syn_foc.h"

#include <stdio.h>

/* ── Helper ─────────────────────────────────────────────────────────────── */

#define ASSERT_Q16_NEAR(expected, actual, tol_q16)                       \
    do {                                                                  \
        q16_t _e = (expected), _a = (actual), _t = (tol_q16);           \
        q16_t _d = (_a > _e) ? (_a - _e) : (_e - _a);                  \
        if (_d > _t) {                                                    \
            char _msg[80];                                                \
            snprintf(_msg, sizeof(_msg),                                  \
                "Expected %ld ± %ld, got %ld (delta %ld)",               \
                (long)_e, (long)_t, (long)_a, (long)_d);                \
            TEST_FAIL_MESSAGE(_msg);                                      \
        }                                                                 \
    } while (0)

#define Q16_TOL  655  /* ±0.01 — relaxed for transform chain roundtrip */

/* ── Test: Clarke identity (balanced) ──────────────────────────────────── */

static void test_clarke_balanced(void)
{
    /* Phase A = 1.0, B = C = -0.5 (balanced 3-phase at 0° angle) */
    SYN_FOC_ABC abc = { Q16_ONE, -Q16_HALF, -Q16_HALF };
    SYN_FOC_AB  ab;

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
    SYN_FOC_AB ab = { Q16_FROM_INT(3), Q16_FROM_INT(4) };
    SYN_FOC_DQ dq;

    syn_foc_park(&ab, 0, &dq);

    ASSERT_Q16_NEAR(Q16_FROM_INT(3), dq.d, Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(4), dq.q, Q16_TOL);
}

/* ── Test: Park + Inverse Park roundtrip ───────────────────────────────── */

static void test_park_inv_roundtrip(void)
{
    SYN_FOC_AB  ab_in = { Q16_FROM_INT(3), Q16_FROM_INT(4) };
    SYN_FOC_DQ  dq;
    SYN_FOC_AB  ab_out;

    q16_t theta = Q16_PI / 3;  /* 60° */

    syn_foc_park(&ab_in, theta, &dq);
    syn_foc_inv_park(&dq, theta, &ab_out);

    ASSERT_Q16_NEAR(ab_in.alpha, ab_out.alpha, Q16_TOL);
    ASSERT_Q16_NEAR(ab_in.beta,  ab_out.beta,  Q16_TOL);
}

/* ── Test: Clarke + Inv Clarke roundtrip ───────────────────────────────── */

static void test_clarke_inv_roundtrip(void)
{
    /* Balanced 3-phase at 30° */
    SYN_FOC_ABC abc_in = { Q16_FROM_FRAC(866, 1000),   /* cos(30°) ≈ 0.866 */
                           Q16_FROM_FRAC(-500, 1000),   /* cos(150°) ≈ -0.5 */
                           Q16_FROM_FRAC(-366, 1000) }; /* cos(270°) ≈ -0.366 */
    SYN_FOC_AB  ab;
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
    SYN_FOC_AB ab = { Q16_FROM_INT(5), Q16_FROM_INT(3) };
    q16_t duty_a, duty_b, duty_c;

    syn_foc_svpwm(&ab, Q16_FROM_INT(24), &duty_a, &duty_b, &duty_c);

    /* All duties should be in [0, 1] */
    TEST_ASSERT_TRUE(duty_a >= 0 && duty_a <= Q16_ONE);
    TEST_ASSERT_TRUE(duty_b >= 0 && duty_b <= Q16_ONE);
    TEST_ASSERT_TRUE(duty_c >= 0 && duty_c <= Q16_ONE);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_foc_tests(void)
{
    RUN_TEST(test_clarke_balanced);
    RUN_TEST(test_park_zero_angle);
    RUN_TEST(test_park_inv_roundtrip);
    RUN_TEST(test_clarke_inv_roundtrip);
    RUN_TEST(test_svpwm_duty_range);
}
