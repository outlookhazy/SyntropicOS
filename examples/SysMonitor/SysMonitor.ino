/**
 * SyntropicOS — System Monitor & Task Watchdog
 *
 * Demonstrates production firmware reliability patterns:
 *   - Task-level watchdog monitoring (syn_watchdog)
 *   - Circular error logging (syn_errlog)
 *   - Runtime execution profiling (syn_profiler)
 *   - Diagnostic CLI (syn_cli)
 *   - Cooperative task scheduling (syn_sched)
 *
 * Three tasks check in with the software watchdog. A deliberate task stall
 * triggers the watchdog timeout callback, logging a fault to syn_errlog
 * without crashing the rest of the system.
 *
 * Open Serial Monitor at 115200 baud.
 *
 * Documentation & Related Features:
 *   - System & Watchdog:    https://outlookhazy.github.io/SyntropicOS/modules/system/
 *   - System API Ref:       https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__system/
 *   - Crash Core Dumps:     https://outlookhazy.github.io/SyntropicOS/modules/debug/ (syn_coredump_save)
 *   - Flash Wear Leveling:  https://outlookhazy.github.io/SyntropicOS/modules/storage/ (syn_param)
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/sched/syn_watchdog.h>
#include <syntropic/system/syn_errlog.h>
#include <syntropic/debug/syn_profiler.h>
#include <syntropic/output/syn_led.h>
#include <string.h>
#include <stdlib.h>

#define TAG "sysmon"

/* Error codes */
#define ERR_WDT_TIMEOUT 0xE001

static SYN_CLI       cli;
static SYN_Sched     sched;
static SYN_Task      tasks[3];
static SYN_LED       led;

/* Watchdog & Error Log Instances */
static SYN_Watchdog  wdt;
static SYN_WDT_Entry wdt_entries[3];
static int8_t        wdt_id_sensor = -1;
static int8_t        wdt_id_comms  = -1;
static int8_t        wdt_id_staller= -1;

static SYN_ErrLog    errlog;
static SYN_ErrEntry  err_entries[8];

static bool          staller_should_stall = false;

/* ── Watchdog Callback ────────────────────────────────────────────────── */

static void on_wdt_timeout(SYN_Watchdog *w, const SYN_WDT_Entry *entry, void *ctx)
{
    (void)w; (void)ctx;
    syn_cli_printf(&cli, "[WDT] Timeout detected on task: %s!\r\n", entry->name ? entry->name : "unknown");
    syn_errlog_record(&errlog, ERR_WDT_TIMEOUT, SYN_ERR_WARNING, (uint32_t)entry->name);
    syn_led_pattern(&led, "... --- ... |", 100); /* Flash SOS pattern on fault */
}

/* ── CLI Commands ─────────────────────────────────────────────────────── */

static int cmd_sys(int argc, char *argv[])
{
    (void)argc; (void)argv;
    syn_cli_printf(&cli, "--- System Monitor Status ---\r\n");
    syn_cli_printf(&cli, "Uptime:     %lu ms\r\n", (unsigned long)syn_port_get_tick_ms());
    syn_cli_printf(&cli, "ErrLog:     %u entries (%u total recorded)\r\n",
                   (unsigned)syn_errlog_available(&errlog),
                   (unsigned)syn_errlog_count(&errlog));
    syn_cli_printf(&cli, "WDT Tasks:  %u registered\r\n", wdt.count);
    return 0;
}

static int cmd_errors(int argc, char *argv[])
{
    (void)argc; (void)argv;
    syn_cli_printf(&cli, "--- Error Log History ---\r\n");
    size_t count = syn_errlog_available(&errlog);
    if (count == 0) {
        syn_cli_printf(&cli, "No errors recorded.\r\n");
        return 0;
    }
    for (size_t i = 0; i < count; i++) {
        SYN_ErrEntry e;
        if (syn_errlog_read(&errlog, i, &e)) {
            syn_cli_printf(&cli, "[%u] code=0x%04X sev=%u tick=%lu count=%lu\r\n",
                           (unsigned)i, e.code, e.severity,
                           (unsigned long)e.timestamp, (unsigned long)e.boot_count);
        }
    }
    return 0;
}

static int cmd_stall(int argc, char *argv[])
{
    (void)argc; (void)argv;
    staller_should_stall = true;
    syn_cli_printf(&cli, "Triggered intentional stall on 'staller' task...\r\n");
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "sys",    "Print system monitor & uptime status", cmd_sys },
    { "errors", "Dump circular error log history",      cmd_errors },
    { "stall",  "Trigger intentional task stall test",  cmd_stall },
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

/* System Monitor Task — updates watchdog and checks system health every 500ms */
static SYN_PT_Status monitor_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t log_timer = 0;
    PT_BEGIN(pt);
    for (;;) {
        /* Check all task deadlines */
        syn_watchdog_update(&wdt);

        log_timer += 500;
        if (log_timer >= 2000) {
            log_timer = 0;
            size_t errs = syn_errlog_available(&errlog);
            if (errs == 0) {
                syn_cli_printf(&cli, "SysMonitor HEALTH: OK (wdt_tasks=%u uptime=%lums)\r\n",
                               wdt.count, (unsigned long)syn_port_get_tick_ms());
            } else {
                syn_cli_printf(&cli, "SysMonitor HEALTH: DEGRADED (errors=%u)\r\n", (unsigned)errs);
            }
        }

        PT_TASK_DELAY_MS(pt, task, 500);
    }
    PT_END(pt);
}

/* Worker Task — periodic check-ins with software watchdog */
static SYN_PT_Status worker_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        if (!staller_should_stall) {
            syn_watchdog_checkin(&wdt, wdt_id_staller);
        } else {
            /* Deliberately skip check-in to demonstrate WDT catch */
        }
        syn_watchdog_checkin(&wdt, wdt_id_sensor);
        syn_watchdog_checkin(&wdt, wdt_id_comms);

        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

/* ── Entry Point ──────────────────────────────────────────────────────── */

void setup()
{
    syn_port_serial_init(115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Initialize Error Log */
    syn_errlog_init(&errlog, err_entries, sizeof(err_entries) / sizeof(err_entries[0]), 1);

    /* Initialize Task Watchdog */
    syn_watchdog_init(&wdt, wdt_entries, sizeof(wdt_entries) / sizeof(wdt_entries[0]),
                      on_wdt_timeout, NULL);
    wdt_id_sensor  = syn_watchdog_register(&wdt, "sensor", 2000);
    wdt_id_comms   = syn_watchdog_register(&wdt, "comms",  2000);
    wdt_id_staller = syn_watchdog_register(&wdt, "staller",1000);

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "System Monitor Ready");

    syn_cli_init(&cli, commands, sizeof(commands) / sizeof(commands[0]), "> ");
    syn_cli_printf(&cli, "\r\n--- SyntropicOS SysMonitor Demo ---\r\n");

    syn_task_create(&tasks[0], "blink",   blink_task,   2, NULL);
    syn_task_create(&tasks[1], "monitor", monitor_task, 1, NULL);
    syn_task_create(&tasks[2], "worker",  worker_task,  0, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
