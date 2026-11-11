/**
 * @file test_sensor_fusion.c
 * @brief Unity tests for 6-DOF IMU Sensor Fusion & AHRS Mahony Filter.
 */

#include "mocks/mock_port.h"
#include "syntropic/sensor/syn_sensor_fusion.h"
#include "syntropic/syntropic.h"
#include "unity/unity.h"

#define FUSION_TOL Q16_FROM_FLOAT(0.05)

static void test_sensor_fusion_init(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(2.0), Q16_FROM_FLOAT(0.01), Q16_FROM_FLOAT(0.01));

    SYN_Quaternion q;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_quaternion(&f, &q));
    TEST_ASSERT_EQUAL(Q16_ONE, q.w);
    TEST_ASSERT_EQUAL(0, q.x);
    TEST_ASSERT_EQUAL(0, q.y);
    TEST_ASSERT_EQUAL(0, q.z);

    SYN_EulerAngles euler;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));
    TEST_ASSERT_EQUAL(0, euler.roll_rad);
    TEST_ASSERT_EQUAL(0, euler.pitch_rad);
    TEST_ASSERT_EQUAL(0, euler.yaw_rad);

    /* Zero dt default fallback test */
    SYN_SensorFusion f0;
    syn_sensor_fusion_init(&f0, Q16_ONE, 0, 0);
    TEST_ASSERT_EQUAL(Q16_FROM_FLOAT(0.01), f0.dt);
}

static void test_sensor_fusion_static_gravity(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(2.0), Q16_FROM_FLOAT(0.0), Q16_FROM_FLOAT(0.01));

    /* Level accel: [0, 0, 1g], zero gyro */
    for (int i = 0; i < 50; i++) {
        TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_update(&f, 0, 0, 0, 0, 0, Q16_ONE));
    }

    SYN_EulerAngles euler;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, 0, euler.roll_rad);
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, 0, euler.pitch_rad);
}

static void test_sensor_fusion_pitch_tilt(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(5.0), Q16_FROM_FLOAT(0.01), Q16_FROM_FLOAT(0.01));

    /* 30 degree pitch tilt: sin(30°)=0.5, cos(30°)=0.866 */
    q16_t ax = Q16_HALF;                 /* sin(30 deg) */
    q16_t az = Q16_FROM_FLOAT(0.866025); /* cos(30 deg) */

    for (int i = 0; i < 100; i++) {
        syn_sensor_fusion_update(&f, 0, 0, 0, ax, 0, az);
    }

    SYN_EulerAngles euler;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));

    /* Expected pitch angle: -30 deg ≈ -0.5236 rad under right-hand body frame */
    q16_t expected_pitch = -Q16_FROM_FLOAT(0.5236);
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, expected_pitch, euler.pitch_rad);
}

static void test_sensor_fusion_gyro_integration(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(0.0), Q16_FROM_FLOAT(0.0), Q16_FROM_FLOAT(0.01));

    /* Pure roll gyro rotation: gx = 0.5 rad/s for 100 steps @ 100Hz = 1 sec -> 0.5 rad total roll
     */
    q16_t gx = Q16_HALF;

    for (int i = 0; i < 100; i++) {
        syn_sensor_fusion_update(&f, gx, 0, 0, 0, 0, 0);
    }

    SYN_EulerAngles euler;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));

    q16_t expected_roll = Q16_HALF; /* ~0.5 rad */
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, expected_roll, euler.roll_rad);
}

static void test_sensor_fusion_quaternion_norm(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(2.0), Q16_FROM_FLOAT(0.01), Q16_FROM_FLOAT(0.01));

    /* Dynamic pitch & roll rotation sequence */
    for (int i = 0; i < 200; i++) {
        q16_t gx = Q16_FROM_FLOAT(0.2);
        q16_t gy = Q16_FROM_FLOAT(-0.1);
        q16_t ax = Q16_FROM_FLOAT(0.3);
        q16_t az = Q16_FROM_FLOAT(0.9);
        syn_sensor_fusion_update(&f, gx, gy, 0, ax, 0, az);
    }

    SYN_Quaternion q;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_quaternion(&f, &q));

    /* Unit norm check: w^2 + x^2 + y^2 + z^2 == 1.0 */
    int64_t norm_sq =
        ((int64_t)q.w * q.w) + ((int64_t)q.x * q.x) + ((int64_t)q.y * q.y) + ((int64_t)q.z * q.z);
    norm_sq >>= Q16_SHIFT;

    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.01), Q16_ONE, (q16_t)norm_sq);
}

static void test_sensor_fusion_reset_and_invalid(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(2.0), Q16_FROM_FLOAT(0.01), Q16_FROM_FLOAT(0.01));
    syn_sensor_fusion_update(&f, Q16_ONE, 0, 0, 0, 0, Q16_ONE);

    syn_sensor_fusion_reset(&f);
    SYN_Quaternion q;
    syn_sensor_fusion_get_quaternion(&f, &q);
    TEST_ASSERT_EQUAL(Q16_ONE, q.w);
    TEST_ASSERT_EQUAL(0, q.x);

    /* Invalid parameters checks */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_sensor_fusion_update(NULL, 0, 0, 0, 0, 0, 0));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_sensor_fusion_get_quaternion(NULL, &q));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_sensor_fusion_get_quaternion(&f, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_sensor_fusion_get_euler(&f, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_sensor_fusion_get_euler(NULL, NULL));
}

static void test_sensor_fusion_9dof_magnetometer(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(2.0), Q16_FROM_FLOAT(0.01), Q16_FROM_FLOAT(0.01));

    /* NULL check for 9-DOF */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_sensor_fusion_update_9dof(NULL, 0, 0, 0, 0, 0, Q16_ONE, Q16_ONE, 0, 0));

    /* Zero magnetometer fallback */
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_update_9dof(&f, 0, 0, 0, 0, 0, Q16_ONE, 0, 0, 0));

    /* 9-DOF update with level accel [0, 0, 1g] and magnetic field pointing North [1, 0, 0] */
    q16_t mx = Q16_FROM_FLOAT(0.5);
    q16_t mz = Q16_FROM_FLOAT(0.5);

    for (int i = 0; i < 50; i++) {
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_sensor_fusion_update_9dof(&f, 0, 0, 0, 0, 0, Q16_ONE, mx, 0, mz));
    }

    SYN_EulerAngles euler;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, 0, euler.roll_rad);
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, 0, euler.pitch_rad);
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, 0, euler.yaw_rad);
}

static void test_sensor_fusion_mg_scale_9dof_yaw_and_bias_rejection(void)
{
    /* 1. mG-scale accelerometer (1000 mG > 181.02 Q16.16 sqrt ceiling): 30 deg pitch */
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(5.0), 0, Q16_FROM_FLOAT(0.01));
    for (int i = 0; i < 120; i++) {
        TEST_ASSERT_EQUAL(
            SYN_OK, syn_sensor_fusion_update(&f, 0, 0, 0, Q16_FROM_INT(500), 0, Q16_FROM_INT(866)));
    }
    SYN_EulerAngles euler;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, -Q16_FROM_FLOAT(0.5236), euler.pitch_rad);

    /* 2. 9-DOF with mG accel and uT magnetometer pointing +Y -> yaw converges to -pi/2 */
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(5.0), 0, Q16_FROM_FLOAT(0.01));
    for (int i = 0; i < 200; i++) {
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_sensor_fusion_update_9dof(&f, 0, 0, 0, 0, 0, Q16_FROM_INT(1000), 0,
                                                        Q16_FROM_INT(500), 0));
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));
    TEST_ASSERT_INT_WITHIN(FUSION_TOL, -Q16_PI_2, euler.yaw_rad);

    /* 3. Integral feedback rejects steady gyro bias */
    syn_sensor_fusion_init(&f, Q16_FROM_FLOAT(2.0), Q16_ONE, Q16_FROM_FLOAT(0.01));
    for (int i = 0; i < 400; i++) {
        TEST_ASSERT_EQUAL(SYN_OK,
                          syn_sensor_fusion_update(&f, Q16_FROM_FLOAT(0.2), 0, 0, 0, 0, Q16_ONE));
    }
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_euler(&f, &euler));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), 0, euler.roll_rad);
}

static void test_sensor_fusion_extreme_gyro_saturation_preserves_unit_norm(void)
{
    SYN_SensorFusion f;
    syn_sensor_fusion_init(&f, 0, 0, Q16_FROM_INT(4));
    f.q.w = Q16_HALF;
    f.q.x = -Q16_ONE;
    f.q.y = Q16_HALF;
    f.q.z = Q16_HALF;

    /* Extreme INT32_MIN and INT32_MAX angular rates must saturate safely and preserve unit norm */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_sensor_fusion_update(&f, INT32_MIN, INT32_MIN, INT32_MIN, 0, 0, 0));
    SYN_Quaternion q;
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_quaternion(&f, &q));
    int64_t norm_sq = (((int64_t)q.w * q.w) + ((int64_t)q.x * q.x) + ((int64_t)q.y * q.y) +
                       ((int64_t)q.z * q.z)) >>
                      Q16_SHIFT;
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), Q16_ONE, (q16_t)norm_sq);

    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_sensor_fusion_update(&f, INT32_MAX, INT32_MAX, INT32_MAX, 0, 0, 0));
    TEST_ASSERT_EQUAL(SYN_OK, syn_sensor_fusion_get_quaternion(&f, &q));
    norm_sq = (((int64_t)q.w * q.w) + ((int64_t)q.x * q.x) + ((int64_t)q.y * q.y) +
               ((int64_t)q.z * q.z)) >>
              Q16_SHIFT;
    TEST_ASSERT_INT_WITHIN(Q16_FROM_FLOAT(0.02), Q16_ONE, (q16_t)norm_sq);
}

void run_sensor_fusion_tests(void)
{
    RUN_TEST(test_sensor_fusion_init);
    RUN_TEST(test_sensor_fusion_static_gravity);
    RUN_TEST(test_sensor_fusion_pitch_tilt);
    RUN_TEST(test_sensor_fusion_gyro_integration);
    RUN_TEST(test_sensor_fusion_quaternion_norm);
    RUN_TEST(test_sensor_fusion_reset_and_invalid);
    RUN_TEST(test_sensor_fusion_9dof_magnetometer);
    RUN_TEST(test_sensor_fusion_mg_scale_9dof_yaw_and_bias_rejection);
    RUN_TEST(test_sensor_fusion_extreme_gyro_saturation_preserves_unit_norm);
}
