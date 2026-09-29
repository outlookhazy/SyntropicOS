

# File syn\_flight\_pid.c

[**File List**](files.md) **>** [**control**](dir_d63400c13859c7c99cca12fba8ce6e16.md) **>** [**syn\_flight\_pid.c**](syn__flight__pid_8c.md)

[Go to the documentation of this file](syn__flight__pid_8c.md)


```C++

#include "syntropic/control/syn_flight_pid.h"

#include <string.h>

SYN_Status syn_flight_init(SYN_Flight_Controller *fc)
{
    if (!fc) {
        return SYN_INVALID_PARAM;
    }

    memset(fc, 0, sizeof(*fc));

    /* Initialize Rate Loop PID gains (Roll, Pitch, Yaw) */
    SYN_PID_Config rate_roll_cfg = SYN_PID_GAINS(1.5f, 0.2f, 0.05f, 100, -500, 500);
    SYN_PID_Config rate_pitch_cfg = SYN_PID_GAINS(1.5f, 0.2f, 0.05f, 100, -500, 500);
    SYN_PID_Config rate_yaw_cfg = SYN_PID_GAINS(2.0f, 0.1f, 0.0f, 100, -500, 500);

    /* Initialize Outer Angle Loop P-Gains */
    SYN_PID_Config angle_roll_cfg = SYN_PID_GAINS(4.5f, 0.0f, 0.0f, 100, -200, 200);
    SYN_PID_Config angle_pitch_cfg = SYN_PID_GAINS(4.5f, 0.0f, 0.0f, 100, -200, 200);

    syn_pid_init(&fc->pid_rate_roll, &rate_roll_cfg);
    syn_pid_init(&fc->pid_rate_pitch, &rate_pitch_cfg);
    syn_pid_init(&fc->pid_rate_yaw, &rate_yaw_cfg);
    syn_pid_init(&fc->pid_angle_roll, &angle_roll_cfg);
    syn_pid_init(&fc->pid_angle_pitch, &angle_pitch_cfg);

    return SYN_OK;
}

static uint16_t clamp_us(int32_t val)
{
    if (val < 1000) {
        return 1000U;
    }
    if (val > 2000) {
        return 2000U;
    }
    return (uint16_t)val;
}

SYN_Status syn_flight_update(SYN_Flight_Controller *fc, const SYN_Flight_IMU *imu,
                             const SYN_Flight_Commands *cmd, uint32_t dt_ms,
                             SYN_Flight_MotorOutputs *motors)
{
    if (!fc || !imu || !cmd || !motors) {
        return SYN_INVALID_PARAM;
    }

    q16_t roll_rate_setpoint = cmd->roll_target;
    q16_t pitch_rate_setpoint = cmd->pitch_target;
    q16_t yaw_rate_setpoint = cmd->yaw_target;

    /* Outer Angle Loop (Self-Leveling Mode) */
    if (cmd->angle_mode) {
        int32_t roll_sp =
            syn_pid_update(&fc->pid_angle_roll, cmd->roll_target, imu->angle_roll, dt_ms);
        int32_t pitch_sp =
            syn_pid_update(&fc->pid_angle_pitch, cmd->pitch_target, imu->angle_pitch, dt_ms);
        roll_rate_setpoint = (q16_t)roll_sp;
        pitch_rate_setpoint = (q16_t)pitch_sp;
    }

    /* Inner Gyro Rate Loop */
    int32_t u_roll = syn_pid_update(&fc->pid_rate_roll, roll_rate_setpoint, imu->gyro_roll, dt_ms);
    int32_t u_pitch =
        syn_pid_update(&fc->pid_rate_pitch, pitch_rate_setpoint, imu->gyro_pitch, dt_ms);
    int32_t u_yaw = syn_pid_update(&fc->pid_rate_yaw, yaw_rate_setpoint, imu->gyro_yaw, dt_ms);

    /* Convert Q16.16 torque commands to microsecond pulse offsets */
    int32_t r = Q16_TO_INT((q16_t)u_roll);
    int32_t p = Q16_TO_INT((q16_t)u_pitch);
    int32_t y = Q16_TO_INT((q16_t)u_yaw);

    int32_t base_throttle = (int32_t)cmd->throttle_us;

    /* Quad-X Motor Mixer */
    motors->m1 = clamp_us(base_throttle - r + p + y); /* Front-Right (CCW) */
    motors->m2 = clamp_us(base_throttle - r - p - y); /* Rear-Right  (CW)  */
    motors->m3 = clamp_us(base_throttle + r - p + y); /* Rear-Left   (CCW) */
    motors->m4 = clamp_us(base_throttle + r + p - y); /* Front-Left  (CW)  */

    return SYN_OK;
}
```


