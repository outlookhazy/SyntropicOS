/**
 * @file test_kalman.c
 * @brief Unity tests for syn_kalman (Kalman filter).
 */

#include "mocks/mock_port.h"
#include "syntropic/dsp/syn_kalman.h"
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

#define Q16_TOL 131     /* ±0.002 */
#define Q16_KF_TOL 6554 /* ±0.1 — Kalman convergence tolerance */

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

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Q;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas = 1;

    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_init(&kf, &cfg));
}

static void test_kalman_init_bad_dims(void)
{
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;

    SYN_MAT_DECL(x, 2, 1);
    SYN_MAT_DECL(P, 2, 2);
    SYN_MAT_DECL(F, 2, 2);
    SYN_MAT_DECL(Q, 2, 2);
    SYN_MAT_DECL(H, 1, 2);
    SYN_MAT_DECL(R, 1, 1);

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Q;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas = 1;

    /* 1. x mismatch */
    SYN_MAT_DECL(x_bad, 3, 1);
    SYN_MAT_DECL(x_bad_cols, 2, 2);
    cfg.x = &x_bad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.x = &x_bad_cols;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.x = &x;

    /* 2. P mismatch */
    SYN_MAT_DECL(P_bad, 3, 2);
    SYN_MAT_DECL(P_bad_cols, 2, 1);
    cfg.P = &P_bad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.P = &P_bad_cols;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.P = &P;

    /* 3. F mismatch */
    SYN_MAT_DECL(F_bad, 3, 2);
    SYN_MAT_DECL(F_bad_cols, 2, 1);
    cfg.F = &F_bad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.F = &F_bad_cols;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.F = &F;

    /* 4. Q mismatch */
    SYN_MAT_DECL(Q_bad, 3, 2);
    SYN_MAT_DECL(Q_bad_cols, 2, 1);
    cfg.Q = &Q_bad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.Q = &Q_bad_cols;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.Q = &Q;

    /* 5. H mismatch */
    SYN_MAT_DECL(H_bad, 3, 2);
    SYN_MAT_DECL(H_bad_cols, 1, 1);
    cfg.H = &H_bad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.H = &H_bad_cols;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.H = &H;

    /* 6. R mismatch */
    SYN_MAT_DECL(R_bad, 3, 1);
    SYN_MAT_DECL(R_bad_cols, 1, 2);
    cfg.R = &R_bad;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.R = &R_bad_cols;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.R = &R;

    /* 7. Max dimensions exceeded */
    cfg.n_state = SYN_KALMAN_MAX_STATE + 1;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.n_state = 2;
    cfg.n_meas = SYN_KALMAN_MAX_MEAS + 1;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.n_meas = 0;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.n_meas = 1;

    /* 8. NULL matrix pointer in config */
    cfg.x = NULL;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_init(&kf, &cfg));
    cfg.x = &x;
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
    SYN_MAT_AT(&Qn, 0, 0) = Q16_FROM_FRAC(1, 100); /* 0.01 */

    /* Measurement noise: moderate */
    syn_matrix_zero(&R);
    SYN_MAT_AT(&R, 0, 0) = Q16_ONE; /* 1.0 */

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Qn;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 1;
    cfg.n_meas = 1;

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

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Qn;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 1;
    cfg.n_meas = 1;

    syn_kalman_init(&kf, &cfg);
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    q16_t P_before = P.data[0];
    syn_kalman_predict(&kf);
    q16_t P_after = P.data[0];

    /* P should grow after predict (P⁻ = F·P·Fᵀ + Q > P) */
    TEST_ASSERT_TRUE(P_after > P_before);
}

/* ── Runner ─────────────────────────────────────────────────────────────── */

static void test_kalman_singular_S_matrix_update_fail(void)
{
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

    syn_matrix_identity(&F);
    syn_matrix_zero(&H); /* H=0 makes S = 0 + R */
    syn_matrix_zero(&x);
    syn_matrix_zero(&P);
    syn_matrix_zero(&Qn);
    syn_matrix_zero(&R); /* R=0 makes S singular (0) */

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Qn;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 1;
    cfg.n_meas = 1;

    syn_kalman_init(&kf, &cfg);
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    z.data[0] = 100;
    /* Updating with singular covariance matrix returns SYN_ERROR */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kalman_update(&kf, &z));
}

static void test_kalman_2state_update(void)
{
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;

    SYN_MAT_DECL(x, 2, 1);
    SYN_MAT_DECL(P, 2, 2);
    SYN_MAT_DECL(F, 2, 2);
    SYN_MAT_DECL(Qn, 2, 2);
    SYN_MAT_DECL(H, 1, 2);
    SYN_MAT_DECL(R, 1, 1);
    SYN_MAT_DECL(z, 1, 1);

    SYN_KALMAN_SCRATCH_DECL(s, 2, 1);

    syn_matrix_identity(&F);
    syn_matrix_identity(&P);
    syn_matrix_identity(&Qn);
    syn_matrix_identity(&R);
    syn_matrix_zero(&x);

    H.data[0] = Q16_ONE;
    H.data[1] = Q16_ONE;

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Qn;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas = 1;

    syn_kalman_init(&kf, &cfg);
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    syn_kalman_predict(&kf);
    z.data[0] = Q16_FROM_INT(5);
    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_update(&kf, &z));
}

static void test_tier_a_kalman_null_scratch_and_param_guards(void)
{
    SYN_Kalman kf = {0};
    SYN_Kalman_Config cfg = {0};

    SYN_MAT_DECL(x, 2, 1);
    SYN_MAT_DECL(P, 2, 2);
    SYN_MAT_DECL(F, 2, 2);
    SYN_MAT_DECL(Qn, 2, 2);
    SYN_MAT_DECL(H, 1, 2);
    SYN_MAT_DECL(R, 1, 1);
    SYN_MAT_DECL(z, 1, 1);
    SYN_MAT_DECL(z_bad, 2, 1);
    SYN_KALMAN_SCRATCH_DECL(s, 2, 1);

    syn_matrix_identity(&F);
    syn_matrix_identity(&P);
    syn_matrix_identity(&Qn);
    syn_matrix_identity(&R);
    syn_matrix_zero(&x);
    syn_matrix_zero(&H);
    H.data[0] = Q16_ONE;

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Qn;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas = 1;

    /* NUM-03a: NULL kf or uninitialized cfg handled safely */
    syn_kalman_predict(NULL);
    syn_kalman_predict(&kf);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_update(NULL, &z));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_update(&kf, &z));

    /* NUM-03b respected: syn_kalman_init succeeds before scratch assignment */
    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_init(&kf, &cfg));

    /* NUM-03a: Calling predict/update with NULL scratch buffers fails safely without crash */
    x.data[0] = Q16_FROM_INT(3);
    syn_kalman_predict(&kf);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(3), x.data[0]);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_update(&kf, &z));

    /* Assign scratch, then verify NULL/mismatched z, missing update-only scratch, and bad cfg */
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_update(&kf, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_update(&kf, &z_bad));

    kf.temp_m1 = NULL;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kalman_update(&kf, &z));
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    cfg.n_state = 0U;
    syn_kalman_predict(&kf);
    cfg.n_state = 2U;

    z.data[0] = Q16_FROM_INT(3);
    syn_kalman_predict(&kf);
    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_update(&kf, &z));
}

static void test_kalman_coupled_velocity_and_2meas_fusion(void)
{
    /* 1. Coupled 2-state [pos, vel] filter estimating unmeasured velocity from position ramp */
    SYN_Kalman kf;
    SYN_Kalman_Config cfg;
    SYN_MAT_DECL(x, 2, 1);
    SYN_MAT_DECL(P, 2, 2);
    SYN_MAT_DECL(F, 2, 2);
    SYN_MAT_DECL(Qn, 2, 2);
    SYN_MAT_DECL(H, 1, 2);
    SYN_MAT_DECL(R, 1, 1);
    SYN_MAT_DECL(z, 1, 1);
    SYN_KALMAN_SCRATCH_DECL(s, 2, 1);

    syn_matrix_zero(&x);
    syn_matrix_identity(&P);
    SYN_MAT_AT(&P, 0, 0) = Q16_FROM_INT(10);
    SYN_MAT_AT(&P, 1, 1) = Q16_FROM_INT(10);

    syn_matrix_identity(&F);
    SYN_MAT_AT(&F, 0, 1) = Q16_FROM_FRAC(1, 10); /* dt = 0.1 s */

    syn_matrix_zero(&Qn);
    SYN_MAT_AT(&Qn, 0, 0) = Q16_FROM_FRAC(1, 100);
    SYN_MAT_AT(&Qn, 1, 1) = Q16_FROM_FRAC(1, 100);

    syn_matrix_zero(&H);
    SYN_MAT_AT(&H, 0, 0) = Q16_ONE; /* Measure position only */

    syn_matrix_zero(&R);
    SYN_MAT_AT(&R, 0, 0) = Q16_FROM_FRAC(1, 20);

    cfg.x = &x;
    cfg.P = &P;
    cfg.F = &F;
    cfg.Q = &Qn;
    cfg.H = &H;
    cfg.R = &R;
    cfg.n_state = 2;
    cfg.n_meas = 1;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_init(&kf, &cfg));
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s);

    /* Constant velocity v = 2.0 units/s -> pos(k) = 0.2 * (k + 1) */
    for (int k = 0; k < 40; k++) {
        syn_kalman_predict(&kf);
        z.data[0] = Q16_FROM_FRAC(2 * (k + 1), 10);
        TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_update(&kf, &z));
    }
    ASSERT_Q16_NEAR(Q16_FROM_INT(8), SYN_MAT_AT(&x, 0, 0), Q16_KF_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), SYN_MAT_AT(&x, 1, 0), Q16_KF_TOL);

    /* 2. Multi-measurement (n_meas = 2) 2x2 innovation inverse path */
    SYN_MAT_DECL(x2, 2, 1);
    SYN_MAT_DECL(P2, 2, 2);
    SYN_MAT_DECL(F2, 2, 2);
    SYN_MAT_DECL(Q2, 2, 2);
    SYN_MAT_DECL(H2, 2, 2);
    SYN_MAT_DECL(R2, 2, 2);
    SYN_MAT_DECL(z2, 2, 1);
    SYN_KALMAN_SCRATCH_DECL(s2, 2, 2);

    syn_matrix_zero(&x2);
    syn_matrix_identity(&P2);
    SYN_MAT_AT(&P2, 0, 0) = Q16_FROM_INT(10);
    SYN_MAT_AT(&P2, 1, 1) = Q16_FROM_INT(10);
    syn_matrix_identity(&F2);
    syn_matrix_identity(&H2);
    syn_matrix_zero(&Q2);
    SYN_MAT_AT(&Q2, 0, 0) = Q16_FROM_FRAC(1, 100);
    SYN_MAT_AT(&Q2, 1, 1) = Q16_FROM_FRAC(1, 100);
    syn_matrix_zero(&R2);
    SYN_MAT_AT(&R2, 0, 0) = Q16_HALF;
    SYN_MAT_AT(&R2, 1, 1) = Q16_HALF;

    cfg.x = &x2;
    cfg.P = &P2;
    cfg.F = &F2;
    cfg.Q = &Q2;
    cfg.H = &H2;
    cfg.R = &R2;
    cfg.n_state = 2;
    cfg.n_meas = 2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_init(&kf, &cfg));
    SYN_KALMAN_SCRATCH_ASSIGN(&kf, s2);

    for (int k = 0; k < 25; k++) {
        syn_kalman_predict(&kf);
        SYN_MAT_AT(&z2, 0, 0) = Q16_FROM_INT(7);
        SYN_MAT_AT(&z2, 1, 0) = -Q16_FROM_INT(3);
        TEST_ASSERT_EQUAL(SYN_OK, syn_kalman_update(&kf, &z2));
    }
    ASSERT_Q16_NEAR(Q16_FROM_INT(7), SYN_MAT_AT(&x2, 0, 0), Q16_KF_TOL);
    ASSERT_Q16_NEAR(-Q16_FROM_INT(3), SYN_MAT_AT(&x2, 1, 0), Q16_KF_TOL);
}

void run_kalman_tests(void)
{
    RUN_TEST(test_kalman_init_valid);
    RUN_TEST(test_kalman_init_bad_dims);
    RUN_TEST(test_kalman_constant_converge);
    RUN_TEST(test_kalman_predict_grows_P);
    RUN_TEST(test_kalman_singular_S_matrix_update_fail);
    RUN_TEST(test_kalman_2state_update);
    RUN_TEST(test_tier_a_kalman_null_scratch_and_param_guards);
    RUN_TEST(test_kalman_coupled_velocity_and_2meas_fusion);
}
