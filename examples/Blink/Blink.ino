/**
 * SyntropicOS — Blink
 *
 * The simplest SyntropicOS program: one task, one LED.
 * The on-board LED blinks using the cooperative scheduler
 * and the LED pattern driver.
 *
 * Works on: Arduino Uno, Mega, Nano, ESP32, STM32duino, RP2040, etc.
 *
 * Documentation & Related Features:
 *   - Getting Started:      https://outlookhazy.github.io/SyntropicOS/getting-started/
 *   - Multitasking & Sched: https://outlookhazy.github.io/SyntropicOS/modules/multitasking/
 *   - LED Patterns:         https://outlookhazy.github.io/SyntropicOS/modules/io/
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>

static SYN_Sched sched;
static SYN_Task  tasks[1];
static SYN_LED   led;

/* Blink task — updates the LED pattern every 50ms */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

void setup()
{
    syn_port_serial_init(115200);
    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I("main", "SyntropicOS Blink Ready");

    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    syn_task_create(&tasks[0], "blink", blink_task, 0, NULL);
    syn_sched_init(&sched, tasks, 1);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
