/**
 * @file syn_sensor_fusion.c
 * @brief 6-DOF IMU Mahony Sensor Fusion & AHRS Filter implementation.
 */

#include "syn_sensor_fusion.h"

#include "../util/syn_assert.h"
#include "../util/syn_matrix.h"

#include <string.h>

/**
 * @brief Normalize a 3D Q16.16 vector in place using 64-bit Euclidean norm.
 * @param vx Pointer to X component in Q16.16.
 * @param vy Pointer to Y component in Q16.16.
 * @param vz Pointer to Z component in Q16.16.
 * @return true if norm > 0 and vector normalized, false if zero vector.
 */
static bool fusion_normalize_vec3(q16_t *vx, q16_t *vy, q16_t *vz)
{
    uint64_t ux = (uint64_t)((*vx < 0) ? -(int64_t)(*vx) : (int64_t)(*vx));
    uint64_t uy = (uint64_t)((*vy < 0) ? -(int64_t)(*vy) : (int64_t)(*vy));
    uint64_t uz = (uint64_t)((*vz < 0) ? -(int64_t)(*vz) : (int64_t)(*vz));
    uint32_t norm = syn_isqrt64((ux * ux) + (uy * uy) + (uz * uz));
    if (norm == 0U) {
        return false;
    }

    *vx = (q16_t)(((int64_t)(*vx) * (int64_t)Q16_ONE) / (int64_t)norm);
    *vy = (q16_t)(((int64_t)(*vy) * (int64_t)Q16_ONE) / (int64_t)norm);
    *vz = (q16_t)(((int64_t)(*vz) * (int64_t)Q16_ONE) / (int64_t)norm);
    return true;
}

/**
 * @brief Symmetrically saturate a 64-bit value to [-INT32_MAX, INT32_MAX] (avoiding INT32_MIN).
 * @param val 64-bit signed fixed-point intermediate.
 * @return Clamped Q16.16 value.
 */
static q16_t fusion_clamp_sym_q16(int64_t val)
{
    if (val > (int64_t)INT32_MAX) {
        return INT32_MAX;
    }
    if (val < -(int64_t)INT32_MAX) {
        return -INT32_MAX;
    }
    return (q16_t)val;
}

/**
 * @brief Apply Mahony PI error feedback, integrate quaternion kinematics, and renormalize.
 * @param f         Sensor fusion filter instance.
 * @param gx        Body X angular rate in Q16.16 rad/s.
 * @param gy        Body Y angular rate in Q16.16 rad/s.
 * @param gz        Body Z angular rate in Q16.16 rad/s.
 * @param err       3D orientation error vector in Q16.16.
 * @param apply_err Whether reference vector feedback is valid this sample.
 */
static void fusion_integrate_state(SYN_SensorFusion *f, q16_t gx, q16_t gy, q16_t gz,
                                   const q16_t err[3], bool apply_err)
{
    if (apply_err && f->Ki > 0) {
        f->e_int[0] = q16_add_sat(f->e_int[0], q16_mul_sat(err[0], f->dt));
        f->e_int[1] = q16_add_sat(f->e_int[1], q16_mul_sat(err[1], f->dt));
        f->e_int[2] = q16_add_sat(f->e_int[2], q16_mul_sat(err[2], f->dt));
        gx = q16_add_sat(gx, q16_mul_sat(f->Ki, f->e_int[0]));
        gy = q16_add_sat(gy, q16_mul_sat(f->Ki, f->e_int[1]));
        gz = q16_add_sat(gz, q16_mul_sat(f->Ki, f->e_int[2]));
    }

    if (apply_err && f->Kp > 0) {
        gx = q16_add_sat(gx, q16_mul_sat(f->Kp, err[0]));
        gy = q16_add_sat(gy, q16_mul_sat(f->Kp, err[1]));
        gz = q16_add_sat(gz, q16_mul_sat(f->Kp, err[2]));
    }

    q16_t qw = f->q.w;
    q16_t qx = f->q.x;
    q16_t qy = f->q.y;
    q16_t qz = f->q.z;
    q16_t half_dt = f->dt >> 1;

    q16_t rw = fusion_clamp_sym_q16(
        (-((int64_t)qx * gx) - ((int64_t)qy * gy) - ((int64_t)qz * gz)) >> Q16_SHIFT);
    q16_t rx = fusion_clamp_sym_q16(
        (((int64_t)qw * gx) + ((int64_t)qy * gz) - ((int64_t)qz * gy)) >> Q16_SHIFT);
    q16_t ry = fusion_clamp_sym_q16(
        (((int64_t)qw * gy) - ((int64_t)qx * gz) + ((int64_t)qz * gx)) >> Q16_SHIFT);
    q16_t rz = fusion_clamp_sym_q16(
        (((int64_t)qw * gz) + ((int64_t)qx * gy) - ((int64_t)qy * gx)) >> Q16_SHIFT);

    q16_t dqw = fusion_clamp_sym_q16(((int64_t)rw * (int64_t)half_dt) >> Q16_SHIFT);
    q16_t dqx = fusion_clamp_sym_q16(((int64_t)rx * (int64_t)half_dt) >> Q16_SHIFT);
    q16_t dqy = fusion_clamp_sym_q16(((int64_t)ry * (int64_t)half_dt) >> Q16_SHIFT);
    q16_t dqz = fusion_clamp_sym_q16(((int64_t)rz * (int64_t)half_dt) >> Q16_SHIFT);

    qw = fusion_clamp_sym_q16((int64_t)qw + (int64_t)dqw);
    qx = fusion_clamp_sym_q16((int64_t)qx + (int64_t)dqx);
    qy = fusion_clamp_sym_q16((int64_t)qy + (int64_t)dqy);
    qz = fusion_clamp_sym_q16((int64_t)qz + (int64_t)dqz);

    uint64_t uw = (uint64_t)((qw < 0) ? -(int64_t)qw : (int64_t)qw);
    uint64_t ux = (uint64_t)((qx < 0) ? -(int64_t)qx : (int64_t)qx);
    uint64_t uy = (uint64_t)((qy < 0) ? -(int64_t)qy : (int64_t)qy);
    uint64_t uz = (uint64_t)((qz < 0) ? -(int64_t)qz : (int64_t)qz);
    uint32_t q_norm = syn_isqrt64((uw * uw) + (ux * ux) + (uy * uy) + (uz * uz));
    if (q_norm > 0U) {
        f->q.w = (q16_t)(((int64_t)qw * (int64_t)Q16_ONE) / (int64_t)q_norm);
        f->q.x = (q16_t)(((int64_t)qx * (int64_t)Q16_ONE) / (int64_t)q_norm);
        f->q.y = (q16_t)(((int64_t)qy * (int64_t)Q16_ONE) / (int64_t)q_norm);
        f->q.z = (q16_t)(((int64_t)qz * (int64_t)Q16_ONE) / (int64_t)q_norm);
    }
}

void syn_sensor_fusion_init(SYN_SensorFusion *f, q16_t Kp, q16_t Ki, q16_t dt)
{
    SYN_ASSERT(f != NULL);
    if (f == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    memset(f, 0, sizeof(*f));
    f->Kp = Kp;
    f->Ki = Ki;
    f->dt = (dt > 0) ? dt : Q16_FROM_FLOAT(0.01); /* Default 100 Hz */

    f->q.w = Q16_ONE;
    f->q.x = 0;
    f->q.y = 0;
    f->q.z = 0;
}

void syn_sensor_fusion_reset(SYN_SensorFusion *f)
{
    SYN_ASSERT(f != NULL);
    if (f == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    f->q.w = Q16_ONE;
    f->q.x = 0;
    f->q.y = 0;
    f->q.z = 0;
    f->e_int[0] = 0;
    f->e_int[1] = 0;
    f->e_int[2] = 0;
}

SYN_Status syn_sensor_fusion_update(SYN_SensorFusion *f, q16_t gx, q16_t gy, q16_t gz, q16_t ax,
                                    q16_t ay, q16_t az)
{
    if (f == NULL)
        return SYN_INVALID_PARAM;

    q16_t e_vec[3] = {0, 0, 0};
    bool has_acc = fusion_normalize_vec3(&ax, &ay, &az);
    if (has_acc) {
        q16_t qw = f->q.w;
        q16_t qx = f->q.x;
        q16_t qy = f->q.y;
        q16_t qz = f->q.z;

        q16_t vx = q16_mul(Q16_FROM_INT(2), q16_mul(qx, qz) - q16_mul(qw, qy));
        q16_t vy = q16_mul(Q16_FROM_INT(2), q16_mul(qw, qx) + q16_mul(qy, qz));
        q16_t vz = q16_mul(qw, qw) - q16_mul(qx, qx) - q16_mul(qy, qy) + q16_mul(qz, qz);

        q16_t a_vec[3] = {ax, ay, az};
        q16_t v_vec[3] = {vx, vy, vz};
        syn_vec3_cross(a_vec, v_vec, e_vec);
    }

    fusion_integrate_state(f, gx, gy, gz, e_vec, has_acc);
    return SYN_OK;
}

SYN_Status syn_sensor_fusion_update_9dof(SYN_SensorFusion *f, q16_t gx, q16_t gy, q16_t gz,
                                         q16_t ax, q16_t ay, q16_t az, q16_t mx, q16_t my, q16_t mz)
{
    if (f == NULL)
        return SYN_INVALID_PARAM;

    if (!fusion_normalize_vec3(&mx, &my, &mz)) {
        /* Fallback to 6-DOF update if magnetometer payload is zero/invalid */
        return syn_sensor_fusion_update(f, gx, gy, gz, ax, ay, az);
    }

    q16_t e_tot[3] = {0, 0, 0};
    bool has_acc = fusion_normalize_vec3(&ax, &ay, &az);
    if (has_acc) {
        q16_t qw = f->q.w;
        q16_t qx = f->q.x;
        q16_t qy = f->q.y;
        q16_t qz = f->q.z;

        q16_t vx = q16_mul(Q16_FROM_INT(2), q16_mul(qx, qz) - q16_mul(qw, qy));
        q16_t vy = q16_mul(Q16_FROM_INT(2), q16_mul(qw, qx) + q16_mul(qy, qz));
        q16_t vz = q16_mul(qw, qw) - q16_mul(qx, qx) - q16_mul(qy, qy) + q16_mul(qz, qz);

        /* Rotate magnetometer vector to Earth reference frame: h = C(q) * m */
        q16_t hx =
            q16_mul(mx, Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qy, qy) + q16_mul(qz, qz))) +
            q16_mul(my, q16_mul(Q16_FROM_INT(2), q16_mul(qx, qy) - q16_mul(qw, qz))) +
            q16_mul(mz, q16_mul(Q16_FROM_INT(2), q16_mul(qx, qz) + q16_mul(qw, qy)));

        q16_t hy =
            q16_mul(mx, q16_mul(Q16_FROM_INT(2), q16_mul(qx, qy) + q16_mul(qw, qz))) +
            q16_mul(my, Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qx, qx) + q16_mul(qz, qz))) +
            q16_mul(mz, q16_mul(Q16_FROM_INT(2), q16_mul(qy, qz) - q16_mul(qw, qx)));

        q16_t hz =
            q16_mul(mx, q16_mul(Q16_FROM_INT(2), q16_mul(qx, qz) - q16_mul(qw, qy))) +
            q16_mul(my, q16_mul(Q16_FROM_INT(2), q16_mul(qy, qz) + q16_mul(qw, qx))) +
            q16_mul(mz, Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qx, qx) + q16_mul(qy, qy)));

        /* Reference magnetic field vector: b = (bx, 0, bz) */
        uint64_t uhx = (uint64_t)((hx < 0) ? -(int64_t)hx : (int64_t)hx);
        uint64_t uhy = (uint64_t)((hy < 0) ? -(int64_t)hy : (int64_t)hy);
        q16_t bx = (q16_t)syn_isqrt64((uhx * uhx) + (uhy * uhy));
        q16_t bz = hz;

        /* Estimated direction of magnetic field vector in sensor frame: w = C(q)^T * b */
        q16_t wx =
            q16_mul(bx, Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qy, qy) + q16_mul(qz, qz))) +
            q16_mul(bz, q16_mul(Q16_FROM_INT(2), q16_mul(qx, qz) - q16_mul(qw, qy)));

        q16_t wy = q16_mul(bx, q16_mul(Q16_FROM_INT(2), q16_mul(qx, qy) - q16_mul(qw, qz))) +
                   q16_mul(bz, q16_mul(Q16_FROM_INT(2), q16_mul(qw, qx) + q16_mul(qy, qz)));

        q16_t wz =
            q16_mul(bx, q16_mul(Q16_FROM_INT(2), q16_mul(qw, qy) + q16_mul(qx, qz))) +
            q16_mul(bz, Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qx, qx) + q16_mul(qy, qy)));

        q16_t a_vec[3] = {ax, ay, az};
        q16_t v_vec[3] = {vx, vy, vz};
        q16_t e_acc[3];
        syn_vec3_cross(a_vec, v_vec, e_acc);

        q16_t m_vec[3] = {mx, my, mz};
        q16_t w_vec[3] = {wx, wy, wz};
        q16_t e_mag[3];
        syn_vec3_cross(m_vec, w_vec, e_mag);

        e_tot[0] = q16_add_sat(e_acc[0], e_mag[0]);
        e_tot[1] = q16_add_sat(e_acc[1], e_mag[1]);
        e_tot[2] = q16_add_sat(e_acc[2], e_mag[2]);
    }

    fusion_integrate_state(f, gx, gy, gz, e_tot, has_acc);
    return SYN_OK;
}

SYN_Status syn_sensor_fusion_get_quaternion(const SYN_SensorFusion *f, SYN_Quaternion *q)
{
    if (f == NULL || q == NULL)
        return SYN_INVALID_PARAM;
    *q = f->q;
    return SYN_OK;
}

SYN_Status syn_sensor_fusion_get_euler(const SYN_SensorFusion *f, SYN_EulerAngles *euler)
{
    if (f == NULL || euler == NULL)
        return SYN_INVALID_PARAM;

    q16_t qw = f->q.w;
    q16_t qx = f->q.x;
    q16_t qy = f->q.y;
    q16_t qz = f->q.z;

    /* Roll: atan2(2*(qw*qx + qy*qz), 1 - 2*(qx^2 + qy^2)) */
    q16_t roll_num = q16_mul(Q16_FROM_INT(2), q16_mul(qw, qx) + q16_mul(qy, qz));
    q16_t roll_den = Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qx, qx) + q16_mul(qy, qy));
    euler->roll_rad = q16_atan2(roll_num, roll_den);

    /* Pitch: asin(2*(qw*qy - qz*qx)) */
    q16_t pitch_sin = q16_mul(Q16_FROM_INT(2), q16_mul(qw, qy) - q16_mul(qz, qx));
    euler->pitch_rad = q16_asin(pitch_sin);

    /* Yaw: atan2(2*(qw*qz + qx*qy), 1 - 2*(qy^2 + qz^2)) */
    q16_t yaw_num = q16_mul(Q16_FROM_INT(2), q16_mul(qw, qz) + q16_mul(qx, qy));
    q16_t yaw_den = Q16_ONE - q16_mul(Q16_FROM_INT(2), q16_mul(qy, qy) + q16_mul(qz, qz));
    euler->yaw_rad = q16_atan2(yaw_num, yaw_den);

    return SYN_OK;
}
