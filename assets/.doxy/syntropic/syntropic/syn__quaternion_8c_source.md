

# File syn\_quaternion.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_quaternion.c**](syn__quaternion_8c.md)

[Go to the documentation of this file](syn__quaternion_8c.md)


```C++

#include "syn_quaternion.h"

#include "syn_assert.h"

void syn_quat_identity(SYN_Quaternion *q)
{
    SYN_ASSERT(q != NULL);
    q->w = Q16_ONE;
    q->x = 0;
    q->y = 0;
    q->z = 0;
}

void syn_quat_init(SYN_Quaternion *q, q16_t w, q16_t x, q16_t y, q16_t z)
{
    SYN_ASSERT(q != NULL);
    q->w = w;
    q->x = x;
    q->y = y;
    q->z = z;
}

void syn_quat_mul(const SYN_Quaternion *q1, const SYN_Quaternion *q2, SYN_Quaternion *out)
{
    SYN_ASSERT(q1 != NULL && q2 != NULL && out != NULL);
    if (q1 == NULL || q2 == NULL || out == NULL)
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */

    int64_t w1 = q1->w, x1 = q1->x, y1 = q1->y, z1 = q1->z;

    int64_t w2 = q2->w, x2 = q2->x, y2 = q2->y, z2 = q2->z;

    int64_t w = (w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2) >> Q16_SHIFT;
    int64_t x = (w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2) >> Q16_SHIFT;
    int64_t y = (w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2) >> Q16_SHIFT;
    int64_t z = (w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2) >> Q16_SHIFT;

    out->w = (q16_t)w;
    out->x = (q16_t)x;
    out->y = (q16_t)y;
    out->z = (q16_t)z;
}

q16_t syn_quat_norm(const SYN_Quaternion *q)
{
    SYN_ASSERT(q != NULL);
    int64_t sum =
        (int64_t)q->w * q->w + (int64_t)q->x * q->x + (int64_t)q->y * q->y + (int64_t)q->z * q->z;
    q16_t sum_q16 = (q16_t)(sum >> Q16_SHIFT);
    return q16_sqrt(sum_q16);
}

SYN_Status syn_quat_normalize(SYN_Quaternion *q)
{
    SYN_ASSERT(q != NULL);
    int64_t sum =
        (int64_t)q->w * q->w + (int64_t)q->x * q->x + (int64_t)q->y * q->y + (int64_t)q->z * q->z;
    q16_t norm_sq = (q16_t)(sum >> Q16_SHIFT);
    if (norm_sq == 0)
        return SYN_ERROR;

    q16_t inv_norm = q16_rsqrt(norm_sq);
    q->w = q16_mul(q->w, inv_norm);
    q->x = q16_mul(q->x, inv_norm);
    q->y = q16_mul(q->y, inv_norm);
    q->z = q16_mul(q->z, inv_norm);
    return SYN_OK;
}

void syn_quat_conjugate(const SYN_Quaternion *q, SYN_Quaternion *out)
{
    SYN_ASSERT(q != NULL && out != NULL);
    if (q == NULL || out == NULL)
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    out->w = q->w;

    out->x = -q->x;
    out->y = -q->y;
    out->z = -q->z;
}

SYN_Status syn_quat_inverse(const SYN_Quaternion *q, SYN_Quaternion *out)
{
    SYN_ASSERT(q != NULL && out != NULL);
    int64_t norm_sq =
        (int64_t)q->w * q->w + (int64_t)q->x * q->x + (int64_t)q->y * q->y + (int64_t)q->z * q->z;
    q16_t n2 = (q16_t)(norm_sq >> Q16_SHIFT);
    if (n2 == 0)
        return SYN_ERROR;

    out->w = q16_div(q->w, n2);
    out->x = q16_div(-q->x, n2);
    out->y = q16_div(-q->y, n2);
    out->z = q16_div(-q->z, n2);
    return SYN_OK;
}

void syn_quat_rotate_vec3(const SYN_Quaternion *q, const q16_t *v, q16_t *out)
{
    SYN_ASSERT(q != NULL && v != NULL && out != NULL);

    SYN_Quaternion v_quat;
    syn_quat_init(&v_quat, 0, v[0], v[1], v[2]);

    SYN_Quaternion q_conj, tmp;
    syn_quat_conjugate(q, &q_conj);

    syn_quat_mul(q, &v_quat, &tmp);
    syn_quat_mul(&tmp, &q_conj, &v_quat);

    out[0] = v_quat.x;
    out[1] = v_quat.y;
    out[2] = v_quat.z;
}

void syn_quat_to_mat3x3(const SYN_Quaternion *q, SYN_Matrix *out)
{
    SYN_ASSERT(q != NULL && out != NULL);
    SYN_ASSERT(out->rows == 3 && out->cols == 3);

    q16_t w = q->w, x = q->x, y = q->y, z = q->z;

    q16_t xx = (q16_t)(((int64_t)x * x) >> Q16_SHIFT);
    q16_t yy = (q16_t)(((int64_t)y * y) >> Q16_SHIFT);
    q16_t zz = (q16_t)(((int64_t)z * z) >> Q16_SHIFT);
    q16_t xy = (q16_t)(((int64_t)x * y) >> Q16_SHIFT);
    q16_t xz = (q16_t)(((int64_t)x * z) >> Q16_SHIFT);
    q16_t yz = (q16_t)(((int64_t)y * z) >> Q16_SHIFT);
    q16_t wx = (q16_t)(((int64_t)w * x) >> Q16_SHIFT);
    q16_t wy = (q16_t)(((int64_t)w * y) >> Q16_SHIFT);
    q16_t wz = (q16_t)(((int64_t)w * z) >> Q16_SHIFT);

    SYN_MAT_AT(out, 0, 0) = Q16_ONE - (yy + zz) * 2;
    SYN_MAT_AT(out, 0, 1) = (xy - wz) * 2;
    SYN_MAT_AT(out, 0, 2) = (xz + wy) * 2;

    SYN_MAT_AT(out, 1, 0) = (xy + wz) * 2;
    SYN_MAT_AT(out, 1, 1) = Q16_ONE - (xx + zz) * 2;
    SYN_MAT_AT(out, 1, 2) = (yz - wx) * 2;

    SYN_MAT_AT(out, 2, 0) = (xz - wy) * 2;
    SYN_MAT_AT(out, 2, 1) = (yz + wx) * 2;
    SYN_MAT_AT(out, 2, 2) = Q16_ONE - (xx + yy) * 2;
}

void syn_quat_from_euler(SYN_Quaternion *q, q16_t roll, q16_t pitch, q16_t yaw)
{
    SYN_ASSERT(q != NULL);

    q16_t cr = q16_cos(roll / 2);
    q16_t sr = q16_sin(roll / 2);
    q16_t cp = q16_cos(pitch / 2);
    q16_t sp = q16_sin(pitch / 2);
    q16_t cy = q16_cos(yaw / 2);
    q16_t sy = q16_sin(yaw / 2);

    int64_t w = ((int64_t)cr * cp >> Q16_SHIFT) * cy >> Q16_SHIFT;
    w += ((int64_t)sr * sp >> Q16_SHIFT) * sy >> Q16_SHIFT;

    int64_t x = ((int64_t)sr * cp >> Q16_SHIFT) * cy >> Q16_SHIFT;
    x -= ((int64_t)cr * sp >> Q16_SHIFT) * sy >> Q16_SHIFT;

    int64_t y = ((int64_t)cr * sp >> Q16_SHIFT) * cy >> Q16_SHIFT;
    y += ((int64_t)sr * cp >> Q16_SHIFT) * sy >> Q16_SHIFT;

    int64_t z = ((int64_t)cr * cp >> Q16_SHIFT) * sy >> Q16_SHIFT;
    z -= ((int64_t)sr * sp >> Q16_SHIFT) * cy >> Q16_SHIFT;

    q->w = (q16_t)w;
    q->x = (q16_t)x;
    q->y = (q16_t)y;
    q->z = (q16_t)z;
}

void syn_quat_to_euler(const SYN_Quaternion *q, q16_t *roll, q16_t *pitch, q16_t *yaw)
{
    SYN_ASSERT(q != NULL && roll != NULL && pitch != NULL && yaw != NULL);

    q16_t w = q->w, x = q->x, y = q->y, z = q->z;

    /* Roll (x-axis rotation) */
    int64_t sinr_cosp = 2 * (((int64_t)w * x + (int64_t)y * z) >> Q16_SHIFT);
    int64_t cosr_cosp = Q16_ONE - 2 * (((int64_t)x * x + (int64_t)y * y) >> Q16_SHIFT);
    *roll = q16_atan2((q16_t)sinr_cosp, (q16_t)cosr_cosp);

    /* Pitch (y-axis rotation) */
    int64_t sinp = 2 * (((int64_t)w * y - (int64_t)z * x) >> Q16_SHIFT);
    if (q16_abs((q16_t)sinp) >= Q16_ONE)
        *pitch = ((q16_t)sinp > 0) ? Q16_PI_2 : -Q16_PI_2;
    else
        *pitch = q16_asin((q16_t)sinp);

    /* Yaw (z-axis rotation) */
    int64_t siny_cosp = 2 * (((int64_t)w * z + (int64_t)x * y) >> Q16_SHIFT);
    int64_t cosy_cosp = Q16_ONE - 2 * (((int64_t)y * y + (int64_t)z * z) >> Q16_SHIFT);
    *yaw = q16_atan2((q16_t)siny_cosp, (q16_t)cosy_cosp);
}

void syn_quat_slerp(const SYN_Quaternion *q1, const SYN_Quaternion *q2, q16_t t,
                    SYN_Quaternion *out)
{
    SYN_ASSERT(q1 != NULL && q2 != NULL && out != NULL);

    int64_t dot = ((int64_t)q1->w * q2->w + (int64_t)q1->x * q2->x + (int64_t)q1->y * q2->y +
                   (int64_t)q1->z * q2->z) >>
                  Q16_SHIFT;

    SYN_Quaternion q2_norm = *q2;
    if (dot < 0) {
        dot = -dot;
        q2_norm.w = -q2_norm.w;
        q2_norm.x = -q2_norm.x;
        q2_norm.y = -q2_norm.y;
        q2_norm.z = -q2_norm.z;
    }

    if (dot > Q16_ONE - Q16_FROM_FRAC(1, 1000)) {
        /* Linear interpolation for small angles */
        out->w = q1->w + (q16_t)(((int64_t)(q2_norm.w - q1->w) * t) >> Q16_SHIFT);
        out->x = q1->x + (q16_t)(((int64_t)(q2_norm.x - q1->x) * t) >> Q16_SHIFT);
        out->y = q1->y + (q16_t)(((int64_t)(q2_norm.y - q1->y) * t) >> Q16_SHIFT);
        out->z = q1->z + (q16_t)(((int64_t)(q2_norm.z - q1->z) * t) >> Q16_SHIFT);
        (void)syn_quat_normalize(out);
        return;
    }

    q16_t theta = q16_acos((q16_t)dot);
    q16_t sin_theta = q16_sin(theta);

    q16_t ratio_a = q16_div(q16_sin(q16_mul(Q16_ONE - t, theta)), sin_theta);
    q16_t ratio_b = q16_div(q16_sin(q16_mul(t, theta)), sin_theta);

    out->w = (q16_t)(((int64_t)q1->w * ratio_a + (int64_t)q2_norm.w * ratio_b) >> Q16_SHIFT);
    out->x = (q16_t)(((int64_t)q1->x * ratio_a + (int64_t)q2_norm.x * ratio_b) >> Q16_SHIFT);
    out->y = (q16_t)(((int64_t)q1->y * ratio_a + (int64_t)q2_norm.y * ratio_b) >> Q16_SHIFT);
    out->z = (q16_t)(((int64_t)q1->z * ratio_a + (int64_t)q2_norm.z * ratio_b) >> Q16_SHIFT);
}
```


