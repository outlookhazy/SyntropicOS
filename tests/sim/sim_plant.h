/**
 * @file sim_plant.h
 * @brief Simulated linear-rail cart (mover) for host-side testing.
 *
 * Models a heavy cart on rails with:
 *   - Mass and inertia
 *   - Coulomb (static + dynamic) friction
 *   - Viscous damping (proportional to velocity)
 *   - DC motor with back-EMF and driver deadband
 *   - Encoder quantization and noise
 *   - Optional belt compliance (spring-damper between motor and load)
 *
 * The simulation runs in discrete time steps. Call sim_plant_step() at
 * the same rate as the motor controller update (e.g., 100 Hz).
 */

#ifndef SIM_PLANT_H
#define SIM_PLANT_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Physical parameters for the simulated plant. */
typedef struct {    /* ── Physical parameters ─────────────────────────────────────── */
    double mass_kg;              /**< Cart mass (e.g., 136 kg = 300 lb)   */

    /* Motor model: F = Km * (command - Kb * velocity)
     * Km incorporates torque constant, gear ratio, wheel radius.
     * Kb is back-EMF constant (limits top speed). */
    double motor_Km;             /**< Motor force constant (N per %)      */
    double motor_Kb;             /**< Back-EMF constant                   */

    /* Friction */
    double friction_static;      /**< Static friction / stiction (N)      */
    double friction_coulomb;     /**< Dynamic Coulomb friction (N)        */
    double friction_viscous;     /**< Viscous damping (N·s/m)             */

    /* Driver nonlinearity */
    double driver_deadband;      /**< Output below this produces no torque (%) */
    double driver_asymmetry;     /**< Reverse gain = (1 - asymmetry) × fwd  */

    /* Encoder */
    double counts_per_meter;     /**< Encoder resolution                  */
    double encoder_noise_counts; /**< ± noise amplitude (counts, uniform) */

    /* Track limits (hard endstops) */
    double track_min_m;          /**< Hard endstop position (m)           */
    double track_max_m;          /**< Hard endstop position (m)           */

    /* Simulation */
    double dt_s;                 /**< Time step (1/update_hz)             */
} SimPlantParams;

/** @brief Simulated plant instance (state + parameters). */
typedef struct {
    SimPlantParams params;           /**< Physical parameters              */

    /* State */
    double position_m;           /**< Position (meters)                   */
    double velocity_mps;         /**< Velocity (m/s)                      */
    double acceleration_mps2;    /**< Acceleration (m/s²)                 */
    double motor_force_N;        /**< Current motor force                 */

    /* Input */
    double command_pct;          /**< Motor command (-100 to +100 %)      */

    /* Encoder output */
    int32_t encoder_counts;      /**< Quantized + noisy encoder reading   */

    /* Timing */
    uint32_t tick_ms;            /**< Simulated tick counter              */
    uint32_t step_count;         /**< Total steps executed                */

    /* Crash detection */
    bool     crashed;            /**< True if hit a hard endstop          */
    int      crash_end;          /**< -1 = min end, +1 = max end          */

    /** @brief RNG state for encoder noise (xorshift32). */
    uint32_t rng_state;
} SimPlant;

/**
 * @brief Initialize the plant with default "heavy mover on rails" parameters.
 * @param plant  Plant instance to initialize.
 */
void sim_plant_init_defaults(SimPlant *plant);

/**
 * @brief Initialize the plant with custom parameters.
 * @param plant   Plant instance to initialize.
 * @param params  Physical parameters to copy in.
 */
void sim_plant_init(SimPlant *plant, const SimPlantParams *params);

/**
 * @brief Set the motor command (% of max, -100 to +100).
 * @param plant        Plant instance.
 * @param command_pct  Motor command percentage.
 */
void sim_plant_set_command(SimPlant *plant, double command_pct);

/**
 * @brief Advance the simulation by one time step.
 *
 * Computes motor force, friction, acceleration, integrates velocity
 * and position, updates encoder output with noise.
 *
 * @param plant  Plant instance.
 */
void sim_plant_step(SimPlant *plant);

/**
 * @brief Get the current encoder reading (with quantization + noise).
 */
static inline int32_t sim_plant_encoder(const SimPlant *plant)
{
    return plant->encoder_counts;
}

#endif /* SIM_PLANT_H */
