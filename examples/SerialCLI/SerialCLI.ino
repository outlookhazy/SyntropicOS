/**
 * SyntropicOS — Serial CLI
 *
 * Interactive command-line interface over the serial port.
 * Demonstrates:
 *   - Cooperative scheduler with multiple tasks
 *   - CLI with custom commands
 *   - LED driver (blink, flash, SOS Morse pattern)
 *   - Finite State Machine (FSM)
 *   - Structured logging
 *
 * Open the Serial Monitor at 115200 baud and type "help".
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_uart.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/util/syn_fsm.h>
#include <string.h>

#define TAG "main"

static SYN_CLI   cli;
static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   led;

/* ── FSM: a simple IDLE → RUNNING → FAULT state machine ──────────────── */

static SYN_FSM    fsm;
static bool       fsm_ready = false;
static const char *state_names[] = { "IDLE", "RUNNING", "FAULT" };

static void on_running(void *ctx) { (void)ctx; SYN_LOG_I(TAG, "FSM -> RUNNING"); }
static void on_fault(void *ctx)   { (void)ctx; SYN_LOG_W(TAG, "FSM -> FAULT"); }
static void on_idle(void *ctx)    { (void)ctx; SYN_LOG_I(TAG, "FSM -> IDLE"); }

static const SYN_FSM_Transition transitions[] = {
    { 0, 0, 1, NULL, on_running },   /* IDLE    --start--> RUNNING */
    { 1, 1, 0, NULL, on_idle },      /* RUNNING --stop-->  IDLE    */
    { 1, 2, 2, NULL, on_fault },     /* RUNNING --fault--> FAULT   */
    { 2, 1, 0, NULL, on_idle },      /* FAULT   --stop-->  IDLE    */
    SYN_FSM_END
};

/* ── CLI Commands ─────────────────────────────────────────────────────── */

static int cmd_led(int argc, char *argv[])
{
    if (argc < 2) { syn_cli_printf(&cli, "Usage: led <on|off|blink|flash|sos>\r\n"); return 1; }
    if      (strcmp(argv[1], "on")    == 0) syn_led_on(&led);
    else if (strcmp(argv[1], "off")   == 0) syn_led_off(&led);
    else if (strcmp(argv[1], "blink") == 0) syn_led_blink(&led, 200, 200);
    else if (strcmp(argv[1], "flash") == 0) syn_led_flash(&led, 100, 100, 5);
    else if (strcmp(argv[1], "sos")   == 0) syn_led_pattern(&led, "... --- ... |", 150);
    else { syn_cli_printf(&cli, "Unknown: %s\r\n", argv[1]); return 1; }
    syn_cli_printf(&cli, "LED: %s\r\n", argv[1]);
    return 0;
}

static int cmd_fsm(int argc, char *argv[])
{
    if (argc < 2) { syn_cli_printf(&cli, "Usage: fsm <start|stop|fault>\r\n"); return 1; }
    if (!fsm_ready) {
        syn_fsm_init(&fsm, transitions, 0, "demo");
        syn_fsm_set_state_names(&fsm, state_names);
        fsm_ready = true;
    }
    SYN_FSM_Event ev = -1;
    if      (strcmp(argv[1], "start") == 0) ev = 0;
    else if (strcmp(argv[1], "stop")  == 0) ev = 1;
    else if (strcmp(argv[1], "fault") == 0) ev = 2;
    else { syn_cli_printf(&cli, "Unknown: %s\r\n", argv[1]); return 1; }
    bool ok = syn_fsm_dispatch(&fsm, ev);
    syn_cli_printf(&cli, "FSM: %s -> %s (state=%s)\r\n",
                   argv[1], ok ? "OK" : "IGNORED",
                   state_names[syn_fsm_state(&fsm)]);
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "led", "Control the status LED",       cmd_led },
    { "fsm", "Drive the demo state machine", cmd_fsm },
};

/* ── Platform hooks ───────────────────────────────────────────────────── */

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
    uint8_t ch; size_t n; SYN_Status st;
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        n = 0;
        st = syn_port_uart_receive(0, &ch, 1, &n, 1);
        if (st == SYN_OK && n > 0) syn_cli_process_char(&cli, (char)ch);
        PT_YIELD(pt);
    }
    PT_END(pt);
}

/* ── Arduino entry points ─────────────────────────────────────────────── */

void setup()
{
    syn_port_uart_init(0, 115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "boot");

    syn_cli_init(&cli, commands, sizeof(commands)/sizeof(commands[0]),
                 "> ");
    syn_cli_set_scheduler(&sched);
    syn_cli_printf(&cli, "\r\n--- SyntropicOS ---\r\n");
    syn_cli_print_prompt(&cli);

    syn_task_create(&tasks[0], "blink", blink_task, 2, NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   1, NULL);
    syn_sched_init(&sched, tasks, 2);
}

void loop()
{
    syn_sched_run(&sched);
}
