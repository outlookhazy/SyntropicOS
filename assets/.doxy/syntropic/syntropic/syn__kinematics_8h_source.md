

# File syn\_kinematics.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_kinematics.h**](syn__kinematics_8h.md)

[Go to the documentation of this file](syn__kinematics_8h.md)


```C++

#ifndef SYN_KINEMATICS_H
#define SYN_KINEMATICS_H

#include "../common/syn_defs.h"
#include "../util/syn_matrix.h"
#include "../util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_KINEMATICS) || SYN_USE_KINEMATICS

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_KINEMATICS_MAX_JOINTS 8U 
/* ── Denavit-Hartenberg Types ────────────────────────────────────────────── */

typedef enum {
    SYN_DH_CONVENTION_STANDARD = 0, 
    SYN_DH_CONVENTION_MODIFIED = 1  
} SYN_DH_Convention;

typedef struct {
    q16_t a;           
    q16_t alpha;       
    q16_t d;           
    q16_t theta;       
    bool is_prismatic; 
    q16_t min_limit;   
    q16_t max_limit;   
} SYN_DH_Param;

/* ── Spatial Pose Types ──────────────────────────────────────────────────── */

typedef struct {
    q16_t x; 
    q16_t y; 
    q16_t z; 
} SYN_Position3D;

typedef struct {
    q16_t roll;  
    q16_t pitch; 
    q16_t yaw;   
} SYN_Orientation3D;

typedef struct {
    SYN_Position3D position;       
    SYN_Orientation3D orientation; 
} SYN_Pose6D;

/* ── Mechanism Configuration Descriptors ─────────────────────────────────── */

typedef struct {
    q16_t l1; 
    q16_t l2; 
    q16_t l3; 
} SYN_Kinematics_Planar3Config;

typedef enum {
    SYN_ARM_ELBOW_UP = 0,  
    SYN_ARM_ELBOW_DOWN = 1 
} SYN_ArmElbow;

typedef struct {
    q16_t l1;     
    q16_t l2;     
    q16_t d_max;  
    q16_t z_home; 
} SYN_Kinematics_SCARAConfig;

typedef struct {
    q16_t d1; 
    q16_t a1; 
    q16_t a2; 
    q16_t d4; 
    q16_t d6; 
} SYN_Kinematics_6DOFConfig;

typedef struct {
    q16_t base_radius;     
    q16_t effector_radius; 
    q16_t upper_arm_len;   
    q16_t lower_arm_len;   
} SYN_Kinematics_DeltaConfig;

/* ── General Denavit-Hartenberg & Chain API ──────────────────────────────── */

SYN_Status syn_dh_matrix(const SYN_DH_Param *param, q16_t joint_val, SYN_DH_Convention conv,
                         SYN_Matrix *out_t44);

SYN_Status syn_kinematics_forward(const SYN_DH_Param *chain, size_t num_joints,
                                  const q16_t *joint_vals, SYN_DH_Convention conv,
                                  SYN_Matrix *out_t44, SYN_Pose6D *out_pose);

SYN_Status syn_kinematics_jacobian(const SYN_DH_Param *chain, size_t num_joints,
                                   const q16_t *joint_vals, SYN_DH_Convention conv,
                                   SYN_Matrix *out_j6xn);

/* ── 3-DOF Planar Arm API ────────────────────────────────────────────────── */

SYN_Status syn_kinematics_planar3_fk(const SYN_Kinematics_Planar3Config *cfg, q16_t q1, q16_t q2,
                                     q16_t q3, q16_t *out_x, q16_t *out_y, q16_t *out_phi);

SYN_Status syn_kinematics_planar3_ik(const SYN_Kinematics_Planar3Config *cfg, q16_t x, q16_t y,
                                     q16_t phi, SYN_ArmElbow elbow, q16_t *out_q1, q16_t *out_q2,
                                     q16_t *out_q3);

/* ── 4-DOF SCARA API ─────────────────────────────────────────────────────── */

SYN_Status syn_kinematics_scara_fk(const SYN_Kinematics_SCARAConfig *cfg, q16_t q1, q16_t q2,
                                   q16_t d3, q16_t q4, SYN_Pose6D *out_pose);

SYN_Status syn_kinematics_scara_ik(const SYN_Kinematics_SCARAConfig *cfg, const SYN_Pose6D *target,
                                   SYN_ArmElbow elbow, q16_t *out_q1, q16_t *out_q2, q16_t *out_d3,
                                   q16_t *out_q4);

/* ── 6-DOF Articulated Arm (Spherical Wrist) API ─────────────────────────── */

SYN_Status syn_kinematics_6dof_fk(const SYN_Kinematics_6DOFConfig *cfg, const q16_t q[6],
                                  SYN_Pose6D *out_pose);

SYN_Status syn_kinematics_6dof_ik(const SYN_Kinematics_6DOFConfig *cfg, const SYN_Pose6D *target,
                                  SYN_ArmElbow elbow, bool wrist_flip, q16_t out_q[6]);

/* ── 3-Axis Delta Parallel Robot API ─────────────────────────────────────── */

SYN_Status syn_kinematics_delta_fk(const SYN_Kinematics_DeltaConfig *cfg, q16_t theta1,
                                   q16_t theta2, q16_t theta3, SYN_Position3D *out_pos);

SYN_Status syn_kinematics_delta_ik(const SYN_Kinematics_DeltaConfig *cfg,
                                   const SYN_Position3D *target, q16_t *out_theta1,
                                   q16_t *out_theta2, q16_t *out_theta3);

#ifdef __cplusplus
}
#endif

#endif /* !defined(SYN_USE_KINEMATICS) || SYN_USE_KINEMATICS */

#endif /* SYN_KINEMATICS_H */
```


