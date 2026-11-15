

# File syn\_kinematics.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_kinematics.c**](syn__kinematics_8c.md)

[Go to the source code of this file](syn__kinematics_8c_source.md)

_Multi-Axis Robot Forward & Inverse Kinematics Engine Implementation._ 

* `#include "../port/syn_port_system.h"`
* `#include "syn_kinematics.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dh\_matrix**](#function-syn_dh_matrix) (const [**SYN\_DH\_Param**](structSYN__DH__Param.md) \* param, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) joint\_val, [**SYN\_DH\_Convention**](syn__kinematics_8h.md#enum-syn_dh_convention) conv, [**SYN\_Matrix**](structSYN__Matrix.md) \* out\_t44) <br>_Compute single-link 4x4 Homogeneous Transformation Matrix from DH parameters._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_6dof\_fk**](#function-syn_kinematics_6dof_fk) (const [**SYN\_Kinematics\_6DOFConfig**](structSYN__Kinematics__6DOFConfig.md) \* cfg, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q, [**SYN\_Pose6D**](structSYN__Pose6D.md) \* out\_pose) <br>_Forward kinematics for 6-DOF articulated arm._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_6dof\_ik**](#function-syn_kinematics_6dof_ik) (const [**SYN\_Kinematics\_6DOFConfig**](structSYN__Kinematics__6DOFConfig.md) \* cfg, const [**SYN\_Pose6D**](structSYN__Pose6D.md) \* target, [**SYN\_ArmElbow**](syn__kinematics_8h.md#enum-syn_armelbow) elbow, [**bool**](syn__defs_8h.md#enum-bool) wrist\_flip, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) out\_q) <br>_Closed-form inverse kinematics for 6-DOF arm with spherical wrist (Pieper solution)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_delta\_fk**](#function-syn_kinematics_delta_fk) (const [**SYN\_Kinematics\_DeltaConfig**](structSYN__Kinematics__DeltaConfig.md) \* cfg, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) theta3, [**SYN\_Position3D**](structSYN__Position3D.md) \* out\_pos) <br>_Forward kinematics for 3-axis Delta parallel robot._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_delta\_ik**](#function-syn_kinematics_delta_ik) (const [**SYN\_Kinematics\_DeltaConfig**](structSYN__Kinematics__DeltaConfig.md) \* cfg, const [**SYN\_Position3D**](structSYN__Position3D.md) \* target, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_theta1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_theta2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_theta3) <br>_Inverse kinematics for 3-axis Delta parallel robot._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_forward**](#function-syn_kinematics_forward) (const [**SYN\_DH\_Param**](structSYN__DH__Param.md) \* chain, size\_t num\_joints, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* joint\_vals, [**SYN\_DH\_Convention**](syn__kinematics_8h.md#enum-syn_dh_convention) conv, [**SYN\_Matrix**](structSYN__Matrix.md) \* out\_t44, [**SYN\_Pose6D**](structSYN__Pose6D.md) \* out\_pose) <br>_Compute Forward Kinematics for a general serial DH chain._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_jacobian**](#function-syn_kinematics_jacobian) (const [**SYN\_DH\_Param**](structSYN__DH__Param.md) \* chain, size\_t num\_joints, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* joint\_vals, [**SYN\_DH\_Convention**](syn__kinematics_8h.md#enum-syn_dh_convention) conv, [**SYN\_Matrix**](structSYN__Matrix.md) \* out\_j6xn) <br>_Calculate Geometric 6xN Jacobian Matrix for a serial link chain._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_planar3\_fk**](#function-syn_kinematics_planar3_fk) (const [**SYN\_Kinematics\_Planar3Config**](structSYN__Kinematics__Planar3Config.md) \* cfg, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q3, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_phi) <br>_Forward kinematics for 3-DOF planar arm._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_planar3\_ik**](#function-syn_kinematics_planar3_ik) (const [**SYN\_Kinematics\_Planar3Config**](structSYN__Kinematics__Planar3Config.md) \* cfg, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) phi, [**SYN\_ArmElbow**](syn__kinematics_8h.md#enum-syn_armelbow) elbow, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q3) <br>_Closed-form inverse kinematics for 3-DOF planar arm._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_scara\_fk**](#function-syn_kinematics_scara_fk) (const [**SYN\_Kinematics\_SCARAConfig**](structSYN__Kinematics__SCARAConfig.md) \* cfg, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) d3, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) q4, [**SYN\_Pose6D**](structSYN__Pose6D.md) \* out\_pose) <br>_Forward kinematics for 4-DOF SCARA robot._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_kinematics\_scara\_ik**](#function-syn_kinematics_scara_ik) (const [**SYN\_Kinematics\_SCARAConfig**](structSYN__Kinematics__SCARAConfig.md) \* cfg, const [**SYN\_Pose6D**](structSYN__Pose6D.md) \* target, [**SYN\_ArmElbow**](syn__kinematics_8h.md#enum-syn_armelbow) elbow, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q2, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_d3, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q4) <br>_Closed-form inverse kinematics for 4-DOF SCARA robot._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**delta\_calc\_arm**](#function-delta_calc_arm) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) z0, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) L, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) l, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_theta) <br>_Helper to solve single Delta arm angle using 64-bit arithmetic._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**delta\_fk\_solve\_z**](#function-delta_fk_solve_z) (int64\_t a, int64\_t b, int64\_t c, int64\_t \* out\_z) <br>_Solve Delta FK quadratic a\*z^2 + b\*z + c = 0 (lower root) without 64-bit overflow._  |
|  void | [**delta\_prescale\_dw**](#function-delta_prescale_dw) (int64\_t \* dw21, int64\_t \* dw31, int64\_t \* dd\_b) <br>_Prescale (dw21, dw31, dd\_b) together so 3-term sphere products fit in int64\_t._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**delta\_q16\_vec3\_ok**](#function-delta_q16_vec3_ok) (int64\_t u, int64\_t v, int64\_t w) <br>_Check whether three 64-bit Q48.16 values fit within 32-bit signed Q16.16 range._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**delta\_slope\_ok**](#function-delta_slope_ok) (int64\_t c1, int64\_t c2) <br>_Check whether radical-line coefficients fit inside 64-bit quadratic bounds._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**hypot2d\_q16**](#function-hypot2d_q16) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) x, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) y) <br>_Overflow-safe 2D Euclidean distance sqrt(x^2 + y^2) in Q16.16._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**kin\_out\_mat44\_valid**](#function-kin_out_mat44_valid) (const [**SYN\_Matrix**](structSYN__Matrix.md) \* mat) <br>_Validate optional 4x4 homogeneous transform output matrix descriptor._  |
|  void | [**rot\_matrix\_to\_rpy**](#function-rot_matrix_to_rpy) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) r33, [**SYN\_Orientation3D**](structSYN__Orientation3D.md) \* out\_ori) <br>_Extract Roll-Pitch-Yaw Euler angles from 3x3 rotation matrix._  |
|  void | [**rpy\_to\_rot\_matrix**](#function-rpy_to_rot_matrix) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) roll, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) pitch, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) yaw, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) r33) <br>_Build 3x3 rotation matrix from Roll-Pitch-Yaw Euler angles (Z-Y-X order)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**solve\_2link\_planar**](#function-solve_2link_planar) ([**q16\_t**](syn__qmath_8h.md#typedef-q16_t) rx, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) ry, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) l1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) l2, [**SYN\_ArmElbow**](syn__kinematics_8h.md#enum-syn_armelbow) elbow, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q1, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) \* out\_q2) <br>_Solve 2-link planar inverse kinematics using 64-bit cosine law._  |
|  void | [**solve\_spherical\_wrist**](#function-solve_spherical_wrist) (const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) r03, const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) r06, [**bool**](syn__defs_8h.md#enum-bool) wrist\_flip, [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) out\_q) <br>_Extract Z-Y-Z spherical wrist angles (q4, q5, q6) from R\_36 = R\_03^T \* R\_06._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**Q16\_COS120**](syn__kinematics_8c.md#define-q16_cos120)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t)) - 32768)`<br> |
| define  | [**Q16\_COS240**](syn__kinematics_8c.md#define-q16_cos240)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t)) - 32768)`<br> |
| define  | [**Q16\_SIN120**](syn__kinematics_8c.md#define-q16_sin120)  `56756`<br> |
| define  | [**Q16\_SIN240**](syn__kinematics_8c.md#define-q16_sin240)  `(([**q16\_t**](syn__qmath_8h.md#typedef-q16_t)) - 56756)`<br> |
| define  | [**Q16\_SQRT3\_2**](syn__kinematics_8c.md#define-q16_sqrt3_2)  `56756`<br> |

## Public Functions Documentation




### function syn\_dh\_matrix 

_Compute single-link 4x4 Homogeneous Transformation Matrix from DH parameters._ 
```C++
SYN_Status syn_dh_matrix (
    const SYN_DH_Param * param,
    q16_t joint_val,
    SYN_DH_Convention conv,
    SYN_Matrix * out_t44
) 
```





**Parameters:**


* `param` DH parameters for link. 
* `joint_val` Joint variable (angle in rad for revolute, displacement for prismatic). 
* `conv` DH convention (standard or modified). 
* `out_t44` Output 4x4 matrix (must be initialized with 4 rows, 4 cols). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on NULL or dimension error. 





        

<hr>



### function syn\_kinematics\_6dof\_fk 

_Forward kinematics for 6-DOF articulated arm._ 
```C++
SYN_Status syn_kinematics_6dof_fk (
    const SYN_Kinematics_6DOFConfig * cfg,
    const q16_t q,
    SYN_Pose6D * out_pose
) 
```





**Parameters:**


* `cfg` 6-DOF arm configuration. 
* `q` Array of 6 joint angles in radians. 
* `out_pose` Output 6-DOF Cartesian pose. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_kinematics\_6dof\_ik 

_Closed-form inverse kinematics for 6-DOF arm with spherical wrist (Pieper solution)._ 
```C++
SYN_Status syn_kinematics_6dof_ik (
    const SYN_Kinematics_6DOFConfig * cfg,
    const SYN_Pose6D * target,
    SYN_ArmElbow elbow,
    bool wrist_flip,
    q16_t out_q
) 
```





**Parameters:**


* `cfg` 6-DOF arm configuration. 
* `target` Target 6-DOF pose. 
* `elbow` Shoulder/elbow solution choice. 
* `wrist_flip` Invert wrist pitch/yaw solution if true. 
* `out_q` Output array of 6 joint angles in radians. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if target position or orientation unreachable. 





        

<hr>



### function syn\_kinematics\_delta\_fk 

_Forward kinematics for 3-axis Delta parallel robot._ 
```C++
SYN_Status syn_kinematics_delta_fk (
    const SYN_Kinematics_DeltaConfig * cfg,
    q16_t theta1,
    q16_t theta2,
    q16_t theta3,
    SYN_Position3D * out_pos
) 
```





**Parameters:**


* `cfg` Delta robot geometry configuration. 
* `theta1` Actuator 1 angle (rad). 
* `theta2` Actuator 2 angle (rad). 
* `theta3` Actuator 3 angle (rad). 
* `out_pos` Output 3D Cartesian position of traveling plate center. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if no real geometric intersection exists. 





        

<hr>



### function syn\_kinematics\_delta\_ik 

_Inverse kinematics for 3-axis Delta parallel robot._ 
```C++
SYN_Status syn_kinematics_delta_ik (
    const SYN_Kinematics_DeltaConfig * cfg,
    const SYN_Position3D * target,
    q16_t * out_theta1,
    q16_t * out_theta2,
    q16_t * out_theta3
) 
```





**Parameters:**


* `cfg` Delta robot geometry configuration. 
* `target` Target 3D Cartesian position (X, Y, Z). 
* `out_theta1` Output actuator 1 angle (rad). 
* `out_theta2` Output actuator 2 angle (rad). 
* `out_theta3` Output actuator 3 angle (rad). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if position is outside workspace. 





        

<hr>



### function syn\_kinematics\_forward 

_Compute Forward Kinematics for a general serial DH chain._ 
```C++
SYN_Status syn_kinematics_forward (
    const SYN_DH_Param * chain,
    size_t num_joints,
    const q16_t * joint_vals,
    SYN_DH_Convention conv,
    SYN_Matrix * out_t44,
    SYN_Pose6D * out_pose
) 
```





**Parameters:**


* `chain` Array of DH joint parameters. 
* `num_joints` Number of joints in chain (&lt;= SYN\_KINEMATICS\_MAX\_JOINTS). 
* `joint_vals` Array of joint values. 
* `conv` DH convention. 
* `out_t44` Optional output 4x4 end-effector transform matrix (can be NULL). 
* `out_pose` Optional output 6-DOF pose (can be NULL). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_kinematics\_jacobian 

_Calculate Geometric 6xN Jacobian Matrix for a serial link chain._ 
```C++
SYN_Status syn_kinematics_jacobian (
    const SYN_DH_Param * chain,
    size_t num_joints,
    const q16_t * joint_vals,
    SYN_DH_Convention conv,
    SYN_Matrix * out_j6xn
) 
```





**Parameters:**


* `chain` Array of DH joint parameters. 
* `num_joints` Number of joints in chain. 
* `joint_vals` Current joint positions. 
* `conv` DH convention. 
* `out_j6xn` Output 6xN matrix (rows=6, cols=num\_joints). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM on invalid inputs. 





        

<hr>



### function syn\_kinematics\_planar3\_fk 

_Forward kinematics for 3-DOF planar arm._ 
```C++
SYN_Status syn_kinematics_planar3_fk (
    const SYN_Kinematics_Planar3Config * cfg,
    q16_t q1,
    q16_t q2,
    q16_t q3,
    q16_t * out_x,
    q16_t * out_y,
    q16_t * out_phi
) 
```





**Parameters:**


* `cfg` Planar arm configuration. 
* `q1` Joint 1 angle (rad). 
* `q2` Joint 2 angle (rad). 
* `q3` Joint 3 angle (rad). 
* `out_x` Pointer to receive X position. 
* `out_y` Pointer to receive Y position. 
* `out_phi` Pointer to receive tool orientation angle (rad). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_kinematics\_planar3\_ik 

_Closed-form inverse kinematics for 3-DOF planar arm._ 
```C++
SYN_Status syn_kinematics_planar3_ik (
    const SYN_Kinematics_Planar3Config * cfg,
    q16_t x,
    q16_t y,
    q16_t phi,
    SYN_ArmElbow elbow,
    q16_t * out_q1,
    q16_t * out_q2,
    q16_t * out_q3
) 
```





**Parameters:**


* `cfg` Planar arm configuration. 
* `x` Target X position. 
* `y` Target Y position. 
* `phi` Target end-effector orientation angle (rad). 
* `elbow` Elbow configuration (up or down). 
* `out_q1` Output joint 1 angle. 
* `out_q2` Output joint 2 angle. 
* `out_q3` Output joint 3 angle. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if target unreachable. 





        

<hr>



### function syn\_kinematics\_scara\_fk 

_Forward kinematics for 4-DOF SCARA robot._ 
```C++
SYN_Status syn_kinematics_scara_fk (
    const SYN_Kinematics_SCARAConfig * cfg,
    q16_t q1,
    q16_t q2,
    q16_t d3,
    q16_t q4,
    SYN_Pose6D * out_pose
) 
```





**Parameters:**


* `cfg` SCARA configuration. 
* `q1` Joint 1 shoulder angle (rad). 
* `q2` Joint 2 elbow angle (rad). 
* `d3` Joint 3 prismatic vertical displacement (distance down). 
* `q4` Joint 4 wrist roll angle (rad). 
* `out_pose` Output 4-DOF pose (X, Y, Z, Yaw). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_kinematics\_scara\_ik 

_Closed-form inverse kinematics for 4-DOF SCARA robot._ 
```C++
SYN_Status syn_kinematics_scara_ik (
    const SYN_Kinematics_SCARAConfig * cfg,
    const SYN_Pose6D * target,
    SYN_ArmElbow elbow,
    q16_t * out_q1,
    q16_t * out_q2,
    q16_t * out_d3,
    q16_t * out_q4
) 
```





**Parameters:**


* `cfg` SCARA configuration. 
* `target` Target 6-DOF pose (X, Y, Z, Yaw). 
* `elbow` Elbow configuration (up/left or down/right). 
* `out_q1` Output joint 1 angle. 
* `out_q2` Output joint 2 angle. 
* `out_d3` Output joint 3 vertical position. 
* `out_q4` Output joint 4 wrist angle. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if target unreachable. 





        

<hr>
## Public Static Functions Documentation




### function delta\_calc\_arm 

_Helper to solve single Delta arm angle using 64-bit arithmetic._ 
```C++
static SYN_Status delta_calc_arm (
    q16_t x0,
    q16_t y0,
    q16_t z0,
    q16_t L,
    q16_t l,
    q16_t * out_theta
) 
```





**Parameters:**


* `x0` Projected X coordinate. 
* `y0` Projected Y coordinate. 
* `z0` Projected Z coordinate. 
* `L` Upper arm length. 
* `l` Lower arm parallelogram length. 
* `out_theta` Pointer to receive solved arm angle. 



**Returns:**

SYN\_OK on success, SYN\_ERROR if target is unreachable. 





        

<hr>



### function delta\_fk\_solve\_z 

_Solve Delta FK quadratic a\*z^2 + b\*z + c = 0 (lower root) without 64-bit overflow._ 
```C++
static SYN_Status delta_fk_solve_z (
    int64_t a,
    int64_t b,
    int64_t c,
    int64_t * out_z
) 
```





**Parameters:**


* `a` Quadratic coefficient in Q48.16 (&gt; 0). 
* `b` Linear coefficient in Q48.16. 
* `c` Constant coefficient in Q48.16. 
* `out_z` Output lower Z intersection root in Q48.16. 



**Returns:**

SYN\_OK on real intersection, SYN\_ERROR if discriminant &lt; 0. 





        

<hr>



### function delta\_prescale\_dw 

_Prescale (dw21, dw31, dd\_b) together so 3-term sphere products fit in int64\_t._ 
```C++
static void delta_prescale_dw (
    int64_t * dw21,
    int64_t * dw31,
    int64_t * dd_b
) 
```





**Parameters:**


* `dw21` In/out w2 - w1 difference. 
* `dw31` In/out w3 - w1 difference. 
* `dd_b` In/out denominator copy. 




        

<hr>



### function delta\_q16\_vec3\_ok 

_Check whether three 64-bit Q48.16 values fit within 32-bit signed Q16.16 range._ 
```C++
static bool delta_q16_vec3_ok (
    int64_t u,
    int64_t v,
    int64_t w
) 
```





**Parameters:**


* `u` First coordinate in Q48.16. 
* `v` Second coordinate in Q48.16. 
* `w` Third coordinate in Q48.16. 



**Returns:**

true if all three values lie within [INT32\_MIN, INT32\_MAX]. 





        

<hr>



### function delta\_slope\_ok 

_Check whether radical-line coefficients fit inside 64-bit quadratic bounds._ 
```C++
static bool delta_slope_ok (
    int64_t c1,
    int64_t c2
) 
```





**Parameters:**


* `c1` First coefficient. 
* `c2` Second coefficient. 



**Returns:**

true if both lie in [-(2^29), +(2^29)]. 





        

<hr>



### function hypot2d\_q16 

_Overflow-safe 2D Euclidean distance sqrt(x^2 + y^2) in Q16.16._ 
```C++
static q16_t hypot2d_q16 (
    q16_t x,
    q16_t y
) 
```





**Parameters:**


* `x` X coordinate in Q16.16. 
* `y` Y coordinate in Q16.16. 



**Returns:**

Euclidean norm in Q16.16 (clamped to INT32\_MAX). 





        

<hr>



### function kin\_out\_mat44\_valid 

_Validate optional 4x4 homogeneous transform output matrix descriptor._ 
```C++
static bool kin_out_mat44_valid (
    const SYN_Matrix * mat
) 
```





**Parameters:**


* `mat` Optional matrix descriptor (NULL permitted). 



**Returns:**

true if NULL or valid 4x4 matrix with non-NULL backing storage. 





        

<hr>



### function rot\_matrix\_to\_rpy 

_Extract Roll-Pitch-Yaw Euler angles from 3x3 rotation matrix._ 
```C++
static void rot_matrix_to_rpy (
    const q16_t r33,
    SYN_Orientation3D * out_ori
) 
```





**Parameters:**


* `r33` Flat 9-element Q16 array. 
* `out_ori` Output 3D orientation. 




        

<hr>



### function rpy\_to\_rot\_matrix 

_Build 3x3 rotation matrix from Roll-Pitch-Yaw Euler angles (Z-Y-X order)._ 
```C++
static void rpy_to_rot_matrix (
    q16_t roll,
    q16_t pitch,
    q16_t yaw,
    q16_t r33
) 
```





**Parameters:**


* `roll` Roll angle (rad). 
* `pitch` Pitch angle (rad). 
* `yaw` Yaw angle (rad). 
* `r33` Flat 9-element Q16 array for 3x3 rotation matrix. 




        

<hr>



### function solve\_2link\_planar 

_Solve 2-link planar inverse kinematics using 64-bit cosine law._ 
```C++
static SYN_Status solve_2link_planar (
    q16_t rx,
    q16_t ry,
    q16_t l1,
    q16_t l2,
    SYN_ArmElbow elbow,
    q16_t * out_q1,
    q16_t * out_q2
) 
```





**Parameters:**


* `rx` Target X in planar arm frame (Q16.16). 
* `ry` Target Y in planar arm frame (Q16.16). 
* `l1` First link length (Q16.16, &gt; 0). 
* `l2` Second link length (Q16.16, &gt; 0). 
* `elbow` Elbow configuration (up or down). 
* `out_q1` Output first joint angle (rad, Q16.16). 
* `out_q2` Output second joint relative angle (rad, Q16.16). 



**Returns:**

SYN\_OK on success, SYN\_ERROR if target is outside workspace. 





        

<hr>



### function solve\_spherical\_wrist 

_Extract Z-Y-Z spherical wrist angles (q4, q5, q6) from R\_36 = R\_03^T \* R\_06._ 
```C++
static void solve_spherical_wrist (
    const q16_t r03,
    const q16_t r06,
    bool wrist_flip,
    q16_t out_q
) 
```





**Parameters:**


* `r03` 3x3 rotation matrix from base to link 3. 
* `r06` 3x3 target end-effector rotation matrix. 
* `wrist_flip` Invert wrist pitch sign if true. 
* `out_q` Output joint array (writes indices 3, 4, 5). 




        

<hr>
## Macro Definition Documentation





### define Q16\_COS120 

```C++
#define Q16_COS120 `(( q16_t ) - 32768)`
```



cos(120 deg) in Q16 


        

<hr>



### define Q16\_COS240 

```C++
#define Q16_COS240 `(( q16_t ) - 32768)`
```



cos(240 deg) in Q16 


        

<hr>



### define Q16\_SIN120 

```C++
#define Q16_SIN120 `56756`
```



sin(120 deg) in Q16 


        

<hr>



### define Q16\_SIN240 

```C++
#define Q16_SIN240 `(( q16_t ) - 56756)`
```



sin(240 deg) in Q16 


        

<hr>



### define Q16\_SQRT3\_2 

```C++
#define Q16_SQRT3_2 `56756`
```



sqrt(3)/2 ≈ 0.866025 in Q16 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_kinematics.c`

