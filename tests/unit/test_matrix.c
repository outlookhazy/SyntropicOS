/**
 * @file test_matrix.c
 * @brief Unity tests for syn_matrix and extended syn_qmath.
 */

#include "mocks/mock_port.h"
#include "syntropic/syntropic.h"
#include "syntropic/util/syn_matrix.h"
#include "syntropic/util/syn_qmath.h"
#include "unity/unity.h"

/* ── Helper: assert Q16 value is within tolerance of expected ──────────── */

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

/* Tolerance: ±0.002 in Q16 ≈ 131 LSBs */
#define Q16_TOL 131

/* Tolerance for matrix roundtrip: ±0.02 ≈ 1311 LSBs */
#define Q16_MAT_TOL 1311

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
    ASSERT_Q16_NEAR(Q16_FROM_INT(5), q16_hypot(Q16_FROM_INT(3), Q16_FROM_INT(4)), Q16_TOL);

    /* hypot(0, 5) = 5 */
    ASSERT_Q16_NEAR(Q16_FROM_INT(5), q16_hypot(0, Q16_FROM_INT(5)), Q16_TOL);

    /* hypot(1, 1) ≈ 1.41421 */
    ASSERT_Q16_NEAR(Q16_SQRT2, q16_hypot(Q16_ONE, Q16_ONE), Q16_TOL);
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

    /* sin/cos wrapping outside [-π, π] */
    TEST_ASSERT_NOT_EQUAL(0, q16_sin(Q16_PI * 3));
    TEST_ASSERT_NOT_EQUAL(0, q16_sin(-Q16_PI * 3));

    /* fast sin/cos and sincos_fast */
    q16_t sf = 0, cf = 0;
    q16_sincos_fast(Q16_PI_2, &sf, &cf);
    ASSERT_Q16_NEAR(Q16_ONE, sf, Q16_TOL);
    ASSERT_Q16_NEAR(0, cf, Q16_TOL);

    /* fast sin/cos with large negative/positive inputs */
    TEST_ASSERT_NOT_EQUAL(0, q16_sin_fast(Q16_PI * 3));
    TEST_ASSERT_NOT_EQUAL(0, q16_cos_fast(-Q16_PI * 3));
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
    ASSERT_Q16_NEAR(expected_neg_3pi4, q16_atan2(-Q16_ONE, -Q16_ONE), Q16_TOL * 4);
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
    ASSERT_Q16_NEAR(Q16_FROM_INT(1024), q16_pow(Q16_FROM_INT(2), Q16_FROM_INT(10)),
                    Q16_FROM_INT(2));

    /* 4^0.5 = 2 (square root via pow) */
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), q16_pow(Q16_FROM_INT(4), Q16_HALF), Q16_TOL * 4);
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
    A.data[0] = Q16_FROM_INT(1);
    A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(3);
    A.data[3] = Q16_FROM_INT(4);
    A.data[4] = Q16_FROM_INT(5);
    A.data[5] = Q16_FROM_INT(6);

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
    A.data[0] = Q16_FROM_INT(3);
    A.data[1] = Q16_FROM_INT(8);
    A.data[2] = Q16_FROM_INT(4);
    A.data[3] = Q16_FROM_INT(6);

    ASSERT_Q16_NEAR(Q16_FROM_INT(-14), syn_matrix_det(&A), Q16_TOL);
}

static void test_mat_inverse_2x2(void)
{
    /* A = [[4, 7], [2, 6]]  →  A⁻¹ = (1/10)*[[6, -7], [-2, 4]] */
    SYN_MAT2_DECL(A);
    SYN_MAT2_DECL(Ainv);
    SYN_MAT2_DECL(I_check);

    A.data[0] = Q16_FROM_INT(4);
    A.data[1] = Q16_FROM_INT(7);
    A.data[2] = Q16_FROM_INT(2);
    A.data[3] = Q16_FROM_INT(6);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv(&A, &Ainv));

    /* Verify A × A⁻¹ ≈ I */
    syn_matrix_mul(&A, &Ainv, &I_check);

    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&I_check, 0, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&I_check, 0, 1), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&I_check, 1, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&I_check, 1, 1), Q16_MAT_TOL);
}

static void test_mat_inverse_3x3(void)
{
    /* A = [[1, 2, 3], [0, 1, 4], [5, 6, 0]] → det = 1 */
    SYN_MAT3_DECL(A);
    SYN_MAT3_DECL(Ainv);
    SYN_MAT3_DECL(I_check);

    A.data[0] = Q16_FROM_INT(1);
    A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(3);
    A.data[3] = Q16_FROM_INT(0);
    A.data[4] = Q16_FROM_INT(1);
    A.data[5] = Q16_FROM_INT(4);
    A.data[6] = Q16_FROM_INT(5);
    A.data[7] = Q16_FROM_INT(6);
    A.data[8] = Q16_FROM_INT(0);

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

static void test_mat_inverse_lu_nn(void)
{
    /* 4×4 Pascal matrix inverse test */
    SYN_MAT_DECL(A, 4, 4);
    SYN_MAT_DECL(Ainv, 4, 4);
    SYN_MAT_DECL(I_check, 4, 4);

    A.data[0] = Q16_ONE;
    A.data[1] = Q16_ONE;
    A.data[2] = Q16_ONE;
    A.data[3] = Q16_ONE;
    A.data[4] = Q16_ONE;
    A.data[5] = Q16_FROM_INT(2);
    A.data[6] = Q16_FROM_INT(3);
    A.data[7] = Q16_FROM_INT(4);
    A.data[8] = Q16_ONE;
    A.data[9] = Q16_FROM_INT(3);
    A.data[10] = Q16_FROM_INT(6);
    A.data[11] = Q16_FROM_INT(10);
    A.data[12] = Q16_ONE;
    A.data[13] = Q16_FROM_INT(4);
    A.data[14] = Q16_FROM_INT(10);
    A.data[15] = Q16_FROM_INT(20);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv_lu(&A, &Ainv));
    syn_matrix_mul(&A, &Ainv, &I_check);

    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 4; j++) {
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

    A.data[0] = Q16_FROM_INT(1);
    A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(2);
    A.data[3] = Q16_FROM_INT(4);

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_inv(&A, &Ainv));
}

static void test_mat_rotate_2d(void)
{
    /* 90° rotation of (1, 0) → (0, 1) */
    SYN_MAT3_DECL(R);
    syn_matrix_rotate_2d(&R, Q16_PI_2);

    /* Apply to homogeneous vector [1, 0, 1] */
    q16_t v_in[3] = {Q16_ONE, 0, Q16_ONE};
    q16_t v_out[3];
    syn_matrix_mul_vec(&R, v_in, v_out, 3);

    /* Expect ≈ (0, 1, 1) */
    ASSERT_Q16_NEAR(0, v_out[0], Q16_TOL);
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
    A.data[0] = Q16_FROM_INT(1);
    A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(3);
    A.data[3] = Q16_FROM_INT(4);
    A.data[4] = Q16_FROM_INT(5);
    A.data[5] = Q16_FROM_INT(6);

    /* B = [[7, 8], [9, 10], [11, 12]] */
    B.data[0] = Q16_FROM_INT(7);
    B.data[1] = Q16_FROM_INT(8);
    B.data[2] = Q16_FROM_INT(9);
    B.data[3] = Q16_FROM_INT(10);
    B.data[4] = Q16_FROM_INT(11);
    B.data[5] = Q16_FROM_INT(12);

    syn_matrix_mul(&A, &B, &C);

    /* C = [[1*7+2*9+3*11, 1*8+2*10+3*12],  = [[58, 64],
     *      [4*7+5*9+6*11, 4*8+5*10+6*12]]     [139, 154]] */
    ASSERT_Q16_NEAR(Q16_FROM_INT(58), SYN_MAT_AT(&C, 0, 0), Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(64), SYN_MAT_AT(&C, 0, 1), Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(139), SYN_MAT_AT(&C, 1, 0), Q16_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(154), SYN_MAT_AT(&C, 1, 1), Q16_TOL);
}

static void test_vec3_cross(void)
{
    /* (1,0,0) × (0,1,0) = (0,0,1) */
    q16_t a[3] = {Q16_ONE, 0, 0};
    q16_t b[3] = {0, Q16_ONE, 0};
    q16_t out[3];

    syn_vec3_cross(a, b, out);

    ASSERT_Q16_NEAR(0, out[0], Q16_TOL);
    ASSERT_Q16_NEAR(0, out[1], Q16_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, out[2], Q16_TOL);
}

static void test_vec_dot(void)
{
    /* (1,2,3) · (4,5,6) = 4+10+18 = 32 */
    q16_t a[3] = {Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3)};
    q16_t b[3] = {Q16_FROM_INT(4), Q16_FROM_INT(5), Q16_FROM_INT(6)};

    ASSERT_Q16_NEAR(Q16_FROM_INT(32), syn_vec_dot(a, b, 3), Q16_TOL);
}

static void test_vec_normalize(void)
{
    /* Normalize (3, 4, 0) → (0.6, 0.8, 0), magnitude = 5 */
    q16_t v[3] = {Q16_FROM_INT(3), Q16_FROM_INT(4), 0};
    q16_t out[3];

    TEST_ASSERT_EQUAL(SYN_OK, syn_vec_normalize(v, out, 3));

    /* Magnitude of result should be ~1.0 */
    q16_t mag = syn_vec_norm(out, 3);
    ASSERT_Q16_NEAR(Q16_ONE, mag, Q16_TOL * 2);
}

/* ════════════════════════════════════════════════════════════════════════ */
/*  Linear Solvers Tests                                                    */
/* ════════════════════════════════════════════════════════════════════════ */

static void test_mat_solve_lu(void)
{
    /* 1. 2×2 system: A = [[2, 1], [5, 7]], b = [11, 13] */
    SYN_MAT2_DECL(A2);
    SYN_MAT_DECL(b2, 2, 1);
    SYN_MAT_DECL(x2, 2, 1);
    SYN_MAT_DECL(b2_check, 2, 1);

    A2.data[0] = Q16_FROM_INT(2);
    A2.data[1] = Q16_FROM_INT(1);
    A2.data[2] = Q16_FROM_INT(5);
    A2.data[3] = Q16_FROM_INT(7);

    b2.data[0] = Q16_FROM_INT(11);
    b2.data[1] = Q16_FROM_INT(13);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_lu(&A2, &b2, &x2));
    syn_matrix_mul(&A2, &x2, &b2_check);
    ASSERT_Q16_NEAR(b2.data[0], b2_check.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b2.data[1], b2_check.data[1], Q16_MAT_TOL);

    /* 2. 3×3 system with negative terms:
     * A = [[2, 1, -1], [-3, -1, 2], [-2, 1, 2]], b = [8, -11, -3]
     * Solution: x = [2, 3, -1] */
    SYN_MAT_DECL(A3, 3, 3);
    SYN_MAT_DECL(b3, 3, 1);
    SYN_MAT_DECL(x3, 3, 1);

    A3.data[0] = Q16_FROM_INT(2);
    A3.data[1] = Q16_FROM_INT(1);
    A3.data[2] = Q16_FROM_INT(-1);
    A3.data[3] = Q16_FROM_INT(-3);
    A3.data[4] = Q16_FROM_INT(-1);
    A3.data[5] = Q16_FROM_INT(2);
    A3.data[6] = Q16_FROM_INT(-2);
    A3.data[7] = Q16_FROM_INT(1);
    A3.data[8] = Q16_FROM_INT(2);

    b3.data[0] = Q16_FROM_INT(8);
    b3.data[1] = Q16_FROM_INT(-11);
    b3.data[2] = Q16_FROM_INT(-3);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_lu(&A3, &b3, &x3));
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), x3.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(3), x3.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(-1), x3.data[2], Q16_MAT_TOL);

    /* Verify A3 · x3 = b3 explicitly */
    SYN_MAT_DECL(b3_check, 3, 1);
    syn_matrix_mul(&A3, &x3, &b3_check);
    ASSERT_Q16_NEAR(b3.data[0], b3_check.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b3.data[1], b3_check.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b3.data[2], b3_check.data[2], Q16_MAT_TOL);

    /* 3. 4×4 system: Pascal matrix A, b = [4, 10, 20, 35] -> x = [1, 1, 1, 1] */
    SYN_MAT_DECL(A4, 4, 4);
    SYN_MAT_DECL(b4, 4, 1);
    SYN_MAT_DECL(x4, 4, 1);

    A4.data[0] = Q16_ONE;
    A4.data[1] = Q16_ONE;
    A4.data[2] = Q16_ONE;
    A4.data[3] = Q16_ONE;
    A4.data[4] = Q16_ONE;
    A4.data[5] = Q16_FROM_INT(2);
    A4.data[6] = Q16_FROM_INT(3);
    A4.data[7] = Q16_FROM_INT(4);
    A4.data[8] = Q16_ONE;
    A4.data[9] = Q16_FROM_INT(3);
    A4.data[10] = Q16_FROM_INT(6);
    A4.data[11] = Q16_FROM_INT(10);
    A4.data[12] = Q16_ONE;
    A4.data[13] = Q16_FROM_INT(4);
    A4.data[14] = Q16_FROM_INT(10);
    A4.data[15] = Q16_FROM_INT(20);

    b4.data[0] = Q16_FROM_INT(4);
    b4.data[1] = Q16_FROM_INT(10);
    b4.data[2] = Q16_FROM_INT(20);
    b4.data[3] = Q16_FROM_INT(35);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_lu(&A4, &b4, &x4));
    ASSERT_Q16_NEAR(Q16_ONE, x4.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, x4.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, x4.data[2], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, x4.data[3], Q16_MAT_TOL);

    /* Verify A4 · x4 = b4 explicitly */
    SYN_MAT_DECL(b4_check, 4, 1);
    syn_matrix_mul(&A4, &x4, &b4_check);
    ASSERT_Q16_NEAR(b4.data[0], b4_check.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b4.data[1], b4_check.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b4.data[2], b4_check.data[2], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b4.data[3], b4_check.data[3], Q16_MAT_TOL);

    /* 4. Singular matrix error detection */
    SYN_MAT2_DECL(A_sing);
    A_sing.data[0] = Q16_ONE;
    A_sing.data[1] = Q16_FROM_INT(2);
    A_sing.data[2] = Q16_FROM_INT(2);
    A_sing.data[3] = Q16_FROM_INT(4);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_solve_lu(&A_sing, &b2, &x2));

    /* 5. Dimension mismatch validation */
    SYN_MAT_DECL(b_bad, 3, 1);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_solve_lu(&A2, &b_bad, &x2));

    /* 6. Test caller-supplied workspace API directly */
    q16_t lu_w[4];
    uint8_t p_w[2];
    q16_t y_w[2];
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_lu_work(&A2, &b2, &x2, lu_w, p_w, y_w));
    ASSERT_Q16_NEAR(b2.data[0], b2_check.data[0], Q16_MAT_TOL);
}

static void test_mat_solve_cholesky(void)
{
    /* 1. 2×2 SPD: A = [[4, 12], [12, 45]], b = [16, 57] -> x = [1, 1] */
    SYN_MAT2_DECL(A2);
    SYN_MAT_DECL(b2, 2, 1);
    SYN_MAT_DECL(x2, 2, 1);

    A2.data[0] = Q16_FROM_INT(4);
    A2.data[1] = Q16_FROM_INT(12);
    A2.data[2] = Q16_FROM_INT(12);
    A2.data[3] = Q16_FROM_INT(45);

    b2.data[0] = Q16_FROM_INT(16);
    b2.data[1] = Q16_FROM_INT(57);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_cholesky(&A2, &b2, &x2));
    ASSERT_Q16_NEAR(Q16_ONE, x2.data[0], Q16_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, x2.data[1], Q16_TOL);

    /* Verify A2 · x2 = b2 explicitly */
    SYN_MAT_DECL(b2_check, 2, 1);
    syn_matrix_mul(&A2, &x2, &b2_check);
    ASSERT_Q16_NEAR(b2.data[0], b2_check.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b2.data[1], b2_check.data[1], Q16_MAT_TOL);

    /* 2. 3×3 SPD: A = [[4, 12, -16], [12, 37, -43], [-16, -43, 98]], b = [12, 43, -4]
     * Exact solution x = [1, 2, 1] */
    SYN_MAT_DECL(A3, 3, 3);
    SYN_MAT_DECL(b3, 3, 1);
    SYN_MAT_DECL(x3, 3, 1);

    A3.data[0] = Q16_FROM_INT(4);
    A3.data[1] = Q16_FROM_INT(12);
    A3.data[2] = Q16_FROM_INT(-16);
    A3.data[3] = Q16_FROM_INT(12);
    A3.data[4] = Q16_FROM_INT(37);
    A3.data[5] = Q16_FROM_INT(-43);
    A3.data[6] = Q16_FROM_INT(-16);
    A3.data[7] = Q16_FROM_INT(-43);
    A3.data[8] = Q16_FROM_INT(98);

    b3.data[0] = Q16_FROM_INT(12);
    b3.data[1] = Q16_FROM_INT(43);
    b3.data[2] = Q16_FROM_INT(-4);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_cholesky(&A3, &b3, &x3));
    ASSERT_Q16_NEAR(Q16_FROM_INT(1), x3.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), x3.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(1), x3.data[2], Q16_MAT_TOL);

    /* Verify A3 · x3 = b3 explicitly */
    SYN_MAT_DECL(b3_check, 3, 1);
    syn_matrix_mul(&A3, &x3, &b3_check);
    ASSERT_Q16_NEAR(b3.data[0], b3_check.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b3.data[1], b3_check.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(b3.data[2], b3_check.data[2], Q16_MAT_TOL);

    /* 3. Non-positive definite matrix detection */
    SYN_MAT2_DECL(A_not_pd);
    A_not_pd.data[0] = Q16_ONE;
    A_not_pd.data[1] = Q16_FROM_INT(2);
    A_not_pd.data[2] = Q16_FROM_INT(2);
    A_not_pd.data[3] = Q16_ONE;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_solve_cholesky(&A_not_pd, &b2, &x2));

    /* 4. Dimension mismatch validation */
    SYN_MAT_DECL(b_bad, 3, 1);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_solve_cholesky(&A2, &b_bad, &x2));
}

static void test_mat_least_squares(void)
{
    /* 1. Overdetermined 3×2 system (line fit y = c0 + c1*x to points (0,1), (1,2), (2,4))
     * Exact least-squares fit: c0 = 5/6, c1 = 3/2 */
    SYN_MAT_DECL(A32, 3, 2);
    SYN_MAT_DECL(b3, 3, 1);
    SYN_MAT_DECL(x2, 2, 1);

    A32.data[0] = Q16_ONE;
    A32.data[1] = 0;
    A32.data[2] = Q16_ONE;
    A32.data[3] = Q16_ONE;
    A32.data[4] = Q16_ONE;
    A32.data[5] = Q16_FROM_INT(2);

    b3.data[0] = Q16_FROM_INT(1);
    b3.data[1] = Q16_FROM_INT(2);
    b3.data[2] = Q16_FROM_INT(4);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_least_squares(&A32, &b3, &x2));
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(5, 6), x2.data[0], Q16_TOL * 2);
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(3, 2), x2.data[1], Q16_TOL * 2);

    /* 2. Overdetermined 4×3 system (quadratic fit y = c0 + c1*x + c2*x^2 to points (0,1), (1,3),
     * (2,7), (3,13)) Exact curve: y = 1 + x + x^2 -> x = [1, 1, 1] */
    SYN_MAT_DECL(A43, 4, 3);
    SYN_MAT_DECL(b4, 4, 1);
    SYN_MAT_DECL(x3, 3, 1);

    A43.data[0] = Q16_ONE;
    A43.data[1] = 0;
    A43.data[2] = 0;
    A43.data[3] = Q16_ONE;
    A43.data[4] = Q16_ONE;
    A43.data[5] = Q16_ONE;
    A43.data[6] = Q16_ONE;
    A43.data[7] = Q16_FROM_INT(2);
    A43.data[8] = Q16_FROM_INT(4);
    A43.data[9] = Q16_ONE;
    A43.data[10] = Q16_FROM_INT(3);
    A43.data[11] = Q16_FROM_INT(9);

    b4.data[0] = Q16_FROM_INT(1);
    b4.data[1] = Q16_FROM_INT(3);
    b4.data[2] = Q16_FROM_INT(7);
    b4.data[3] = Q16_FROM_INT(13);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_least_squares(&A43, &b4, &x3));
    ASSERT_Q16_NEAR(Q16_ONE, x3.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, x3.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, x3.data[2], Q16_MAT_TOL);

    /* Verify Normal Equations identity: Aᵀ · A · x == Aᵀ · b */
    SYN_MAT_DECL(AT43, 3, 4);
    SYN_MAT_DECL(AtA43, 3, 3);
    SYN_MAT_DECL(Atb43, 3, 1);
    SYN_MAT_DECL(AtAx43, 3, 1);

    syn_matrix_transpose(&A43, &AT43);
    syn_matrix_mul(&AT43, &A43, &AtA43);
    syn_matrix_mul(&AT43, &b4, &Atb43);
    syn_matrix_mul(&AtA43, &x3, &AtAx43);

    ASSERT_Q16_NEAR(Atb43.data[0], AtAx43.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Atb43.data[1], AtAx43.data[1], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Atb43.data[2], AtAx43.data[2], Q16_MAT_TOL);

    /* 3. Underdetermined parameter validation (M < N -> SYN_INVALID_PARAM) */
    SYN_MAT_DECL(A23, 2, 3);
    SYN_MAT_DECL(b2, 2, 1);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_least_squares(&A23, &b2, &x3));
}

static void test_mat_blocks(void)
{
    SYN_MAT_DECL(M, 4, 4);
    syn_matrix_zero(&M);

    SYN_MAT2_DECL(sub);
    sub.data[0] = Q16_FROM_INT(1);
    sub.data[1] = Q16_FROM_INT(2);
    sub.data[2] = Q16_FROM_INT(3);
    sub.data[3] = Q16_FROM_INT(4);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_set_block(&M, 1, 1, &sub));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(1), SYN_MAT_AT(&M, 1, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4), SYN_MAT_AT(&M, 2, 2));

    SYN_MAT2_DECL(extracted);
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_get_block(&M, 1, 1, &extracted));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(1), extracted.data[0]);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4), extracted.data[3]);

    /* Out of bounds */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_set_block(&M, 3, 3, &sub));
}

static void test_mat_outer_product(void)
{
    q16_t u[2] = {Q16_FROM_INT(1), Q16_FROM_INT(2)};
    q16_t v[3] = {Q16_FROM_INT(3), Q16_FROM_INT(4), Q16_FROM_INT(5)};

    SYN_MAT_DECL(M, 2, 3);
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_outer_product(u, 2, v, 3, &M));

    TEST_ASSERT_EQUAL(Q16_FROM_INT(3), SYN_MAT_AT(&M, 0, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4), SYN_MAT_AT(&M, 0, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), SYN_MAT_AT(&M, 0, 2));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(6), SYN_MAT_AT(&M, 1, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(8), SYN_MAT_AT(&M, 1, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(10), SYN_MAT_AT(&M, 1, 2));
}

static void test_mat_qr(void)
{
    /* A = [[1, 1], [1, 0], [1, 2]] (3×2 matrix) */
    SYN_MAT_DECL(A, 3, 2);
    SYN_MAT_DECL(Q, 3, 2);
    SYN_MAT_DECL(R, 2, 2);

    A.data[0] = Q16_ONE;
    A.data[1] = Q16_ONE;
    A.data[2] = Q16_ONE;
    A.data[3] = 0;
    A.data[4] = Q16_ONE;
    A.data[5] = Q16_FROM_INT(2);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_qr(&A, &Q, &R));

    /* 1. R must be upper-triangular: R[1][0] == 0 */
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&R, 1, 0), Q16_MAT_TOL);

    /* 2. Q must be orthogonal: Qᵀ · Q == I (2×2) */
    SYN_MAT_DECL(QT, 2, 3);
    SYN_MAT_DECL(QTQ, 2, 2);
    syn_matrix_transpose(&Q, &QT);
    syn_matrix_mul(&QT, &Q, &QTQ);

    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&QTQ, 0, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&QTQ, 0, 1), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&QTQ, 1, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&QTQ, 1, 1), Q16_MAT_TOL);

    /* 3. Reconstruction: Q · R == A */
    SYN_MAT_DECL(QR_check, 3, 2);
    syn_matrix_mul(&Q, &R, &QR_check);

    for (uint8_t i = 0; i < 6; i++) {
        ASSERT_Q16_NEAR(A.data[i], QR_check.data[i], Q16_MAT_TOL);
    }

    /* 4. Invalid dimensions test: M < N (2×3 matrix) */
    SYN_MAT_DECL(A_bad, 2, 3);
    SYN_MAT_DECL(Q_bad, 2, 3);
    SYN_MAT_DECL(R_bad, 3, 3);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_qr(&A_bad, &Q_bad, &R_bad));
}

static void test_mat_eigen_sym2(void)
{
    /* 1. A = [[4, 2], [2, 1]] -> evals: 5, 0 */
    SYN_MAT2_DECL(A);
    SYN_MAT2_DECL(E);
    q16_t evals[2];

    A.data[0] = Q16_FROM_INT(4);
    A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(2);
    A.data[3] = Q16_FROM_INT(1);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym2(&A, evals, &E));

    ASSERT_Q16_NEAR(Q16_FROM_INT(5), evals[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0, evals[1], Q16_MAT_TOL);

    /* Verify A · e0 == λ0 * e0 */
    SYN_MAT_DECL(e0, 2, 1);
    SYN_MAT_DECL(Ae0, 2, 1);
    e0.data[0] = SYN_MAT_AT(&E, 0, 0);
    e0.data[1] = SYN_MAT_AT(&E, 1, 0);
    syn_matrix_mul(&A, &e0, &Ae0);
    ASSERT_Q16_NEAR(q16_mul(evals[0], e0.data[0]), Ae0.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(q16_mul(evals[0], e0.data[1]), Ae0.data[1], Q16_MAT_TOL);

    /* Verify A · e1 == λ1 * e1 */
    SYN_MAT_DECL(e1, 2, 1);
    SYN_MAT_DECL(Ae1, 2, 1);
    e1.data[0] = SYN_MAT_AT(&E, 0, 1);
    e1.data[1] = SYN_MAT_AT(&E, 1, 1);
    syn_matrix_mul(&A, &e1, &Ae1);
    ASSERT_Q16_NEAR(q16_mul(evals[1], e1.data[0]), Ae1.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(q16_mul(evals[1], e1.data[1]), Ae1.data[1], Q16_MAT_TOL);

    /* 2. Diagonal matrix A_diag = [[3, 0], [0, 1]] (b == 0) */
    SYN_MAT2_DECL(A_diag);
    SYN_MAT2_DECL(E_diag);
    q16_t evals_diag[2];

    A_diag.data[0] = Q16_FROM_INT(3);
    A_diag.data[1] = 0;
    A_diag.data[2] = 0;
    A_diag.data[3] = Q16_FROM_INT(1);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym2(&A_diag, evals_diag, &E_diag));
    ASSERT_Q16_NEAR(Q16_FROM_INT(3), evals_diag[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(1), evals_diag[1], Q16_MAT_TOL);
    TEST_ASSERT_EQUAL(Q16_ONE, SYN_MAT_AT(&E_diag, 0, 0));
    TEST_ASSERT_EQUAL(0, SYN_MAT_AT(&E_diag, 0, 1));
}

static void test_mat_eigen_sym3(void)
{
    /* 1. A = [[3, 1, 1], [1, 3, 1], [1, 1, 3]] -> evals: 5, 2, 2 */
    SYN_MAT3_DECL(A);
    SYN_MAT3_DECL(E);
    q16_t evals[3];

    A.data[0] = Q16_FROM_INT(3);
    A.data[1] = Q16_FROM_INT(1);
    A.data[2] = Q16_FROM_INT(1);
    A.data[3] = Q16_FROM_INT(1);
    A.data[4] = Q16_FROM_INT(3);
    A.data[5] = Q16_FROM_INT(1);
    A.data[6] = Q16_FROM_INT(1);
    A.data[7] = Q16_FROM_INT(1);
    A.data[8] = Q16_FROM_INT(3);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym3(&A, evals, &E));

    ASSERT_Q16_NEAR(Q16_FROM_INT(5), evals[0], Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), evals[1], Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), evals[2], Q16_MAT_TOL * 2);

    /* Verify Eᵀ · E == I (orthonormal eigenvectors) */
    SYN_MAT3_DECL(ET);
    SYN_MAT3_DECL(ETE);
    syn_matrix_transpose(&E, &ET);
    syn_matrix_mul(&ET, &E, &ETE);

    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&ETE, 0, 0), Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&ETE, 1, 1), Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&ETE, 2, 2), Q16_MAT_TOL * 2);

    /* Verify A · e_i == λ_i * e_i for all 3 eigenvectors */
    for (uint8_t i = 0; i < 3; i++) {
        SYN_MAT_DECL(ei, 3, 1);
        SYN_MAT_DECL(Aei, 3, 1);
        ei.data[0] = SYN_MAT_AT(&E, 0, i);
        ei.data[1] = SYN_MAT_AT(&E, 1, i);
        ei.data[2] = SYN_MAT_AT(&E, 2, i);

        syn_matrix_mul(&A, &ei, &Aei);
        ASSERT_Q16_NEAR(q16_mul(evals[i], ei.data[0]), Aei.data[0], Q16_MAT_TOL * 2);
        ASSERT_Q16_NEAR(q16_mul(evals[i], ei.data[1]), Aei.data[1], Q16_MAT_TOL * 2);
        ASSERT_Q16_NEAR(q16_mul(evals[i], ei.data[2]), Aei.data[2], Q16_MAT_TOL * 2);
    }

    /* 2. Tridiagonal A2 = [[2, 1, 0], [1, 2, 1], [0, 1, 2]] */
    SYN_MAT3_DECL(A2);
    SYN_MAT3_DECL(E2);
    q16_t evals2[3];

    A2.data[0] = Q16_FROM_INT(2);
    A2.data[1] = Q16_FROM_INT(1);
    A2.data[2] = 0;
    A2.data[3] = Q16_FROM_INT(1);
    A2.data[4] = Q16_FROM_INT(2);
    A2.data[5] = Q16_FROM_INT(1);
    A2.data[6] = 0;
    A2.data[7] = Q16_FROM_INT(1);
    A2.data[8] = Q16_FROM_INT(2);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym3(&A2, evals2, &E2));

    /* λ = 2 + sqrt(2) ≈ 3.414, 2.0, 2 - sqrt(2) ≈ 0.586 */
    q16_t expected_l0 = Q16_FROM_INT(2) + Q16_SQRT2;
    q16_t expected_l1 = Q16_FROM_INT(2);
    q16_t expected_l2 = Q16_FROM_INT(2) - Q16_SQRT2;

    ASSERT_Q16_NEAR(expected_l0, evals2[0], Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(expected_l1, evals2[1], Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(expected_l2, evals2[2], Q16_MAT_TOL * 2);
}

static void test_matrix_transforms_and_errors(void)
{
    /* 2D transforms */
    q16_t t2d_store[9], s2d_store[9];
    SYN_MAT_INIT(T2D, t2d_store, 3, 3);
    SYN_MAT_INIT(S2D, s2d_store, 3, 3);
    syn_matrix_translate_2d(&T2D, Q16_FROM_INT(5), Q16_FROM_INT(-3));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), SYN_MAT_AT(&T2D, 0, 2));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(-3), SYN_MAT_AT(&T2D, 1, 2));

    syn_matrix_scale_2d(&S2D, Q16_FROM_INT(2), Q16_FROM_INT(4));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(2), SYN_MAT_AT(&S2D, 0, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4), SYN_MAT_AT(&S2D, 1, 1));

    /* 3D transforms */
    q16_t rx_store[16], ry_store[16], rz_store[16], t3d_store[16];
    SYN_MAT_INIT(Rx, rx_store, 4, 4);
    SYN_MAT_INIT(Ry, ry_store, 4, 4);
    SYN_MAT_INIT(Rz, rz_store, 4, 4);
    SYN_MAT_INIT(T3D, t3d_store, 4, 4);

    syn_matrix_rotate_x(&Rx, Q16_PI_2);
    syn_matrix_rotate_y(&Ry, Q16_PI_2);
    syn_matrix_rotate_z(&Rz, Q16_PI_2);
    syn_matrix_translate_3d(&T3D, Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3));

    TEST_ASSERT_EQUAL(Q16_FROM_INT(1), SYN_MAT_AT(&T3D, 0, 3));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(2), SYN_MAT_AT(&T3D, 1, 3));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(3), SYN_MAT_AT(&T3D, 2, 3));

    /* Error paths */
    q16_t sing_store[4] = {0, 0, 0, 0};
    SYN_MAT_INIT(Sing, sing_store, 2, 2);
    q16_t out_store[4];
    SYN_MAT_INIT(Out, out_store, 2, 2);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_inv(&Sing, &Out));

    q16_t m5_store[25] = {0};
    SYN_MAT_INIT(M5, m5_store, 5, 5);
    SYN_MAT_INIT(Out5, m5_store, 5, 5);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_inv(&M5, &Out5));
    TEST_ASSERT_EQUAL(0, syn_matrix_det(&M5));
}

static void test_matrix_singular_inverse_and_dimension_mismatch(void)
{
    /* 1x1 matrix det and inv */
    q16_t d1[1] = {Q16_FROM_INT(5)};
    q16_t inv1[1] = {0};
    SYN_MAT_INIT(M1, d1, 1, 1);
    SYN_MAT_INIT(Inv1, inv1, 1, 1);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5), syn_matrix_det(&M1));
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv(&M1, &Inv1));
    TEST_ASSERT_INT_WITHIN(Q16_TOL, Q16_FROM_FRAC(1, 5), inv1[0]);
    /* Trace & Scale */
    q16_t d2[4] = {Q16_FROM_INT(3), Q16_ONE, Q16_FROM_INT(2), Q16_FROM_INT(4)};
    q16_t scaled[4] = {0};
    SYN_MAT_INIT(M2, d2, 2, 2);
    SYN_MAT_INIT(ScaledM2, scaled, 2, 2);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(7), syn_matrix_trace(&M2));
    syn_matrix_scale(&M2, Q16_FROM_INT(2), &ScaledM2);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(6), scaled[0]);

    /* Vector norm test */
    TEST_ASSERT_TRUE(syn_vec_norm(d2, 4) > 0);

    /* 4x4 matrix det & inverse */
    q16_t d4[16] = {Q16_ONE,         0, 0, 0, 0, Q16_FROM_INT(2), 0, 0, 0, 0,
                    Q16_FROM_INT(3), 0, 0, 0, 0, Q16_FROM_INT(4)};
    q16_t inv4[16] = {0};
    SYN_MAT_INIT(M4, d4, 4, 4);
    SYN_MAT_INIT(Inv4, inv4, 4, 4);
    TEST_ASSERT_EQUAL(Q16_FROM_INT(24), syn_matrix_det(&M4));
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv(&M4, &Inv4));
    TEST_ASSERT_INT_WITHIN(Q16_TOL, Q16_ONE, inv4[0]);
    TEST_ASSERT_INT_WITHIN(Q16_TOL, Q16_FROM_FRAC(1, 4), inv4[15]);

    /* Test syn_matrix_inv_lu explicitly */
    q16_t inv4_lu[16] = {0};
    SYN_MAT_INIT(Inv4_LU, inv4_lu, 4, 4);
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv_lu(&M4, &Inv4_LU));
    TEST_ASSERT_INT_WITHIN(Q16_TOL, Q16_ONE, inv4_lu[0]);
}

static void test_mat_inverse_4x4_pivot_swap(void)
{
    SYN_MAT_DECL(A, 4, 4);
    SYN_MAT_DECL(invA, 4, 4);

    /* 4x4 matrix with 0 on first diagonal element to force row swap pivot */
    SYN_MAT_AT(&A, 0, 0) = 0;
    SYN_MAT_AT(&A, 0, 1) = Q16_ONE;
    SYN_MAT_AT(&A, 0, 2) = 0;
    SYN_MAT_AT(&A, 0, 3) = 0;

    SYN_MAT_AT(&A, 1, 0) = Q16_ONE;
    SYN_MAT_AT(&A, 1, 1) = 0;
    SYN_MAT_AT(&A, 1, 2) = 0;
    SYN_MAT_AT(&A, 1, 3) = 0;

    SYN_MAT_AT(&A, 2, 0) = 0;
    SYN_MAT_AT(&A, 2, 1) = 0;
    SYN_MAT_AT(&A, 2, 2) = Q16_ONE;
    SYN_MAT_AT(&A, 2, 3) = 0;

    SYN_MAT_AT(&A, 3, 0) = 0;
    SYN_MAT_AT(&A, 3, 1) = 0;
    SYN_MAT_AT(&A, 3, 2) = 0;
    SYN_MAT_AT(&A, 3, 3) = Q16_ONE;

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_inv(&A, &invA));
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, SYN_MAT_AT(&invA, 0, 1));
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, SYN_MAT_AT(&invA, 1, 0));
}

static void test_matrix_qr_singular_and_null_checks(void)
{
    /* Singular matrix for QR */
    SYN_MAT_DECL(A, 2, 2);
    SYN_MAT_DECL(Q, 2, 2);
    SYN_MAT_DECL(R, 2, 2);
    syn_matrix_zero(&A); /* All zeros -> singular column -> norm_v == 0 */

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_qr(&A, &Q, &R));

    /* Outer product dimension mismatch */
    q16_t u[2] = {Q16_ONE, Q16_ONE};
    q16_t v[2] = {Q16_ONE, Q16_ONE};
    SYN_MAT_DECL(out_bad, 3, 3);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_outer_product(u, 2, v, 2, &out_bad));

    /* Eigen parameter error paths */
    q16_t evals[3];
    SYN_MAT_DECL(E_bad, 2, 2);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_eigen_sym2(NULL, evals, &E_bad));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_eigen_sym3(NULL, evals, &E_bad));
}

static void test_matrix_det_1x1_2x2_3x3_explicit(void)
{
    /* 1x1 determinant */
    SYN_MAT_DECL(m1, 1, 1);
    m1.data[0] = Q16_FROM_INT(42);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(42), syn_matrix_det(&m1));

    /* 2x2 determinant */
    SYN_MAT_DECL(m2, 2, 2);
    m2.data[0] = Q16_FROM_INT(4);
    m2.data[1] = Q16_FROM_INT(2);
    m2.data[2] = Q16_FROM_INT(3);
    m2.data[3] = Q16_FROM_INT(1);
    /* det = 4*1 - 2*3 = -2 */
    ASSERT_Q16_NEAR(-Q16_FROM_INT(2), syn_matrix_det(&m2), Q16_TOL);

    /* 3x3 determinant */
    SYN_MAT_DECL(m3, 3, 3);
    m3.data[0] = Q16_ONE;
    m3.data[1] = 0;
    m3.data[2] = 0;
    m3.data[3] = 0;
    m3.data[4] = Q16_ONE;
    m3.data[5] = 0;
    m3.data[6] = 0;
    m3.data[7] = 0;
    m3.data[8] = Q16_ONE;
    /* det = 1 */
    ASSERT_Q16_NEAR(Q16_ONE, syn_matrix_det(&m3), Q16_TOL);
}

static void test_mat_inv_3x3_singular(void)
{
    SYN_MAT3_DECL(m);
    SYN_MAT3_DECL(out);
    /* Row 2 = Row 0 + Row 1 (dependent -> det = 0) */
    m.data[0] = Q16_ONE;
    m.data[1] = Q16_FROM_INT(2);
    m.data[2] = Q16_FROM_INT(3);
    m.data[3] = Q16_FROM_INT(4);
    m.data[4] = Q16_FROM_INT(5);
    m.data[5] = Q16_FROM_INT(6);
    m.data[6] = Q16_FROM_INT(5);
    m.data[7] = Q16_FROM_INT(7);
    m.data[8] = Q16_FROM_INT(9);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_inv(&m, &out));
}

static void test_mat_inv_4x4_singular(void)
{
    SYN_MAT4_DECL(m);
    SYN_MAT4_DECL(out);
    /* All rows zeros -> singular */
    memset(m.data, 0, 16 * sizeof(q16_t));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_inv(&m, &out));
}

static void test_matrix_inv_lu_dimension_mismatch(void)
{
    SYN_MAT_DECL(src, 2, 3);
    SYN_MAT_DECL(dst, 2, 2);
    q16_t lu_work[9];
    uint8_t p_work[3];
    q16_t y_work[3];

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_matrix_inv_lu_work(&src, &dst, lu_work, p_work, y_work));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_inv_lu(&src, &dst));
}

static void test_matrix_vec_normalize_zero_vector(void)
{
    q16_t v[3] = {0, 0, 0};
    q16_t out[3];
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_vec_normalize(v, out, 3));
}

static void test_matrix_solve_lu_and_cholesky_oversized_dim(void)
{
    /* Oversized solver dimensions & invalid parameters (lines 709, 852, 867, 911, 941, 969) */
    q16_t dummy[17 * 17];
    SYN_Matrix A = {.rows = 17, .cols = 17, .data = dummy};
    SYN_Matrix b = {.rows = 17, .cols = 1, .data = dummy};
    SYN_Matrix x = {.rows = 17, .cols = 1, .data = dummy};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_solve_lu(&A, &b, &x));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_solve_cholesky(&A, &b, &x));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_least_squares(&A, &b, &x));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_qr(&A, &A, &A));

    SYN_Matrix A_rect = {.rows = 2, .cols = 4, .data = dummy};
    SYN_Matrix x_rect = {.rows = 4, .cols = 1, .data = dummy};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_least_squares_work(&A_rect, &b, &x_rect, dummy,
                                                                       dummy, dummy, dummy, dummy));

    SYN_MAT_DECL(src, 2, 2);
    SYN_MAT_DECL(dst, 2, 2);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_get_block(&src, 1, 1, &dst));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_outer_product(NULL, 2, NULL, 2, &dst));

    SYN_Matrix A0 = {.rows = 0, .cols = 0, .data = NULL};
    SYN_Matrix b0 = {.rows = 0, .cols = 1, .data = NULL};
    SYN_Matrix x0 = {.rows = 0, .cols = 1, .data = NULL};
    q16_t L[1], y[1];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_solve_cholesky_work(&A0, &b0, &x0, L, y));

    /* LU solver n > SYN_SOLVER_MAX_N (line 709 of syn_matrix.c) */
    q16_t big_buf[100];
    uint8_t P_big[10];
    SYN_Matrix Abig = {.rows = SYN_SOLVER_MAX_N + 1, .cols = SYN_SOLVER_MAX_N + 1, .data = big_buf};
    SYN_Matrix bbig = {.rows = SYN_SOLVER_MAX_N + 1, .cols = 1, .data = big_buf};
    SYN_Matrix xbig = {.rows = SYN_SOLVER_MAX_N + 1, .cols = 1, .data = big_buf};
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_matrix_solve_lu_work(&Abig, &bbig, &xbig, big_buf, P_big, big_buf));

    /* Eigen sym2 and sym3 dimension mismatch (lines 1013 & 1057 of syn_matrix.c) */
    SYN_MAT_DECL(m3, 3, 3);
    SYN_MAT_DECL(m2, 2, 2);
    q16_t ev2[2], ev3[3];
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_eigen_sym2(&m3, ev2, &m2));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_eigen_sym3(&m2, ev3, &m3));

    /* Least squares Cholesky fallback to LU (line 884 of syn_matrix.c) */
    SYN_MAT_DECL(A_sing_ls, 3, 2);
    SYN_MAT_DECL(b_sing_ls, 3, 1);
    SYN_MAT_DECL(x_sing_ls, 2, 1);
    A_sing_ls.data[0] = Q16_ONE;
    A_sing_ls.data[1] = Q16_ONE;
    A_sing_ls.data[2] = Q16_ONE;
    A_sing_ls.data[3] = Q16_ONE;
    A_sing_ls.data[4] = Q16_ONE;
    A_sing_ls.data[5] = Q16_ONE;
    b_sing_ls.data[0] = Q16_ONE;
    b_sing_ls.data[1] = Q16_ONE;
    b_sing_ls.data[2] = Q16_ONE;
    syn_matrix_least_squares(&A_sing_ls, &b_sing_ls, &x_sing_ls);

    /* Overdetermined m > SYN_SOLVER_MAX_N (12x2 line fit y = 3 + 2*k for k = 0..11) */
    SYN_MAT_DECL(A12, 12, 2);
    SYN_MAT_DECL(b12, 12, 1);
    SYN_MAT_DECL(x12, 2, 1);
    for (uint8_t k = 0U; k < 12U; k++) {
        SYN_MAT_AT(&A12, k, 0) = Q16_ONE;
        SYN_MAT_AT(&A12, k, 1) = Q16_FROM_INT(k);
        SYN_MAT_AT(&b12, k, 0) = Q16_FROM_INT(3 + 2 * (int)k);
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_least_squares(&A12, &b12, &x12));
    ASSERT_Q16_NEAR(Q16_FROM_INT(3), x12.data[0], Q16_TOL * 2);
    ASSERT_Q16_NEAR(Q16_FROM_INT(2), x12.data[1], Q16_TOL * 2);
}

static void test_r5_vec_norm_large_and_extremes(void)
{
    /* Norm >= 181.02 must not wrap negative: ||(200, 200, 100)|| = 300 */
    q16_t v300[3] = {Q16_FROM_INT(200), Q16_FROM_INT(-200), Q16_FROM_INT(100)};
    TEST_ASSERT_INT32_WITHIN(4, Q16_FROM_INT(300), syn_vec_norm(v300, 3U));

    /* Single INT32_MIN element and multi-element saturation (> INT32_MAX) */
    q16_t vmin[2] = {INT32_MIN, 0};
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_vec_norm(vmin, 2U));

    q16_t vhuge[2] = {Q16_FROM_INT(25000), Q16_FROM_INT(25000)};
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_vec_norm(vhuge, 2U));
}

void run_matrix_tests(void)
{
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
    RUN_TEST(test_mat_inverse_lu_nn);
    RUN_TEST(test_mat_singular);
    RUN_TEST(test_mat_rotate_2d);
    RUN_TEST(test_mat_nonsquare_mul);
    RUN_TEST(test_mat_blocks);
    RUN_TEST(test_mat_outer_product);
    RUN_TEST(test_mat_qr);
    RUN_TEST(test_mat_eigen_sym2);
    RUN_TEST(test_mat_eigen_sym3);

    /* Vector operations */
    RUN_TEST(test_vec3_cross);
    RUN_TEST(test_vec_dot);
    RUN_TEST(test_vec_normalize);

    /* Linear Solvers */
    RUN_TEST(test_mat_solve_lu);
    RUN_TEST(test_mat_solve_cholesky);
    RUN_TEST(test_mat_least_squares);

    RUN_TEST(test_matrix_singular_inverse_and_dimension_mismatch);
    RUN_TEST(test_matrix_transforms_and_errors);
    RUN_TEST(test_mat_inverse_4x4_pivot_swap);
    RUN_TEST(test_matrix_qr_singular_and_null_checks);
    RUN_TEST(test_matrix_det_1x1_2x2_3x3_explicit);
    RUN_TEST(test_mat_inv_3x3_singular);
    RUN_TEST(test_mat_inv_4x4_singular);
    RUN_TEST(test_matrix_inv_lu_dimension_mismatch);
    RUN_TEST(test_matrix_vec_normalize_zero_vector);
    RUN_TEST(test_matrix_solve_lu_and_cholesky_oversized_dim);
    RUN_TEST(test_r5_vec_norm_large_and_extremes);
}
