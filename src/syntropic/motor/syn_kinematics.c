/**
 * @file syn_kinematics.c
 * @brief Multi-Axis Robot Forward & Inverse Kinematics Engine Implementation.
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_KINEMATICS) || SYN_USE_KINEMATICS

#include "../port/syn_port_system.h"
#include "syn_kinematics.h"

#include <string.h>

/* ── Internal Constants ─────────────────────────────────────────────────── */

#define Q16_SQRT3_2 56756            /**< sqrt(3)/2 ≈ 0.866025 in Q16 */
#define Q16_COS120 ((q16_t) - 32768) /**< cos(120 deg) in Q16 */
#define Q16_SIN120 56756             /**< sin(120 deg) in Q16 */
#define Q16_COS240 ((q16_t) - 32768) /**< cos(240 deg) in Q16 */
#define Q16_SIN240 ((q16_t) - 56756) /**< sin(240 deg) in Q16 */

/* ── Matrix Helper Functions ─────────────────────────────────────────────── */

/**
 * @brief Build 3x3 rotation matrix from Roll-Pitch-Yaw Euler angles (Z-Y-X order).
 * @param roll Roll angle (rad).
 * @param pitch Pitch angle (rad).
 * @param yaw Yaw angle (rad).
 * @param r33 Flat 9-element Q16 array for 3x3 rotation matrix.
 */
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

/**
 * @brief Extract Roll-Pitch-Yaw Euler angles from 3x3 rotation matrix.
 * @param r33 Flat 9-element Q16 array.
 * @param out_ori Output 3D orientation.
 */
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

/**
 * @brief Validate optional 4x4 homogeneous transform output matrix descriptor.
 * @param mat Optional matrix descriptor (NULL permitted).
 * @return true if NULL or valid 4x4 matrix with non-NULL backing storage.
 */
static bool kin_out_mat44_valid(const SYN_Matrix *mat)
{
    return (mat == NULL) || ((mat->data != NULL) && (mat->rows == 4U) && (mat->cols == 4U));
}

SYN_Status syn_kinematics_forward(const SYN_DH_Param *chain, size_t num_joints,
                                  const q16_t *joint_vals, SYN_DH_Convention conv,
                                  SYN_Matrix *out_t44, SYN_Pose6D *out_pose)
{
    if (chain == NULL || joint_vals == NULL || num_joints == 0U ||
        num_joints > SYN_KINEMATICS_MAX_JOINTS || !kin_out_mat44_valid(out_t44)) {
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

    if (out_t44 != NULL) {
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
        /* Standard DH actuates around frame i-1 (mats[i]); Modified DH applies Rx*Tx first so
         * joint i's axis and axis point reside in mats[i + 1] (Rz*Tz preserves the Z axis and
         * shifts the origin purely along Z, leaving z x (pe - pi) invariant). */
        const SYN_Matrix *joint_tf =
            (conv == SYN_DH_CONVENTION_MODIFIED) ? &mats[i + 1U] : &mats[i];
        q16_t pi_x = SYN_MAT_AT(joint_tf, 0, 3);
        q16_t pi_y = SYN_MAT_AT(joint_tf, 1, 3);
        q16_t pi_z = SYN_MAT_AT(joint_tf, 2, 3);

        q16_t zi_x = SYN_MAT_AT(joint_tf, 0, 2);
        q16_t zi_y = SYN_MAT_AT(joint_tf, 1, 2);
        q16_t zi_z = SYN_MAT_AT(joint_tf, 2, 2);

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

/* ── 64-Bit Fixed-Point Geometry Helpers ────────────────────────────────── */

/**
 * @brief Overflow-safe 2D Euclidean distance sqrt(x^2 + y^2) in Q16.16.
 * @param x X coordinate in Q16.16.
 * @param y Y coordinate in Q16.16.
 * @return Euclidean norm in Q16.16 (clamped to INT32_MAX).
 */
static q16_t hypot2d_q16(q16_t x, q16_t y)
{
    uint64_t sum = (uint64_t)((int64_t)x * (int64_t)x) + (uint64_t)((int64_t)y * (int64_t)y);
    uint64_t root = syn_isqrt64(sum);
    if (root > (uint64_t)INT32_MAX) {
        return INT32_MAX;
    }
    return (q16_t)root;
}

/**
 * @brief Solve 2-link planar inverse kinematics using 64-bit cosine law.
 * @param rx Target X in planar arm frame (Q16.16).
 * @param ry Target Y in planar arm frame (Q16.16).
 * @param l1 First link length (Q16.16, > 0).
 * @param l2 Second link length (Q16.16, > 0).
 * @param elbow Elbow configuration (up or down).
 * @param out_q1 Output first joint angle (rad, Q16.16).
 * @param out_q2 Output second joint relative angle (rad, Q16.16).
 * @return SYN_OK on success, SYN_ERROR if target is outside workspace.
 */
static SYN_Status solve_2link_planar(q16_t rx, q16_t ry, q16_t l1, q16_t l2, SYN_ArmElbow elbow,
                                     q16_t *out_q1, q16_t *out_q2)
{
    uint64_t r2_u64 = (uint64_t)((int64_t)rx * (int64_t)rx) + (uint64_t)((int64_t)ry * (int64_t)ry);
    int64_t r2 = (int64_t)(r2_u64 >> 16U);
    int64_t l1_sq = ((int64_t)l1 * (int64_t)l1) >> 16;
    int64_t l2_sq = ((int64_t)l2 * (int64_t)l2) >> 16;
    int64_t num = r2 - l1_sq - l2_sq;
    int64_t den = (2LL * (int64_t)l1 * (int64_t)l2) >> 16;

    if (den <= 0 || num > den || num < -den) {
        return SYN_ERROR;
    }

    int64_t scale_num = num;
    int64_t scale_den = den;
    while (scale_den > (1LL << 45)) {
        scale_num >>= 2;
        scale_den >>= 2;
    }

    q16_t cos_q2 = (q16_t)((scale_num * (int64_t)Q16_ONE) / scale_den);
    int64_t sin_sq = (int64_t)Q16_ONE - (((int64_t)cos_q2 * (int64_t)cos_q2) >> 16);
    q16_t sin_q2 = (q16_t)syn_isqrt64((uint64_t)sin_sq << 16U);
    if (elbow == SYN_ARM_ELBOW_DOWN) {
        sin_q2 = -sin_q2;
    }

    *out_q2 = q16_atan2(sin_q2, cos_q2);
    int64_t k1 = (int64_t)l1 + (((int64_t)l2 * (int64_t)cos_q2) >> 16);
    int64_t k2 = ((int64_t)l2 * (int64_t)sin_q2) >> 16;
    while ((k1 > (int64_t)INT32_MAX) || (k1 < -(int64_t)INT32_MAX)) {
        k1 >>= 1;
        k2 >>= 1;
    }
    *out_q1 = q16_sub_sat(q16_atan2(ry, rx), q16_atan2((q16_t)k2, (q16_t)k1));
    return SYN_OK;
}

/* ── 3-DOF Planar Arm ────────────────────────────────────────────────────── */

SYN_Status syn_kinematics_planar3_fk(const SYN_Kinematics_Planar3Config *cfg, q16_t q1, q16_t q2,
                                     q16_t q3, q16_t *out_x, q16_t *out_y, q16_t *out_phi)
{
    if (cfg == NULL) {
        return SYN_INVALID_PARAM;
    }

    q16_t q12 = q16_add_sat(q1, q2);
    q16_t q123 = q16_add_sat(q12, q3);

    q16_t x =
        q16_add_sat(q16_add_sat(q16_mul(cfg->l1, q16_cos(q1)), q16_mul(cfg->l2, q16_cos(q12))),
                    q16_mul(cfg->l3, q16_cos(q123)));
    q16_t y =
        q16_add_sat(q16_add_sat(q16_mul(cfg->l1, q16_sin(q1)), q16_mul(cfg->l2, q16_sin(q12))),
                    q16_mul(cfg->l3, q16_sin(q123)));

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
    if (cfg == NULL || cfg->l1 <= 0 || cfg->l2 <= 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t xw = q16_sub_sat(x, q16_mul(cfg->l3, q16_cos(phi)));
    q16_t yw = q16_sub_sat(y, q16_mul(cfg->l3, q16_sin(phi)));

    q16_t q1 = 0;
    q16_t q2 = 0;
    if (solve_2link_planar(xw, yw, cfg->l1, cfg->l2, elbow, &q1, &q2) != SYN_OK) {
        return SYN_ERROR;
    }
    q16_t q3 = q16_sub_sat(q16_sub_sat(phi, q1), q2);

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

    q16_t q12 = q16_add_sat(q1, q2);
    out_pose->position.x =
        q16_add_sat(q16_mul(cfg->l1, q16_cos(q1)), q16_mul(cfg->l2, q16_cos(q12)));
    out_pose->position.y =
        q16_add_sat(q16_mul(cfg->l1, q16_sin(q1)), q16_mul(cfg->l2, q16_sin(q12)));
    out_pose->position.z = q16_sub_sat(cfg->z_home, d3);

    out_pose->orientation.roll = 0;
    out_pose->orientation.pitch = 0;
    out_pose->orientation.yaw = q16_add_sat(q12, q4);
    return SYN_OK;
}

SYN_Status syn_kinematics_scara_ik(const SYN_Kinematics_SCARAConfig *cfg, const SYN_Pose6D *target,
                                   SYN_ArmElbow elbow, q16_t *out_q1, q16_t *out_q2, q16_t *out_d3,
                                   q16_t *out_q4)
{
    if (cfg == NULL || target == NULL || cfg->l1 <= 0 || cfg->l2 <= 0) {
        return SYN_INVALID_PARAM;
    }

    int64_t d3_64 = (int64_t)cfg->z_home - (int64_t)target->position.z;
    if ((d3_64 < (int64_t)INT32_MIN) || (d3_64 > (int64_t)INT32_MAX)) {
        return SYN_ERROR;
    }
    q16_t d3 = (q16_t)d3_64;
    if (cfg->d_max > 0 && (d3 < 0 || d3 > cfg->d_max)) {
        return SYN_ERROR;
    }

    q16_t q1 = 0;
    q16_t q2 = 0;
    if (solve_2link_planar(target->position.x, target->position.y, cfg->l1, cfg->l2, elbow, &q1,
                           &q2) != SYN_OK) {
        return SYN_ERROR;
    }

    q16_t q4 = q16_sub_sat(q16_sub_sat(target->orientation.yaw, q1), q2);

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

/**
 * @brief Extract Z-Y-Z spherical wrist angles (q4, q5, q6) from R_36 = R_03^T * R_06.
 * @param r03 3x3 rotation matrix from base to link 3.
 * @param r06 3x3 target end-effector rotation matrix.
 * @param wrist_flip Invert wrist pitch sign if true.
 * @param out_q Output joint array (writes indices 3, 4, 5).
 */
static void solve_spherical_wrist(const q16_t r03[9], const q16_t r06[9], bool wrist_flip,
                                  q16_t out_q[6])
{
    q16_t r36[9];
    for (uint8_t r = 0U; r < 3U; r++) {
        for (uint8_t c = 0U; c < 3U; c++) {
            r36[r * 3U + c] = q16_mul(r03[0U * 3U + r], r06[0U * 3U + c]) +
                              q16_mul(r03[1U * 3U + r], r06[1U * 3U + c]) +
                              q16_mul(r03[2U * 3U + r], r06[2U * 3U + c]);
        }
    }

    q16_t r36_22 = r36[8];
    q16_t sin_q5 = hypot2d_q16(r36[2], r36[5]);
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
    } else if (r36_22 >= 0) {
        q4 = 0;
        q6 = q16_atan2(-r36[1], r36[0]);
    } else {
        q4 = 0;
        q6 = q16_atan2(r36[1], -r36[0]);
    }

    out_q[3] = q4;
    out_q[4] = q5;
    out_q[5] = q6;
}

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
    if (cfg == NULL || target == NULL || out_q == NULL || cfg->a2 <= 0 || cfg->d4 <= 0) {
        return SYN_INVALID_PARAM;
    }

    q16_t r06[9];
    rpy_to_rot_matrix(target->orientation.roll, target->orientation.pitch, target->orientation.yaw,
                      r06);

    /* Wrist center: Pw = P_06 - d6 * z6 */
    q16_t pw_x = target->position.x - q16_mul(cfg->d6, r06[2]);
    q16_t pw_y = target->position.y - q16_mul(cfg->d6, r06[5]);
    q16_t pw_z = target->position.z - q16_mul(cfg->d6, r06[8]);

    q16_t q1 = q16_atan2(pw_y, pw_x);
    q16_t r_xy = hypot2d_q16(pw_x, pw_y) - cfg->a1;
    q16_t s_z = pw_z - cfg->d1;

    /* Forearm d4 lies along z3 (rotated -90 deg from x2 when q3 = 0): phi3 = q3 - pi/2 */
    q16_t q2 = 0;
    q16_t phi3 = 0;
    if (solve_2link_planar(r_xy, s_z, cfg->a2, cfg->d4, elbow, &q2, &phi3) != SYN_OK) {
        return SYN_ERROR;
    }
    q16_t q3 = phi3 + Q16_PI_2;
    if (q3 > Q16_PI) {
        q3 -= Q16_2_PI;
    }

    /* R_03 for Standard DH alpha = [+pi/2, 0, +pi/2] */
    q16_t q23 = q2 + q3;
    q16_t c1 = q16_cos(q1);
    q16_t s1 = q16_sin(q1);
    q16_t c23 = q16_cos(q23);
    q16_t s23 = q16_sin(q23);

    q16_t r03[9];
    r03[0] = q16_mul(c1, c23);
    r03[1] = s1;
    r03[2] = q16_mul(c1, s23);
    r03[3] = q16_mul(s1, c23);
    r03[4] = -c1;
    r03[5] = q16_mul(s1, s23);
    r03[6] = s23;
    r03[7] = 0;
    r03[8] = -c23;

    out_q[0] = q1;
    out_q[1] = q2;
    out_q[2] = q3;
    solve_spherical_wrist(r03, r06, wrist_flip, out_q);
    return SYN_OK;
}

/* ── 3-Axis Delta Parallel Robot ─────────────────────────────────────────── */

/**
 * @brief Helper to solve single Delta arm angle using 64-bit arithmetic.
 * @param x0 Projected X coordinate.
 * @param y0 Projected Y coordinate.
 * @param z0 Projected Z coordinate.
 * @param L Upper arm length.
 * @param l Lower arm parallelogram length.
 * @param out_theta Pointer to receive solved arm angle.
 * @return SYN_OK on success, SYN_ERROR if target is unreachable.
 */
static SYN_Status delta_calc_arm(q16_t x0, q16_t y0, q16_t z0, q16_t L, q16_t l, q16_t *out_theta)
{
    int64_t E =
        (((int64_t)x0 * (int64_t)x0) + ((int64_t)y0 * (int64_t)y0) + ((int64_t)z0 * (int64_t)z0) +
         ((int64_t)L * (int64_t)L) - ((int64_t)l * (int64_t)l)) >>
        16;
    int64_t two_L_y0 = (2LL * (int64_t)L * (int64_t)y0) >> 16;
    int64_t A = E - two_L_y0;
    int64_t B = (4LL * (int64_t)L * (int64_t)z0) >> 16;
    int64_t C = E + two_L_y0;

    int64_t max_a = (A < 0) ? -A : A;
    int64_t max_b = (B < 0) ? -B : B;
    int64_t max_c = (C < 0) ? -C : C;
    int64_t max_abc = (max_a > max_b) ? max_a : max_b;
    if (max_c > max_abc) {
        max_abc = max_c;
    }
    while (max_abc > (1LL << 29)) {
        A >>= 1;
        B >>= 1;
        C >>= 1;
        max_abc >>= 1;
    }

    int64_t disc_64 = (B * B) - (4LL * A * C);
    if (disc_64 < 0) {
        return SYN_ERROR;
    }

    if (A == 0) {
        if (B == 0) {
            return SYN_ERROR;
        }
        int64_t num_lin = -C;
        int64_t den_lin = B;
        if (den_lin < 0) {
            num_lin = -num_lin;
            den_lin = -den_lin;
        }
        *out_theta = q16_mul(Q16_FROM_INT(2), q16_atan2((q16_t)num_lin, (q16_t)den_lin));
        return SYN_OK;
    }

    int64_t sqrt_disc = (int64_t)syn_isqrt64((uint64_t)disc_64);
    int64_t num = -B - sqrt_disc;
    int64_t den = 2LL * A;
    if (den < 0) {
        num = -num;
        den = -den;
    }
    *out_theta = q16_mul(Q16_FROM_INT(2), q16_atan2((q16_t)num, (q16_t)den));
    return SYN_OK;
}

SYN_Status syn_kinematics_delta_ik(const SYN_Kinematics_DeltaConfig *cfg,
                                   const SYN_Position3D *target, q16_t *out_theta1,
                                   q16_t *out_theta2, q16_t *out_theta3)
{
    if (cfg == NULL || target == NULL || cfg->upper_arm_len <= 0 || cfg->lower_arm_len <= 0) {
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

/**
 * @brief Check whether three 64-bit Q48.16 values fit within 32-bit signed Q16.16 range.
 * @param u First coordinate in Q48.16.
 * @param v Second coordinate in Q48.16.
 * @param w Third coordinate in Q48.16.
 * @return true if all three values lie within [INT32_MIN, INT32_MAX].
 */
static bool delta_q16_vec3_ok(int64_t u, int64_t v, int64_t w)
{
    return (u >= (int64_t)INT32_MIN) && (u <= (int64_t)INT32_MAX) && (v >= (int64_t)INT32_MIN) &&
           (v <= (int64_t)INT32_MAX) && (w >= (int64_t)INT32_MIN) && (w <= (int64_t)INT32_MAX);
}

/**
 * @brief Check whether radical-line coefficients fit inside 64-bit quadratic bounds.
 * @param c1 First coefficient.
 * @param c2 Second coefficient.
 * @return true if both lie in [-(2^29), +(2^29)].
 */
static bool delta_slope_ok(int64_t c1, int64_t c2)
{
    return (c1 >= -(1LL << 29)) && (c1 <= (1LL << 29)) && (c2 >= -(1LL << 29)) &&
           (c2 <= (1LL << 29));
}

/**
 * @brief Prescale (dw21, dw31, dd_b) together so 3-term sphere products fit in int64_t.
 * @param dw21 In/out w2 - w1 difference.
 * @param dw31 In/out w3 - w1 difference.
 * @param dd_b In/out denominator copy.
 */
static void delta_prescale_dw(int64_t *dw21, int64_t *dw31, int64_t *dd_b)
{
    for (;;) {
        int64_t abs21 = (*dw21 < 0) ? -(*dw21) : *dw21;
        int64_t abs31 = (*dw31 < 0) ? -(*dw31) : *dw31;
        int64_t max_dw = (abs21 > abs31) ? abs21 : abs31;
        if (max_dw <= (1LL << 28)) {
            break;
        }
        *dw21 >>= 1;
        *dw31 >>= 1;
        *dd_b >>= 1;
    }
}

/**
 * @brief Solve Delta FK quadratic a*z^2 + b*z + c = 0 (lower root) without 64-bit overflow.
 * @param a     Quadratic coefficient in Q48.16 (> 0).
 * @param b     Linear coefficient in Q48.16.
 * @param c     Constant coefficient in Q48.16.
 * @param out_z Output lower Z intersection root in Q48.16.
 * @return SYN_OK on real intersection, SYN_ERROR if discriminant < 0.
 */
static SYN_Status delta_fk_solve_z(int64_t a, int64_t b, int64_t c, int64_t *out_z)
{
    /* Substitute z = (2^k) * u -> a*u^2 + (b >> k)*u + (c >> 2k) = 0 so a keeps full precision */
    int shift_k = 0;
    int64_t b_s = b;
    int64_t c_s = c;
    int64_t c_limit = (1LL << 58) / a;

    for (;;) {
        int64_t abs_b = (b_s < 0) ? -b_s : b_s;
        int64_t abs_c = (c_s < 0) ? -c_s : c_s;
        if ((abs_b <= (1LL << 29)) && (abs_c <= c_limit)) {
            break;
        }
        b_s >>= 1;
        c_s >>= 2;
        shift_k++;
    }

    int64_t disc_q32 = (b_s * b_s) - (4LL * a * c_s);
    if (disc_q32 < 0) {
        return SYN_ERROR;
    }

    int64_t sqrt_disc = (int64_t)syn_isqrt64((uint64_t)disc_q32);
    int64_t u_q16 = ((-b_s - sqrt_disc) * (int64_t)Q16_ONE) / (2LL * a);
    *out_z = u_q16 * (int64_t)(1ULL << (uint32_t)shift_k);
    return SYN_OK;
}

SYN_Status syn_kinematics_delta_fk(const SYN_Kinematics_DeltaConfig *cfg, q16_t theta1,
                                   q16_t theta2, q16_t theta3, SYN_Position3D *out_pos)
{
    if (cfg == NULL || out_pos == NULL || cfg->upper_arm_len <= 0 || cfg->lower_arm_len <= 0) {
        return SYN_INVALID_PARAM;
    }

    int64_t wb = (int64_t)cfg->base_radius - (int64_t)cfg->effector_radius;
    int64_t L = (int64_t)cfg->upper_arm_len;
    int64_t l = (int64_t)cfg->lower_arm_len;

    /* 3 elbow joint positions J1, J2, J3 widened to 64-bit before validation */
    int64_t y1 = -(wb + ((L * (int64_t)q16_cos(theta1)) >> 16));
    int64_t z1 = -((L * (int64_t)q16_sin(theta1)) >> 16);

    int64_t y2_p = -(wb + ((L * (int64_t)q16_cos(theta2)) >> 16));
    int64_t z2 = -((L * (int64_t)q16_sin(theta2)) >> 16);
    int64_t y3_p = -(wb + ((L * (int64_t)q16_cos(theta3)) >> 16));
    int64_t z3 = -((L * (int64_t)q16_sin(theta3)) >> 16);
    if (!delta_q16_vec3_ok(y1, y2_p, y3_p)) {
        return SYN_ERROR;
    }

    int64_t x2 = (y2_p * -(int64_t)Q16_SIN120) >> 16;
    int64_t y2 = (y2_p * (int64_t)Q16_COS120) >> 16;
    int64_t x3 = (y3_p * -(int64_t)Q16_SIN240) >> 16;
    int64_t y3 = (y3_p * (int64_t)Q16_COS240) >> 16;

    /* 3-Sphere intersection in 64-bit Q48.16 */
    int64_t d_denom = (((y2 - y1) * x3) >> 16) - (((y3 - y1) * x2) >> 16);
    if (d_denom == 0) {
        return SYN_ERROR;
    }

    int64_t w1 = ((y1 * y1) >> 16) + ((z1 * z1) >> 16);
    int64_t w2 = ((x2 * x2) >> 16) + ((y2 * y2) >> 16) + ((z2 * z2) >> 16);
    int64_t w3 = ((x3 * x3) >> 16) + ((y3 * y3) >> 16) + ((z3 * z3) >> 16);

    int64_t na1 = (((z2 - z1) >> 1) * ((y3 - y1) >> 1)) - (((z3 - z1) >> 1) * ((y2 - y1) >> 1));
    int64_t a1 = ((na1 <= (INT64_MAX / 4LL)) && (na1 >= (INT64_MIN / 4LL)))
                     ? ((na1 * 4LL) / d_denom)
                     : ((na1 / d_denom) * 4LL);
    int64_t na2 = -((((z2 - z1) >> 1) * (x3 >> 1)) - (((z3 - z1) >> 1) * (x2 >> 1)));
    int64_t a2_coeff = ((na2 <= (INT64_MAX / 4LL)) && (na2 >= (INT64_MIN / 4LL)))
                           ? ((na2 * 4LL) / d_denom)
                           : ((na2 / d_denom) * 4LL);

    int64_t dw21 = w2 - w1;
    int64_t dw31 = w3 - w1;
    int64_t dd_b = d_denom;
    delta_prescale_dw(&dw21, &dw31, &dd_b);

    int64_t b1 = (dd_b != 0) ? (-((dw21 * (y3 - y1)) - (dw31 * (y2 - y1))) / (2LL * dd_b)) : 0;
    int64_t b2_coeff = (dd_b != 0) ? (((dw21 * x3) - (dw31 * x2)) / (2LL * dd_b)) : 0;
    if ((dd_b == 0) || !delta_slope_ok(a1, a2_coeff) || !delta_slope_ok(b1 >> 1, b2_coeff >> 1)) {
        return SYN_ERROR;
    }

    int64_t a = ((a1 * a1) >> 16) + ((a2_coeff * a2_coeff) >> 16) + (int64_t)Q16_ONE;
    int64_t dy1 = b2_coeff - y1;
    int64_t b = 2LL * (((a1 * b1) >> 16) + ((a2_coeff * dy1) >> 16) - z1);
    int64_t c =
        ((b1 * b1) >> 16) + (((dy1 >> 1) * (dy1 >> 1)) >> 14) + ((z1 * z1) >> 16) - ((l * l) >> 16);

    int64_t z = 0;
    if (delta_fk_solve_z(a, b, c, &z) != SYN_OK) {
        return SYN_ERROR;
    }
    int64_t z_clamped = (z < (int64_t)INT32_MIN)
                            ? (int64_t)INT32_MIN
                            : ((z > (int64_t)INT32_MAX) ? (int64_t)INT32_MAX : z);
    int64_t x = ((a1 * z_clamped) >> 16) + b1;
    int64_t y = ((a2_coeff * z_clamped) >> 16) + b2_coeff;
    if (!delta_q16_vec3_ok(x, y, z)) {
        return SYN_ERROR;
    }

    out_pos->x = (q16_t)x;
    out_pos->y = (q16_t)y;
    out_pos->z = (q16_t)z;
    return SYN_OK;
}

#endif /* !defined(SYN_USE_KINEMATICS) || SYN_USE_KINEMATICS */
