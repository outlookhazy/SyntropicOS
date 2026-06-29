# Control & Motor

## PID Controller

| Module | Header | Config |
|---|---|---|
| PID | `control/syn_pid.h` | `SYN_USE_PID` |
| Auto-Tune | `control/syn_autotune.h` | `SYN_USE_AUTOTUNE` |

General-purpose PID controller with anti-windup and derivative-term filtering. Integer-only math — no floating point.

The **Auto-Tune** module provides automatic PID gain tuning (requires `SYN_USE_MOTOR_CTRL`).

## Motor Drivers

| Module | Header | Config | Description |
|---|---|---|---|
| Stepper | `motor/syn_stepper.h` | `SYN_USE_STEPPER` | Step/dir stepper motor driver with trapezoidal acceleration profiles |
| Servo | `motor/syn_servo.h` | `SYN_USE_SERVO` | Hobby servo positioning via pulse-width control, with smooth move support |
| DC Motor | `motor/syn_dc_motor.h` | `SYN_USE_DC_MOTOR` | H-bridge DC motor control: PWM+direction or dual-PWM mode, with speed ramping |
| Motor Ctrl | `motor/syn_motor_ctrl.h` | `SYN_USE_MOTOR_CTRL` | Closed-loop position/velocity controller: PID + feedback + stall detection. Stall and limit events auto-record to `syn_errlog` if configured. |
| Actuator | `motor/syn_actuator.h` | `SYN_USE_ACTUATOR` | Linear actuator controller (requires PID) |
