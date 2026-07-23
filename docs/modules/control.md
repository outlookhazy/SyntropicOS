# Control & Motor

## PID Controller

| Module | Header | Config |
|---|---|---|
| PID | `control/syn_pid.h` | `SYN_USE_PID` |
| Auto-Tune | `control/syn_autotune.h` | `SYN_USE_AUTOTUNE` |

General-purpose PID controller with anti-windup and derivative-term filtering. Integer-only math — no floating point.

Use `SYN_PID_GAINS(kp, ki, kd, scale, out_min, out_max)` to configure gains from human-readable float values. The macro handles the internal ki×1000 time normalization automatically.

The **Auto-Tune** module provides automatic PID gain tuning and feedforward identification (requires `SYN_USE_MOTOR_CTRL`). It uses a robust 9-stage automated sequence to safely identify motor dynamics and compute optimal gains.

## Motor Drivers

| Module | Header | Config | Description |
|---|---|---|---|
| Motor Output | `motor/syn_motor_output.h` | — | Motor output vtable interface — decouples controllers from specific driver types |
| DC Motor | `motor/syn_dc_motor.h` | `SYN_USE_DC_MOTOR` | H-bridge DC motor control: PWM+direction or dual-PWM mode, with speed ramping and configurable duty range |
| Stepper | `motor/syn_stepper.h` | `SYN_USE_STEPPER` | Step/dir stepper motor driver with trapezoidal acceleration profiles |
| Servo | `motor/syn_servo.h` | `SYN_USE_SERVO` | Hobby servo positioning via pulse-width control, with smooth move support |
| Motor Ctrl | `motor/syn_motor_ctrl.h` | `SYN_USE_MOTOR_CTRL` | Closed-loop position/velocity controller with PID, feedforward, open-loop mode, and built-in trapezoid profiling. Stall and limit events auto-record to `syn_errlog` if configured. |
| Actuator | `motor/syn_actuator.h` | `SYN_USE_ACTUATOR` | Linear actuator controller (requires DC Motor + Motor Ctrl) |
| FOC | `motor/syn_foc.h` | `SYN_USE_FOC` | Field-Oriented Control transforms for BLDC/PMSM motors: Clarke, Park, inverse transforms, and Space Vector PWM (SVPWM). All Q16.16 fixed-point. |
| FOC Observer | `motor/syn_foc_observer.h` | `SYN_USE_FOC_OBSERVER` | Sensorless FOC Rotor Position & Speed Sliding Mode Observer (SMO) with software PLL phase/speed tracking. |

### FOC (Field-Oriented Control) & Observer

The FOC module provides the mathematical transforms for sinusoidal BLDC/PMSM motor control:

- **Clarke**: 3-phase (a,b,c) → 2-phase stationary (α,β)
- **Park**: stationary (α,β) → rotating (d,q) using rotor angle θ
- **Inverse Park/Clarke**: for voltage command generation
- **SVPWM**: Space Vector PWM duty cycle computation for maximum DC bus utilization
- **Sliding Mode Observer (SMO)**: Sensorless estimation of rotor position ($\hat{\theta}_e$) and speed ($\hat{\omega}_e$) from phase voltage and current measurements in Q16.16 fixed-point.

```c
SYN_FOCObserver obs;
SYN_FOCObserverConfig obs_cfg = {
    .R = Q16_HALF,                // 0.5 Ohm phase resistance
    .L = Q16_FROM_FLOAT(0.001),   // 1 mH phase inductance
    .G = Q16_FROM_INT(10),        // Sliding mode gain
    .dt = Q16_FROM_FLOAT(0.0001), // 10 kHz sample time
    .Kp_pll = Q16_FROM_INT(150),  // Software PLL proportional gain
    .Ki_pll = Q16_FROM_INT(6000)  // Software PLL integral gain
};
syn_foc_observer_init(&obs, &obs_cfg);

// In 10 kHz FOC interrupt loop:
syn_foc_observer_update(&obs, v_alpha, v_beta, i_alpha, i_beta);
q16_t est_angle = syn_foc_observer_get_angle(&obs); // Estimated theta_e [0, 2pi)
q16_t est_speed = syn_foc_observer_get_speed(&obs); // Estimated omega_e (rad/s)
```

```c
SYN_FOC_ABC phase_currents = { ia, ib, ic };
SYN_FOC_AB  ab;
SYN_FOC_DQ  dq;
q16_t theta = electrical_angle;  // from encoder or observer

// Forward: measure → control frame
syn_foc_clarke(&phase_currents, &ab);
syn_foc_park(&ab, theta, &dq);

// dq.d = flux current, dq.q = torque current
// Run PID controllers on dq...
SYN_FOC_DQ dq_cmd = { id_pid_output, iq_pid_output };

// Inverse: control frame → PWM
SYN_FOC_AB ab_cmd;
syn_foc_inv_park(&dq_cmd, theta, &ab_cmd);

q16_t duty_a, duty_b, duty_c;
syn_foc_svpwm(&ab_cmd, v_bus, &duty_a, &duty_b, &duty_c);
```

### Motor Output Abstraction

All motor drivers provide a `SYN_MotorOutput` interface via factory functions:

- `syn_dc_motor_output(&motor)` — DC motor
- `syn_stepper_output(&stepper)` — Stepper motor

Pass the returned `SYN_MotorOutput` to `SYN_MotorCtrl_Config.motor` to wire any driver into the closed-loop controller.

### Motor Controller Modes

| Mode | Function | Description |
|---|---|---|
| Velocity | `syn_motor_ctrl_set_velocity()` | Maintain a target velocity via PID |
| Position | `syn_motor_ctrl_set_position()` | Move to a target position via PID |
| Move-To (Linear) | `syn_motor_ctrl_move_to()` | Move with built-in trapezoidal velocity profile. Units are per-second (e.g., units/sec, units/sec²). |
| Move-To (S-Curve) | `syn_motor_ctrl_move_to_scurve()` | Jerk-limited 7-phase move for ultra-smooth motion. Uses native p/v/a feedforward. |
| Trajectory | `syn_motor_ctrl_set_trajectory()` | Feed external profile points for PID + feedforward tracking |
| Open-Loop | `syn_motor_ctrl_set_output()` | Direct motor output, no PID — for jogging, testing, or simple control |

### Ergonomics & Tuning

Use `SYN_MOTOR_CTRL_DEFAULTS()` to initialize a controller with sane, stable P-only defaults. This reduces boilerplate and ensures a safe starting point for tuning.

```c
SYN_MotorCtrl_Config cfg = SYN_MOTOR_CTRL_DEFAULTS(
    syn_dc_motor_output(&motor), 
    encoder_read, 
    NULL, 
    1000,   // 1kHz loop rate
    1000    // max duty
);
cfg.pid_ki = 50; // Add integral if needed
syn_motor_ctrl_init(&ctrl, &cfg);
```

### Automated Tuning

The Auto-Tune module eliminates the "nightmare" of manual tuning by identifying plant dynamics and calculating gains automatically. It is designed to run at the standard **1kHz** loop rate.

#### One-Call Auto-Tuning
Use `syn_autotune_start()` to begin a full automated sequence. The tuner will:
1. **Probe**: Find the minimum output needed to overcome static friction.
2. **FF Ident**: Measure steady-state velocity at two different output points for precise $K_v$ and $K_a$ separation.
3. **Relay**: Perform relay-based oscillation to find ultimate gain/period.
4. **Braking**: Active braking to bring the system to a safe stop before finalizing.

```c
SYN_AutoTune at;
SYN_AutoTune_Limits limits = {
    .position_min = -10000,
    .position_max = 10000,
    .watchdog_ms = 1000
};

// Start the sequence. Identifies FF + PID with an 80% safety margin.
syn_autotune_start(&at, &ctrl, &limits, SYN_ATUNE_FLAG_ALL, 80);

// In your 1kHz loop:
syn_autotune_update(&at);

if (at.state == SYN_ATUNE_DONE) {
    // Controller gains are now updated and applied.
}
```

#### Gain Multiplier Safety
The `gain_multiplier` parameter in `syn_autotune_start()` (percentage, 1-100) allows applying a safety margin to the calculated PID gains. This is highly recommended for systems with variable loads (e.g., a cart that may be empty or full). A multiplier of `80` applies 80% of the calculated ultimate gain, providing a stable buffer against oscillations.

#### Tuning Safety
The tuner monitors position limits and watchdog timers. If the motor fails to move or hits a limit, the tuner immediately zeros the output and transitions to `SYN_ATUNE_ABORTED`.

#### Per-Second Units
The controller handles all time-base conversions internally using the `update_hz` field. Profiles (`move_to`, `move_to_scurve`) accept kinematic limits in units per second (or sec², sec³). This ensures consistent behavior regardless of the control loop frequency.

#### Feedforward
Feedforward is automatically computed from the active profile. The controller supports both velocity feedforward (`ff_kv`) and acceleration feedforward (`ff_ka`). Acceleration is derived from profile velocity deltas.


## Profile Generators

| Module | Header | Config | Description |
|---|---|---|---|
| Ramp | `util/syn_ramp.h` | — | Linear or trapezoidal velocity ramp generator |
| S-Curve | `util/syn_scurve.h` | — | 7-phase S-curve trajectory with jerk limiting |
