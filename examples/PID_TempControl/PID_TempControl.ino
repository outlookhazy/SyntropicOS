/**
 * SyntropicOS — PID Temperature Controller
 *
 * Demonstrates deterministic, integer-only closed-loop control using:
 *   - Discrete PID controller with anti-windup (syn_pid)
 *   - Exponential Moving Average digital filter (syn_filter)
 *   - Interactive Serial CLI for setpoint and gain adjustment (syn_cli)
 *   - Cooperative scheduler with multi-task timing (syn_sched)
 *
 * Includes a thermal plant simulation inside the control task,
 * allowing full validation without physical hardware attached.
 *
 * Open Serial Monitor at 115200 baud. Type "sp 600" to set target to 60.0°C.
 *
 * Documentation & Related Features:
 *   - PID & Control Docs:  https://outlookhazy.github.io/SyntropicOS/modules/control/
 *   - Real Hardware PWM:    https://outlookhazy.github.io/SyntropicOS/modules/drivers/ (syn_port_pwm_set_duty)
 *   - Parameter Storage:    https://outlookhazy.github.io/SyntropicOS/modules/storage/ (syn_param_save)
 *   - Closed-Loop Motors:   https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__motor/ (syn_motor_ctrl.h)
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/control/syn_pid.h>
#include <syntropic/dsp/syn_filter.h>
#include <syntropic/output/syn_led.h>
#include <stdlib.h>
#include <string.h>

#define TAG "temp_ctrl"

/* Global Objects */
static SYN_CLI       cli;
static SYN_Sched     sched;
static SYN_Task      tasks[3];
static SYN_LED       led;
static SYN_PID       pid;
static SYN_FilterEMA temp_filter;

/* Control & Simulation State (units in tenths of °C, e.g., 250 = 25.0°C) */
static int32_t setpoint = 500;   /* Default target: 50.0°C */
static int32_t raw_temp = 250;   /* Initial ambient temperature: 25.0°C */
static int32_t filt_temp = 250;
static int32_t heater_pwm = 0;   /* 0 to 1000 (0.0% to 100.0% power) */
static uint8_t settled_count = 0;

/* ── Thermal Plant Simulation ─────────────────────────────────────────── */
/* Simulates heater input and thermal dissipation to ambient (25.0°C) */
static int32_t simulate_thermal_plant(int32_t current_temp_x10, int32_t power_permille)
{
    const int32_t ambient_x10 = 250;
    /* Heat gain: 100% power adds ~3.0°C per 100ms update */
    int32_t heat_input = (power_permille * 30) / 1000;
    /* Thermal loss: proportional to difference from ambient */
    int32_t heat_loss = (current_temp_x10 - ambient_x10) / 25;

    int32_t next_temp = current_temp_x10 + heat_input - heat_loss;
    if (next_temp < ambient_x10) next_temp = ambient_x10;
    return next_temp;
}

/* ── CLI Commands ─────────────────────────────────────────────────────── */

static int cmd_sp(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Current setpoint: %ld.%ld degC\r\nUsage: sp <temp_x10> (e.g. 600 for 60.0C)\r\n",
                       setpoint / 10, abs(setpoint % 10));
        return 1;
    }
    setpoint = atoi(argv[1]);
    settled_count = 0;
    syn_cli_printf(&cli, "Setpoint updated to %ld.%ld degC\r\n", setpoint / 10, abs(setpoint % 10));
    return 0;
}

static int cmd_gains(int argc, char *argv[])
{
    if (argc < 4) {
        syn_cli_printf(&cli, "Usage: gains <kp> <ki> <kd>\r\n");
        return 1;
    }
    int32_t kp = atoi(argv[1]);
    int32_t ki = atoi(argv[2]);
    int32_t kd = atoi(argv[3]);
    syn_pid_set_gains(&pid, kp, ki, kd);
    syn_cli_printf(&cli, "PID gains updated: Kp=%ld Ki=%ld Kd=%ld\r\n", kp, ki, kd);
    return 0;
}

static int cmd_status(int argc, char *argv[])
{
    (void)argc; (void)argv;
    syn_cli_printf(&cli, "--- PID Status ---\r\n");
    syn_cli_printf(&cli, "Setpoint:  %ld.%ld degC\r\n", setpoint / 10, abs(setpoint % 10));
    syn_cli_printf(&cli, "Temp (PV): %ld.%ld degC (raw=%ld.%ld)\r\n",
                   filt_temp / 10, abs(filt_temp % 10), raw_temp / 10, abs(raw_temp % 10));
    syn_cli_printf(&cli, "Heater:    %ld.%ld%%\r\n", heater_pwm / 10, abs(heater_pwm % 10));
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "sp",     "Set target temperature (tenths of C)", cmd_sp },
    { "gains",  "Set raw integer PID gains (kp ki kd)", cmd_gains },
    { "status", "Print system status and PID state",    cmd_status },
};

/* ── Platform Hooks ───────────────────────────────────────────────────── */

extern "C" void syn_assert_failed(const char *f, int l) { (void)f; (void)l; for(;;); }

/* ── Tasks ────────────────────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

static SYN_PT_Status cli_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t ch;
    PT_BEGIN(pt);
    for (;;) {
        int r = syn_port_serial_read(&ch, 1);
        if (r > 0) {
            syn_cli_process_char(&cli, (char)ch);
            PT_YIELD(pt);
        } else {
            PT_TASK_DELAY_MS(pt, task, 20);
        }
    }
    PT_END(pt);
}

/* PID Control Loop Task — runs every 100ms */
static SYN_PT_Status pid_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        /* 1. Simulate thermal plant behavior */
        raw_temp = simulate_thermal_plant(raw_temp, heater_pwm);

        /* 2. Filter temperature measurement */
        filt_temp = syn_filter_ema_update(&temp_filter, raw_temp);

        /* 3. Compute PID update (100ms dt) */
        heater_pwm = syn_pid_update(&pid, setpoint, filt_temp, 100);

        /* 4. Check convergence stability */
        int32_t err = abs(setpoint - filt_temp);
        if (err <= 5) { /* within ±0.5°C */
            if (settled_count < 10) {
                settled_count++;
                if (settled_count == 10) {
                    syn_cli_printf(&cli, "[PID] Settled at setpoint (%ld.%ld degC)\r\n",
                                   filt_temp / 10, abs(filt_temp % 10));
                    syn_led_on(&led);
                }
            }
        } else {
            if (settled_count >= 10) {
                syn_led_blink(&led, 200, 200);
            }
            settled_count = 0;
        }

        syn_cli_printf(&cli, "SP=%ld.%ld C | PV=%ld.%ld C | OUT=%ld.%ld%%\r\n",
                       setpoint / 10, abs(setpoint % 10),
                       filt_temp / 10, abs(filt_temp % 10),
                       heater_pwm / 10, abs(heater_pwm % 10));

        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}

/* ── Entry Point ──────────────────────────────────────────────────────── */

void setup()
{
    syn_port_serial_init(115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Initialize EMA filter (alpha = 64/256 = 0.25) */
    syn_filter_ema_init(&temp_filter, 64);

    /* Initialize integer PID: Kp=1.5, Ki=0.01, Kd=0.2, Scale=256, Output=0..1000 permille */
    SYN_PID_Config pid_cfg = SYN_PID_GAINS(1.5f, 0.01f, 0.2f, 256, 0, 1000);
    syn_pid_init(&pid, &pid_cfg);

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "PID Temperature Controller Ready");

    syn_cli_init(&cli, commands, sizeof(commands) / sizeof(commands[0]), "> ");
    syn_cli_printf(&cli, "\r\n--- SyntropicOS PID Temp Controller ---\r\n");
    syn_cli_print_prompt(&cli);

    syn_task_create(&tasks[0], "blink", blink_task, 2, NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   1, NULL);
    syn_task_create(&tasks[2], "pid",   pid_task,   0, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
