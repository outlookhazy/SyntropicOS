/**
 * @file syn_sensor_fusion.h
 * @brief 6-DOF IMU Sensor Fusion & AHRS Complementary / Mahony Filter (Q16.16 fixed-point).
 */

#ifndef SYN_SENSOR_FUSION_H
#define SYN_SENSOR_FUSION_H

#include "../common/syn_defs.h"
#include "../util/syn_qmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 3D Quaternion representation (w + xi + yj + zk). */
typedef struct {
    q16_t w; /**< Scalar component (w) */
    q16_t x; /**< Vector x component  */
    q16_t y; /**< Vector y component  */
    q16_t z; /**< Vector z component  */
} SYN_Quaternion;

/** @brief 3D Euler attitude angles (in radians, Q16.16). */
typedef struct {
    q16_t roll_rad;  /**< Roll angle around X axis (-pi to +pi)  */
    q16_t pitch_rad; /**< Pitch angle around Y axis (-pi/2 to +pi/2) */
    q16_t yaw_rad;   /**< Yaw angle around Z axis (-pi to +pi)   */
} SYN_EulerAngles;

/** @brief Mahony 6-DOF IMU Sensor Fusion state structure. */
typedef struct {
    q16_t Kp;         /**< Proportional feedback gain           */
    q16_t Ki;         /**< Integral error feedback gain        */
    q16_t dt;         /**< Sampling period in seconds (Q16.16) */

    SYN_Quaternion q; /**< Current estimated quaternion orientation */
    q16_t e_int[3];   /**< Integral error vector accumulator    */
} SYN_SensorFusion;

/**
 * @brief Initialize the IMU sensor fusion filter.
 * @param f   Filter instance.
 * @param Kp  Proportional error gain (e.g., Q16_FROM_FLOAT(2.0)).
 * @param Ki  Integral error gain (e.g., Q16_FROM_FLOAT(0.005)).
 * @param dt  Sampling interval in seconds (e.g., Q16_FROM_FLOAT(0.01) for 100 Hz).
 */
void syn_sensor_fusion_init(SYN_SensorFusion *f, q16_t Kp, q16_t Ki, q16_t dt);

/**
 * @brief Update orientation estimate using 6-axis IMU sample.
 *
 * @param f   Filter instance.
 * @param gx  Gyroscope X rate in rad/s (Q16.16).
 * @param gy  Gyroscope Y rate in rad/s (Q16.16).
 * @param gz  Gyroscope Z rate in rad/s (Q16.16).
 * @param ax  Accelerometer X acceleration in g or m/s² (Q16.16).
 * @param ay  Accelerometer Y acceleration in g or m/s² (Q16.16).
 * @param az  Accelerometer Z acceleration in g or m/s² (Q16.16).
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_sensor_fusion_update(SYN_SensorFusion *f, q16_t gx, q16_t gy, q16_t gz,
                                     q16_t ax, q16_t ay, q16_t az);

/**
 * @brief Retrieve current quaternion orientation estimate.
 * @param f Filter instance.
 * @param q Output quaternion pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_sensor_fusion_get_quaternion(const SYN_SensorFusion *f, SYN_Quaternion *q);

/**
 * @brief Retrieve current Euler attitude angles (Roll, Pitch, Yaw).
 * @param f     Filter instance.
 * @param euler Output Euler angles pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_sensor_fusion_get_euler(const SYN_SensorFusion *f, SYN_EulerAngles *euler);

/**
 * @brief Reset filter state to initial identity orientation (q = [1, 0, 0, 0]).
 * @param f Filter instance.
 */
void syn_sensor_fusion_reset(SYN_SensorFusion *f);

#ifdef __cplusplus
}
#endif

#endif /* SYN_SENSOR_FUSION_H */
