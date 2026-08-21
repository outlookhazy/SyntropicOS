

# File syn\_kinematics.c

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_kinematics.c**](syn__kinematics_8c.md)

[Go to the documentation of this file](syn__kinematics_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_KINEMATICS) || SYN_USE_KINEMATICS

#include "../port/syn_port_system.h"
#include "syn_kinematics.h"

#include <string.h>

/* ── Internal Constants ─────────────────────────────────────────────────── */

#define Q16_SQRT3_2 56756            
#define Q16_COS120 ((q16_t) - 32768) 
#define Q16_SIN120 56756             
#define Q16_COS240 ((q16_t) - 32768) 
#define Q16_SIN240 ((q16_t) - 56756) 
/* ── Matrix Helper Functions ─────────────────────────────────────────────── */

static void rpy_to_rot_matrix(q16_t roll, q16_t pitch, q16_t yaw, q16_t r33[9])
{
    q16_t cr = q16_cos(roll);
    q16_t sr = q16_sin(roll);
    q16_t cp = q16_cos(pitch);
    q16_t sp = q16_sin(pitch);
    q16_t cy = q16_cos(yaw);
    q16_t sy = q16_sin(yaw);

    /* R = Rz(yaw) * Ry(pitch) * Rx(roll) */
    r33[0] = q16_mul(cy, cp);
    r33[1] = q16_mul(cy, q16_mul(sp, sr)) - q16_mul(sy, cr);
    r33[2] = q16_mul(cy, q16_mul(sp, cr)) + q16_mul(sy, sr);

    r33[3] = q16_mul(sy, cp);
    r33[4] = q16_mul(sy, q16_mul(sp, sr)) + q16_mul(cy, cr);
    r33[5] = q16_mul(sy, q16_mul(sp, cr)) - q16_mul(cy, sr);

    r33[6] = -sp;
    r33[7] = q16_mul(cp, sr);
    r33[8] = q16_mul(cp, cr);
}

static void rot_matrix_to_rpy(const q16_t r33[9], SYN_Orientation3D *out_ori)
{
    q16_t r20 = r33[6];

    if (q16_abs(r20) < Q16_ONE - Q16_FROM_FRAC(1, 1000)) {
        out_ori->pitch = q16_asin(-r20);
        out_ori->roll = q16_atan2(r33[7], r33[8]);
        out_ori->yaw = q16_atan2(r33[3], r33[0]);
    } else {
        /* Gimbal lock at pitch = +-90 deg */
        out_ori->roll = 0;
        out_ori->pitch = (r20 < 0) ? Q16_PI_2 : -Q16_PI_2;
        out_ori->yaw = q16_atan2(-r33[1], r33[4]);
    }
}

/* ── Denavit-Hartenberg Implementation ──────────────────────────────────── */

SYN_Status syn_dh_matrix(const SYN_DH_Param *param, q16_t joint_val, SYN_DH_Convention conv,
                         SYN_Matrix *out_t44)
{
    if (param == NULL || out_t44 == NULL || out_t44->data == NULL || out_t44->rows != 4U ||
        out_t44->cols != 4U ||
        (conv != SYN_DH_CONVENTION_STANDARD && conv != SYN_DH_CONVENTION_MODIFIED)) {
        return SYN_INVALID_PARAM;
    }

    q16_t theta = param->is_prismatic ? param->theta : (param->theta + joint_val);
    q16_t d = param->is_prismatic ? (param->d + joint_val) : param->d;
    q16_t ct = q16_cos(theta);
    q16_t st = q16_sin(theta);
    q16_t ca = q16_cos(param->alpha);
    q16_t sa = q16_sin(param->alpha);

    (void)memset(out_t44->data, 0, 16U * sizeof(q16_t));

    if (conv == SYN_DH_CONVENTION_STANDARD) {
        /* Standard DH: Rz(theta) * Tz(d) * Tx(a) * Rx(alpha) */
        SYN_MAT_AT(out_t44, 0, 0) = ct;
        SYN_MAT_AT(out_t44, 0, 1) = -q16_mul(st, ca);
        SYN_MAT_AT(out_t44, 0, 2) = q16_mul(st, sa);
        SYN_MAT_AT(out_t44, 0, 3) = q16_mul(param->a, ct);

        SYN_MAT_AT(out_t44, 1, 0) = st;
        SYN_MAT_AT(out_t44, 1, 1) = q16_mul(ct, ca);
        SYN_MAT_AT(out_t44, 1, 2) = -q16_mul(ct, sa);
        SYN_MAT_AT(out_t44, 1, 3) = q16_mul(param->a, st);

        SYN_MAT_AT(out_t44, 2, 0) = 0;
        SYN_MAT_AT(out_t44, 2, 1) = sa;
        SYN_MAT_AT(out_t44, 2, 2) = ca;
        SYN_MAT_AT(out_t44, 2, 3) = d;

        SYN_MAT_AT(out_t44, 3, 3) = Q16_ONE;
    } else {
        /* Modified DH (Craig): Rx(alpha_{i-1}) * Tx(a_{i-1}) * Rz(theta_i) * Tz(d_i) */
        SYN_MAT_AT(out_t44, 0, 0) = ct;
        SYN_MAT_AT(out_t44, 0, 1) = -st;
        SYN_MAT_AT(out_t44, 0, 2) = 0;
        SYN_MAT_AT(out_t44, 0, 3) = param->a;

        SYN_MAT_AT(out_t44, 1, 0) = q16_mul(st, ca);
        SYN_MAT_AT(out_t44, 1, 1) = q16_mul(ct, ca);
        SYN_MAT_AT(out_t44, 1, 2) = -sa;
        SYN_MAT_AT(out_t44, 1, 3) = -q16_mul(d, sa);

        SYN_MAT_AT(out_t44, 2, 0) = q16_mul(st, sa);
        SYN_MAT_AT(out_t44, 2, 1) = q16_mul(ct, sa);
        SYN_MAT_AT(out_t44, 2, 2) = ca;
        SYN_MAT_AT(out_t44, 2, 3) = q16_mul(d, ca);

        SYN_MAT_AT(out_t44, 3, 3) = Q16_ONE;
    }

    return SYN_OK;
}

SYN_Status syn_kinematics_forward(const SYN_DH_Param *chain, size_t num_joints,
                                  const q16_t *joint_vals, SYN_DH_Convention conv,
                                  SYN_Matrix *out_t44, SYN_Pose6D *out_pose)
{
    if (chain == NULL || joint_vals == NULL || num_joints == 0U ||
        num_joints > SYN_KINEMATICS_MAX_JOINTS) {
        return SYN_INVALID_PARAM;
    }

    SYN_MAT_DECL(T_acc, 4, 4);
    SYN_MAT_DECL(T_link, 4, 4);
    SYN_MAT_DECL(T_next, 4, 4);

    syn_matrix_identity(&T_acc);

    for (size_t i = 0U; i < num_joints; i++) {
        if (syn_dh_matrix(&chain[i], joint_vals[i], conv, &T_link) != SYN_OK) {
            return SYN_ERROR;
        }
        syn_matrix_mul(&T_acc, &T_link, &T_next);
        (void)memcpy(T_acc.data, T_next.data, 16U * sizeof(q16_t));
    }

    if (out_t44 != NULL && out_t44->data != NULL) {
        (void)memcpy(out_t44->data, T_acc.data, 16U * sizeof(q16_t));
    }

    if (out_pose != NULL) {
        out_pose->position.x = SYN_MAT_AT(&T_acc, 0, 3);
        out_pose->position.y = SYN_MAT_AT(&T_acc, 1, 3);
        out_pose->position.z = SYN_MAT_AT(&T_acc, 2, 3);

        q16_t r33[9];
        r33[0] = SYN_MAT_AT(&T_acc, 0, 0);
        r33[1] = SYN_MAT_AT(&T_acc, 0, 1);
        r33[2] = SYN_MAT_AT(&T_acc, 0, 2);
        r33[3] = SYN_MAT_AT(&T_acc, 1, 0);
        r33[4] = SYN_MAT_AT(&T_acc, 1, 1);
        r33[5] = SYN_MAT_AT(&T_acc, 1, 2);
        r33[6] = SYN_MAT_AT(&T_acc, 2, 0);
        r33[7] = SYN_MAT_AT(&T_acc, 2, 1);
        r33[8] = SYN_MAT_AT(&T_acc, 2, 2);
        rot_matrix_to_rpy(r33, &out_pose->orientation);
    }

    return SYN_OK;
}

SYN_Status syn_kinematics_jacobian(const SYN_DH_Param *chain, size_t num_joints,
                                   const q16_t *joint_vals, SYN_DH_Convention conv,
                                   SYN_Matrix *out_j6xn)
{
    if (chain == NULL || joint_vals == NULL || out_j6xn == NULL || out_j6xn->data == NULL ||
        num_joints == 0U || num_joints > SYN_KINEMATICS_MAX_JOINTS || out_j6xn->rows != 6U ||
        out_j6xn->cols != (uint8_t)num_joints) {
        return SYN_INVALID_PARAM;
    }

    /* Compute cumulative transforms T_0^i for each link */
    q16_t t_chain[SYN_KINEMATICS_MAX_JOINTS + 1U][16];
    SYN_Matrix mats[SYN_KINEMATICS_MAX_JOINTS + 1U];

    for (size_t i = 0U; i <= num_joints; i++) {
        mats[i].data = t_chain[i];
        mats[i].rows = 4U;
        mats[i].cols = 4U;
    }
    syn_matrix_identity(&mats[0]);

    SYN_MAT_DECL(T_link, 4, 4);
    for (size_t i = 0U; i < num_joints; i++) {
        if (syn_dh_matrix(&chain[i], joint_vals[i], conv, &T_link) != SYN_OK) {
            return SYN_ERROR;
        }
        syn_matrix_mul(&mats[i], &T_link, &mats[i + 1U]);
    }

    /* End-effector position p_e */
    q16_t pe_x = SYN_MAT_AT(&mats[num_joints], 0, 3);
    q16_t pe_y = SYN_MAT_AT(&mats[num_joints], 1, 3);
    q16_t pe_z = SYN_MAT_AT(&mats[num_joints], 2, 3);

    for (size_t i = 0U; i < num_joints; i++) {
        /* Joint i-1 position p_{i-1} and z-axis z_{i-1} */
        q16_t pi_x = SYN_MAT_AT(&mats[i], 0, 3);
        q16_t pi_y = SYN_MAT_AT(&mats[i], 1, 3);
        q16_t pi_z = SYN_MAT_AT(&mats[i], 2, 3);

        q16_t zi_x = SYN_MAT_AT(&mats[i], 0, 2);
        q16_t zi_y = SYN_MAT_AT(&mats[i], 1, 2);
        q16_t zi_z = SYN_MAT_AT(&mats[i], 2, 2);

        if (chain[i].is_prismatic) {
            /* Prismatic joint: Jv = z_{i-1}, Jw = 0 */
            SYN_MAT_AT(out_j6xn, 0, i) = zi_x;
            SYN_MAT_AT(out_j6xn, 1, i) = zi_y;
            SYN_MAT_AT(out_j6xn, 2, i) = zi_z;
            SYN_MAT_AT(out_j6xn, 3, i) = 0;
            SYN_MAT_AT(out_j6xn, 4, i) = 0;
            SYN_MAT_AT(out_j6xn, 5, i) = 0;
        } else {
            /* Revolute joint: Jv = z_{i-1} x (pe - pi), Jw = z_{i-1} */
            q16_t r_x = pe_x - pi_x;
            q16_t r_y = pe_y - pi_y;
            q16_t r_z = pe_z - pi_z;

            /* Cross product z x r */
            SYN_MAT_AT(out_j6xn, 0, i) = q16_mul(zi_y, r_z) - q16_mul(zi_z, r_y);
            SYN_MAT_AT(out_j6xn, 1, i) = q16_mul(zi_z, r_x) - q16_mul(zi_x, r_z);
            SYN_MAT_AT(out_j6xn, 2, i) = q16_mul(zi_x, r_y) - q16_mul(zi_y, r_x);

            SYN_MAT_AT(out_j6xn, 3, i) = zi_x;
            SYN_MAT_AT(out_j6xn, 4, i) = zi_y;
            SYN_MAT_AT(out_j6xn, 5, i) = zi_z;
        }
    }

    return SYN_OK;
}

/* ── 3-DOF Planar Arm ────────────────────────────────────────────────────── */

SYN_Status syn_kinematics_planar3_fk(const SYN_Kinematics_Planar3Config *cfg, q16_t q1, q16_t q2,
                                     q16_t q3, q16_t *out_x, q16_t *out_y, q16_t *out_phi)
{
    if (cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    q16_t q12 = q1 + q2;
    q16_t q123 = q12 + q3;

    q16_t x = q16_mul(cfg->l1, q16_cos(q1)) + q16_mul(cfg->l2, q16_cos(q12)) +
              q16_mul(cfg->l3, q16_cos(q123));
    q16_t y = q16_mul(cfg->l1, q16_sin(q1)) + q16_mul(cfg->l2, q16_sin(q12)) +
              q16_mul(cfg->l3, q16_sin(q123));

    if (out_x != NULL) {
        *out_x = x;
    }
    if (out_y != NULL) {
        *out_y = y;
    }
    if (out_phi != NULL) {
        *out_phi = q123;
    }
    return SYN_OK;
}

SYN_Status syn_kinematics_planar3_ik(const SYN_Kinematics_Planar3Config *cfg, q16_t x, q16_t y,
                                     q16_t phi, SYN_ArmElbow elbow, q16_t *out_q1, q16_t *out_q2,
                                     q16_t *out_q3)
{
    if (cfg == NULL || cfg->l1 == 0 || cfg->l2 == 0) {
        return SYN_INVALID_PARAM;
    }

    /* Target wrist position */
    q16_t xw = x - q16_mul(cfg->l3, q16_cos(phi));
    q16_t yw = y - q16_mul(cfg->l3, q16_sin(phi));

    q16_t r2 = q16_mul(xw, xw) + q16_mul(yw, yw);
    q16_t l1_sq = q16_mul(cfg->l1, cfg->l1);
    q16_t l2_sq = q16_mul(cfg->l2, cfg->l2);

    /* Cosine rule: cos(q2) = (r^2 - l1^2 - l2^2) / (2 * l1 * l2) */
    q16_t num = r2 - l1_sq - l2_sq;
    q16_t den = q16_mul(Q16_FROM_INT(2), q16_mul(cfg->l1, cfg->l2));
    q16_t cos_q2 = q16_div(num, den);

    if (cos_q2 > Q16_ONE || cos_q2 < -Q16_ONE) {
        return SYN_ERROR; /* Unreachable position */
    }

    q16_t sin_sq = Q16_ONE - q16_mul(cos_q2, cos_q2);
    q16_t sin_q2 = q16_sqrt(sin_sq);
    if (elbow == SYN_ARM_ELBOW_DOWN) {
        sin_q2 = -sin_q2;
    }

    q16_t q2 = q16_atan2(sin_q2, cos_q2);

    /* q1 = atan2(yw, xw) - atan2(l2 * sin(q2), l1 + l2 * cos(q2)) */
    q16_t k1 = cfg->l1 + q16_mul(cfg->l2, cos_q2);
    q16_t k2 = q16_mul(cfg->l2, sin_q2);
    q16_t q1 = q16_atan2(yw, xw) - q16_atan2(k2, k1);
    q16_t q3 = phi - q1 - q2;

    if (out_q1 != NULL) {
        *out_q1 = q1;
    }
    if (out_q2 != NULL) {
        *out_q2 = q2;
    }
    if (out_q3 != NULL) {
        *out_q3 = q3;
    }
    return SYN_OK;
}

/* ── 4-DOF SCARA ─────────────────────────────────────────────────────────── */

SYN_Status syn_kinematics_scara_fk(const SYN_Kinematics_SCARAConfig *cfg, q16_t q1, q16_t q2,
                                   q16_t d3, q16_t q4, SYN_Pose6D *out_pose)
{
    if (cfg == NULL || out_pose == NULL) {
        return SYN_INVALID_PARAM;
    }

    q16_t q12 = q1 + q2;
    out_pose->position.x = q16_mul(cfg->l1, q16_cos(q1)) + q16_mul(cfg->l2, q16_cos(q12));
    out_pose->position.y = q16_mul(cfg->l1, q16_sin(q1)) + q16_mul(cfg->l2, q16_sin(q12));
    out_pose->position.z = cfg->z_home - d3;

    out_pose->orientation.roll = 0;
    out_pose->orientation.pitch = 0;
    out_pose->orientation.yaw = q12 + q4;
    return SYN_OK;
}

SYN_Status syn_kinematics_scara_ik(const SYN_Kinematics_SCARAConfig *cfg, const SYN_Pose6D *target,
                                   SYN_ArmElbow elbow, q16_t *out_q1, q16_t *out_q2, q16_t *out_d3,
                                   q16_t *out_q4)
{
    if (cfg == NULL || target == NULL || cfg->l1 == 0 || cfg->l2 == 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t x = target->position.x;
    q16_t y = target->position.y;
    q16_t z = target->position.z;
    q16_t yaw = target->orientation.yaw;

    q16_t r2 = q16_mul(x, x) + q16_mul(y, y);
    q16_t l1_sq = q16_mul(cfg->l1, cfg->l1);
    q16_t l2_sq = q16_mul(cfg->l2, cfg->l2);

    q16_t num = r2 - l1_sq - l2_sq;
    q16_t den = q16_mul(Q16_FROM_INT(2), q16_mul(cfg->l1, cfg->l2));
    q16_t cos_q2 = q16_div(num, den);

    if (cos_q2 > Q16_ONE || cos_q2 < -Q16_ONE) {
        return SYN_ERROR;
    }

    q16_t sin_sq = Q16_ONE - q16_mul(cos_q2, cos_q2);
    q16_t sin_q2 = q16_sqrt(sin_sq);
    if (elbow == SYN_ARM_ELBOW_DOWN) {
        sin_q2 = -sin_q2;
    }

    q16_t q2 = q16_atan2(sin_q2, cos_q2);
    q16_t k1 = cfg->l1 + q16_mul(cfg->l2, cos_q2);
    q16_t k2 = q16_mul(cfg->l2, sin_q2);
    q16_t q1 = q16_atan2(y, x) - q16_atan2(k2, k1);

    q16_t d3 = cfg->z_home - z;
    if (cfg->d_max > 0 && (d3 < 0 || d3 > cfg->d_max)) {
        return SYN_ERROR;
    }

    q16_t q4 = yaw - q1 - q2;

    if (out_q1 != NULL) {
        *out_q1 = q1;
    }
    if (out_q2 != NULL) {
        *out_q2 = q2;
    }
    if (out_d3 != NULL) {
        *out_d3 = d3;
    }
    if (out_q4 != NULL) {
        *out_q4 = q4;
    }
    return SYN_OK;
}

/* ── 6-DOF Articulated Arm (Spherical Wrist) ─────────────────────────────── */

SYN_Status syn_kinematics_6dof_fk(const SYN_Kinematics_6DOFConfig *cfg, const q16_t q[6],
                                  SYN_Pose6D *out_pose)
{
    if (cfg == NULL || q == NULL || out_pose == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_DH_Param dh[6] = {
        {.a = cfg->a1, .alpha = Q16_PI_2, .d = cfg->d1, .theta = 0, .is_prismatic = false},
        {.a = cfg->a2, .alpha = 0, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = 0, .alpha = Q16_PI_2, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = 0, .alpha = -Q16_PI_2, .d = cfg->d4, .theta = 0, .is_prismatic = false},
        {.a = 0, .alpha = Q16_PI_2, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = 0, .alpha = 0, .d = cfg->d6, .theta = 0, .is_prismatic = false}};

    return syn_kinematics_forward(dh, 6U, q, SYN_DH_CONVENTION_STANDARD, NULL, out_pose);
}

SYN_Status syn_kinematics_6dof_ik(const SYN_Kinematics_6DOFConfig *cfg, const SYN_Pose6D *target,
                                  SYN_ArmElbow elbow, bool wrist_flip, q16_t out_q[6])
{
    if (cfg == NULL || target == NULL || out_q == NULL || cfg->a2 == 0 || cfg->d4 == 0) {
        return SYN_INVALID_PARAM;
    }

    /* 1. Calculate target rotation matrix R_06 */
    q16_t r06[9];
    rpy_to_rot_matrix(target->orientation.roll, target->orientation.pitch, target->orientation.yaw,
                      r06);

    /* 2. Wrist center position: Pw = P - d6 * z6 */
    q16_t z6_x = r06[2];
    q16_t z6_y = r06[5];
    q16_t z6_z = r06[8];

    q16_t pw_x = target->position.x - q16_mul(cfg->d6, z6_x);
    q16_t pw_y = target->position.y - q16_mul(cfg->d6, z6_y);
    q16_t pw_z = target->position.z - q16_mul(cfg->d6, z6_z);

    /* 3. Base angle q1 */
    q16_t q1 = q16_atan2(pw_y, pw_x);

    /* 4. Planar projection for shoulder q2 & elbow q3 */
    q16_t r_xy = q16_hypot(pw_x, pw_y) - cfg->a1;
    q16_t s_z = pw_z - cfg->d1;

    q16_t d_sq = q16_mul(r_xy, r_xy) + q16_mul(s_z, s_z);
    q16_t a2_sq = q16_mul(cfg->a2, cfg->a2);
    q16_t d4_sq = q16_mul(cfg->d4, cfg->d4);

    q16_t cos_q3 =
        q16_div(d_sq - a2_sq - d4_sq, q16_mul(Q16_FROM_INT(2), q16_mul(cfg->a2, cfg->d4)));
    if (cos_q3 > Q16_ONE || cos_q3 < -Q16_ONE) {
        return SYN_ERROR;
    }

    q16_t sin_sq3 = Q16_ONE - q16_mul(cos_q3, cos_q3);
    q16_t sin_q3 = q16_sqrt(sin_sq3);
    if (elbow == SYN_ARM_ELBOW_DOWN) {
        sin_q3 = -sin_q3;
    }
    q16_t q3 = q16_atan2(sin_q3, cos_q3);

    q16_t beta = q16_atan2(s_z, r_xy);
    q16_t gamma = q16_atan2(q16_mul(cfg->d4, sin_q3), cfg->a2 + q16_mul(cfg->d4, cos_q3));
    q16_t q2 = beta - gamma;

    /* 5. Compute R_03 and R_36 = R_03^T * R_06 */
    q16_t q23 = q2 + q3;
    q16_t c1 = q16_cos(q1);
    q16_t s1 = q16_sin(q1);
    q16_t c23 = q16_cos(q23);
    q16_t s23 = q16_sin(q23);

    /* R_03 columns */
    q16_t r03[9];
    r03[0] = q16_mul(c1, c23);
    r03[1] = -q16_mul(c1, s23);
    r03[2] = s1;
    r03[3] = q16_mul(s1, c23);
    r03[4] = -q16_mul(s1, s23);
    r03[5] = -c1;
    r03[6] = s23;
    r03[7] = c23;
    r03[8] = 0;

    /* R_36 = R_03^T * R_06 */
    q16_t r36[9];
    for (uint8_t r = 0; r < 3; r++) {
        for (uint8_t c = 0; c < 3; c++) {
            r36[r * 3 + c] = q16_mul(r03[0 * 3 + r], r06[0 * 3 + c]) +
                             q16_mul(r03[1 * 3 + r], r06[1 * 3 + c]) +
                             q16_mul(r03[2 * 3 + r], r06[2 * 3 + c]);
        }
    }

    /* 6. Extract Euler Z-Y-Z angles for wrist joints q4, q5, q6 */
    q16_t r36_22 = r36[8];
    q16_t sin_q5 = q16_hypot(r36[2], r36[5]);
    if (wrist_flip) {
        sin_q5 = -sin_q5;
    }
    q16_t q5 = q16_atan2(sin_q5, r36_22);

    q16_t q4 = 0;
    q16_t q6 = 0;
    if (q16_abs(sin_q5) > Q16_FROM_FRAC(5, 1000)) {
        if (!wrist_flip) {
            q4 = q16_atan2(r36[5], r36[2]);
            q6 = q16_atan2(r36[7], -r36[6]);
        } else {
            q4 = q16_atan2(-r36[5], -r36[2]);
            q6 = q16_atan2(-r36[7], r36[6]);
        }
    } else {
        /* Wrist singular configuration */
        q4 = 0;
        q6 = q16_atan2(-r36[1], r36[0]);
    }

    out_q[0] = q1;
    out_q[1] = q2;
    out_q[2] = q3;
    out_q[3] = q4;
    out_q[4] = q5;
    out_q[5] = q6;
    return SYN_OK;
}

/* ── 3-Axis Delta Parallel Robot ─────────────────────────────────────────── */

static SYN_Status delta_calc_arm(q16_t x0, q16_t y0, q16_t z0, q16_t L, q16_t l, q16_t *out_theta)
{
    q16_t E = q16_mul(x0, x0) + q16_mul(y0, y0) + q16_mul(z0, z0) + q16_mul(L, L) - q16_mul(l, l);
    q16_t two_L_y0 = q16_mul(Q16_FROM_INT(2), q16_mul(L, y0));
    q16_t A = E - two_L_y0;
    q16_t B = q16_mul(Q16_FROM_INT(4), q16_mul(L, z0));
    q16_t C = E + two_L_y0;

    int64_t b_sq = ((int64_t)B * (int64_t)B) >> 16;
    int64_t ac = ((int64_t)A * (int64_t)C) >> 16;
    int64_t four_ac = ac * 4;
    int64_t disc_64 = b_sq - four_ac;
    if (disc_64 < 0 || disc_64 > INT32_MAX) {
        return SYN_ERROR;
    }

    q16_t disc = (q16_t)disc_64;

    if (A == 0) {
        if (B == 0) {
            return SYN_ERROR;
        }
        q16_t t = -q16_div(C, B);
        *out_theta = q16_mul(Q16_FROM_INT(2), q16_atan2(t, Q16_ONE));
        return SYN_OK;
    }

    q16_t sqrt_disc = q16_sqrt(disc);
    q16_t t = q16_div(-B - sqrt_disc, q16_mul(Q16_FROM_INT(2), A));
    *out_theta = q16_mul(Q16_FROM_INT(2), q16_atan2(t, Q16_ONE));
    return SYN_OK;
}

SYN_Status syn_kinematics_delta_ik(const SYN_Kinematics_DeltaConfig *cfg,
                                   const SYN_Position3D *target, q16_t *out_theta1,
                                   q16_t *out_theta2, q16_t *out_theta3)
{
    if (cfg == NULL || target == NULL || cfg->upper_arm_len == 0 || cfg->lower_arm_len == 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t x0 = target->x;
    q16_t y0 = target->y;
    q16_t z0 = target->z;

    q16_t wb = cfg->base_radius - cfg->effector_radius;

    /* Arm 1 (at angle 0 deg) */
    q16_t t1 = 0;
    if (delta_calc_arm(x0, y0 + wb, z0, cfg->upper_arm_len, cfg->lower_arm_len, &t1) != SYN_OK) {
        return SYN_ERROR;
    }

    /* Arm 2 (rotated by 120 deg) */
    q16_t x2 = q16_mul(x0, Q16_COS120) + q16_mul(y0, Q16_SIN120);
    q16_t y2 = -q16_mul(x0, Q16_SIN120) + q16_mul(y0, Q16_COS120);
    q16_t t2 = 0;
    if (delta_calc_arm(x2, y2 + wb, z0, cfg->upper_arm_len, cfg->lower_arm_len, &t2) != SYN_OK) {
        return SYN_ERROR;
    }

    /* Arm 3 (rotated by 240 deg) */
    q16_t x3 = q16_mul(x0, Q16_COS240) + q16_mul(y0, Q16_SIN240);
    q16_t y3 = -q16_mul(x0, Q16_SIN240) + q16_mul(y0, Q16_COS240);
    q16_t t3 = 0;
    if (delta_calc_arm(x3, y3 + wb, z0, cfg->upper_arm_len, cfg->lower_arm_len, &t3) != SYN_OK) {
        return SYN_ERROR;
    }

    if (out_theta1 != NULL) {
        *out_theta1 = t1;
    }
    if (out_theta2 != NULL) {
        *out_theta2 = t2;
    }
    if (out_theta3 != NULL) {
        *out_theta3 = t3;
    }
    return SYN_OK;
}

SYN_Status syn_kinematics_delta_fk(const SYN_Kinematics_DeltaConfig *cfg, q16_t theta1,
                                   q16_t theta2, q16_t theta3, SYN_Position3D *out_pos)
{
    if (cfg == NULL || out_pos == NULL || cfg->upper_arm_len == 0 || cfg->lower_arm_len == 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t wb = cfg->base_radius - cfg->effector_radius;
    q16_t L = cfg->upper_arm_len;
    q16_t l = cfg->lower_arm_len;

    /* 3 elbow joint positions J1, J2, J3 */
    q16_t y1 = -(wb + q16_mul(L, q16_cos(theta1)));
    q16_t z1 = -q16_mul(L, q16_sin(theta1));

    q16_t y2_p = -(wb + q16_mul(L, q16_cos(theta2)));
    q16_t z2 = -q16_mul(L, q16_sin(theta2));
    q16_t x2 = q16_mul(y2_p, -Q16_SIN120);
    q16_t y2 = q16_mul(y2_p, Q16_COS120);

    q16_t y3_p = -(wb + q16_mul(L, q16_cos(theta3)));
    q16_t z3 = -q16_mul(L, q16_sin(theta3));
    q16_t x3 = q16_mul(y3_p, -Q16_SIN240);
    q16_t y3 = q16_mul(y3_p, Q16_COS240);

    /* 3-Sphere intersection: (x - xi)^2 + (y - yi)^2 + (z - zi)^2 = l^2 */
    q16_t d_denom = q16_mul(y2 - y1, x3) - q16_mul(y3 - y1, x2);
    if (d_denom == 0) {
        return SYN_ERROR;
    }

    q16_t w1 = q16_mul(y1, y1) + q16_mul(z1, z1);
    q16_t w2 = q16_mul(x2, x2) + q16_mul(y2, y2) + q16_mul(z2, z2);
    q16_t w3 = q16_mul(x3, x3) + q16_mul(y3, y3) + q16_mul(z3, z3);

    q16_t a1 = q16_div(q16_mul(z2 - z1, y3 - y1) - q16_mul(z3 - z1, y2 - y1), d_denom);
    q16_t b1 = -q16_div(q16_mul(w2 - w1, y3 - y1) - q16_mul(w3 - w1, y2 - y1),
                        q16_mul(Q16_FROM_INT(2), d_denom));

    q16_t a2_coeff = -q16_div(q16_mul(z2 - z1, x3) - q16_mul(z3 - z1, x2), d_denom);
    q16_t b2_coeff =
        q16_div(q16_mul(w2 - w1, x3) - q16_mul(w3 - w1, x2), q16_mul(Q16_FROM_INT(2), d_denom));

    q16_t a = q16_mul(a1, a1) + q16_mul(a2_coeff, a2_coeff) + Q16_ONE;
    q16_t b = q16_mul(Q16_FROM_INT(2), (q16_mul(a1, b1) + q16_mul(a2_coeff, b2_coeff - y1) - z1));
    q16_t c =
        q16_mul(b1, b1) + q16_mul(b2_coeff - y1, b2_coeff - y1) + q16_mul(z1, z1) - q16_mul(l, l);

    q16_t disc = q16_mul(b, b) - q16_mul(Q16_FROM_INT(4), q16_mul(a, c));
    if (disc < 0) {
        return SYN_ERROR;
    }

    q16_t z = q16_div(-b - q16_sqrt(disc), q16_mul(Q16_FROM_INT(2), a));
    q16_t x = q16_mul(a1, z) + b1;
    q16_t y = q16_mul(a2_coeff, z) + b2_coeff;

    out_pos->x = x;
    out_pos->y = y;
    out_pos->z = z;
    return SYN_OK;
}

#endif /* !defined(SYN_USE_KINEMATICS) || SYN_USE_KINEMATICS */
```


