# Control & Motor Modules

SyntropicOS provides integer-only control algorithms, closed-loop PID controllers, trapezoidal motion profiling, and Field-Oriented Control (FOC) for BLDC/PMSM motors.

---

## Technical Specifications

| Feature | Specification |
|---|---|
| **Math Format** | Fixed-point Q16.16 (`q16_t`) or normalized integer scaling. |
| **FPU Requirement** | **None**. Runs on ARM Cortex-M0/M3/M4, RISC-V, and 8-bit MCUs. |
| **Safety Features** | Anti-windup clamping, derivative filtering, stall detection, & limit events. |

---

## 1. Closed-Loop PID Controller (`control/syn_pid.h`)

The `syn_pid` module provides an integer-only PID controller with anti-windup clamping and derivative-term filtering.

### Control Loop Diagram

```mermaid
flowchart LR
    Setpoint["Target Setpoint"] --> ErrorCalc["Error = Setpoint - Feedback"]
    Feedback --> ErrorCalc
    ErrorCalc --> PID["SYN_PID (Kp, Ki, Kd)"]
    PID --> AntiWindup["Anti-Windup Clamp (Min/Max Out)"]
    AntiWindup --> Actuator["PWM / Motor Drive Output"]
```

### Complete Code Example (Integer PID Temperature / Speed Control)

```c
#include <syntropic/control/syn_pid.h>

static SYN_PID pid;

void control_init(void) {
    // Configure PID gains: kp = 2.5, ki = 0.5/s, kd = 0.1, scale = 256, Output: 0..1000
    SYN_PID_Config cfg = SYN_PID_GAINS(2.5, 0.5, 0.1, 256, 0, 1000);
    cfg.integral_max = 500;
    syn_pid_init(&pid, &cfg);
}

uint16_t control_step(int32_t setpoint, int32_t current_reading, uint32_t dt_ms) {
    // Compute control output step
    int32_t output = syn_pid_update(&pid, setpoint, current_reading, dt_ms);
    return (uint16_t)output;
}
```

---

## 2. Stepper Motor Control (`motor/syn_stepper.h`)

Provides Step/Direction motor driving with trapezoidal velocity acceleration profiles.

```c
#include <syntropic/motor/syn_stepper.h>

static SYN_Stepper stepper;

void stepper_init(void) {
    // Initialize pin 4 (step), pin 5 (dir), max_speed = 1000 steps/s, accel = 500 steps/s²
    syn_stepper_init(&stepper, 4, 5);
    syn_stepper_set_speed(&stepper, 1000, 500);
}

void move_relative(int32_t steps) {
    syn_stepper_move(&stepper, steps);
}

void main_timer_isr(void) {
    // Serves step generation (call at fixed interrupt interval e.g. 10 kHz)
    syn_stepper_tick(&stepper);
}
```

---

## 3. Field-Oriented Control (FOC) (`motor/syn_foc.h`)

Provides Clarke/Park transforms, Space Vector PWM (SVPWM), and Sliding Mode Observers (SMO) for sensorless BLDC/PMSM motor control in pure Q16.16 fixed-point math.

```c
#include <syntropic/motor/syn_foc.h>
#include <syntropic/motor/syn_foc_observer.h>

static SYN_FOCObserver observer;

void foc_setup(void) {
    SYN_FOCObserverConfig cfg = {
        .R = Q16_FROM_FLOAT(0.5f),    // 0.5 Ohm phase resistance
        .L = Q16_FROM_FLOAT(0.001f),  // 1 mH phase inductance
        .G = Q16_FROM_INT(10),        // Sliding mode observer gain
        .dt = Q16_FROM_FLOAT(0.0001f),// 10 kHz sampling period (100 µs)
        .Kp_pll = Q16_FROM_INT(150),
        .Ki_pll = Q16_FROM_INT(6000)
    };
    syn_foc_observer_init(&observer, &cfg);
}

void foc_10khz_isr(q16_t v_alpha, q16_t v_beta, q16_t i_alpha, q16_t i_beta) {
    // 1. Update Sliding Mode Observer with phase voltages and currents
    syn_foc_observer_update(&observer, v_alpha, v_beta, i_alpha, i_beta);

    // 2. Read estimated rotor angle theta_e [0, 2pi) and speed omega_e
    q16_t est_angle = syn_foc_observer_get_angle(&observer);
    q16_t est_speed = syn_foc_observer_get_speed(&observer);
}
```

---

## 4. G-Code & NIST RS-274 Motion Interpreter (`motor/syn_gcode.h`)

Zero-heap streaming ASCII G-code parser and multi-axis trajectory coordinator:
- **Parser**: Streaming word extraction (`G`, `M`, `X`, `Y`, `Z`, `I`, `J`, `K`, `F`, `S`, `P`, `T`).
- **Modal Engine**: G0/G1/G2/G3/G4, G17/G18/G19 planes, G20/G21 units, G90/G91 distance modes, G92 offsets, M3/M4/M5 spindle control, M7/M8/M9 coolant.
- **Interpolator Integration**: Automatic trajectory planning via `SYN_Interpolator` with S-curve/trapezoidal velocity profiles.

```c
#include <syntropic/motor/syn_gcode.h>
#include <syntropic/motor/syn_interpolator.h>

static SYN_Interpolator g_interp;
static SYN_GCode_Controller g_gcode_ctrl;

void gcode_setup(void) {
    SYN_GCode_Config cfg = {
        .interpolator = &g_interp,
        .default_feedrate = 200.0f,
        .max_acceleration = 1000.0f,
        .max_jerk = 5000.0f,
        .step_resolution = 0.001f,
    };
    syn_gcode_init(&g_gcode_ctrl, &cfg);

    /* Stream G-code blocks */
    syn_gcode_execute_line(&g_gcode_ctrl, "G21 G90 G0 X0 Y0 Z5");
    syn_gcode_execute_line(&g_gcode_ctrl, "G1 Z-1 F100");
    syn_gcode_execute_line(&g_gcode_ctrl, "G2 X10 Y10 I5 J0 F300");
}
```

---

## 5. Multi-Axis Robot Kinematics (`motor/syn_kinematics.h`)

Zero-heap integer Q16.16 multi-axis robot kinematics engine:
- **Transforms & Conventions**: Standard and Modified Denavit-Hartenberg (DH) 4x4 matrix transforms.
- **Forward Kinematics**: Arbitrary N-DOF serial manipulator forward kinematics with 3D position and roll/pitch/yaw Euler orientations.
- **Differential Kinematics**: Exact 6xN Geometric Jacobian computation for velocity mapping and singularity analysis.
- **Standard Robot Models**:
  - 3-DOF Planar Articulated Arm (FK & analytical IK with Elbow-Up / Elbow-Down selection).
  - 4-DOF SCARA Robot (FK & analytical IK with tool pitch/yaw).
  - 6-DOF PUMA Manipulator with Spherical Wrist (FK & Pieper analytical decoupling IK).
  - 3-Axis Delta Parallel Robot (FK via 3-sphere intersection & analytical IK).

```c
#include <syntropic/motor/syn_kinematics.h>

void scara_pick_and_place_demo(void) {
    SYN_Kinematics_SCARAConfig cfg = {
        .l1 = Q16_FROM_INT(200),     /* 200 mm inner arm */
        .l2 = Q16_FROM_INT(150),     /* 150 mm outer arm */
        .d_max = Q16_FROM_INT(100),  /* 100 mm Z stroke */
        .z_home = Q16_FROM_INT(100), /* 100 mm home height */
    };

    /* Define target 6D pose */
    SYN_Pose6D target = {
        .position = {.x = Q16_FROM_INT(250), .y = Q16_FROM_INT(100), .z = Q16_FROM_INT(20)},
        .orientation = {.roll = 0, .pitch = 0, .yaw = Q16_PI / 4}
    };

    q16_t q1 = 0, q2 = 0, d3 = 0, q4 = 0;
    if (syn_kinematics_scara_ik(&cfg, &target, SYN_ARM_ELBOW_UP, &q1, &q2, &d3, &q4) == SYN_OK) {
        /* Drive joint actuators to solved angles */
    }
}
```

---

## 6. Additional Motor Drivers, Closed-Loop Controllers & Flight / RC Control

### Motor Drivers & Closed-Loop Servo/Actuator Stack (`motor/*.h`)
- **Motor Hardware Abstraction (`motor/syn_motor_output.h`, header-only)**: Three-callback vtable (`SYN_MotorOutput`: `set_output`, `coast`, `brake`) decoupling closed-loop controllers from PWM/H-bridge drivers.
- **H-Bridge DC Motor (`motor/syn_dc_motor.h`, `SYN_USE_DC_MOTOR`)**: Drives PWM+Dir or Dual-PWM H-bridges (`SYN_DCMotor`: `syn_dc_motor_init`, `syn_dc_motor_set_speed`, `syn_dc_motor_ramp_to`, `syn_dc_motor_coast`, `syn_dc_motor_brake`, `syn_dc_motor_output`).
- **Hobby Servo (`motor/syn_servo.h`, `SYN_USE_SERVO`)**: Maps degree targets or raw microsecond commands onto `pulse_min..pulse_max` (`SYN_Servo`: `syn_servo_init`, `syn_servo_set_angle`, `syn_servo_move_to`, `syn_servo_update`).
- **6-Step Trapezoidal BLDC (`motor/syn_bldc_6step.h`, `SYN_USE_BLDC_6STEP`)**: Commutates 3-phase bridges from 3-bit Hall states (`SYN_BLDC_6Step`: `syn_bldc_6step_init`, `syn_bldc_6step_set_hall`, `syn_bldc_6step_update_speed`) with invalid-Hall fault detection and closed-loop RPM PID.
- **FOC Rotor Encoder Interface (`motor/syn_foc_encoder.h`)**: Converts ABZ quadrature counts, 3-phase Hall transitions, or 14-bit absolute angles into Q16.16 electrical angle and speed (`syn_foc_encoder_init`, `syn_foc_encoder_get_elec_angle`, `syn_foc_encoder_get_elec_speed`).
- **Closed-Loop Motor Controller (`motor/syn_motor_ctrl.h`, `SYN_USE_MOTOR_CTRL`)**: Combines `SYN_MotorOutput` + position reader with PID, velocity/acceleration feedforward (`ff_kv`, `ff_ka`), trapezoidal and S-curve moves (`syn_motor_ctrl_move_to`, `syn_motor_ctrl_move_to_scurve`), soft travel limits, and stall detection.
- **Linear Actuator (`motor/syn_actuator.h`, `SYN_USE_ACTUATOR`)**: Closed-loop stroke positioner (`0..1000` = `0.0%..100.0%`) over potentiometer ADC feedback (`syn_actuator_init`, `syn_actuator_set_position`, `syn_actuator_update`).
- **Multi-Axis Trajectory Interpolator (`motor/syn_interpolator.h`, `SYN_USE_INTERPOLATOR`)**: Coordinated 2D/3D linear, XY/ZX/YZ helical circular arc, and cubic Bezier S-curve motion planner (`syn_interpolator_plan_linear`, `syn_interpolator_plan_circular`, `syn_interpolator_plan_circular_plane`, `syn_interpolator_plan_bezier`, `syn_interpolator_step`).

### Auto-Tuning, Loop Diagnostics & Flight / RC Control (`control/*.h`)
- **Relay & Feedforward Auto-Tuner (`control/syn_autotune.h`, `SYN_USE_AUTOTUNE`)**: Identifies `ff_kv`/`ff_ka` and Ziegler-Nichols or Tyreus-Luyben PID gains on `SYN_MotorCtrl` within bounded travel/watchdog limits (`syn_autotune_init`, `syn_autotune_start`, `syn_autotune_update`, `syn_autotune_apply`).
- **Control Loop Statistics (`control/syn_control_stats.h`)**: Tracks RMS error, peak error, RMS effort, output slew jitter, and ITAE (`syn_control_stats_update`, `syn_control_stats_report`).
- **Cascaded 3-Axis Flight PID & Quad-X Mixer (`control/syn_flight_pid.h`, `SYN_USE_FLIGHT_PID`)**: Outer-angle + inner-rate Q16.16 roll/pitch/yaw loops mixing into 4 motor `1000..2000 us` outputs (`syn_flight_init`, `syn_flight_update`).
- **RC Stick Curve & Link Failsafe (`control/syn_rc_curve.h`, `SYN_USE_RC_CURVE`; `control/syn_rc_failsafe.h`, `SYN_USE_RC_FAILSAFE`)**: Applies deadband, cubic expo, and dual-rate scaling (`syn_rc_curve_apply`) and enforces per-channel hold/preset/disarm actions on receiver timeout (`syn_failsafe_init`, `syn_failsafe_feed_frame`, `syn_failsafe_step`).


