

# File syn\_kinematics.h



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_kinematics.h**](syn__kinematics_8h.md)

[Go to the source code of this file](syn__kinematics_8h_source.md)

_Multi-Axis Robot Forward & Inverse Kinematics Engine (Q16.16 fixed-point)._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../util/syn_matrix.h"`
* `#include "../util/syn_qmath.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DH\_Param**](structSYN__DH__Param.md) <br>_Single joint Denavit-Hartenberg parameter specification._  |
| struct | [**SYN\_Kinematics\_6DOFConfig**](structSYN__Kinematics__6DOFConfig.md) <br>_Configuration parameters for 6-DOF Articulated Arm with Spherical Wrist._  |
| struct | [**SYN\_Kinematics\_DeltaConfig**](structSYN__Kinematics__DeltaConfig.md) <br>_Configuration parameters for 3-Axis Delta Parallel Robot._  |
| struct | [**SYN\_Kinematics\_Planar3Config**](structSYN__Kinematics__Planar3Config.md) <br>_Configuration parameters for 3-DOF Planar Arm._  |
| struct | [**SYN\_Kinematics\_SCARAConfig**](structSYN__Kinematics__SCARAConfig.md) <br>_Configuration parameters for 4-DOF SCARA Robot._  |
| struct | [**SYN\_Orientation3D**](structSYN__Orientation3D.md) <br>_3D Orientation (Roll, Pitch, Yaw Euler angles) in Q16.16 radians._  |
| struct | [**SYN\_Pose6D**](structSYN__Pose6D.md) <br>_Complete 6-DOF Spatial Pose (Position + Orientation)._  |
| struct | [**SYN\_Position3D**](structSYN__Position3D.md) <br>_3D Cartesian Position (X, Y, Z) in Q16.16._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_ArmElbow**](#enum-syn_armelbow)  <br>_Elbow configuration for 2D/3D arm inverse kinematics._  |
| enum  | [**SYN\_DH\_Convention**](#enum-syn_dh_convention)  <br>_Denavit-Hartenberg parameter convention._  |




















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



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_KINEMATICS\_MAX\_JOINTS**](syn__kinematics_8h.md#define-syn_kinematics_max_joints)  `8U`<br> |

## Detailed Description


Provides a zero-heap robotics kinematics engine for embedded microcontrollers:
* Standard & Modified Denavit-Hartenberg (DH) parameter chain transforms.
* Multi-joint forward kinematics with position vector and orientation extraction.
* Closed-form inverse kinematics solvers for:
  * 3-DOF Planar Articulated Arm.
  * 4-DOF SCARA Robot (X, Y, Z, Yaw) with elbow-left / elbow-right configurations.
  * 6-DOF Articulated Robot Arm with spherical wrist (Pieper's decoupling).
  * 3-Axis Delta Parallel Robot.


* Geometric Jacobian computation (6 x N) for differential velocity kinematics. 




    
## Public Types Documentation




### enum SYN\_ArmElbow 

_Elbow configuration for 2D/3D arm inverse kinematics._ 
```C++
enum SYN_ArmElbow {
    SYN_ARM_ELBOW_UP = 0,
    SYN_ARM_ELBOW_DOWN = 1
};
```




<hr>



### enum SYN\_DH\_Convention 

_Denavit-Hartenberg parameter convention._ 
```C++
enum SYN_DH_Convention {
    SYN_DH_CONVENTION_STANDARD = 0,
    SYN_DH_CONVENTION_MODIFIED = 1
};
```




<hr>
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
## Macro Definition Documentation





### define SYN\_KINEMATICS\_MAX\_JOINTS 

```C++
#define SYN_KINEMATICS_MAX_JOINTS `8U`
```



Maximum supported joints in a kinematic chain 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_kinematics.h`

