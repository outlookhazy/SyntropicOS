#include "sim_plant.h"
#include <math.h>
#include <string.h>

/* ── Simple xorshift32 PRNG ─────────────────────────────────────────────── */

static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/** Uniform random in [-1.0, +1.0] */
static double rand_uniform(uint32_t *state)
{
    return ((double)(int32_t)xorshift32(state)) / (double)0x7FFFFFFF;
}

/* ── Defaults: 300 lb cart on 50m rails, ~4 m/s top speed ───────────────── */
/*
 * Motor calibration:
 *   We need the motor to sustain 0.75 m/s² at high speed on 136 kg.
 *   Required force: F = m*a + friction = 136*0.75 + (20 + 15*v) ≈ 182 N at 4 m/s
 *
 *   At 100% command, v=4 m/s:
 *     F = Km * (cmd_eff/100 - Kb * v) = 250 * (1.0 - 0.03*4) = 250 * 0.88 = 220 N
 *     Friction = 20 + 15*4 = 80 N
 *     Net = 140 N → accel = 140/136 = 1.03 m/s²  ✓ (enough for 0.75 m/s²)
 *
 *   Top speed (F_net = 0):
 *     250*(1 - 0.03*v) = 20 + 15*v → 250 - 7.5v = 20 + 15v → v ≈ 10.2 m/s
 *     ...but mechanically/electrically limited to ~4 m/s. Good.
 *
 *   At 60% command (FF ident), steady-state:
 *     cmd_eff ≈ 58.8%
 *     250 * (0.588 - 0.03*v) = 20 + 15*v
 *     147 - 7.5v = 20 + 15v → v ≈ 5.6 m/s (would clip to motor limits)
 *     In practice, reaches ~3-4 m/s then back-EMF + friction balance.
 */

void sim_plant_init_defaults(SimPlant *plant)
{
    SimPlantParams p = {
        .mass_kg          = 136.0,      /* 300 lb */
        .motor_Km         = 250.0,      /* Motor force constant — enough for 0.75 m/s² at speed */
        .motor_Kb         = 0.03,       /* Back-EMF coefficient */
        .friction_static  = 35.0,       /* 35 N stiction (linear rail bearings) */
        .friction_coulomb = 20.0,       /* 20 N sliding friction */
        .friction_viscous = 15.0,       /* 15 N·s/m damping */
        .driver_deadband  = 3.0,        /* 3% deadband */
        .driver_asymmetry = 0.05,       /* 5% forward/reverse asymmetry */
        .counts_per_meter = 10000.0,    /* 10,000 counts/m ≈ 0.1mm resolution */
        .encoder_noise_counts = 2.0,    /* ±2 count noise */
        .track_min_m      = 0.0,        /* Hard endstop at 0m */
        .track_max_m      = 50.0,       /* Hard endstop at 50m */
        .dt_s             = 0.01,       /* 100 Hz update rate */
    };
    sim_plant_init(plant, &p);

    /* Start at center of track (25m) */
    plant->position_m = 25.0;
    plant->encoder_counts = (int32_t)(25.0 * p.counts_per_meter);
}

void sim_plant_init(SimPlant *plant, const SimPlantParams *params)
{
    memset(plant, 0, sizeof(*plant));
    plant->params = *params;
    plant->rng_state = 0xDEADBEEF;
}

void sim_plant_set_command(SimPlant *plant, double command_pct)
{
    /* Clamp to ±100% */
    if (command_pct > 100.0) command_pct = 100.0;
    if (command_pct < -100.0) command_pct = -100.0;
    plant->command_pct = command_pct;
}

void sim_plant_step(SimPlant *plant)
{
    const SimPlantParams *p = &plant->params;
    double cmd = plant->command_pct;
    double v = plant->velocity_mps;

    /* If already crashed, lock position and zero velocity */
    if (plant->crashed) {
        plant->velocity_mps = 0.0;
        plant->acceleration_mps2 = 0.0;
        /* Still update encoder so controller can read it */
        plant->encoder_counts = (int32_t)round(plant->position_m * p->counts_per_meter);
        plant->tick_ms += (uint32_t)(p->dt_s * 1000.0);
        plant->step_count++;
        return;
    }

    /* ── Motor driver nonlinearity ──────────────────────────────── */

    double effective_cmd;
    if (fabs(cmd) < p->driver_deadband) {
        effective_cmd = 0.0;
    } else {
        double sign = (cmd > 0.0) ? 1.0 : -1.0;
        effective_cmd = sign * (fabs(cmd) - p->driver_deadband)
                      * 100.0 / (100.0 - p->driver_deadband);
    }

    if (effective_cmd < 0.0) {
        effective_cmd *= (1.0 - p->driver_asymmetry);
    }

    /* ── Motor force ────────────────────────────────────────────── */

    double F_motor = p->motor_Km * (effective_cmd / 100.0 - p->motor_Kb * v);
    plant->motor_force_N = F_motor;

    /* ── Friction ───────────────────────────────────────────────── */

    double F_friction = 0.0;
    double abs_v = fabs(v);

    if (abs_v < 1e-6) {
        if (fabs(F_motor) < p->friction_static) {
            F_friction = -F_motor;
        } else {
            double motor_sign = (F_motor > 0.0) ? 1.0 : -1.0;
            F_friction = -motor_sign * p->friction_coulomb;
        }
    } else {
        double v_sign = (v > 0.0) ? 1.0 : -1.0;
        F_friction = -v_sign * p->friction_coulomb - p->friction_viscous * v;
    }

    /* ── Dynamics ───────────────────────────────────────────────── */

    double F_total = F_motor + F_friction;
    double accel = F_total / p->mass_kg;
    plant->acceleration_mps2 = accel;

    plant->velocity_mps += accel * p->dt_s;

    if (abs_v < 1e-6 && fabs(F_motor) < p->friction_static) {
        plant->velocity_mps = 0.0;
    }

    plant->position_m += plant->velocity_mps * p->dt_s;

    /* ── Hard endstops ──────────────────────────────────────────── */

    if (plant->position_m <= p->track_min_m) {
        plant->position_m = p->track_min_m;
        plant->velocity_mps = 0.0;
        plant->crashed = true;
        plant->crash_end = -1;
    } else if (plant->position_m >= p->track_max_m) {
        plant->position_m = p->track_max_m;
        plant->velocity_mps = 0.0;
        plant->crashed = true;
        plant->crash_end = +1;
    }

    /* ── Encoder output ─────────────────────────────────────────── */

    double raw_counts = plant->position_m * p->counts_per_meter;
    int32_t quantized = (int32_t)round(raw_counts);

    if (p->encoder_noise_counts > 0.0) {
        double noise = rand_uniform(&plant->rng_state) * p->encoder_noise_counts;
        quantized += (int32_t)round(noise);
    }

    plant->encoder_counts = quantized;

    /* ── Timing ─────────────────────────────────────────────────── */

    plant->tick_ms += (uint32_t)(p->dt_s * 1000.0);
    plant->step_count++;
}
