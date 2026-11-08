/**
 * @file syn_quaternion.h
 * @brief Fixed-point Q16.16 3D Quaternion operations — zero heap allocation.
 * @ingroup syn_util
 */

#ifndef SYN_QUATERNION_H
#define SYN_QUATERNION_H

#include "../common/syn_defs.h"
#include "syn_matrix.h"
#include "syn_qmath.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_QUATERNION_TYPE_DEFINED
#define SYN_QUATERNION_TYPE_DEFINED /**< Guard for SYN_Quaternion typedef */
/**
 * @brief Fixed-point Q16.16 Quaternion representation q = w + xi + yj + zk.
 */
typedef struct {
    q16_t w; /**< Scalar (real) component */
    q16_t x; /**< Vector i component      */
    q16_t y; /**< Vector j component      */
    q16_t z; /**< Vector k component      */
} SYN_Quaternion;
#endif

/**
 * @brief Initialize identity quaternion q = (1, 0, 0, 0).
 * @param q Pointer to quaternion instance.
 */
void syn_quat_identity(SYN_Quaternion *q);

/**
 * @brief Initialize quaternion with given components.
 * @param q Pointer to quaternion instance.
 * @param w Scalar (real) component.
 * @param x Vector i component.
 * @param y Vector j component.
 * @param z Vector k component.
 */
void syn_quat_init(SYN_Quaternion *q, q16_t w, q16_t x, q16_t y, q16_t z);

/**
 * @brief Quaternion multiplication q_out = q1 * q2 (Hamilton product).
 * @param q1   First quaternion operand.
 * @param q2   Second quaternion operand.
 * @param out  Output product quaternion. May alias q1 or q2.
 */
void syn_quat_mul(const SYN_Quaternion *q1, const SYN_Quaternion *q2, SYN_Quaternion *out);

/**
 * @brief Calculate quaternion norm (magnitude).
 * @param q Pointer to input quaternion.
 * @return Magnitude |q| in Q16.16 format.
 */
q16_t syn_quat_norm(const SYN_Quaternion *q);

/**
 * @brief Normalize quaternion to unit length.
 * @param q Pointer to quaternion to normalize.
 * @return SYN_OK on success, SYN_ERROR if zero magnitude.
 */
SYN_Status syn_quat_normalize(SYN_Quaternion *q);

/**
 * @brief Calculate quaternion conjugate q* = (w, -x, -y, -z).
 * @param q   Pointer to input quaternion.
 * @param out Destination conjugate quaternion.
 */
void syn_quat_conjugate(const SYN_Quaternion *q, SYN_Quaternion *out);

/**
 * @brief Calculate quaternion inverse q^-1 = q* / |q|^2.
 * @param q   Pointer to input quaternion.
 * @param out Destination inverse quaternion.
 * @return SYN_OK on success, SYN_ERROR if zero magnitude.
 */
SYN_Status syn_quat_inverse(const SYN_Quaternion *q, SYN_Quaternion *out);

/**
 * @brief Rotate a 3D vector v by unit quaternion q: v_out = q * v * q*.
 * @param q    Unit quaternion rotation.
 * @param v    Input 3D vector (3 elements).
 * @param out  Output rotated 3D vector (3 elements). May alias v.
 */
void syn_quat_rotate_vec3(const SYN_Quaternion *q, const q16_t *v, q16_t *out);

/**
 * @brief Convert unit quaternion to 3x3 rotation matrix.
 * @param q    Unit quaternion.
 * @param out  Output 3x3 rotation matrix.
 */
void syn_quat_to_mat3x3(const SYN_Quaternion *q, SYN_Matrix *out);

/**
 * @brief Create orientation quaternion from Euler angles (roll, pitch, yaw) in Q16 radians (Z-Y-X
 * sequence).
 * @param q     Destination quaternion.
 * @param roll  Roll angle in Q16 radians.
 * @param pitch Pitch angle in Q16 radians.
 * @param yaw   Yaw angle in Q16 radians.
 */
void syn_quat_from_euler(SYN_Quaternion *q, q16_t roll, q16_t pitch, q16_t yaw);

/**
 * @brief Extract Euler angles (roll, pitch, yaw) in Q16 radians from unit quaternion.
 * @param q     Input unit quaternion.
 * @param roll  Output pointer for roll angle in Q16 radians.
 * @param pitch Output pointer for pitch angle in Q16 radians.
 * @param yaw   Output pointer for yaw angle in Q16 radians.
 */
void syn_quat_to_euler(const SYN_Quaternion *q, q16_t *roll, q16_t *pitch, q16_t *yaw);

/**
 * @brief Spherical linear interpolation (SLERP) between q1 and q2 by factor t (0.0 to 1.0 in Q16).
 * @param q1   Start unit quaternion (t = 0).
 * @param q2   End unit quaternion (t = 1).
 * @param t    Interpolation parameter [0, Q16_ONE].
 * @param out  Output interpolated unit quaternion.
 */
void syn_quat_slerp(const SYN_Quaternion *q1, const SYN_Quaternion *q2, q16_t t,
                    SYN_Quaternion *out);

#ifdef __cplusplus
}
#endif

#endif /* SYN_QUATERNION_H */
