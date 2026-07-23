/**
 * @file TaskWatchdog.ino
 * @brief SyntropicOS Non-Blocking Task-Level Watchdog Example
 *
 * Demonstrates:
 *   - Task-level software watchdog (syn_watchdog) monitoring individual task deadlines
 *   - Task registration (syn_watchdog_register), periodic check-ins (syn_watchdog_checkin)
 *   - Detection of task stalls / deadlocks and execution of deadline miss callbacks
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Open Serial Monitor at 115200 baud
 *   2. Observe normal task check-ins for the first 5 seconds
 *   3. Watch the software watchdog detect the intentional stall of 'faulty_task' and report the violation!
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/sched/syn_watchdog.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    /* Safe fallback */
}
#endif

/* ── Global Declarations ───────────────────────────────────────────────── */

static SYN_Watchdog   wdt;
static SYN_WDT_Entry wdt_entries[2];
static int8_t         wdt_healthy_id = -1;
static int8_t         wdt_faulty_id  = -1;

static SYN_Sched sched;
static SYN_Task  tasks[3];
static SYN_LED   status_led;

#define STATUS_LED_PIN 7  /* Pin 7 status LED */

/* ── Callbacks ─────────────────────────────────────────────────────────── */

/** Watchdog Deadline Miss Callback */
static void on_wdt_timeout(SYN_Watchdog *wdt_ptr, const SYN_WDT_Entry *entry, void *ctx)
{
    (void)wdt_ptr; (void)ctx;
    Serial.print(F("\r\n[WATCHDOG ALARM] Task '"));
    Serial.print(entry->name);
    Serial.print(F("' missed check-in deadline of "));
    Serial.print(entry->timeout_ms);
    Serial.println(F(" ms!"));
}

/* ── Protothread Tasks ─────────────────────────────────────────────────── */

/** Task 1: Blink Status LED */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/** Task 2: Healthy Task — Continually checks in every 500ms */
static SYN_PT_Status healthy_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_watchdog_checkin(&wdt, wdt_healthy_id);
        PT_TASK_DELAY_MS(pt, task, 500);
    }
    PT_END(pt);
}

/** Task 3: Faulty Task — Checks in for 5s, then intentionally stalls */
static SYN_PT_Status faulty_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t start_ms = 0;
    PT_BEGIN(pt);
    start_ms = millis();
    for (;;) {
        if (millis() - start_ms < 5000) {
            syn_watchdog_checkin(&wdt, wdt_faulty_id);
            Serial.println(F("[HEALTHY] Faulty task checking in normally..."));
        } else if (millis() - start_ms < 6000) {
            Serial.println(F("[SIMULATED STALL] Faulty task STOPPING check-ins!"));
        }

        PT_TASK_DELAY_MS(pt, task, 1000);
    }
    PT_END(pt);
}

/** Task 4: Watchdog Monitor Update Task */
static SYN_PT_Status watchdog_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_watchdog_update(&wdt);
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}

/* ── Setup & Main Loop ─────────────────────────────────────────────────── */

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS Task Watchdog Monitor Example ==="));

    /* 1. Initialize Software Watchdog Engine */
    syn_watchdog_init(&wdt, wdt_entries, 2, on_wdt_timeout, NULL);

    /* 2. Register Monitored Tasks */
    wdt_healthy_id = syn_watchdog_register(&wdt, "healthy_task", 2000);  /* 2s deadline */
    wdt_faulty_id  = syn_watchdog_register(&wdt, "faulty_task",  3000);  /* 3s deadline */

    Serial.println(F("[WDT INITIALIZED] Monitored 'healthy_task' (2s) and 'faulty_task' (3s)."));

    /* 3. Initialize LED */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    /* 4. Register Scheduler Tasks */
    syn_task_create(&tasks[0], "wdt_mon",  watchdog_task, 1, NULL);
    syn_task_create(&tasks[1], "healthy",  healthy_task,  2, NULL);
    syn_task_create(&tasks[2], "faulty",   faulty_task,   3, NULL);

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. Watchdog active."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
