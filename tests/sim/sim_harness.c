/**
 * @file sim_harness.c
 * @brief Multi-plant integration test for syn_motor_ctrl + syn_autotune.
 *
 * Tests the auto-tuner and trajectory tracker against a bank of different
 * plant configurations — from heavy rail carts to light actuators.
 * The auto-tuner must adapt to each one without any hand-tuning.
 *
 * Each plant runs:
 *   1. FF identification → measure ff_kv
 *   2. Relay PID auto-tune → compute PID gains
 *   3. Trajectory tracking → evaluate performance
 *
 * Pass criteria:
 *   - No crashes (endstop collisions)
 *   - RMS tracking error < 2% of total move distance
 *   - Final position within 1% of target
 *
 * Build:
 *   gcc -O2 -I../.. -o sim_harness sim_harness.c sim_plant.c \
 *       ../../syntropic/motor/syn_motor_ctrl.c \
 *       ../../syntropic/control/syn_autotune.c \
 *       ../../syntropic/control/syn_pid.c \
 *       ../../syntropic/log/syn_datalog.c \
 *       ../../syntropic/util/syn_ringbuf.c \
 *       -lm
 *
 * Run:
 *   ./sim_harness
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* ── Plant model (included early so stubs can reference it) ─────────────── */

#include "sim_plant.h"
static SimPlant g_plant;

/* ── Port stubs ─────────────────────────────────────────────────────────── */

static uint32_t g_sim_tick_ms = 0;
uint32_t syn_port_get_tick_ms(void) { return g_sim_tick_ms; }

uint32_t syn_port_flash_sector_size(void) { return 4096; }
int syn_port_flash_erase(uint32_t addr, uint32_t size) { (void)addr; (void)size; return 0; }
int syn_port_flash_write(uint32_t addr, const void *data, uint32_t len) { (void)addr; (void)data; (void)len; return 0; }
int syn_port_flash_read(uint32_t addr, void *data, uint32_t len) { (void)addr; (void)data; (void)len; return -1; }

void syn_assert_handler(const char *file, int line, const char *expr)
{
    fprintf(stderr, "ASSERT FAILED: %s:%d: %s\n", file, line, expr);
    exit(1);
}

void syn_assert_failed(const char *file, int line, const char *expr)
{
    syn_assert_handler(file, line, expr);
}

/* ── SyntropicOS includes ────────────────────────────────────────────────── */

#define SYN_USE_MOTOR_CTRL 1
#define SYN_USE_AUTOTUNE   1
#define SYN_USE_DATALOG    1
#define SYN_USE_PID        1
#define SYN_USE_DC_MOTOR   1

#include "../../syntropic/motor/syn_motor_ctrl.h"
#include "../../syntropic/control/syn_autotune.h"
#include "../../syntropic/system/syn_errlog.h"

/* ── Hardware stubs (after includes for correct types) ──────────────────── */

void syn_stepper_stop(SYN_Stepper *s) { (void)s; }
void syn_stepper_tick(SYN_Stepper *s) { (void)s; }

void syn_dc_motor_set_speed(SYN_DCMotor *motor, int16_t speed)
{
    (void)motor;
    sim_plant_set_command(&g_plant, (double)speed);
}

void syn_dc_motor_update(SYN_DCMotor *motor) { (void)motor; }
void syn_dc_motor_stop(SYN_DCMotor *motor) { (void)motor; sim_plant_set_command(&g_plant, 0.0); }
void syn_dc_motor_brake(SYN_DCMotor *motor) { (void)motor; sim_plant_set_command(&g_plant, 0.0); }
void syn_dc_motor_coast(SYN_DCMotor *motor) { (void)motor; sim_plant_set_command(&g_plant, 0.0); }

void syn_errlog_record(SYN_ErrLog *log, uint16_t code,
                        SYN_ErrSeverity severity, uint32_t context)
{
    (void)log;
    fprintf(stderr, "      [errlog] code=%u sev=%d ctx=%u\n", code, severity, context);
}

/* ── Globals ────────────────────────────────────────────────────────────── */

static SYN_DCMotor   g_dc_motor;
static SYN_MotorCtrl g_ctrl;

#define UPDATE_HZ  100

static int32_t read_encoder(void *ctx) { (void)ctx; return sim_plant_encoder(&g_plant); }

static void step_sim(void)
{
    sim_plant_step(&g_plant);
    g_sim_tick_ms = g_plant.tick_ms;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PLANT CONFIGURATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char     *name;
    SimPlantParams  params;
    double          start_m;         /* starting position on track */
    double          move_dist_m;     /* trajectory distance */
    double          cruise_mps;      /* max cruise velocity */
    double          accel_mpss;      /* trajectory accel */
} TestScenario;

static const TestScenario g_scenarios[] = {
    {
        .name = "Heavy rail cart (300 lb, 50m track)",
        .params = {
            .mass_kg          = 136.0,
            .motor_Km         = 250.0,
            .motor_Kb         = 0.03,
            .friction_static  = 35.0,
            .friction_coulomb = 20.0,
            .friction_viscous = 15.0,
            .driver_deadband  = 3.0,
            .driver_asymmetry = 0.05,
            .counts_per_meter = 10000.0,
            .encoder_noise_counts = 2.0,
            .track_min_m      = 0.0,
            .track_max_m      = 50.0,
            .dt_s             = 0.01,
        },
        .start_m    = 25.0,
        .move_dist_m = 15.0,
        .cruise_mps  = 4.0,
        .accel_mpss  = 0.75,
    },
    {
        .name = "Light rail cart (120 lb, 50m track)",
        .params = {
            .mass_kg          = 54.4,
            .motor_Km         = 250.0,
            .motor_Kb         = 0.03,
            .friction_static  = 20.0,
            .friction_coulomb = 12.0,
            .friction_viscous = 10.0,
            .driver_deadband  = 3.0,
            .driver_asymmetry = 0.05,
            .counts_per_meter = 10000.0,
            .encoder_noise_counts = 2.0,
            .track_min_m      = 0.0,
            .track_max_m      = 50.0,
            .dt_s             = 0.01,
        },
        .start_m    = 25.0,
        .move_dist_m = 15.0,
        .cruise_mps  = 4.0,
        .accel_mpss  = 1.5,
    },
    {
        .name = "Small linear actuator (5 kg, 1m stroke)",
        .params = {
            .mass_kg          = 5.0,
            .motor_Km         = 40.0,
            .motor_Kb         = 0.10,
            .friction_static  = 5.0,
            .friction_coulomb = 3.0,
            .friction_viscous = 2.0,
            .driver_deadband  = 2.0,
            .driver_asymmetry = 0.02,
            .counts_per_meter = 50000.0,     /* high-res encoder */
            .encoder_noise_counts = 1.0,
            .track_min_m      = 0.0,
            .track_max_m      = 1.0,
            .dt_s             = 0.01,
        },
        .start_m    = 0.5,
        .move_dist_m = 0.4,
        .cruise_mps  = 0.5,
        .accel_mpss  = 2.0,
    },
    {
        .name = "Heavy cart, worn bearings (300 lb, high friction)",
        .params = {
            .mass_kg          = 136.0,
            .motor_Km         = 250.0,
            .motor_Kb         = 0.03,
            .friction_static  = 80.0,      /* nasty stiction */
            .friction_coulomb = 50.0,
            .friction_viscous = 25.0,
            .driver_deadband  = 5.0,       /* worse driver */
            .driver_asymmetry = 0.10,
            .counts_per_meter = 10000.0,
            .encoder_noise_counts = 4.0,   /* noisier encoder */
            .track_min_m      = 0.0,
            .track_max_m      = 50.0,
            .dt_s             = 0.01,
        },
        .start_m    = 25.0,
        .move_dist_m = 10.0,
        .cruise_mps  = 3.0,
        .accel_mpss  = 0.5,
    },
    {
        .name = "Rotary platform (20 kg, 2m circumference)",
        .params = {
            .mass_kg          = 20.0,
            .motor_Km         = 80.0,
            .motor_Kb         = 0.08,
            .friction_static  = 10.0,
            .friction_coulomb = 6.0,
            .friction_viscous = 8.0,
            .driver_deadband  = 2.0,
            .driver_asymmetry = 0.03,
            .counts_per_meter = 20000.0,
            .encoder_noise_counts = 1.0,
            .track_min_m      = -100.0,    /* effectively no limits (wraps) */
            .track_max_m      =  100.0,
            .dt_s             = 0.01,
        },
        .start_m    = 0.0,
        .move_dist_m = 1.5,
        .cruise_mps  = 1.0,
        .accel_mpss  = 3.0,
    },
};

#define NUM_SCENARIOS (sizeof(g_scenarios) / sizeof(g_scenarios[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST RESULTS
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    bool    passed;
    bool    crashed;
    bool    autotune_ok;

    /* FF ident */
    int32_t steady_velocity;
    int32_t ff_kv;
    uint8_t ff_scale;

    /* Relay */
    int32_t Ku;
    uint32_t Tu_ms;
    int32_t kp, ki, kd;
    uint8_t pid_scale;

    /* Trajectory performance */
    int32_t max_error;
    int32_t rms_error;
    int32_t overshoot;
    double  final_pos_m;
    double  target_pos_m;
    double  error_pct;     /* RMS as % of move distance */
} TestResult;

/* ═══════════════════════════════════════════════════════════════════════════
 * TEST EXECUTION
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_plant_and_ctrl(const TestScenario *sc)
{
    /* Init plant */
    sim_plant_init(&g_plant, &sc->params);
    g_plant.position_m = sc->start_m;
    g_plant.encoder_counts = (int32_t)(sc->start_m * sc->params.counts_per_meter);

    /* Init controller with zero gains */
    SYN_MotorCtrl_Config cfg = {
        .type         = SYN_MCTRL_DC,
        .read_pos     = read_encoder,
        .read_pos_ctx = NULL,
        .dc_motor     = &g_dc_motor,
        .pid_kp       = 0,
        .pid_ki       = 0,
        .pid_kd       = 0,
        .pid_scale    = 8,
        .ff_kv        = 0,
        .ff_ka        = 0,
        .ff_scale     = 8,
        .update_hz    = UPDATE_HZ,
        .output_min   = -100,
        .output_max   = 100,
        .position_deadband = (int32_t)(sc->params.encoder_noise_counts * 3),
    };

    /* Soft limits: 5% of track range inward from each end.
     * Disable for very large ranges (e.g., rotary with ±100m virtual limits). */
    double track_len = sc->params.track_max_m - sc->params.track_min_m;
    if (track_len < 200.0) {
        double lim_margin = track_len * 0.05;
        cfg.position_min = (int32_t)((sc->params.track_min_m + lim_margin)
                         * sc->params.counts_per_meter);
        cfg.position_max = (int32_t)((sc->params.track_max_m - lim_margin)
                         * sc->params.counts_per_meter);
    }
    cfg.stall_timeout_ms = 0;
    syn_motor_ctrl_init(&g_ctrl, &cfg);
    g_sim_tick_ms = 0;
}

static void brake_to_stop(const char *label)
{
    for (int i = 0; i < 1000; i++) {
        double v = g_plant.velocity_mps;
        if (fabs(v) < 0.001) break;
        double brake = -v * 30.0;
        if (brake > 100.0) brake = 100.0;
        if (brake < -100.0) brake = -100.0;
        sim_plant_set_command(&g_plant, brake);
        step_sim();
        if (g_plant.crashed) {
            fprintf(stderr, "      CRASH during %s!\n", label);
            return;
        }
    }
    sim_plant_set_command(&g_plant, 0.0);
    for (int i = 0; i < 50; i++) step_sim();
}

static bool run_ff_ident(const TestScenario *sc, TestResult *res)
{
    /* Choose test_output based on what won't run us off the track.
     * Estimate: at test_output%, the cart will travel roughly
     * (settle_ms + measure_ms) * estimated_velocity.
     * Use 30% as a safe default. */
    int32_t test_out = 30;

    double margin = (sc->params.track_max_m - sc->params.track_min_m) * 0.05;
    if (margin < 0.01) margin = 0.01;

    double avail_fwd = sc->params.track_max_m - margin - g_plant.position_m;
    double avail_rev = g_plant.position_m - sc->params.track_min_m - margin;
    double avail = (avail_fwd > avail_rev) ? avail_fwd : avail_rev;
    if (avail < 0.01) avail = 0.01;

    /* If we have lots of room, use higher test output for better accuracy */
    if (avail > 20.0) test_out = 50;
    else if (avail > 10.0) test_out = 40;

    SYN_AutoTune at;
    SYN_AutoTune_Config acfg = {
        .mode           = SYN_ATUNE_MODE_FF_IDENT,
        .test_output    = test_out,
        .settle_ms      = 2000,
        .measure_ms     = 2000,
        .position_limit = (int32_t)(avail * sc->params.counts_per_meter * 0.7),
        .velocity_limit = 0,  /* disabled — position limit is the real constraint */
        .watchdog_ms    = 300,
        .ramp_ms        = 500,
    };

    syn_autotune_init(&at, &g_ctrl, &acfg);

    int max_steps = 15 * UPDATE_HZ;
    int steps = 0;

    while (at.state != SYN_ATUNE_DONE && at.state != SYN_ATUNE_ABORTED
           && steps < max_steps && !g_plant.crashed) {
        step_sim();
        syn_autotune_update(&at);
        steps++;
    }

    if (at.state == SYN_ATUNE_DONE) {
        const SYN_AutoTune_Result *r = syn_autotune_result(&at);
        res->steady_velocity = r->steady_velocity;
        res->ff_kv = r->ff_kv;
        res->ff_scale = r->ff_scale;
        syn_autotune_apply(&at);
        fprintf(stderr, "      FF: velocity=%d counts/s (%.2f m/s), ff_kv=%d, scale=%d\n",
                r->steady_velocity,
                r->steady_velocity / sc->params.counts_per_meter,
                r->ff_kv, r->ff_scale);
        brake_to_stop("FF braking");
        return true;
    }

    fprintf(stderr, "      FF ident failed (state=%d, abort=%d)\n",
            at.state, syn_autotune_abort_reason(&at));
    brake_to_stop("FF abort braking");
    return false;
}

static bool run_relay_tune(const TestScenario *sc, TestResult *res)
{
    /* Re-sync controller position */
    g_ctrl.measured_position = read_encoder(NULL);
    g_ctrl.last_position = g_ctrl.measured_position;

    double margin = (sc->params.track_max_m - sc->params.track_min_m) * 0.05;
    if (margin < 0.01) margin = 0.01;

    double avail_fwd = sc->params.track_max_m - margin - g_plant.position_m;
    double avail_rev = g_plant.position_m - sc->params.track_min_m - margin;
    double avail = (avail_fwd < avail_rev) ? avail_fwd : avail_rev;
    if (avail < 0.01) avail = 0.01;

    /* Scale relay amplitude to the plant — heavier = needs more */
    int32_t test_out = 20;
    if (sc->params.mass_kg > 100) test_out = 30;
    if (sc->params.mass_kg > 200) test_out = 40;

    SYN_AutoTune at;
    SYN_AutoTune_Config acfg = {
        .mode           = SYN_ATUNE_MODE_RELAY,
        .test_output    = test_out,
        .setpoint       = g_ctrl.measured_position,
        .relay_cycles   = 3,
        .method         = SYN_ATUNE_TYREUS_LUYBEN,
        .position_limit = (int32_t)(avail * sc->params.counts_per_meter * 0.5),
        .velocity_limit = (int32_t)(sc->cruise_mps * sc->params.counts_per_meter * 2),
        .watchdog_ms    = 500,
        .ramp_ms        = 300,
    };

    syn_autotune_init(&at, &g_ctrl, &acfg);

    int max_steps = 60 * UPDATE_HZ;
    int steps = 0;

    while (at.state != SYN_ATUNE_DONE && at.state != SYN_ATUNE_ABORTED
           && steps < max_steps && !g_plant.crashed) {
        step_sim();
        syn_autotune_update(&at);
        steps++;
    }

    if (at.state == SYN_ATUNE_DONE) {
        const SYN_AutoTune_Result *r = syn_autotune_result(&at);
        res->Ku = r->Ku;
        res->Tu_ms = r->Tu_ms;
        res->kp = r->kp;
        res->ki = r->ki;
        res->kd = r->kd;
        res->pid_scale = r->pid_scale;
        syn_autotune_apply(&at);
        fprintf(stderr, "      Relay: Ku=%d Tu=%ums → Kp=%d Ki=%d Kd=%d (scale=%d)\n",
                r->Ku, r->Tu_ms, r->kp, r->ki, r->kd, r->pid_scale);
        brake_to_stop("relay braking");
        return true;
    }

    fprintf(stderr, "      Relay tune failed (state=%d, abort=%d, steps=%d)\n",
            at.state, syn_autotune_abort_reason(&at), steps);
    brake_to_stop("relay abort braking");
    return false;
}

static void run_trajectory(const TestScenario *sc, TestResult *res)
{
    /* Re-sync controller position */
    g_ctrl.measured_position = read_encoder(NULL);
    g_ctrl.last_position = g_ctrl.measured_position;
    syn_motor_ctrl_reset_metrics(&g_ctrl);

    double  start_m   = g_plant.position_m;

    /* Pick direction with more room */
    double traj_margin = (sc->params.track_max_m - sc->params.track_min_m) * 0.05;
    if (traj_margin < 0.01) traj_margin = 0.01;

    double avail_fwd = sc->params.track_max_m - traj_margin - start_m;
    double avail_rev = start_m - sc->params.track_min_m - traj_margin;
    double direction = (avail_fwd >= avail_rev) ? 1.0 : -1.0;
    double avail = (direction > 0) ? avail_fwd : avail_rev;
    if (avail < 0.01) avail = 0.01;

    double max_dist = sc->move_dist_m;
    if (max_dist > avail * 0.8) max_dist = avail * 0.8;
    if (max_dist < 0.001) {
        fprintf(stderr, "      ERROR: No room for trajectory (avail=%.3fm)\n", avail);
        res->passed = false;
        return;
    }

    double accel_rate = sc->accel_mpss;
    double max_cruise = sc->cruise_mps;

    /* Compute trapezoidal profile */
    double v_cruise;
    double ramp_dist = max_cruise * max_cruise / accel_rate;
    if (ramp_dist >= max_dist) {
        v_cruise = sqrt(max_dist * accel_rate);
    } else {
        v_cruise = max_cruise;
    }

    double accel_time  = v_cruise / accel_rate;
    double decel_time  = accel_time;
    double accel_dist  = 0.5 * accel_rate * accel_time * accel_time;
    double decel_dist  = accel_dist;
    double cruise_dist = max_dist - accel_dist - decel_dist;
    if (cruise_dist < 0) cruise_dist = 0;
    double cruise_time = (v_cruise > 0) ? cruise_dist / v_cruise : 0;
    double total_time  = accel_time + cruise_time + decel_time;

    double cpm = sc->params.counts_per_meter;

    fprintf(stderr, "      Trajectory: %.1fm %s, v=%.2f m/s, a=%.2f m/s², t=%.1fs\n",
            max_dist, direction > 0 ? "fwd" : "rev", v_cruise, accel_rate, total_time);

    int total_steps = (int)((total_time + 5.0) * UPDATE_HZ);

    for (int i = 0; i < total_steps; i++) {
        double t = (double)i / UPDATE_HZ;

        double traj_vel_m = 0.0;
        double traj_accel_m = 0.0;
        double traj_pos_m = start_m;

        if (t < accel_time) {
            traj_accel_m = accel_rate * direction;
            traj_vel_m   = accel_rate * t * direction;
            traj_pos_m   = start_m + direction * 0.5 * accel_rate * t * t;
        } else if (t < accel_time + cruise_time) {
            double dt = t - accel_time;
            traj_accel_m = 0.0;
            traj_vel_m   = v_cruise * direction;
            traj_pos_m   = start_m + direction * (accel_dist + v_cruise * dt);
        } else if (t < total_time) {
            double dt = t - accel_time - cruise_time;
            traj_accel_m = -accel_rate * direction;
            traj_vel_m   = (v_cruise - accel_rate * dt) * direction;
            traj_pos_m   = start_m + direction * (accel_dist + cruise_dist
                         + v_cruise * dt - 0.5 * accel_rate * dt * dt);
        } else {
            traj_vel_m   = 0.0;
            traj_accel_m = 0.0;
            traj_pos_m   = start_m + direction * max_dist;
        }

        SYN_MotorCtrl_Trajectory traj = {
            .position     = (int32_t)(traj_pos_m * cpm),
            .velocity     = (int32_t)(traj_vel_m * cpm),
            .acceleration = (int32_t)(traj_accel_m * cpm),
        };
        syn_motor_ctrl_set_trajectory(&g_ctrl, &traj);

        step_sim();
        if (g_plant.crashed) {
            res->crashed = true;
            fprintf(stderr, "      *** CRASH at %.2f m during trajectory! ***\n",
                    g_plant.position_m);
            return;
        }
        syn_motor_ctrl_update(&g_ctrl);
    }

    /* Collect results */
    const SYN_MotorCtrl_Metrics *m = syn_motor_ctrl_get_metrics(&g_ctrl);
    res->max_error = m->max_error;
    res->rms_error = syn_motor_ctrl_rms_error(&g_ctrl);
    res->overshoot = m->overshoot;
    res->final_pos_m = g_plant.position_m;
    res->target_pos_m = start_m + direction * max_dist;

    double rms_m = res->rms_error / cpm;
    res->error_pct = (max_dist > 0) ? (rms_m / max_dist) * 100.0 : 0;

    double final_err_m = fabs(res->final_pos_m - res->target_pos_m);
    double final_err_pct = (max_dist > 0) ? (final_err_m / max_dist) * 100.0 : 0;

    fprintf(stderr, "      Max err: %d counts (%.1f mm)\n",
            res->max_error, res->max_error / (cpm / 1000.0));
    fprintf(stderr, "      RMS err: %d counts (%.1f mm, %.1f%% of move)\n",
            res->rms_error, rms_m * 1000, res->error_pct);
    fprintf(stderr, "      Final:   %.3f m (target %.3f m, err %.1f mm, %.1f%%)\n",
            res->final_pos_m, res->target_pos_m, final_err_m * 1000, final_err_pct);

    /* Pass criteria */
    res->passed = !res->crashed
               && res->error_pct < 2.0
               && final_err_pct < 1.0
               && res->rms_error > 0;   /* must have actually moved */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(void)
{
    fprintf(stderr, "╔══════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║  SyntropicOS Auto-Tune Integration Test Suite            ║\n");
    fprintf(stderr, "║  Testing %zu plant configurations                        ║\n",
            NUM_SCENARIOS);
    fprintf(stderr, "╚══════════════════════════════════════════════════════════╝\n\n");

    TestResult results[NUM_SCENARIOS];
    memset(results, 0, sizeof(results));

    int pass_count = 0;
    int fail_count = 0;

    for (size_t i = 0; i < NUM_SCENARIOS; i++) {
        const TestScenario *sc = &g_scenarios[i];
        TestResult *res = &results[i];

        fprintf(stderr, "━━━ [%zu/%zu] %s ━━━\n", i + 1, NUM_SCENARIOS, sc->name);
        fprintf(stderr, "    mass=%.0fkg, Km=%.0f, Kb=%.2f, friction=%.0f/%.0f/%.0f\n",
                sc->params.mass_kg, sc->params.motor_Km, sc->params.motor_Kb,
                sc->params.friction_static, sc->params.friction_coulomb,
                sc->params.friction_viscous);
        fprintf(stderr, "    track=%.0f-%.0fm, encoder=%.0f counts/m, noise=±%.0f\n",
                sc->params.track_min_m, sc->params.track_max_m,
                sc->params.counts_per_meter, sc->params.encoder_noise_counts);
        fprintf(stderr, "    target: %.1f m/s cruise, %.2f m/s² accel, %.1f m move\n",
                sc->cruise_mps, sc->accel_mpss, sc->move_dist_m);

        init_plant_and_ctrl(sc);

        /* Phase 1: FF identification */
        fprintf(stderr, "    Phase 1: FF Identification\n");
        bool ff_ok = run_ff_ident(sc, res);

        if (g_plant.crashed) {
            fprintf(stderr, "    *** CRASHED during FF ident ***\n");
            res->crashed = true;
            res->passed = false;
            fail_count++;
            fprintf(stderr, "    RESULT: FAIL (crash)\n\n");
            continue;
        }

        /* Phase 2: Relay PID tune */
        fprintf(stderr, "    Phase 2: Relay PID Tune\n");
        bool relay_ok = run_relay_tune(sc, res);
        res->autotune_ok = ff_ok && relay_ok;

        if (g_plant.crashed) {
            fprintf(stderr, "    *** CRASHED during relay tune ***\n");
            res->crashed = true;
            res->passed = false;
            fail_count++;
            fprintf(stderr, "    RESULT: FAIL (crash)\n\n");
            continue;
        }

        if (!relay_ok) {
            /* Both failed — fall back to safe conservative gains */
            fprintf(stderr, "    Auto-tune incomplete — using fallback gains\n");
            syn_motor_ctrl_set_gains(&g_ctrl, 80, 8, 30);
        }

        /* Phase 3: Trajectory tracking */
        fprintf(stderr, "    Phase 3: Trajectory Tracking\n");
        fprintf(stderr, "      Gains: Kp=%d Ki=%d Kd=%d (scale=%d)\n",
                g_ctrl.pid.cfg.kp, g_ctrl.pid.cfg.ki, g_ctrl.pid.cfg.kd,
                g_ctrl.pid.cfg.scale);
        fprintf(stderr, "      FF: ff_kv=%d ff_ka=%d (scale=%d)\n",
                g_ctrl.cfg.ff_kv, g_ctrl.cfg.ff_ka, g_ctrl.cfg.ff_scale);

        run_trajectory(sc, res);

        if (res->passed) {
            fprintf(stderr, "    RESULT: PASS ✓\n\n");
            pass_count++;
        } else {
            fprintf(stderr, "    RESULT: FAIL ✗%s%s\n\n",
                    res->crashed ? " (crash)" : "",
                    res->error_pct >= 2.0 ? " (tracking)" : "");
            fail_count++;
        }
    }

    /* ── Summary ─────────────────────────────────────────────────────── */

    fprintf(stderr, "╔══════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║  RESULTS SUMMARY                                        ║\n");
    fprintf(stderr, "╠══════════════════════════════════════════════════════════╣\n");

    for (size_t i = 0; i < NUM_SCENARIOS; i++) {
        const TestResult *r = &results[i];
        fprintf(stderr, "║  %s%-50s  ║\n",
                r->passed ? "✓ " : "✗ ", g_scenarios[i].name);
        if (r->crashed) {
            fprintf(stderr, "║    CRASHED                                              ║\n");
        } else {
            fprintf(stderr, "║    RMS: %5d counts (%.1f%% of move) | Max: %5d counts   ║\n",
                    r->rms_error, r->error_pct, r->max_error);
        }
    }

    fprintf(stderr, "╠══════════════════════════════════════════════════════════╣\n");
    fprintf(stderr, "║  %d/%zu PASSED                                            ║\n",
            pass_count, NUM_SCENARIOS);
    fprintf(stderr, "╚══════════════════════════════════════════════════════════╝\n");

    return (fail_count > 0) ? 1 : 0;
}
