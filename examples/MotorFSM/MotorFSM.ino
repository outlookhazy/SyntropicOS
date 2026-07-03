/**
 * SyntropicOS — Motor FSM
 *
 * A practical state machine example controlling a DC motor:
 *
 *   STOPPED --[start]--> RAMPING_UP --[at_speed]--> RUNNING
 *   RUNNING --[stop]-->  RAMPING_DOWN --[stopped]--> STOPPED
 *   (any)   --[fault]--> FAULT --[reset]--> STOPPED
 *
 * The FSM transitions are logged over serial, and the motor speed
 * is simulated with a software ramp. No real motor hardware needed.
 *
 * Type commands in Serial Monitor at 115200 baud:
 *   motor start / motor stop / motor fault / motor reset
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_uart.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/util/syn_fsm.h>
#include <syntropic/util/syn_ramp.h>
#include <string.h>

#define TAG "motor"

/* Motor states */
enum { ST_STOPPED, ST_RAMP_UP, ST_RUNNING, ST_RAMP_DOWN, ST_FAULT };
/* Motor events */
enum { EV_START, EV_STOP, EV_AT_SPEED, EV_AT_ZERO, EV_FAULT, EV_RESET };

static SYN_CLI   cli;
static SYN_Sched sched;
static SYN_Task  tasks[3];
static SYN_LED   led;
static SYN_FSM   fsm;
static SYN_Ramp  speed_ramp;

static const char *state_names[] = {
    "STOPPED", "RAMP_UP", "RUNNING", "RAMP_DOWN", "FAULT"
};

/* ── FSM Actions ──────────────────────────────────────────────────────── */

static void act_start_ramp_up(void *ctx)
{
    (void)ctx;
    SYN_LOG_I(TAG, "Starting ramp up");
    syn_ramp_jump(&speed_ramp, 0);
    syn_ramp_set_target(&speed_ramp, 255, 1);    /* ramp to 255 */
    syn_led_blink(&led, 100, 100);               /* Fast blink = ramping */
}

static void act_running(void *ctx)
{
    (void)ctx;
    SYN_LOG_I(TAG, "At speed — motor running");
    syn_led_on(&led);                             /* Solid = running */
}

static void act_start_ramp_down(void *ctx)
{
    (void)ctx;
    SYN_LOG_I(TAG, "Starting ramp down");
    syn_ramp_set_target(&speed_ramp, 0, 1);     /* ramp to 0 */
    syn_led_blink(&led, 100, 100);
}

static void act_stopped(void *ctx)
{
    (void)ctx;
    SYN_LOG_I(TAG, "Motor stopped");
    syn_led_blink(&led, 500, 500);               /* Slow blink = idle */
}

static void act_fault(void *ctx)
{
    (void)ctx;
    SYN_LOG_W(TAG, "FAULT — motor emergency stop");
    syn_ramp_set_target(&speed_ramp, 0, 10);    /* emergency ramp down */
    syn_led_pattern(&led, "... --- ... |", 100); /* SOS = fault */
}

/* ── Transition Table ─────────────────────────────────────────────────── */

static const SYN_FSM_Transition motor_transitions[] = {
    { ST_STOPPED,   EV_START,    ST_RAMP_UP,   NULL, act_start_ramp_up },
    { ST_RAMP_UP,   EV_AT_SPEED, ST_RUNNING,   NULL, act_running },
    { ST_RUNNING,   EV_STOP,     ST_RAMP_DOWN, NULL, act_start_ramp_down },
    { ST_RAMP_DOWN, EV_AT_ZERO,  ST_STOPPED,   NULL, act_stopped },
    /* Fault from any running state */
    { ST_RAMP_UP,   EV_FAULT,    ST_FAULT,     NULL, act_fault },
    { ST_RUNNING,   EV_FAULT,    ST_FAULT,     NULL, act_fault },
    { ST_RAMP_DOWN, EV_FAULT,    ST_FAULT,     NULL, act_fault },
    /* Reset from fault */
    { ST_FAULT,     EV_RESET,    ST_STOPPED,   NULL, act_stopped },
    SYN_FSM_END
};

/* ── CLI Command ──────────────────────────────────────────────────────── */

static int cmd_motor(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: motor <start|stop|fault|reset>\r\n");
        return 1;
    }
    SYN_FSM_Event ev = -1;
    if      (strcmp(argv[1], "start") == 0) ev = EV_START;
    else if (strcmp(argv[1], "stop")  == 0) ev = EV_STOP;
    else if (strcmp(argv[1], "fault") == 0) ev = EV_FAULT;
    else if (strcmp(argv[1], "reset") == 0) ev = EV_RESET;
    else { syn_cli_printf(&cli, "Unknown: %s\r\n", argv[1]); return 1; }

    bool ok = syn_fsm_dispatch(&fsm, ev);
    syn_cli_printf(&cli, "Motor: %s -> %s (state=%s)\r\n",
                   argv[1], ok ? "OK" : "IGNORED",
                   state_names[syn_fsm_state(&fsm)]);
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "motor", "Control the motor FSM", cmd_motor },
};

/* ── Platform hooks ───────────────────────────────────────────────────── */

extern "C" void syn_assert_failed(const char *f, int l) { (void)f; (void)l; for(;;); }

/* ── Tasks ────────────────────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) { syn_led_update(&led); PT_TASK_DELAY_MS(pt, task, 50); }
    PT_END(pt);
}

static SYN_PT_Status cli_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t ch; size_t n; SYN_Status st; (void)task;
    PT_BEGIN(pt);
    for (;;) {
        n = 0; st = syn_port_uart_receive(0, &ch, 1, &n, 1);
        if (st == SYN_OK && n > 0) syn_cli_process_char(&cli, (char)ch);
        PT_YIELD(pt);
    }
    PT_END(pt);
}

/**
 * Motor control task — updates the ramp and auto-fires
 * AT_SPEED / AT_ZERO events when the ramp completes.
 */
static SYN_PT_Status motor_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t last_print;
    static int32_t speed;
    PT_BEGIN(pt);
    for (;;) {
        syn_ramp_update(&speed_ramp);
        speed = syn_ramp_value(&speed_ramp);

        /* Auto-transition when ramp completes */
        if (syn_fsm_state(&fsm) == ST_RAMP_UP && syn_ramp_done(&speed_ramp)) {
            syn_fsm_dispatch(&fsm, EV_AT_SPEED);
        }
        if (syn_fsm_state(&fsm) == ST_RAMP_DOWN && syn_ramp_done(&speed_ramp)) {
            syn_fsm_dispatch(&fsm, EV_AT_ZERO);
        }

        /* Print speed periodically while ramping */
        {
            uint32_t now = syn_port_get_tick_ms();
            if (now - last_print >= 500 && !syn_ramp_done(&speed_ramp)) {
                last_print = now;
                SYN_LOG_I(TAG, "speed: %ld", speed);
            }
        }

        PT_TASK_DELAY_MS(pt, task, 20);
    }
    PT_END(pt);
}

/* ── Setup ────────────────────────────────────────────────────────────── */

void setup()
{
    syn_port_uart_init(0, 115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    syn_ramp_init(&speed_ramp, 0);
    syn_fsm_init(&fsm, motor_transitions, ST_STOPPED, "motor");
    syn_fsm_set_state_names(&fsm, state_names);

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "Motor FSM ready");

    syn_cli_init(&cli, commands, sizeof(commands)/sizeof(commands[0]),
                 "> ");
    syn_cli_set_scheduler(&sched);
    syn_cli_printf(&cli, "\r\n--- SyntropicOS Motor FSM ---\r\n");
    syn_cli_print_prompt(&cli);

    syn_task_create(&tasks[0], "blink", blink_task, 2, NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   1, NULL);
    syn_task_create(&tasks[2], "motor", motor_task, 1, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    syn_sched_run(&sched);
}
