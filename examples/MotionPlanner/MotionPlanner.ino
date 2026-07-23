/**
 * SyntropicOS — Motion Planner & Event Bus
 *
 * Demonstrates smooth, jerk-limited motion profiling and decoupled events:
 *   - Jerk-limited S-curve trajectory generator (syn_scurve)
 *   - Synchronous Publish/Subscribe event broker (syn_pubsub)
 *   - Interactive Serial CLI (syn_cli)
 *   - Multi-task cooperative scheduler (syn_sched)
 *
 * S-curve trajectory generator calculates smooth velocity and acceleration
 * profiles to eliminate mechanical jerk. PubSub event bus decouples motion updates
 * from status logging and UI tasks.
 *
 * Open Serial Monitor at 115200 baud. Type "move 1000" to plan trajectory.
 *
 * Documentation & Related Features:
 *   - Motion & Control:     https://outlookhazy.github.io/SyntropicOS/modules/control/
 *   - Motor API Ref:        https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__motor/ (syn_motor_ctrl.h)
 *   - Multitasking & Events: https://outlookhazy.github.io/SyntropicOS/modules/multitasking/ (syn_pubsub.h)
 *   - Trajectory Tracking:  For feedforward motor tracking, see syn_motor_ctrl_set_trajectory()
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/util/syn_scurve.h>
#include <syntropic/util/syn_pubsub.h>
#include <syntropic/output/syn_led.h>
#include <string.h>
#include <stdlib.h>

#define TAG "motion"

/* PubSub Topics */
#define TOPIC_MOTION_START    0x0001
#define TOPIC_MOTION_PROGRESS 0x0002
#define TOPIC_MOTION_COMPLETE 0x0003

static SYN_CLI          cli;
static SYN_Sched        sched;
static SYN_Task         tasks[3];
static SYN_LED          led;

/* Motion Generator & Event Broker */
static SYN_SCurve       sc;
static SYN_PubSubBroker broker;
static SYN_PubSubSub    sub_array[4];

static int32_t          last_published_p = 0;

/* ── PubSub Event Handlers ────────────────────────────────────────────── */

static void on_motion_event(uint16_t topic, const void *payload, size_t len, void *ctx)
{
    (void)len; (void)ctx;
    const int32_t *pos = (const int32_t *)payload;
    if (topic == TOPIC_MOTION_START) {
        syn_cli_printf(&cli, "[EVENT] Motion started -> Target=%ld\r\n", *pos);
        syn_led_blink(&led, 100, 100);
    } else if (topic == TOPIC_MOTION_COMPLETE) {
        syn_cli_printf(&cli, "[EVENT] Motion complete: target position reached (pos=%ld)\r\n", *pos);
        syn_led_on(&led);
    }
}

/* ── CLI Commands ─────────────────────────────────────────────────────── */

static int cmd_move(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: move <target_pos> (e.g. move 1000)\r\n");
        return 1;
    }
    int32_t target = atol(argv[1]);
    syn_scurve_set_target(&sc, target);
    syn_pubsub_publish(&broker, TOPIC_MOTION_START, &target, sizeof(target));
    return 0;
}

static int cmd_status(int argc, char *argv[])
{
    (void)argc; (void)argv;
    syn_cli_printf(&cli, "--- Motion Status ---\r\n");
    syn_cli_printf(&cli, "Position:     %ld\r\n", syn_scurve_position(&sc));
    syn_cli_printf(&cli, "Velocity:     %ld\r\n", syn_scurve_velocity(&sc));
    syn_cli_printf(&cli, "Acceleration: %ld\r\n", syn_scurve_acceleration(&sc));
    syn_cli_printf(&cli, "Done:         %s\r\n", syn_scurve_done(&sc) ? "YES" : "NO");
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "move",   "Set target position for S-curve trajectory", cmd_move },
    { "status", "Print current kinematic state",             cmd_status },
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

/* Motion Update Task — updates kinematic profile at 50Hz (20ms step) */
static SYN_PT_Status motion_task(SYN_PT *pt, SYN_Task *task)
{
    static bool was_done = true;
    PT_BEGIN(pt);
    for (;;) {
        if (!syn_scurve_done(&sc)) {
            was_done = false;
            int32_t current_p = syn_scurve_update(&sc);

            /* Publish progress event */
            syn_pubsub_publish(&broker, TOPIC_MOTION_PROGRESS, &current_p, sizeof(current_p));

            if (syn_scurve_done(&sc)) {
                syn_pubsub_publish(&broker, TOPIC_MOTION_COMPLETE, &current_p, sizeof(current_p));
                was_done = true;
            }
        } else if (!was_done) {
            was_done = true;
            int32_t current_p = syn_scurve_position(&sc);
            syn_pubsub_publish(&broker, TOPIC_MOTION_COMPLETE, &current_p, sizeof(current_p));
        }

        PT_TASK_DELAY_MS(pt, task, 20);
    }
    PT_END(pt);
}

/* ── Entry Point ──────────────────────────────────────────────────────── */

void setup()
{
    syn_port_serial_init(115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Initialize PubSub broker */
    syn_pubsub_init(&broker, sub_array, sizeof(sub_array) / sizeof(sub_array[0]));
    syn_pubsub_subscribe(&broker, SYN_PUBSUB_TOPIC_ALL, on_motion_event, NULL);

    /* Initialize S-curve generator: v_max=50, a_max=10, j_max=2 */
    syn_scurve_init(&sc, 0);
    syn_scurve_set_constraints(&sc, 50, 10, 2);

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "Motion Planner Ready");

    syn_cli_init(&cli, commands, sizeof(commands) / sizeof(commands[0]), "> ");
    syn_cli_printf(&cli, "\r\n--- SyntropicOS Motion Planner Demo ---\r\n");

    /* Trigger initial move for automated test validation */
    int32_t init_target = 200;
    syn_scurve_set_target(&sc, init_target);
    syn_pubsub_publish(&broker, TOPIC_MOTION_START, &init_target, sizeof(init_target));

    syn_task_create(&tasks[0], "blink",  blink_task,  2, NULL);
    syn_task_create(&tasks[1], "cli",    cli_task,    1, NULL);
    syn_task_create(&tasks[2], "motion", motion_task, 0, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
