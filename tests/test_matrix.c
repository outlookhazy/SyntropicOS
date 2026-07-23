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
/*  Linear Solvers Tests                                                    */
/* ════════════════════════════════════════════════════════════════════════ */

static void test_mat_solve_lu(void)
{
    /* 1. 2×2 system: A = [[2, 1], [5, 7]], b = [11, 13] */
    SYN_MAT2_DECL(A2);
    SYN_MAT_DECL(b2, 2, 1);
    SYN_MAT_DECL(x2, 2, 1);
    SYN_MAT_DECL(b2_check, 2, 1);

    A2.data[0] = Q16_FROM_INT(2); A2.data[1] = Q16_FROM_INT(1);
    A2.data[2] = Q16_FROM_INT(5); A2.data[3] = Q16_FROM_INT(7);

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

    A3.data[0] = Q16_FROM_INT(2);  A3.data[1] = Q16_FROM_INT(1);  A3.data[2] = Q16_FROM_INT(-1);
    A3.data[3] = Q16_FROM_INT(-3); A3.data[4] = Q16_FROM_INT(-1); A3.data[5] = Q16_FROM_INT(2);
    A3.data[6] = Q16_FROM_INT(-2); A3.data[7] = Q16_FROM_INT(1);  A3.data[8] = Q16_FROM_INT(2);

    b3.data[0] = Q16_FROM_INT(8);
    b3.data[1] = Q16_FROM_INT(-11);
    b3.data[2] = Q16_FROM_INT(-3);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_solve_lu(&A3, &b3, &x3));
    ASSERT_Q16_NEAR(Q16_FROM_INT(2),  x3.data[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(3),  x3.data[1], Q16_MAT_TOL);
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

    A4.data[0]  = Q16_ONE; A4.data[1]  = Q16_ONE;          A4.data[2]  = Q16_ONE;           A4.data[3]  = Q16_ONE;
    A4.data[4]  = Q16_ONE; A4.data[5]  = Q16_FROM_INT(2);  A4.data[6]  = Q16_FROM_INT(3);   A4.data[7]  = Q16_FROM_INT(4);
    A4.data[8]  = Q16_ONE; A4.data[9]  = Q16_FROM_INT(3);  A4.data[10] = Q16_FROM_INT(6);   A4.data[11] = Q16_FROM_INT(10);
    A4.data[12] = Q16_ONE; A4.data[13] = Q16_FROM_INT(4);  A4.data[14] = Q16_FROM_INT(10);  A4.data[15] = Q16_FROM_INT(20);

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
    A_sing.data[0] = Q16_ONE; A_sing.data[1] = Q16_FROM_INT(2);
    A_sing.data[2] = Q16_FROM_INT(2); A_sing.data[3] = Q16_FROM_INT(4);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_matrix_solve_lu(&A_sing, &b2, &x2));

    /* 5. Dimension mismatch validation */
    SYN_MAT_DECL(b_bad, 3, 1);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_matrix_solve_lu(&A2, &b_bad, &x2));
}

static void test_mat_solve_cholesky(void)
{
    /* 1. 2×2 SPD: A = [[4, 12], [12, 45]], b = [16, 57] -> x = [1, 1] */
    SYN_MAT2_DECL(A2);
    SYN_MAT_DECL(b2, 2, 1);
    SYN_MAT_DECL(x2, 2, 1);

    A2.data[0] = Q16_FROM_INT(4);  A2.data[1] = Q16_FROM_INT(12);
    A2.data[2] = Q16_FROM_INT(12); A2.data[3] = Q16_FROM_INT(45);

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

    A3.data[0] = Q16_FROM_INT(4);   A3.data[1] = Q16_FROM_INT(12);  A3.data[2] = Q16_FROM_INT(-16);
    A3.data[3] = Q16_FROM_INT(12);  A3.data[4] = Q16_FROM_INT(37);  A3.data[5] = Q16_FROM_INT(-43);
    A3.data[6] = Q16_FROM_INT(-16); A3.data[7] = Q16_FROM_INT(-43); A3.data[8] = Q16_FROM_INT(98);

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
    A_not_pd.data[0] = Q16_ONE; A_not_pd.data[1] = Q16_FROM_INT(2);
    A_not_pd.data[2] = Q16_FROM_INT(2); A_not_pd.data[3] = Q16_ONE;
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

    A32.data[0] = Q16_ONE; A32.data[1] = 0;
    A32.data[2] = Q16_ONE; A32.data[3] = Q16_ONE;
    A32.data[4] = Q16_ONE; A32.data[5] = Q16_FROM_INT(2);

    b3.data[0] = Q16_FROM_INT(1);
    b3.data[1] = Q16_FROM_INT(2);
    b3.data[2] = Q16_FROM_INT(4);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_least_squares(&A32, &b3, &x2));
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(5, 6), x2.data[0], Q16_TOL * 2);
    ASSERT_Q16_NEAR(Q16_FROM_FRAC(3, 2), x2.data[1], Q16_TOL * 2);

    /* 2. Overdetermined 4×3 system (quadratic fit y = c0 + c1*x + c2*x^2 to points (0,1), (1,3), (2,7), (3,13))
     * Exact curve: y = 1 + x + x^2 -> x = [1, 1, 1] */
    SYN_MAT_DECL(A43, 4, 3);
    SYN_MAT_DECL(b4, 4, 1);
    SYN_MAT_DECL(x3, 3, 1);

    A43.data[0] = Q16_ONE; A43.data[1] = 0;                 A43.data[2] = 0;
    A43.data[3] = Q16_ONE; A43.data[4] = Q16_ONE;          A43.data[5] = Q16_ONE;
    A43.data[6] = Q16_ONE; A43.data[7] = Q16_FROM_INT(2);  A43.data[8] = Q16_FROM_INT(4);
    A43.data[9] = Q16_ONE; A43.data[10] = Q16_FROM_INT(3); A43.data[11] = Q16_FROM_INT(9);

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
    sub.data[0] = Q16_FROM_INT(1); sub.data[1] = Q16_FROM_INT(2);
    sub.data[2] = Q16_FROM_INT(3); sub.data[3] = Q16_FROM_INT(4);

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
    q16_t u[2] = { Q16_FROM_INT(1), Q16_FROM_INT(2) };
    q16_t v[3] = { Q16_FROM_INT(3), Q16_FROM_INT(4), Q16_FROM_INT(5) };

    SYN_MAT_DECL(M, 2, 3);
    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_outer_product(u, 2, v, 3, &M));

    TEST_ASSERT_EQUAL(Q16_FROM_INT(3),  SYN_MAT_AT(&M, 0, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(4),  SYN_MAT_AT(&M, 0, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(5),  SYN_MAT_AT(&M, 0, 2));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(6),  SYN_MAT_AT(&M, 1, 0));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(8),  SYN_MAT_AT(&M, 1, 1));
    TEST_ASSERT_EQUAL(Q16_FROM_INT(10), SYN_MAT_AT(&M, 1, 2));
}

static void test_mat_qr(void)
{
    /* A = [[1, 1], [1, 0], [1, 2]] (3×2 matrix) */
    SYN_MAT_DECL(A, 3, 2);
    SYN_MAT_DECL(Q, 3, 2);
    SYN_MAT_DECL(R, 2, 2);

    A.data[0] = Q16_ONE; A.data[1] = Q16_ONE;
    A.data[2] = Q16_ONE; A.data[3] = 0;
    A.data[4] = Q16_ONE; A.data[5] = Q16_FROM_INT(2);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_qr(&A, &Q, &R));

    /* 1. R must be upper-triangular: R[1][0] == 0 */
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&R, 1, 0), Q16_MAT_TOL);

    /* 2. Q must be orthogonal: Qᵀ · Q == I (2×2) */
    SYN_MAT_DECL(QT, 2, 3);
    SYN_MAT_DECL(QTQ, 2, 2);
    syn_matrix_transpose(&Q, &QT);
    syn_matrix_mul(&QT, &Q, &QTQ);

    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&QTQ, 0, 0), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0,       SYN_MAT_AT(&QTQ, 0, 1), Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0,       SYN_MAT_AT(&QTQ, 1, 0), Q16_MAT_TOL);
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

    A.data[0] = Q16_FROM_INT(4); A.data[1] = Q16_FROM_INT(2);
    A.data[2] = Q16_FROM_INT(2); A.data[3] = Q16_FROM_INT(1);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym2(&A, evals, &E));

    ASSERT_Q16_NEAR(Q16_FROM_INT(5), evals[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(0,               evals[1], Q16_MAT_TOL);

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

    A_diag.data[0] = Q16_FROM_INT(3); A_diag.data[1] = 0;
    A_diag.data[2] = 0;               A_diag.data[3] = Q16_FROM_INT(1);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym2(&A_diag, evals_diag, &E_diag));
    ASSERT_Q16_NEAR(Q16_FROM_INT(3), evals_diag[0], Q16_MAT_TOL);
    ASSERT_Q16_NEAR(Q16_FROM_INT(1), evals_diag[1], Q16_MAT_TOL);
    TEST_ASSERT_EQUAL(Q16_ONE, SYN_MAT_AT(&E_diag, 0, 0));
    TEST_ASSERT_EQUAL(0,       SYN_MAT_AT(&E_diag, 0, 1));
}

static void test_mat_eigen_sym3(void)
{
    /* 1. A = [[3, 1, 1], [1, 3, 1], [1, 1, 3]] -> evals: 5, 2, 2 */
    SYN_MAT3_DECL(A);
    SYN_MAT3_DECL(E);
    q16_t evals[3];

    A.data[0] = Q16_FROM_INT(3); A.data[1] = Q16_FROM_INT(1); A.data[2] = Q16_FROM_INT(1);
    A.data[3] = Q16_FROM_INT(1); A.data[4] = Q16_FROM_INT(3); A.data[5] = Q16_FROM_INT(1);
    A.data[6] = Q16_FROM_INT(1); A.data[7] = Q16_FROM_INT(1); A.data[8] = Q16_FROM_INT(3);

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

    A2.data[0] = Q16_FROM_INT(2); A2.data[1] = Q16_FROM_INT(1); A2.data[2] = 0;
    A2.data[3] = Q16_FROM_INT(1); A2.data[4] = Q16_FROM_INT(2); A2.data[5] = Q16_FROM_INT(1);
    A2.data[6] = 0;               A2.data[7] = Q16_FROM_INT(1); A2.data[8] = Q16_FROM_INT(2);

    TEST_ASSERT_EQUAL(SYN_OK, syn_matrix_eigen_sym3(&A2, evals2, &E2));

    /* λ = 2 + sqrt(2) ≈ 3.414, 2.0, 2 - sqrt(2) ≈ 0.586 */
    q16_t expected_l0 = Q16_FROM_INT(2) + Q16_SQRT2;
    q16_t expected_l1 = Q16_FROM_INT(2);
    q16_t expected_l2 = Q16_FROM_INT(2) - Q16_SQRT2;

    ASSERT_Q16_NEAR(expected_l0, evals2[0], Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(expected_l1, evals2[1], Q16_MAT_TOL * 2);
    ASSERT_Q16_NEAR(expected_l2, evals2[2], Q16_MAT_TOL * 2);
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
}

