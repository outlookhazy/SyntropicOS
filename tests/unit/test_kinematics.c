/**
 * @file test_kinematics.c
 * @brief Unit tests for Multi-Axis Robot Forward & Inverse Kinematics Engine (syn_kinematics).
 */

#include "syntropic/motor/syn_kinematics.h"
#include "syntropic/util/syn_matrix.h"
#include "syntropic/util/syn_qmath.h"
#include "unity/unity.h"

#include <string.h>

void test_kinematics_dh_matrix_and_forward(void)
{
    /* 1. Parameter Validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dh_matrix(NULL, 0, SYN_DH_CONVENTION_STANDARD, NULL));

    SYN_MAT_DECL(T, 4, 4);
    SYN_DH_Param p = {
        .a = Q16_FROM_INT(10), .alpha = 0, .d = Q16_FROM_INT(5), .theta = 0, .is_prismatic = false};

    /* Wrong matrix dimensions */
    SYN_MAT_DECL(T_bad, 3, 3);
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_dh_matrix(&p, 0, SYN_DH_CONVENTION_STANDARD, &T_bad));

    /* Standard DH Matrix Calculation */
    TEST_ASSERT_EQUAL(SYN_OK, syn_dh_matrix(&p, Q16_PI_2, SYN_DH_CONVENTION_STANDARD, &T));
    TEST_ASSERT_INT_WITHIN(200, 0, SYN_MAT_AT(&T, 0, 0));
    TEST_ASSERT_INT_WITHIN(200, Q16_ONE, SYN_MAT_AT(&T, 1, 0));
    TEST_ASSERT_INT_WITHIN(200, 0, SYN_MAT_AT(&T, 0, 3));
    TEST_ASSERT_INT_WITHIN(200, Q16_FROM_INT(10), SYN_MAT_AT(&T, 1, 3));
    TEST_ASSERT_INT_WITHIN(200, Q16_FROM_INT(5), SYN_MAT_AT(&T, 2, 3));

    /* Modified DH Matrix Calculation */
    p.alpha = Q16_PI_2;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dh_matrix(&p, 0, SYN_DH_CONVENTION_MODIFIED, &T));

    /* Prismatic link */
    p.is_prismatic = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_dh_matrix(&p, Q16_FROM_INT(2), SYN_DH_CONVENTION_STANDARD, &T));
    TEST_ASSERT_EQUAL(SYN_OK, syn_dh_matrix(&p, Q16_FROM_INT(2), SYN_DH_CONVENTION_MODIFIED, &T));

    /* Forward Kinematics Chain Validation */
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_kinematics_forward(NULL, 0, NULL, SYN_DH_CONVENTION_STANDARD, NULL, NULL));
    q16_t jvals[2] = {0, 0};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_kinematics_forward(&p, 9U, jvals, SYN_DH_CONVENTION_STANDARD, NULL, NULL));

    /* 2-link revolute chain (Standard DH) */
    SYN_DH_Param chain2[2] = {
        {.a = Q16_FROM_INT(10), .alpha = 0, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = Q16_FROM_INT(10), .alpha = 0, .d = 0, .theta = 0, .is_prismatic = false}};
    q16_t q[2] = {0, 0};
    SYN_Pose6D pose;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_forward(chain2, 2U, q, SYN_DH_CONVENTION_STANDARD, &T, &pose));
    TEST_ASSERT_INT_WITHIN(500, Q16_FROM_INT(20), pose.position.x);
    TEST_ASSERT_INT_WITHIN(500, 0, pose.position.y);
    TEST_ASSERT_INT_WITHIN(500, 0, pose.position.z);

    /* 2-link revolute chain (Modified DH) */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_forward(chain2, 2U, q, SYN_DH_CONVENTION_MODIFIED, &T, &pose));

    /* Forward with invalid convention */
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_kinematics_forward(chain2, 2U, q, (SYN_DH_Convention)99, &T, &pose));

    /* Forward with NULL outputs */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_kinematics_forward(chain2, 2U, q, SYN_DH_CONVENTION_STANDARD, NULL, NULL));

    /* Gimbal lock orientation test in FK (pitch = +90 deg, r20 = -1) */
    SYN_DH_Param gimbal_chain[2] = {
        {.a = 0, .alpha = -Q16_PI_2, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = 0, .alpha = 0, .d = 0, .theta = Q16_PI_2, .is_prismatic = false}};
    q16_t q_gimbal[2] = {0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_forward(gimbal_chain, 2U, q_gimbal,
                                                     SYN_DH_CONVENTION_STANDARD, NULL, &pose));

    /* Gimbal lock orientation test in FK (pitch = -90 deg, r20 = 1) */
    SYN_DH_Param gimbal_chain_neg[2] = {
        {.a = 0, .alpha = Q16_PI_2, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = 0, .alpha = 0, .d = 0, .theta = -Q16_PI_2, .is_prismatic = false}};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_forward(gimbal_chain_neg, 2U, q_gimbal,
                                                     SYN_DH_CONVENTION_STANDARD, NULL, &pose));
}

void test_kinematics_jacobian(void)
{
    /* Parameter validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kinematics_jacobian(NULL, 0, NULL, SYN_DH_CONVENTION_STANDARD, NULL));

    SYN_DH_Param chain[2] = {
        {.a = Q16_FROM_INT(10), .alpha = 0, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = Q16_FROM_INT(10), .alpha = 0, .d = 0, .theta = 0, .is_prismatic = true}};
    q16_t q[2] = {0, 0};

    SYN_MAT_DECL(J, 6, 2);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_jacobian(chain, 2U, q, SYN_DH_CONVENTION_STANDARD, &J));

    /* Jacobian with invalid convention */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_jacobian(chain, 2U, q, (SYN_DH_Convention)99, &J));

    /* First joint is revolute along Z: linear velocity vy = 20 */
    TEST_ASSERT_INT_WITHIN(500, Q16_FROM_INT(20), SYN_MAT_AT(&J, 1, 0));
    TEST_ASSERT_INT_WITHIN(500, Q16_ONE, SYN_MAT_AT(&J, 5, 0));

    /* Second joint is prismatic along Z */
    TEST_ASSERT_INT_WITHIN(500, Q16_ONE, SYN_MAT_AT(&J, 2, 1));
    TEST_ASSERT_INT_WITHIN(500, 0, SYN_MAT_AT(&J, 5, 1));

    /* Modified DH Jacobian */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_jacobian(chain, 2U, q, SYN_DH_CONVENTION_MODIFIED, &J));
}

void test_kinematics_planar3(void)
{
    SYN_Kinematics_Planar3Config cfg = {
        .l1 = Q16_FROM_INT(10), .l2 = Q16_FROM_INT(8), .l3 = Q16_FROM_INT(4)};

    /* Validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kinematics_planar3_fk(NULL, 0, 0, 0, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kinematics_planar3_ik(NULL, 0, 0, 0, SYN_ARM_ELBOW_UP, NULL, NULL, NULL));

    /* FK Test */
    q16_t x = 0, y = 0, phi = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_planar3_fk(&cfg, 0, 0, 0, &x, &y, &phi));
    TEST_ASSERT_INT_WITHIN(1000, Q16_FROM_INT(22), x);
    TEST_ASSERT_INT_WITHIN(1000, 0, y);
    TEST_ASSERT_INT_WITHIN(1000, 0, phi);

    /* IK Roundtrip Test */
    q16_t q1 = 0, q2 = 0, q3 = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_planar3_ik(&cfg, x, y, phi, SYN_ARM_ELBOW_UP, &q1, &q2, &q3));
    TEST_ASSERT_INT_WITHIN(3000, 0, q1);
    TEST_ASSERT_INT_WITHIN(3000, 0, q2);
    TEST_ASSERT_INT_WITHIN(3000, 0, q3);

    /* FK and IK with NULL outputs */
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_planar3_fk(&cfg, 0, 0, 0, NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_kinematics_planar3_ik(&cfg, x, y, phi, SYN_ARM_ELBOW_UP, NULL, NULL, NULL));

    /* Elbow Down Test */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_planar3_ik(&cfg, Q16_FROM_INT(12), Q16_FROM_INT(10), Q16_PI_2,
                                                SYN_ARM_ELBOW_DOWN, &q1, &q2, &q3));

    /* Unreachable Target (Too Far) */
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_planar3_ik(&cfg, Q16_FROM_INT(50), Q16_FROM_INT(50),
                                                           0, SYN_ARM_ELBOW_UP, &q1, &q2, &q3));

    /* Inner reach limit error (|l1 - l2| > reach) */
    SYN_Kinematics_Planar3Config bad_cfg = {.l1 = Q16_FROM_INT(20), .l2 = Q16_FROM_INT(5), .l3 = 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_planar3_ik(&bad_cfg, Q16_FROM_INT(2), 0, 0,
                                                           SYN_ARM_ELBOW_UP, &q1, &q2, &q3));
}

void test_kinematics_scara(void)
{
    SYN_Kinematics_SCARAConfig cfg = {.l1 = Q16_FROM_INT(10),
                                      .l2 = Q16_FROM_INT(10),
                                      .d_max = Q16_FROM_INT(15),
                                      .z_home = Q16_FROM_INT(20)};

    /* Validation */
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kinematics_scara_fk(NULL, 0, 0, 0, 0, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kinematics_scara_ik(NULL, NULL, SYN_ARM_ELBOW_UP, NULL,
                                                                 NULL, NULL, NULL));

    /* FK Test */
    SYN_Pose6D pose;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_scara_fk(&cfg, 0, 0, Q16_FROM_INT(5), 0, &pose));
    TEST_ASSERT_INT_WITHIN(1000, Q16_FROM_INT(20), pose.position.x);
    TEST_ASSERT_INT_WITHIN(1000, 0, pose.position.y);
    TEST_ASSERT_INT_WITHIN(1000, Q16_FROM_INT(15), pose.position.z);

    /* IK Roundtrip Test */
    q16_t q1 = 0, q2 = 0, d3 = 0, q4 = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_scara_ik(&cfg, &pose, SYN_ARM_ELBOW_UP, &q1, &q2, &d3, &q4));
    TEST_ASSERT_INT_WITHIN(3000, 0, q1);
    TEST_ASSERT_INT_WITHIN(3000, 0, q2);
    TEST_ASSERT_INT_WITHIN(1000, Q16_FROM_INT(5), d3);
    TEST_ASSERT_INT_WITHIN(3000, 0, q4);

    /* IK with NULL outputs */
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_kinematics_scara_ik(&cfg, &pose, SYN_ARM_ELBOW_UP, NULL, NULL, NULL, NULL));

    /* Elbow Down */
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_scara_ik(&cfg, &pose, SYN_ARM_ELBOW_DOWN, &q1, &q2, &d3, &q4));

    /* Stroke limit exceeded */
    SYN_Pose6D bad_z = pose;
    bad_z.position.z = -Q16_FROM_INT(10);
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_kinematics_scara_ik(&cfg, &bad_z, SYN_ARM_ELBOW_UP, &q1, &q2, &d3, &q4));

    /* Out of reach radius */
    SYN_Pose6D bad_reach = pose;
    bad_reach.position.x = Q16_FROM_INT(30);
    TEST_ASSERT_EQUAL(
        SYN_ERROR, syn_kinematics_scara_ik(&cfg, &bad_reach, SYN_ARM_ELBOW_UP, &q1, &q2, &d3, &q4));

    /* Inner reach limit (|l1 - l2| > reach) */
    SYN_Kinematics_SCARAConfig bad_arm_cfg = {
        .l1 = Q16_FROM_INT(20), .l2 = Q16_FROM_INT(5), .d_max = Q16_FROM_INT(10), .z_home = 0};
    SYN_Pose6D inner_target = {.position = {.x = Q16_FROM_INT(2), .y = 0, .z = 0}};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_scara_ik(&bad_arm_cfg, &inner_target,
                                                         SYN_ARM_ELBOW_UP, &q1, &q2, &d3, &q4));
}

void test_kinematics_6dof_and_delta(void)
{
    /* 6-DOF PUMA Arm */
    SYN_Kinematics_6DOFConfig cfg6 = {.d1 = Q16_FROM_INT(5),
                                      .a1 = Q16_FROM_INT(2),
                                      .a2 = Q16_FROM_INT(10),
                                      .d4 = Q16_FROM_INT(10),
                                      .d6 = Q16_FROM_INT(3)};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kinematics_6dof_fk(NULL, NULL, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM,
                      syn_kinematics_6dof_ik(NULL, NULL, SYN_ARM_ELBOW_UP, false, NULL));

    /* Singular wrist pose (q5 = 0, q2 + q3 = 0) */
    q16_t q_sing[6] = {0, -Q16_PI_2 / 2, Q16_PI_2 / 2, 0, 0, 0};
    SYN_Pose6D pose_sing;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_6dof_fk(&cfg6, q_sing, &pose_sing));

    q16_t out_q[6];
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_6dof_ik(&cfg6, &pose_sing, SYN_ARM_ELBOW_UP, false, out_q));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_6dof_ik(&cfg6, &pose_sing, SYN_ARM_ELBOW_DOWN, true, out_q));

    /* Exact wrist singularity test where R_36 = I */
    SYN_Pose6D pose_pure_sing = {
        .position = {.x = Q16_FROM_INT(22), .y = -Q16_FROM_INT(3), .z = Q16_FROM_INT(5)},
        .orientation = {.roll = Q16_PI_2, .pitch = 0, .yaw = 0}};
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_kinematics_6dof_ik(&cfg6, &pose_pure_sing, SYN_ARM_ELBOW_UP, false, out_q));

    /* Non-singular wrist pose (q5 != 0) with wrist flip false and true */
    q16_t q_nonsing[6] = {0, Q16_PI_2 / 4, -Q16_PI_2 / 4, Q16_PI_2 / 4, Q16_PI_2 / 4, Q16_PI_2 / 4};
    SYN_Pose6D pose_nonsing;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_6dof_fk(&cfg6, q_nonsing, &pose_nonsing));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_6dof_ik(&cfg6, &pose_nonsing, SYN_ARM_ELBOW_UP, false, out_q));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_6dof_ik(&cfg6, &pose_nonsing, SYN_ARM_ELBOW_UP, true, out_q));

    /* Unreachable target */
    SYN_Pose6D bad_pose = pose_sing;
    bad_pose.position.x = Q16_FROM_INT(100);
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_kinematics_6dof_ik(&cfg6, &bad_pose, SYN_ARM_ELBOW_UP, false, out_q));

    /* Delta Robot */
    SYN_Kinematics_DeltaConfig dcfg = {.base_radius = Q16_FROM_INT(15),
                                       .effector_radius = Q16_FROM_INT(5),
                                       .upper_arm_len = Q16_FROM_INT(10),
                                       .lower_arm_len = Q16_FROM_INT(20)};

    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kinematics_delta_fk(NULL, 0, 0, 0, NULL));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_kinematics_delta_ik(NULL, NULL, NULL, NULL, NULL));

    /* Delta FK at home position (theta1=0, theta2=0, theta3=0) */
    SYN_Position3D pos;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_fk(&dcfg, 0, 0, 0, &pos));
    TEST_ASSERT_INT_WITHIN(500, 0, pos.x);
    TEST_ASSERT_INT_WITHIN(500, 0, pos.y);

    /* Delta IK */
    q16_t t1 = 0, t2 = 0, t3 = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_ik(&dcfg, &pos, &t1, &t2, &t3));
    TEST_ASSERT_INT_WITHIN(2000, 0, t1);
    TEST_ASSERT_INT_WITHIN(2000, 0, t2);
    TEST_ASSERT_INT_WITHIN(2000, 0, t3);

    /* Delta IK with NULL outputs */
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_ik(&dcfg, &pos, NULL, NULL, NULL));

    /* Unreachable Delta position */
    SYN_Position3D out_of_bounds = {.x = Q16_FROM_INT(50), .y = 0, .z = Q16_FROM_INT(50)};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_ik(&dcfg, &out_of_bounds, &t1, &t2, &t3));

    /* Delta IK arm calculation where A == 0, B != 0 */
    SYN_Kinematics_DeltaConfig dcfg_a0 = {.base_radius = 0,
                                          .effector_radius = 0,
                                          .upper_arm_len = Q16_FROM_INT(1),
                                          .lower_arm_len = Q16_FROM_INT(2)};
    SYN_Position3D pos_a0 = {.x = 0, .y = Q16_FROM_INT(1), .z = Q16_FROM_INT(2)};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_ik(&dcfg_a0, &pos_a0, &t1, &t2, &t3));

    /* Delta IK arm calculation where A == 0, B == 0 (unreachable boundary) */
    SYN_Position3D pos_a0_b0 = {.x = 0, .y = Q16_FROM_INT(3), .z = 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_ik(&dcfg_a0, &pos_a0_b0, &t1, &t2, &t3));

    /* Delta IK where Arm 1 succeeds but Arm 2 fails */
    SYN_Position3D pos_arm2_fail = {.x = -Q16_FROM_INT(1) - Q16_FROM_FRAC(25, 100),
                                    .y = Q16_FROM_INT(2) + Q16_FROM_FRAC(165, 1000),
                                    .z = 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_ik(&dcfg_a0, &pos_arm2_fail, &t1, &t2, &t3));

    /* Delta IK where Arm 1 & 2 succeed but Arm 3 fails */
    SYN_Position3D pos_arm3_fail = {.x = -Q16_FROM_INT(1) - Q16_FROM_FRAC(25, 100),
                                    .y = -Q16_FROM_INT(2) - Q16_FROM_FRAC(165, 1000),
                                    .z = 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_ik(&dcfg_a0, &pos_arm3_fail, &t1, &t2, &t3));

    /* Delta FK zero denominator check (theta2 = theta3 = PI/2 with wb = 0) */
    SYN_Kinematics_DeltaConfig dcfg_zero_denom = {.base_radius = 0,
                                                  .effector_radius = 0,
                                                  .upper_arm_len = Q16_FROM_INT(1),
                                                  .lower_arm_len = Q16_FROM_INT(2)};
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_kinematics_delta_fk(&dcfg_zero_denom, 0, Q16_PI_2, Q16_PI_2, &pos));

    /* Delta FK impossible geometry (spheres do not intersect) */
    SYN_Kinematics_DeltaConfig bad_dcfg = {.upper_arm_len = Q16_FROM_INT(20),
                                           .lower_arm_len = Q16_FROM_INT(1),
                                           .base_radius = Q16_FROM_INT(5),
                                           .effector_radius = Q16_FROM_INT(2)};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_fk(&bad_dcfg, 0, 0, 0, &pos));
}

void test_r5_kinematics_industrial_and_6dof_roundtrip(void)
{
    /* 1. Industrial 3-DOF Planar Arm (300/250/80 mm links — previously overflowed int32_t) */
    SYN_Kinematics_Planar3Config p3_cfg = {
        .l1 = Q16_FROM_INT(300), .l2 = Q16_FROM_INT(250), .l3 = Q16_FROM_INT(80)};
    q16_t pq1 = 0, pq2 = 0, pq3 = 0;
    q16_t px = 0, py = 0, pphi = 0;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_planar3_ik(&p3_cfg, Q16_FROM_INT(350),
                                                        Q16_FROM_INT(200), Q16_FROM_FRAC(1, 2),
                                                        SYN_ARM_ELBOW_UP, &pq1, &pq2, &pq3));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_planar3_fk(&p3_cfg, pq1, pq2, pq3, &px, &py, &pphi));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(1), Q16_FROM_INT(350), px);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(1), Q16_FROM_INT(200), py);
    TEST_ASSERT_INT_WITHIN(2000, Q16_FROM_FRAC(1, 2), pphi);

    /* Large-scale 20 m links exercising scale_den >> 2 loop */
    SYN_Kinematics_Planar3Config huge_cfg = {
        .l1 = Q16_FROM_INT(20000), .l2 = Q16_FROM_INT(20000), .l3 = 0};
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_planar3_ik(&huge_cfg, Q16_FROM_INT(20000), Q16_FROM_INT(20000),
                                                0, SYN_ARM_ELBOW_UP, &pq1, &pq2, &pq3));

    /* 2. Industrial 4-DOF SCARA (325/275 mm links) */
    SYN_Kinematics_SCARAConfig scara_cfg = {.l1 = Q16_FROM_INT(325),
                                            .l2 = Q16_FROM_INT(275),
                                            .d_max = Q16_FROM_INT(200),
                                            .z_home = Q16_FROM_INT(250)};
    SYN_Pose6D scara_target = {
        .position = {.x = Q16_FROM_INT(380), .y = Q16_FROM_INT(190), .z = Q16_FROM_INT(120)},
        .orientation = {.roll = 0, .pitch = 0, .yaw = Q16_FROM_FRAC(7, 10)}};
    q16_t sq1 = 0, sq2 = 0, sd3 = 0, sq4 = 0;
    SYN_Pose6D scara_fk_pose;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_scara_ik(&scara_cfg, &scara_target, SYN_ARM_ELBOW_DOWN,
                                                      &sq1, &sq2, &sd3, &sq4));
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_scara_fk(&scara_cfg, sq1, sq2, sd3, sq4, &scara_fk_pose));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(1), scara_target.position.x, scara_fk_pose.position.x);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(1), scara_target.position.y, scara_fk_pose.position.y);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(1), scara_target.position.z, scara_fk_pose.position.z);
    TEST_ASSERT_INT_WITHIN(2000, scara_target.orientation.yaw, scara_fk_pose.orientation.yaw);

    /* 3. Industrial 6-DOF Arm (250 mm links) general 3D target (250, 120, 300) mm + RPY */
    SYN_Kinematics_6DOFConfig arm6 = {.d1 = Q16_FROM_INT(150),
                                      .a1 = Q16_FROM_INT(50),
                                      .a2 = Q16_FROM_INT(250),
                                      .d4 = Q16_FROM_INT(250),
                                      .d6 = Q16_FROM_INT(80)};
    SYN_Pose6D target6 = {
        .position = {.x = Q16_FROM_INT(250), .y = Q16_FROM_INT(120), .z = Q16_FROM_INT(300)},
        .orientation = {.roll = Q16_FROM_FRAC(3, 10),
                        .pitch = -Q16_FROM_FRAC(4, 10),
                        .yaw = Q16_FROM_FRAC(5, 10)}};

    const SYN_ArmElbow elbows[2] = {SYN_ARM_ELBOW_UP, SYN_ARM_ELBOW_DOWN};
    for (size_t e = 0U; e < 2U; e++) {
        for (int wf = 0; wf < 2; wf++) {
            q16_t ik_q[6];
            SYN_Pose6D fk_pose;
            TEST_ASSERT_EQUAL(SYN_OK,
                              syn_kinematics_6dof_ik(&arm6, &target6, elbows[e], wf != 0, ik_q));
            TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_6dof_fk(&arm6, ik_q, &fk_pose));
            TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(2), target6.position.x, fk_pose.position.x);
            TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(2), target6.position.y, fk_pose.position.y);
            TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(2), target6.position.z, fk_pose.position.z);
            TEST_ASSERT_INT_WITHIN(3000, target6.orientation.roll, fk_pose.orientation.roll);
            TEST_ASSERT_INT_WITHIN(3000, target6.orientation.pitch, fk_pose.orientation.pitch);
            TEST_ASSERT_INT_WITHIN(3000, target6.orientation.yaw, fk_pose.orientation.yaw);
        }
    }

    /* Singular wrist at q5 = pi (r36_22 < 0) + hypot2d clamp */
    q16_t q_sing_pi[6] = {0, -Q16_PI_2 / 3, Q16_PI_2 / 3, 0, Q16_PI, Q16_FROM_FRAC(2, 10)};
    SYN_Pose6D pose_sing_pi;
    q16_t out_q[6];
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_6dof_fk(&arm6, q_sing_pi, &pose_sing_pi));
    TEST_ASSERT_EQUAL(
        SYN_OK, syn_kinematics_6dof_ik(&arm6, &pose_sing_pi, SYN_ARM_ELBOW_DOWN, false, out_q));

    SYN_Pose6D far_pose = {
        .position = {.x = Q16_FROM_INT(25000), .y = Q16_FROM_INT(25000), .z = 0}};
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_kinematics_6dof_ik(&arm6, &far_pose, SYN_ARM_ELBOW_UP, false, out_q));

    /* 4. Industrial Delta Robot (200/50/220/480 mm) roundtrip + A==0, B<0 edge case */
    SYN_Kinematics_DeltaConfig delta_ind = {.base_radius = Q16_FROM_INT(200),
                                            .effector_radius = Q16_FROM_INT(50),
                                            .upper_arm_len = Q16_FROM_INT(220),
                                            .lower_arm_len = Q16_FROM_INT(480)};
    SYN_Position3D d_target = {
        .x = Q16_FROM_INT(80), .y = -Q16_FROM_INT(60), .z = -Q16_FROM_INT(380)};
    q16_t dt1 = 0, dt2 = 0, dt3 = 0;
    SYN_Position3D d_fk;
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_ik(&delta_ind, &d_target, &dt1, &dt2, &dt3));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_fk(&delta_ind, dt1, dt2, dt3, &d_fk));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(2), d_target.x, d_fk.x);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(2), d_target.y, d_fk.y);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(2), d_target.z, d_fk.z);

    SYN_Kinematics_DeltaConfig dcfg_a0 = {.base_radius = 0,
                                          .effector_radius = 0,
                                          .upper_arm_len = Q16_FROM_INT(1),
                                          .lower_arm_len = Q16_FROM_INT(2)};
    SYN_Position3D pos_a0_neg = {.x = 0, .y = Q16_FROM_INT(1), .z = -Q16_FROM_INT(2)};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_ik(&dcfg_a0, &pos_a0_neg, &dt1, &dt2, &dt3));

    /* Exercise max_c > max_abc (y0 > 0, E > 0, z0 = 0) */
    SYN_Kinematics_DeltaConfig dcfg_maxc = {.base_radius = Q16_FROM_INT(10),
                                            .effector_radius = 0,
                                            .upper_arm_len = Q16_FROM_INT(10),
                                            .lower_arm_len = Q16_FROM_INT(5)};
    SYN_Position3D pos_maxc = {.x = 0, .y = Q16_FROM_INT(5), .z = 0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_ik(&dcfg_maxc, &pos_maxc, &dt1, &dt2, &dt3));
}

void test_kinematics_modified_dh_jacobian_and_large_delta_fk(void)
{
    /* 1. Modified DH Jacobian must match finite-difference d(FK)/dq on spatial R-R-P chain */
    SYN_DH_Param mdh[3] = {
        {.a = Q16_FROM_INT(5), .alpha = Q16_PI_2, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = Q16_FROM_INT(4), .alpha = 0, .d = 0, .theta = 0, .is_prismatic = false},
        {.a = Q16_FROM_INT(2),
         .alpha = Q16_PI_2,
         .d = Q16_FROM_INT(3),
         .theta = 0,
         .is_prismatic = true},
    };
    q16_t q[3] = {Q16_FROM_FRAC(3, 10), -Q16_FROM_FRAC(4, 10), Q16_FROM_INT(1)};

    SYN_MAT_DECL(J, 6, 3);
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_jacobian(mdh, 3U, q, SYN_DH_CONVENTION_MODIFIED, &J));

    SYN_Pose6D p0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_forward(mdh, 3U, q, SYN_DH_CONVENTION_MODIFIED, NULL, &p0));

    const q16_t eps = 256; /* 1/256 rad or unit */
    for (size_t j = 0U; j < 3U; j++) {
        q16_t qp[3] = {q[0], q[1], q[2]};
        qp[j] += eps;
        SYN_Pose6D p1;
        TEST_ASSERT_EQUAL(
            SYN_OK, syn_kinematics_forward(mdh, 3U, qp, SYN_DH_CONVENTION_MODIFIED, NULL, &p1));

        q16_t fd_vx = (q16_t)(((int64_t)(p1.position.x - p0.position.x) * (int64_t)Q16_ONE) / eps);
        q16_t fd_vy = (q16_t)(((int64_t)(p1.position.y - p0.position.y) * (int64_t)Q16_ONE) / eps);
        q16_t fd_vz = (q16_t)(((int64_t)(p1.position.z - p0.position.z) * (int64_t)Q16_ONE) / eps);

        TEST_ASSERT_INT_WITHIN(3500, fd_vx, SYN_MAT_AT(&J, 0, j));
        TEST_ASSERT_INT_WITHIN(3500, fd_vy, SYN_MAT_AT(&J, 1, j));
        TEST_ASSERT_INT_WITHIN(3500, fd_vz, SYN_MAT_AT(&J, 2, j));
    }

    /* 2. Large-coordinate Delta IK -> FK roundtrip without 64-bit discriminant overflow */
    SYN_Kinematics_DeltaConfig big_delta = {
        .base_radius = Q16_FROM_INT(3000),
        .effector_radius = Q16_FROM_INT(800),
        .upper_arm_len = Q16_FROM_INT(5000),
        .lower_arm_len = Q16_FROM_INT(12000),
    };
    SYN_Position3D big_target = {
        .x = Q16_FROM_INT(600),
        .y = -Q16_FROM_INT(500),
        .z = -Q16_FROM_INT(9500),
    };
    q16_t bt1 = 0, bt2 = 0, bt3 = 0;
    SYN_Position3D big_fk = {0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_ik(&big_delta, &big_target, &bt1, &bt2, &bt3));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_delta_fk(&big_delta, bt1, bt2, bt3, &big_fk));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(25), big_target.x, big_fk.x);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(25), big_target.y, big_fk.y);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(25), big_target.z, big_fk.z);
}

void test_kinematics_forward_validation_2link_scara_and_delta_overflows(void)
{
    /* 1. syn_kinematics_forward must reject undersized or NULL-data out_t44 matrices */
    SYN_DH_Param link = {.a = Q16_ONE, .alpha = 0, .d = 0, .theta = 0, .is_prismatic = false};
    q16_t qv = 0;
    SYN_MAT_DECL(bad_3x4, 3, 4);
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_kinematics_forward(&link, 1U, &qv, SYN_DH_CONVENTION_STANDARD, &bad_3x4, NULL));
    SYN_Matrix null_data_mat = {.data = NULL, .rows = 4U, .cols = 4U};
    TEST_ASSERT_EQUAL(
        SYN_INVALID_PARAM,
        syn_kinematics_forward(&link, 1U, &qv, SYN_DH_CONVENTION_STANDARD, &null_data_mat, NULL));

    /* 2. Large-link 2-link planar IK -> FK roundtrip where l1 + l2*cos(q2) > 32767 */
    SYN_Kinematics_Planar3Config huge_cfg = {
        .l1 = Q16_FROM_INT(20000), .l2 = Q16_FROM_INT(20000), .l3 = 0};
    q16_t q1 = 0, q2 = 0, q3 = 0;
    q16_t fx = 0, fy = 0, fphi = 0;
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_kinematics_planar3_ik(&huge_cfg, Q16_FROM_INT(32700), Q16_FROM_INT(20000),
                                                0, SYN_ARM_ELBOW_UP, &q1, &q2, &q3));
    TEST_ASSERT_EQUAL(SYN_OK, syn_kinematics_planar3_fk(&huge_cfg, q1, q2, q3, &fx, &fy, &fphi));
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(25), Q16_FROM_INT(32700), fx);
    TEST_ASSERT_INT_WITHIN(Q16_FROM_INT(25), Q16_FROM_INT(20000), fy);

    /* 3. SCARA IK must reject z_home - target.z overflowing int32_t into [0, d_max] */
    SYN_Kinematics_SCARAConfig scara_wrap = {.l1 = Q16_FROM_INT(200),
                                             .l2 = Q16_FROM_INT(200),
                                             .d_max = Q16_FROM_INT(32767),
                                             .z_home = -Q16_FROM_INT(20000)};
    SYN_Pose6D bad_z = {
        .position = {.x = Q16_FROM_INT(200), .y = Q16_FROM_INT(100), .z = Q16_FROM_INT(13000)},
        .orientation = {0, 0, 0}};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_scara_ik(&scara_wrap, &bad_z, SYN_ARM_ELBOW_UP, &q1,
                                                         &q2, &q3, NULL));

    /* 4. Delta FK extreme arm geometry, near-singular slope, and out-of-range Z rejected */
    SYN_Kinematics_DeltaConfig extreme_delta = {.base_radius = Q16_FROM_INT(25000),
                                                .effector_radius = -Q16_FROM_INT(20000),
                                                .upper_arm_len = Q16_FROM_INT(25000),
                                                .lower_arm_len = Q16_FROM_INT(30000)};
    SYN_Position3D d_out = {0, 0, 0};
    TEST_ASSERT_EQUAL(SYN_ERROR,
                      syn_kinematics_delta_fk(&extreme_delta, 0, Q16_PI_2, -Q16_PI_2, &d_out));

    SYN_Kinematics_DeltaConfig sing_delta = {.base_radius = Q16_FROM_INT(100),
                                             .effector_radius = Q16_FROM_INT(100),
                                             .upper_arm_len = Q16_FROM_INT(100),
                                             .lower_arm_len = Q16_FROM_INT(200)};
    TEST_ASSERT_EQUAL(
        SYN_ERROR, syn_kinematics_delta_fk(&sing_delta, 0, Q16_PI_2 - 4, -(Q16_PI_2 - 4), &d_out));

    SYN_Kinematics_DeltaConfig deep_delta = {.base_radius = Q16_FROM_INT(10000),
                                             .effector_radius = Q16_FROM_INT(2000),
                                             .upper_arm_len = Q16_FROM_INT(20000),
                                             .lower_arm_len = Q16_FROM_INT(30000)};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_kinematics_delta_fk(&deep_delta, Q16_PI / 3, Q16_PI / 3,
                                                         Q16_PI / 3, &d_out));
}

void run_kinematics_tests(void)
{
    RUN_TEST(test_kinematics_dh_matrix_and_forward);
    RUN_TEST(test_kinematics_jacobian);
    RUN_TEST(test_kinematics_planar3);
    RUN_TEST(test_kinematics_scara);
    RUN_TEST(test_kinematics_6dof_and_delta);
    RUN_TEST(test_r5_kinematics_industrial_and_6dof_roundtrip);
    RUN_TEST(test_kinematics_modified_dh_jacobian_and_large_delta_fk);
    RUN_TEST(test_kinematics_forward_validation_2link_scara_and_delta_overflows);
}
