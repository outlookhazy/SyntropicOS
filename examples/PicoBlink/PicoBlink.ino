/**
 * SyntropicOS — Pi Pico Blink + Diagnostics
 *
 * Two-task test sketch for the Raspberry Pi Pico bring-up.
 * Validates: GPIO, scheduler, tick source, USB serial, and logging.
 *
 * Board: Raspberry Pi Pico (arduino-pico core)
 * Upload: Hold BOOTSEL → plug USB → release → Upload
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/log/syn_log.h>
#include <syntropic/drivers/syn_gpio.h>

/* ── Pin Definitions ────────────────────────────────────────────────────── */

#ifndef LED_BUILTIN
#define LED_BUILTIN 25   /* GP25 = on-board LED on the Pico */
#endif

/* ── Globals ────────────────────────────────────────────────────────────── */

#define TAG "pico"

static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   led;

/* ── Tasks ─────────────────────────────────────────────────────────────── */

/* ── Task 1: Blink LED ──────────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* ── Task 2: Serial report ──────────────────────────────────────────────── */

static SYN_PT_Status report_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        SYN_LOG_I(TAG, "tick=%lu  led=%s",
                  (unsigned long)syn_port_get_tick_ms(),
                  syn_led_is_on(&led) ? "ON" : "OFF");
        PT_TASK_DELAY_MS(pt, task, 1000);
    }
    PT_END(pt);
}

/* ── Arduino setup & loop ───────────────────────────────────────────────── */

void setup()
{
    /* USB serial — wait briefly for the host to connect */
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("=== SyntropicOS Pi Pico Bring-Up ===");
    Serial.println();

    /* Initialize logging over USB serial */
    syn_log_init(SYN_LOG_DEBUG);

    /* LED on GP25: 500ms on / 500ms off */
    syn_gpio_init(LED_BUILTIN, SYN_GPIO_OUTPUT);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Create tasks */
    syn_task_create(&tasks[0], "blink",  blink_task,  0, NULL);
    syn_task_create(&tasks[1], "report", report_task, 1, NULL);
    syn_sched_init(&sched, tasks, 2);

    SYN_LOG_I(TAG, "Scheduler started with %u tasks",
              (unsigned)sched.task_count);
}

void loop()
{
    syn_sched_run(&sched);
}
