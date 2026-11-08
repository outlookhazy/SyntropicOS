/**
 * @file syn_flight_pid.h
 * @brief Zero-Heap 3-Axis Quadcopter Flight PID Stabilization & Motor Mixer.
 *
 * Architecture:
 * - Cascaded Dual-Loop Control:
 *   - Outer Angle Loop: Desired angle (deg) -> Gyro Rate Setpoint (deg/s).
 *   - Inner Rate Loop:  Gyro Rate Error -> Motor Axis Torque Output (Roll, Pitch, Yaw).
 * - Quadcopter X-Configuration Motor Mixer:
 *   - Motor 1 (Front-Right, CCW): Throttle - Roll + Pitch + Yaw
 *   - Motor 2 (Rear-Right,  CW):  Throttle - Roll - Pitch - Yaw
 *   - Motor 3 (Rear-Left,  CCW):  Throttle + Roll - Pitch + Yaw
 *   - Motor 4 (Front-Left,  CW):  Throttle + Roll + Pitch - Yaw
 *
 * All operations use Q16.16 fixed-point arithmetic. Zero float, zero heap.
 */

#ifndef SYN_FLIGHT_PID_H
#define SYN_FLIGHT_PID_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/util/syn_qmath.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** IMU Sensor Measurement Inputs in Q16.16. */
typedef struct {
    q16_t gyro_roll;   /**< Measured Roll rate in deg/s (Q16.16). */
    q16_t gyro_pitch;  /**< Measured Pitch rate in deg/s (Q16.16). */
    q16_t gyro_yaw;    /**< Measured Yaw rate in deg/s (Q16.16). */
    q16_t angle_roll;  /**< Measured Roll angle in deg (Q16.16). */
    q16_t angle_pitch; /**< Measured Pitch angle in deg (Q16.16). */
} SYN_Flight_IMU;

/** Pilot Command Inputs in Q16.16. */
typedef struct {
    uint16_t throttle_us; /**< Base throttle in microseconds (1000..2000 us). */
    q16_t roll_target;    /**< Desired Roll angle or rate (Q16.16). */
    q16_t pitch_target;   /**< Desired Pitch angle or rate (Q16.16). */
    q16_t yaw_target;     /**< Desired Yaw rate (Q16.16). */
    bool angle_mode;      /**< True = Angle self-leveling mode, False = Acro rate mode. */
} SYN_Flight_Commands;

/** Quadcopter X Motor Output Command in Microseconds (us). */
typedef struct {
    uint16_t m1; /**< Front-Right Motor (1000..2000 us). */
    uint16_t m2; /**< Rear-Right Motor (1000..2000 us). */
    uint16_t m3; /**< Rear-Left Motor (1000..2000 us). */
    uint16_t m4; /**< Front-Left Motor (1000..2000 us). */
} SYN_Flight_MotorOutputs;

/** 3-Axis Flight PID Controller Instance. */
typedef struct {
    SYN_PID pid_rate_roll;   /**< Inner rate loop PID controller for Roll axis */
    SYN_PID pid_rate_pitch;  /**< Inner rate loop PID controller for Pitch axis */
    SYN_PID pid_rate_yaw;    /**< Inner rate loop PID controller for Yaw axis */
    SYN_PID pid_angle_roll;  /**< Outer angle loop PID controller for Roll axis */
    SYN_PID pid_angle_pitch; /**< Outer angle loop PID controller for Pitch axis */
} SYN_Flight_Controller;

/**
 * @brief Initialize 3-axis flight PID controller instance.
 *
 * @param fc Pointer to flight controller struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_flight_init(SYN_Flight_Controller *fc);

/**
 * @brief Step 3-axis flight controller and compute Quad-X motor outputs.
 *
 * @param fc      Pointer to flight controller struct.
 * @param imu     Pointer to IMU measurements.
 * @param cmd     Pointer to pilot command inputs.
 * @param dt_ms   Time step in milliseconds (e.g. 1ms = 1kHz loop).
 * @param motors  Pointer to output motor commands struct.
 * @return SYN_OK on success.
 */
SYN_Status syn_flight_update(SYN_Flight_Controller *fc, const SYN_Flight_IMU *imu,
                             const SYN_Flight_Commands *cmd, uint32_t dt_ms,
                             SYN_Flight_MotorOutputs *motors);

#ifdef __cplusplus
}
#endif

#endif /* SYN_FLIGHT_PID_H */
