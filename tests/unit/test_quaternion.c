/**
 * @file test_quaternion.c
 * @brief Unit tests for fixed-point 3D Quaternion algebra.
 */

#include "syntropic/util/syn_quaternion.h"
#include "unity/unity.h"

#include <string.h>

#define Q16_TOL 50 /* ~0.0007 in Q16 */

static void ASSERT_Q16_NEAR(q16_t expected, q16_t actual, q16_t tol)
{
    q16_t diff = q16_abs(expected - actual);
    if (diff > tol) {
        TEST_FAIL_MESSAGE("Q16 values not equal within tolerance");
    }
}

void test_quaternion_identity_and_init(void)
{
    SYN_Quaternion q;
    syn_quat_identity(&q);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q.w);
    TEST_ASSERT_EQUAL_INT32(0, q.x);
    TEST_ASSERT_EQUAL_INT32(0, q.y);
    TEST_ASSERT_EQUAL_INT32(0, q.z);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, syn_quat_norm(&q));

    syn_quat_init(&q, Q16_FROM_INT(1), Q16_FROM_INT(2), Q16_FROM_INT(3), Q16_FROM_INT(4));
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(1), q.w);
    TEST_ASSERT_EQUAL_INT32(Q16_FROM_INT(2), q.x);
}

void test_quaternion_mul_and_rotate(void)
{
    SYN_Quaternion q1, q2, q_prod;
    syn_quat_identity(&q1);
    syn_quat_identity(&q2);

    syn_quat_mul(&q1, &q2, &q_prod);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q_prod.w);
    TEST_ASSERT_EQUAL_INT32(0, q_prod.x);

    /* Rotate vector (1, 0, 0) by 90 degrees around Z axis */
    SYN_Quaternion q_rot;
    syn_quat_from_euler(&q_rot, 0, 0, Q16_PI_2);

    q16_t v[3] = {Q16_ONE, 0, 0};
    q16_t v_out[3];
    syn_quat_rotate_vec3(&q_rot, v, v_out);

    ASSERT_Q16_NEAR(0, v_out[0], Q16_TOL);
    ASSERT_Q16_NEAR(Q16_ONE, v_out[1], Q16_TOL);
    ASSERT_Q16_NEAR(0, v_out[2], Q16_TOL);
}

void test_quaternion_euler_roundtrip(void)
{
    q16_t roll_in = Q16_FROM_FRAC(1, 4);  /* 0.25 rad */
    q16_t pitch_in = Q16_FROM_FRAC(1, 5); /* 0.20 rad */
    q16_t yaw_in = Q16_FROM_FRAC(1, 2);   /* 0.50 rad */

    SYN_Quaternion q;
    syn_quat_from_euler(&q, roll_in, pitch_in, yaw_in);

    q16_t roll_out, pitch_out, yaw_out;
    syn_quat_to_euler(&q, &roll_out, &pitch_out, &yaw_out);

    ASSERT_Q16_NEAR(roll_in, roll_out, Q16_TOL);
    ASSERT_Q16_NEAR(pitch_in, pitch_out, Q16_TOL);
    ASSERT_Q16_NEAR(yaw_in, yaw_out, Q16_TOL);
}

void test_quaternion_slerp(void)
{
    SYN_Quaternion q1, q2, q_mid;
    syn_quat_from_euler(&q1, 0, 0, 0);
    syn_quat_from_euler(&q2, 0, 0, Q16_PI_2);

    /* Halfway slerp (t = 0.5) should equal 45 degree rotation */
    syn_quat_slerp(&q1, &q2, Q16_HALF, &q_mid);

    q16_t roll, pitch, yaw;
    syn_quat_to_euler(&q_mid, &roll, &pitch, &yaw);

    ASSERT_Q16_NEAR(0, roll, Q16_TOL * 5);
    ASSERT_Q16_NEAR(0, pitch, Q16_TOL * 5);
    ASSERT_Q16_NEAR(Q16_PI / 4, yaw, Q16_TOL * 5);
}

void test_quaternion_matrix_and_inverse(void)
{
    SYN_Quaternion q;
    syn_quat_from_euler(&q, 0, 0, Q16_PI_2);

    /* Test conjugate & inverse */
    SYN_Quaternion q_conj, q_inv;
    syn_quat_conjugate(&q, &q_conj);
    SYN_Status st = syn_quat_inverse(&q, &q_inv);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    ASSERT_Q16_NEAR(q_conj.w, q_inv.w, Q16_TOL);
    ASSERT_Q16_NEAR(q_conj.x, q_inv.x, Q16_TOL);

    /* Test inverse of zero norm */
    SYN_Quaternion q_zero = {0, 0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_quat_inverse(&q_zero, &q_inv));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_quat_normalize(&q_zero));

    /* Test to_mat3x3 */
    q16_t mat_buf[9];
    SYN_MAT_INIT(mat, mat_buf, 3, 3);
    syn_quat_to_mat3x3(&q, &mat);
    /* For 90 deg yaw rotation: R[0][0] approx 0, R[0][1] approx -1, R[1][0] approx 1, R[1][1]
     * approx 0 */
    ASSERT_Q16_NEAR(0, SYN_MAT_AT(&mat, 0, 0), Q16_TOL * 20);
    ASSERT_Q16_NEAR(-Q16_ONE, SYN_MAT_AT(&mat, 0, 1), Q16_TOL * 20);
    ASSERT_Q16_NEAR(Q16_ONE, SYN_MAT_AT(&mat, 1, 0), Q16_TOL * 20);
}

void test_quaternion_extended_edge_cases(void)
{
    /* Gimbal lock pitch testing (sinp >= 1.0, line 194 of syn_quaternion.c) */
    SYN_Quaternion q_gimbal, q_out;
    memset(&q_gimbal, 0, sizeof(q_gimbal));
    q_gimbal.w = 46342;
    q_gimbal.y = 46342;
    q16_t roll, pitch, yaw;
    syn_quat_to_euler(&q_gimbal, &roll, &pitch, &yaw);
    TEST_ASSERT_EQUAL_INT32(Q16_PI_2, pitch);

    /* Negative gimbal lock pitch testing (sinp <= -1.0) */
    memset(&q_gimbal, 0, sizeof(q_gimbal));
    q_gimbal.w = 46342;
    q_gimbal.y = -46342;
    syn_quat_to_euler(&q_gimbal, &roll, &pitch, &yaw);
    TEST_ASSERT_EQUAL_INT32(-Q16_PI_2, pitch);

    /* Slerp with negative dot product (opposite orientation quaternions) */
    SYN_Quaternion q1, q2_neg;
    syn_quat_identity(&q1);
    syn_quat_identity(&q2_neg);
    q2_neg.w = -Q16_ONE; /* opposite sign representation of identity */
    syn_quat_slerp(&q1, &q2_neg, Q16_HALF, &q_out);
    TEST_ASSERT_EQUAL_INT32(Q16_ONE, q16_abs(q_out.w));

    /* Slerp with very small angle (dot > 0.999) triggering linear fallback */
    SYN_Quaternion q_near1, q_near2;
    syn_quat_identity(&q_near1);
    syn_quat_from_euler(&q_near2, 0, 0, Q16_FROM_FRAC(1, 100)); /* 0.01 rad */
    syn_quat_slerp(&q_near1, &q_near2, Q16_HALF, &q_out);
    TEST_ASSERT_TRUE(syn_quat_norm(&q_out) > 0);

    /* Slerp with large angle (dot <= 0.999) triggering spherical interpolation */
    SYN_Quaternion q_far1, q_far2;
    syn_quat_identity(&q_far1);
    syn_quat_from_euler(&q_far2, Q16_PI / 4, Q16_PI / 4, Q16_PI / 4);
    syn_quat_slerp(&q_far1, &q_far2, Q16_HALF, &q_out);
    TEST_ASSERT_TRUE(syn_quat_norm(&q_out) > 0);
}

static void test_r5_quaternion_norm_asin_slerp(void)
{
    /* 1. Large quaternion norm (>= 181.02) and INT32_MIN / sum saturation */
    SYN_Quaternion q_large;
    syn_quat_init(&q_large, Q16_FROM_INT(200), Q16_FROM_INT(-200), Q16_FROM_INT(100), 0);
    TEST_ASSERT_INT32_WITHIN(4, Q16_FROM_INT(300), syn_quat_norm(&q_large));
    TEST_ASSERT_EQUAL_INT(SYN_OK, syn_quat_normalize(&q_large));
    TEST_ASSERT_INT32_WITHIN(10, Q16_ONE, syn_quat_norm(&q_large));

    SYN_Quaternion q_min = {INT32_MIN, 0, 0, 0};
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_quat_norm(&q_min));

    SYN_Quaternion q_sum_sat = {Q16_FROM_INT(25000), Q16_FROM_INT(25000), 0, 0};
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, syn_quat_norm(&q_sum_sat));

    /* 2. High-pitch Euler roundtrip near +-90 deg (85 deg = 1.48353 rad = 97224 in Q16) */
    q16_t pitch_85 = 97224;
    SYN_Quaternion q_pitch;
    syn_quat_from_euler(&q_pitch, 0, pitch_85, 0);
    q16_t r_out = 0;
    q16_t p_out = 0;
    q16_t y_out = 0;
    syn_quat_to_euler(&q_pitch, &r_out, &p_out, &y_out);
    ASSERT_Q16_NEAR(pitch_85, p_out, 250);

    /* 3. Small-angle spherical slerp (dot ~ 0.9986, just below 0.999 linear threshold)
     * and verify slerp output is normalized */
    SYN_Quaternion q_a, q_b, q_mid;
    syn_quat_identity(&q_a);
    syn_quat_from_euler(&q_b, 0, 0, 6863); /* ~6 deg yaw -> half-angle 3 deg, cos(3 deg)=0.9986 */
    syn_quat_slerp(&q_a, &q_b, Q16_HALF, &q_mid);
    ASSERT_Q16_NEAR(Q16_ONE, syn_quat_norm(&q_mid), 10);
    syn_quat_to_euler(&q_mid, &r_out, &p_out, &y_out);
    ASSERT_Q16_NEAR(3431, y_out, 100);
}

void run_quaternion_tests(void)
{
    RUN_TEST(test_quaternion_identity_and_init);
    RUN_TEST(test_quaternion_mul_and_rotate);
    RUN_TEST(test_quaternion_euler_roundtrip);
    RUN_TEST(test_quaternion_slerp);
    RUN_TEST(test_quaternion_matrix_and_inverse);
    RUN_TEST(test_quaternion_extended_edge_cases);
    RUN_TEST(test_r5_quaternion_norm_asin_slerp);
}
