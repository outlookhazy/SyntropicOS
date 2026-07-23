/**
 * @file test_matrix.c
 * @brief Unity tests for syn_matrix and extended syn_qmath.
 */

#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_qmath.h"
#include "syntropic/util/syn_matrix.h"

/* ── Helper: assert Q16 value is within tolerance of expected ──────────── */

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

/* Tolerance: ±0.002 in Q16 ≈ 131 LSBs */
#define Q16_TOL  131

/* Tolerance for matrix roundtrip: ±0.02 ≈ 1311 LSBs */
#define Q16_MAT_TOL  1311

#include <stdio.h>

/* ════════════════════════════════════════════════════════════════════════ */
/*  Extended Q16 Math Tests                                                */
/* ════════════════════════════════════════════════════════════════════════ */

static void test_q16_sqrt(void)
{
    /* sqrt(0) = 0 */
    TEST_ASSERT_EQUAL(0, q16_sqrt(0));

    /* sqrt(1) = 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_sqrt(Q16_ONE), Q16_TOL);

    /* sqrt(4) = 2 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), q16_sqrt(Q16_FROM_INT(4)), Q16_TOL);

    /* sqrt(9) = 3 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(3), q16_sqrt(Q16_FROM_INT(9)), Q16_TOL);

    /* sqrt(2) ≈ 1.41421 */
    ASSERT_Q16_NEAR(Q16_SQRT2, q16_sqrt(Q16_FROM_INT(2)), Q16_TOL);

    /* sqrt(0.25) = 0.5 */
    ASSERT_Q16_NEAR(Q16_HALF, q16_sqrt(Q16_FROM_FRAC(1, 4)), Q16_TOL);

    /* sqrt(100) = 10 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(10), q16_sqrt(Q16_FROM_INT(100)), Q16_TOL);

    /* Negative input returns 0 */
    TEST_ASSERT_EQUAL(0, q16_sqrt(Q16_FROM_INT(-1)));
}

static void test_q16_hypot(void)
{
    /* hypot(3, 4) = 5 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(5),
                    q16_hypot(Q16_FROM_INT(3), Q16_FROM_INT(4)), Q16_TOL);

    /* hypot(0, 5) = 5 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(5),
                    q16_hypot(0, Q16_FROM_INT(5)), Q16_TOL);

    /* hypot(1, 1) ≈ 1.41421 */
    ASSERT_Q16_NEAR(Q16_SQRT2,
                    q16_hypot(Q16_ONE, Q16_ONE), Q16_TOL);
}

static void test_q16_trig(void)
{
    /* sin(0) = 0 */
    ASSERT_Q16_NEAR(0, q16_sin(0), Q16_TOL);

    /* sin(π/2) = 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_sin(Q16_PI_2), Q16_TOL);

    /* sin(π) = 0 */
    ASSERT_Q16_NEAR(0, q16_sin(Q16_PI), Q16_TOL);

    /* cos(0) = 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_cos(0), Q16_TOL);

    /* cos(π) = -1 */
    ASSERT_Q16_NEAR(-Q16_ONE, q16_cos(Q16_PI), Q16_TOL);

    /* sin²(π/4) + cos²(π/4) = 1 */
    q16_t s = q16_sin(Q16_PI / 4);
    q16_t c = q16_cos(Q16_PI / 4);
    ASSERT_Q16_NEAR(Q16_ONE, q16_mul(s, s) + q16_mul(c, c), Q16_TOL);

    /* tan(0) = 0 */
    ASSERT_Q16_NEAR(0, q16_tan(0), Q16_TOL);

    /* tan(π/4) ≈ 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_tan(Q16_PI / 4), Q16_TOL);
}

static void test_q16_atan2(void)
{
    /* atan2(0, 1) = 0 */
    ASSERT_Q16_NEAR(0, q16_atan2(0, Q16_ONE), Q16_TOL);

    /* atan2(1, 0) = π/2 */
    ASSERT_Q16_NEAR(Q16_PI_2, q16_atan2(Q16_ONE, 0), Q16_TOL);

    /* atan2(0, -1) = π */
    ASSERT_Q16_NEAR(Q16_PI, q16_atan2(0, -Q16_ONE), Q16_TOL);

    /* atan2(-1, 0) = -π/2 */
    ASSERT_Q16_NEAR(-Q16_PI_2, q16_atan2(-Q16_ONE, 0), Q16_TOL);

    /* atan2(1, 1) = π/4 */
    ASSERT_Q16_NEAR(Q16_PI / 4, q16_atan2(Q16_ONE, Q16_ONE), Q16_TOL * 4);

    /* atan2(-1, -1) = -3π/4 */
    q16_t expected_neg_3pi4 = -(q16_t)((int64_t)Q16_PI * 3 / 4);
    ASSERT_Q16_NEAR(expected_neg_3pi4,
                    q16_atan2(-Q16_ONE, -Q16_ONE), Q16_TOL * 4);
}

static void test_q16_asin_acos(void)
{
    /* asin(0) = 0 */
    ASSERT_Q16_NEAR(0, q16_asin(0), Q16_TOL);

    /* asin(1) = π/2 */
    ASSERT_Q16_NEAR(Q16_PI_2, q16_asin(Q16_ONE), Q16_TOL);

    /* asin(-1) = -π/2 */
    ASSERT_Q16_NEAR(-Q16_PI_2, q16_asin(-Q16_ONE), Q16_TOL);

    /* acos(1) = 0 */
    ASSERT_Q16_NEAR(0, q16_acos(Q16_ONE), Q16_TOL);

    /* acos(0) = π/2 */
    ASSERT_Q16_NEAR(Q16_PI_2, q16_acos(0), Q16_TOL);

    /* acos(-1) = π */
    ASSERT_Q16_NEAR(Q16_PI, q16_acos(-Q16_ONE), Q16_TOL);
}

static void test_q16_exp_log(void)
{
    /* exp(0) = 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_exp(0), Q16_TOL);

    /* exp(1) ≈ e ≈ 2.71828 */
    ASSERT_Q16_NEAR(Q16_E, q16_exp(Q16_ONE), Q16_TOL * 2);

    /* log(1) = 0 */
    ASSERT_Q16_NEAR(0, q16_log(Q16_ONE), Q16_TOL);

    /* log(e) ≈ 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_log(Q16_E), Q16_TOL * 2);

    /* Roundtrip: exp(log(x)) ≈ x for x = 3 */
    q16_t three = Q16_FROM_INT(3);
    ASSERT_Q16_NEAR(three, q16_exp(q16_log(three)), Q16_TOL * 4);

    /* log(2) ≈ 0.6931 */
    ASSERT_Q16_NEAR(Q16_LN2, q16_log(Q16_FROM_INT(2)), Q16_TOL * 2);

    /* exp(-1) ≈ 0.3679 */
    q16_t exp_neg1 = Q16_FROM_FLOAT(0.36788f);
    ASSERT_Q16_NEAR(exp_neg1, q16_exp(-Q16_ONE), Q16_TOL * 2);
}

static void test_q16_pow(void)
{
    /* 2^0 = 1 */
    ASSERT_Q16_NEAR(Q16_ONE, q16_pow(Q16_FROM_INT(2), 0), Q16_TOL);

    /* 2^1 = 2 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), q16_pow(Q16_FROM_INT(2), Q16_ONE), Q16_TOL * 2);

    /* 2^10 = 1024 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(1024),
                    q16_pow(Q16_FROM_INT(2), Q16_FROM_INT(10)), Q16_FROM_INT(2));

    /* 4^0.5 = 2 (square root via pow) */
    ASSERT_Q16_NEAR(Q16_FROM_INT(2),
                    q16_pow(Q16_FROM_INT(4), Q16_HALF), Q16_TOL * 4);
}

static void test_q16_str(void)
{
    char buf[24];

    /* to_str: positive integer */
    q16_to_str(Q16_FROM_INT(42), buf, sizeof(buf), 0);
    TEST_ASSERT_EQUAL_STRING("42", buf);

    /* to_str: negative with decimals */
    q16_to_str(-Q16_FROM_FRAC(7, 2), buf, sizeof(buf), 1);
    TEST_ASSERT_EQUAL_STRING("-3.5", buf);

    /* to_str: zero */
    q16_to_str(0, buf, sizeof(buf), 3);
    TEST_ASSERT_EQUAL_STRING("0.000", buf);

    /* from_str: positive */
    q16_t val;
    size_t consumed = q16_from_str("3.14", &val);
    TEST_ASSERT_EQUAL(4, consumed);
    ASSERT_Q16_NEAR(Q16_FROM_FLOAT(3.14f), val, Q16_TOL);

    /* from_str: negative */
    consumed = q16_from_str("-12.5", &val);
    TEST_ASSERT_EQUAL(5, consumed);
    ASSERT_Q16_NEAR(-Q16_FROM_FRAC(25, 2), val, Q16_TOL);

    /* from_str: integer only */
    consumed = q16_from_str("100", &val);
    TEST_ASSERT_EQUAL(3, consumed);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(100), val);
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Matrix Tests                                                           */
/* ════════════════════════════════════════════════════════════════════════ */

static void test_mat_identity_mul(void)
{
    /* I × A = A for a 3×3 matrix */
    SYN_MAT3_DECL(I);
    SYN_MAT3_DECL(A);
    SYN_MAT3_DECL(C);

    syn_matrix_identity(&I);

    /* Fill A with known values */
    uint8_t i;
    for (i = 0; i < 9; i++) {
        A.data[i] = Q16_FROM_INT(i + 1);
    }

    syn_matrix_mul(&I, &A, &C);

    for (i = 0; i < 9; i++) {
        ASSERT_Q16_NEAR(A.data[i], C.data[i], Q16_TOL);
    }
}

static void test_mat_transpose(void)
{
    SYN_MAT_DECL(A, 2, 3);
    SYN_MAT_DECL(AT, 3, 2);

    /* A = [[1, 2, 3],
     *      [4, 5, 6]] */
    A.data[0] = Q16_FROM_INT(1); A.data[1] = Q16_FROM_INT(2); A.data[2] = Q16_FROM_INT(3);
    A.data[3] = Q16_FROM_INT(4); A.data[4] = Q16_FROM_INT(5); A.data[5] = Q16_FROM_INT(6);

    syn_matrix_transpose(&A, &AT);

    /* AT = [[1, 4],
     *       [2, 5],
     *       [3, 6]] */
    TEST_ASSERT_EQUAL(Q16_FROM_INT(1), SYN_MAT_AT(&AT, 0, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4), SYN_MAT_AT(&AT, 0, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(2), SYN_MAT_AT(&AT, 1, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), SYN_MAT_AT(&AT, 1, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(3), SYN_MAT_AT(&AT, 2, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(6), SYN_MAT_AT(&AT, 2, 1));
}

static void test_mat_det_2x2(void)
{
    /* A = [[3, 8], [4, 6]]  →  det = 3*6 - 8*4 = -14 */
    SYN_MAT2_DECL(A);
    A.data[0] = Q16_FROM_INT(3); A.data[1] = Q16_FROM_INT(8);
    A.data[2] = Q16_FROM_INT(4); A.data[3] = Q16_FROM_INT(6);

    ASSERT_Q16_NEAR(Q16_FROM_INT(-14), syn_matrix_det(&A), Q16_TOL);
}

static void test_mat_inverse_2x2(void)
{
    /* A = [[4, 7], [2, 6]]  →  A⁻¹ = (1/10)*[[6, -7], [-2, 4]] */
    SYN_MAT2_DECL(A);
    SYN_MAT2_DECL(Ainv);
    SYN_MAT2_DECL(I_check);

    A.data[0] = Q16_FROM_INT(4); A.data[1] = Q16_FROM_INT(7);
    A.data[2] = Q16_FROM_INT(2); A.data[3] = Q16_FROM_INT(6);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv(&A, &Ainv));

    /* Verify A × A⁻¹ ≈ I */
    syn_matrix_mul(&A, &Ainv, &I_check);

    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&I_check, 0, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0,       SYN_MAT_AT(&I_check, 0, 1), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0,       SYN_MAT_AT(&I_check, 1, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&I_check, 1, 1), Q16_MAT_TOL);
}

static void test_mat_inverse_3x3(void)
{
    /* A = [[1, 2, 3], [0, 1, 4], [5, 6, 0]] → det = 1 */
    SYN_MAT3_DECL(A);
    SYN_MAT3_DECL(Ainv);
    SYN_MAT3_DECL(I_check);

    A.data[0] = Q16_FROM_INT(1); A.data[1] = Q16_FROM_INT(2); A.data[2] = Q16_FROM_INT(3);
    A.data[3] = Q16_FROM_INT(0); A.data[4] = Q16_FROM_INT(1); A.data[5] = Q16_FROM_INT(4);
    A.data[6] = Q16_FROM_INT(5); A.data[7] = Q16_FROM_INT(6); A.data[8] = Q16_FROM_INT(0);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv(&A, &Ainv));

    syn_matrix_mul(&A, &Ainv, &I_check);

    /* Diagonal should be ~1, off-diagonal ~0 */
    uint8_t i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            q16_t expected = (i == j) ? Q16_ONE : 0;
            ASSERT_Q16_NEAR(expected, SYN_MAT_AT(&I_check, i, j), Q16_MAT_TOL);
        }
    }
}

static void test_mat_singular(void)
{
    /* Singular matrix: [[1, 2], [2, 4]] → det = 0 */
    SYN_MAT2_DECL(A);
    SYN_MAT2_DECL(Ainv);

    A.data[0] = Q16_FROM_INT(1); A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(2); A.data[3] = Q16_FROM_INT(4);

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_inv(&A, &Ainv));
}

static void test_mat_rotate_2d(void)
{
    /* 90° rotation of (1, 0) → (0, 1) */
    SYN_MAT3_DECL(R);
    syn_matrix_rotate_2d(&R, Q16_PI_2);

    /* Apply to homogeneous vector [1, 0, 1] */
    q16_t v_in[3]  = { Q16_ONE, 0, Q16_ONE };
    q16_t v_out[3];
    syn_matrix_mul_vec(&R, v_in, v_out, 3);

    /* Expect ≈ (0, 1, 1) */
    ASSERT_Q16_NEAR(0,       v_out[0], Q16_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, v_out[1], Q16_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, v_out[2], Q16_TOL);
}

static void test_mat_nonsquare_mul(void)
{
    /* 2×3 × 3×2 = 2×2 */
    SYN_MAT_DECL(A, 2, 3);
    SYN_MAT_DECL(B, 3, 2);
    SYN_MAT_DECL(C, 2, 2);

    /* A = [[1, 2, 3], [4, 5, 6]] */
    A.data[0] = Q16_FROM_INT(1); A.data[1] = Q16_FROM_INT(2); A.data[2] = Q16_FROM_INT(3);
    A.data[3] = Q16_FROM_INT(4); A.data[4] = Q16_FROM_INT(5); A.data[5] = Q16_FROM_INT(6);

    /* B = [[7, 8], [9, 10], [11, 12]] */
    B.data[0] = Q16_FROM_INT(7);  B.data[1] = Q16_FROM_INT(8);
    B.data[2] = Q16_FROM_INT(9);  B.data[3] = Q16_FROM_INT(10);
    B.data[4] = Q16_FROM_INT(11); B.data[5] = Q16_FROM_INT(12);

    syn_matrix_mul(&A, &B, &C);

    /* C = [[1*7+2*9+3*11, 1*8+2*10+3*12],  = [[58, 64],
     *      [4*7+5*9+6*11, 4*8+5*10+6*12]]     [139, 154]] */
    ASSERT_Q16_NEAR(Q16_FROM_INT(58),  SYN_MAT_AT(&C, 0, 0), Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(64),  SYN_MAT_AT(&C, 0, 1), Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(139), SYN_MAT_AT(&C, 1, 0), Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(154), SYN_MAT_AT(&C, 1, 1), Q16_TOL);
}

static void test_vec3_cross(void)
{
    /* (1,0,0) × (0,1,0) = (0,0,1) */
    q16_t a[3] = { Q16_ONE, 0, 0 };
    q16_t b[3] = { 0, Q16_ONE, 0 };
    q16_t out[3];

    syn_vec3_cross(a, b, out);

    ASSERT_Q16_NEAR(0,       out[0], Q16_TOL);
    ASSERT_Q16_NEAR(0,       out[1], Q16_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, out[2], Q16_TOL);
}

static void test_vec_dot(void)
{
    /* (1,2,3) · (4,5,6) = 4+10+18 = 32 */
    q16_t a[3] = { Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3) };
    q16_t b[3] = { Q16_FROM_INT(4), Q16_FROM_INT(5), Q16_FROM_INT(6) };

    ASSERT_Q16_NEAR(Q16_FROM_INT(32), syn_vec_dot(a, b, 3), Q16_TOL);
}

static void test_vec_normalize(void)
{
    /* Normalize (3, 4, 0) → (0.6, 0.8, 0), magnitude = 5 */
    q16_t v[3]   = { Q16_FROM_INT(3), Q16_FROM_INT(4), 0 };
    q16_t out[3];

    TEST_ASSERT_EQUAL(SYN_OK, syn_vec_normalize(v, out, 3));

    /* Magnitude of result should be ~1.0 */
    q16_t mag = syn_vec_norm(out, 3);
    ASSERT_Q16_NEAR(Q16_ONE, mag, Q16_TOL * 2);
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Test runner                                                            */
/* ════════════════════════════════════════════════════════════════════════ */

void run_matrix_tests(void)
{
    /* Extended qmath */
    RUN_TEST(test_q16_sqrt);
    RUN_TEST(test_q16_hypot);
    RUN_TEST(test_q16_trig);
    RUN_TEST(test_q16_atan2);
    RUN_TEST(test_q16_asin_acos);
    RUN_TEST(test_q16_exp_log);
    RUN_TEST(test_q16_pow);
    RUN_TEST(test_q16_str);

    /* Matrix operations */
    RUN_TEST(test_mat_identity_mul);
    RUN_TEST(test_mat_transpose);
    RUN_TEST(test_mat_det_2x2);
    RUN_TEST(test_mat_inverse_2x2);
    RUN_TEST(test_mat_inverse_3x3);
    RUN_TEST(test_mat_singular);
    RUN_TEST(test_mat_rotate_2d);
    RUN_TEST(test_mat_nonsquare_mul);

    /* Vector operations */
    RUN_TEST(test_vec3_cross);
    RUN_TEST(test_vec_dot);
    RUN_TEST(test_vec_normalize);
}
