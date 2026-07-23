/**
 * @file test_kalman.c
 * @brief Unity tests for syn_kalman (Kalman filter).
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/dsp/syn_kalman.h"

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

#define Q16_TOL      131   /* ±0.002 */
#define Q16_KF_TOL   6554  /* ±0.1 — Kalman convergence tolerance */

/* ── Test: init validates dimensions ───────────────────────────────────── */

static void test_kalman_init_valid(void)
{
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;

    SYN_MAT_DECL(x, 2, 1);
    SYN_MAT_DECL(P, 2, 2);
    SYN_MAT_DECL(F, 2, 2);
    SYN_MAT_DECL(Q, 2, 2);
    SYN_MAT_DECL(H, 1, 2);
    SYN_MAT_DECL(R, 1, 1);

    cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
    cfg.Q = &Q;  cfg.H = &H;  cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas  = 1;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_init(&kf, &cfg));
}

static void test_kalman_init_bad_dims(void)
{
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;

    /* x is 3×1 but n_state = 2 → mismatch */
    SYN_MAT_DECL(x, 3, 1);
    SYN_MAT_DECL(P, 2, 2);
    SYN_MAT_DECL(F, 2, 2);
    SYN_MAT_DECL(Q, 2, 2);
    SYN_MAT_DECL(H, 1, 2);
    SYN_MAT_DECL(R, 1, 1);

    cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
    cfg.Q = &Q;  cfg.H = &H;  cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas  = 1;

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
}

/* ── Test: constant value converges ────────────────────────────────────── */

static void test_kalman_constant_converge(void)
{
    /*
     * 1-state, 1-measurement filter. State = constant value.
     * F = [1], H = [1], Q = small, R = moderate.
     * Feed the same measurement repeatedly. The filter state
     * should converge to the measurement value.
     */
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;

    SYN_MAT_DECL(x, 1, 1);
    SYN_MAT_DECL(P, 1, 1);
    SYN_MAT_DECL(F, 1, 1);
    SYN_MAT_DECL(Qn, 1, 1);
    SYN_MAT_DECL(H, 1, 1);
    SYN_MAT_DECL(R, 1, 1);
    SYN_MAT_DECL(z, 1, 1);

    SYN_KALMAN_SCRATCH_DECL(s, 1, 1);

    /* State transition: identity */
    syn_matrix_identity(&F);

    /* Measurement model: direct observation */
    syn_matrix_identity(&H);

    /* Initial state: 0 */
    syn_matrix_zero(&x);

    /* Initial covariance: large uncertainty */
    syn_matrix_zero(&P);
    SYN_MAT_AT(&P, 0, 0) = Q16_FROM_INT(100);

    /* Process noise: small */
    syn_matrix_zero(&Qn);
    SYN_MAT_AT(&Qn, 0, 0) = Q16_FROM_FRAC(1, 100);  /* 0.01 */

    /* Measurement noise: moderate */
    syn_matrix_zero(&R);
    SYN_MAT_AT(&R, 0, 0) = Q16_ONE;  /* 1.0 */

    cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
    cfg.Q = &Qn; cfg.H = &H;  cfg.R = &R;
    cfg.n_state = 1;
    cfg.n_meas  = 1;

    syn_kalman_init(&kf, &cfg);
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    /* Feed measurement z = 5.0 repeatedly */
    q16_t meas_val = Q16_FROM_INT(5);
    int i;
    for (i = 0; i < 20; i++) {
        z.data[0] = meas_val;
        syn_kalman_predict(&kf);
        syn_kalman_update(&kf, &z);
    }

    /* State should have converged close to 5.0 */
    ASSERT_Q16_NEAR(meas_val, x.data[0], Q16_KF_TOL);
}

/* ── Test: predict-only grows covariance ───────────────────────────────── */

static void test_kalman_predict_grows_P(void)
{
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;

    SYN_MAT_DECL(x, 1, 1);
    SYN_MAT_DECL(P, 1, 1);
    SYN_MAT_DECL(F, 1, 1);
    SYN_MAT_DECL(Qn, 1, 1);
    SYN_MAT_DECL(H, 1, 1);
    SYN_MAT_DECL(R, 1, 1);

    SYN_KALMAN_SCRATCH_DECL(s, 1, 1);

    syn_matrix_identity(&F);
    syn_matrix_identity(&H);
    syn_matrix_zero(&x);

    SYN_MAT_AT(&P, 0, 0) = Q16_ONE;
    SYN_MAT_AT(&Qn, 0, 0) = Q16_HALF;
    SYN_MAT_AT(&R, 0, 0) = Q16_ONE;

    cfg.x = &x;  cfg.P = &P;  cfg.F = &F;
    cfg.Q = &Qn; cfg.H = &H;  cfg.R = &R;
    cfg.n_state = 1;
    cfg.n_meas  = 1;

    syn_kalman_init(&kf, &cfg);
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    q16_t P_before = P.data[0];
    syn_kalman_predict(&kf);
    q16_t P_after = P.data[0];

    /* P should grow after predict (P⁻ = F·P·Fᵀ + Q > P) */
    TEST_ASSERT_TRUE(P_after > P_before);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

void run_kalman_tests(void)
{
    RUN_TEST(test_kalman_init_valid);
    RUN_TEST(test_kalman_init_bad_dims);
    RUN_TEST(test_kalman_constant_converge);
    RUN_TEST(test_kalman_predict_grows_P);
}
