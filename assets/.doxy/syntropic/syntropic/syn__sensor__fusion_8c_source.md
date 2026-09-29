

# File syn\_sensor\_fusion.c

[**File List**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_sensor\_fusion.c**](syn__sensor__fusion_8c.md)

[Go to the documentation of this file](syn__sensor__fusion_8c.md)


```C++

#include "syn_sensor_fusion.h"

#include "../util/syn_assert.h"
#include "../util/syn_matrix.h"

#include <string.h>

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

    q16_t qw = f->q.w;
    q16_t qx = f->q.x;
    q16_t qy = f->q.y;
    q16_t qz = f->q.z;

    /* Compute accelerometer magnitude squared */
    int64_t a_sq = ((int64_t)ax * ax) + ((int64_t)ay * ay) + ((int64_t)az * az);
    a_sq >>= Q16_SHIFT;

    if (a_sq > 0) {
        q16_t a_norm = q16_sqrt((q16_t)a_sq);
        if (a_norm > 0) {
            /* Normalize accelerometer measurements */
            ax = q16_div(ax, a_norm);
            ay = q16_div(ay, a_norm);
            az = q16_div(az, a_norm);

            /* Estimated direction of gravity vector from current quaternion state */
            /* vx = 2*(qx*qz - qw*qy) */
            q16_t vx = q16_mul(Q16_FROM_INT(2), q16_mul(qx, qz) - q16_mul(qw, qy));
            /* vy = 2*(qw*qx + qy*qz) */
            q16_t vy = q16_mul(Q16_FROM_INT(2), q16_mul(qw, qx) + q16_mul(qy, qz));
            /* vz = qw^2 - qx^2 - qy^2 + qz^2 */
            q16_t vz = q16_mul(qw, qw) - q16_mul(qx, qx) - q16_mul(qy, qy) + q16_mul(qz, qz);

            /* Cross product error vector e = a x v */
            q16_t a_vec[3] = {ax, ay, az};
            q16_t v_vec[3] = {vx, vy, vz};
            q16_t e_vec[3];
            syn_vec3_cross(a_vec, v_vec, e_vec);
            q16_t ex = e_vec[0], ey = e_vec[1], ez = e_vec[2];

            /* Accumulate integral error */
            if (f->Ki > 0) {
                f->e_int[0] += q16_mul(ex, f->dt);
                f->e_int[1] += q16_mul(ey, f->dt);
                f->e_int[2] += q16_mul(ez, f->dt);
                gx += q16_mul(f->Ki, f->e_int[0]);
                gy += q16_mul(f->Ki, f->e_int[1]);
                gz += q16_mul(f->Ki, f->e_int[2]);
            }

            /* Apply proportional feedback to gyro rates */
            if (f->Kp > 0) {
                gx += q16_mul(f->Kp, ex);
                gy += q16_mul(f->Kp, ey);
                gz += q16_mul(f->Kp, ez);
            }
        }
    }

    /* Integrate quaternion rate of change: q_dot = 0.5 * q * omega */
    q16_t half_dt = f->dt >> 1;

    q16_t dqw = q16_mul(-q16_mul(qx, gx) - q16_mul(qy, gy) - q16_mul(qz, gz), half_dt);
    q16_t dqx = q16_mul(q16_mul(qw, gx) + q16_mul(qy, gz) - q16_mul(qz, gy), half_dt);
    q16_t dqy = q16_mul(q16_mul(qw, gy) - q16_mul(qx, gz) + q16_mul(qz, gx), half_dt);
    q16_t dqz = q16_mul(q16_mul(qw, gz) + q16_mul(qx, gy) - q16_mul(qy, gx), half_dt);

    qw += dqw;
    qx += dqx;
    qy += dqy;
    qz += dqz;

    /* Fast single-cycle reciprocal square root quaternion normalization */
    int64_t q_sq_64 =
        ((int64_t)qw * qw) + ((int64_t)qx * qx) + ((int64_t)qy * qy) + ((int64_t)qz * qz);
    q16_t q_sq = (q16_t)(q_sq_64 >> Q16_SHIFT);
    q16_t inv_norm = q16_rsqrt(q_sq);
    if (inv_norm > 0) {
        qw = q16_mul(qw, inv_norm);
        qx = q16_mul(qx, inv_norm);
        qy = q16_mul(qy, inv_norm);
        qz = q16_mul(qz, inv_norm);
    }

    f->q.w = qw;
    f->q.x = qx;
    f->q.y = qy;
    f->q.z = qz;

    return SYN_OK;
}

SYN_Status syn_sensor_fusion_update_9dof(SYN_SensorFusion *f, q16_t gx, q16_t gy, q16_t gz,
                                         q16_t ax, q16_t ay, q16_t az, q16_t mx, q16_t my, q16_t mz)
{
    if (f == NULL)
        return SYN_INVALID_PARAM;

    /* Compute magnetometer magnitude squared */
    int64_t m_sq = ((int64_t)mx * mx) + ((int64_t)my * my) + ((int64_t)mz * mz);
    m_sq >>= Q16_SHIFT;

    if (m_sq == 0) {
        /* Fallback to 6-DOF update if magnetometer payload is zero/invalid */
        return syn_sensor_fusion_update(f, gx, gy, gz, ax, ay, az);
    }

    q16_t qw = f->q.w;
    q16_t qx = f->q.x;
    q16_t qy = f->q.y;
    q16_t qz = f->q.z;

    /* Compute accelerometer magnitude squared */
    int64_t a_sq = ((int64_t)ax * ax) + ((int64_t)ay * ay) + ((int64_t)az * az);
    a_sq >>= Q16_SHIFT;

    if (a_sq > 0) {
        q16_t a_norm = q16_sqrt((q16_t)a_sq);
        q16_t m_norm = q16_sqrt((q16_t)m_sq);

        if ((a_norm > 0) && (m_norm > 0)) {
            /* Normalize accelerometer and magnetometer vectors */
            ax = q16_div(ax, a_norm);
            ay = q16_div(ay, a_norm);
            az = q16_div(az, a_norm);

            mx = q16_div(mx, m_norm);
            my = q16_div(my, m_norm);
            mz = q16_div(mz, m_norm);

            /* Estimated direction of gravity vector from current quaternion state */
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
            int64_t bx_sq = ((int64_t)hx * hx) + ((int64_t)hy * hy);
            q16_t bx = q16_sqrt((q16_t)(bx_sq >> Q16_SHIFT));
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

            /* Total cross product error: e = (a x v) + (m x w) */
            q16_t a_vec[3] = {ax, ay, az};
            q16_t v_vec[3] = {vx, vy, vz};
            q16_t e_acc[3];
            syn_vec3_cross(a_vec, v_vec, e_acc);

            q16_t m_vec[3] = {mx, my, mz};
            q16_t w_vec[3] = {wx, wy, wz};
            q16_t e_mag[3];
            syn_vec3_cross(m_vec, w_vec, e_mag);

            q16_t ex = e_acc[0] + e_mag[0];
            q16_t ey = e_acc[1] + e_mag[1];
            q16_t ez = e_acc[2] + e_mag[2];

            /* Accumulate integral error */
            if (f->Ki > 0) {
                f->e_int[0] += q16_mul(ex, f->dt);
                f->e_int[1] += q16_mul(ey, f->dt);
                f->e_int[2] += q16_mul(ez, f->dt);
                gx += q16_mul(f->Ki, f->e_int[0]);
                gy += q16_mul(f->Ki, f->e_int[1]);
                gz += q16_mul(f->Ki, f->e_int[2]);
            }

            /* Apply proportional feedback to gyro rates */
            if (f->Kp > 0) {
                gx += q16_mul(f->Kp, ex);
                gy += q16_mul(f->Kp, ey);
                gz += q16_mul(f->Kp, ez);
            }
        }
    }

    /* Integrate quaternion rate of change: q_dot = 0.5 * q * omega */
    q16_t half_dt = f->dt >> 1;

    q16_t dqw = q16_mul(-q16_mul(qx, gx) - q16_mul(qy, gy) - q16_mul(qz, gz), half_dt);
    q16_t dqx = q16_mul(q16_mul(qw, gx) + q16_mul(qy, gz) - q16_mul(qz, gy), half_dt);
    q16_t dqy = q16_mul(q16_mul(qw, gy) - q16_mul(qx, gz) + q16_mul(qz, gx), half_dt);
    q16_t dqz = q16_mul(q16_mul(qw, gz) + q16_mul(qx, gy) - q16_mul(qy, gx), half_dt);

    qw += dqw;
    qx += dqx;
    qy += dqy;
    qz += dqz;

    /* Fast single-cycle reciprocal square root quaternion normalization */
    int64_t q_sq_64 =
        ((int64_t)qw * qw) + ((int64_t)qx * qx) + ((int64_t)qy * qy) + ((int64_t)qz * qz);
    q16_t q_sq = (q16_t)(q_sq_64 >> Q16_SHIFT);
    q16_t inv_norm = q16_rsqrt(q_sq);
    if (inv_norm > 0) {
        qw = q16_mul(qw, inv_norm);
        qx = q16_mul(qx, inv_norm);
        qy = q16_mul(qy, inv_norm);
        qz = q16_mul(qz, inv_norm);
    }

    f->q.w = qw;
    f->q.x = qx;
    f->q.y = qy;
    f->q.z = qz;

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
```


