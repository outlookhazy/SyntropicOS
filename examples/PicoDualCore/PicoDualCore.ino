/**
 * SyntropicOS — Pi Pico Dual-Core AMP Demo
 *
 * Demonstrates Asymmetric Multi-Processing on the RP2040:
 * - Core 0: Producer — increments a counter and posts messages
 *           to a cross-core mailbox every 200ms.
 * - Core 1: Consumer — receives mailbox messages, blinks the LED
 *           based on received values, and logs to USB serial.
 *
 * Cross-core communication uses the SPSC mailbox with acquire/release
 * memory barriers. USB serial output is protected by a hardware
 * spinlock to prevent garbled output.
 *
 * Board: Raspberry Pi Pico (arduino-pico core)
 *
 * Documentation & Related Features:
 *   - Multitasking & Mailbox:https://outlookhazy.github.io/SyntropicOS/modules/multitasking/ (syn_mailbox.h)
 *   - Spinlocks & Barrier:  https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__sched/ (syn_spinlock.h)
 */

#include "syn_config.h"
#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/sched/syn_mailbox.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/log/syn_log.h>
#include <syntropic/port/syn_port_spinlock.h>
#include <syntropic/util/syn_spinlock.h>

#include "pico/multicore.h"

/* ── Pin Definitions ────────────────────────────────────────────────────── */

#ifndef LED_BUILTIN
#define LED_BUILTIN 25
#endif

/* ── Cross-core mailbox ─────────────────────────────────────────────────── */

typedef struct {
    uint32_t seq;        /* Sequence number */
    uint32_t tick;       /* Timestamp from producer */
    uint32_t value;      /* Payload value */
} DualCoreMsg;

SYN_MAILBOX_DEFINE(cross_core_mbox, DualCoreMsg, 8);

/* ════════════════════════════════════════════════════════════════════════
 * CORE 0 — Producer
 * ════════════════════════════════════════════════════════════════════════ */

#define TAG_C0 "C0"

static SYN_Sched sched0;
static SYN_Task  tasks0[1];
static uint32_t  producer_seq = 0;

/* Producer task: post messages to the cross-core mailbox */
static SYN_PT_Status producer_task(SYN_PT *pt, SYN_Task *task)
{
    static DualCoreMsg msg;
    static bool ok;

    PT_BEGIN(pt);
    for (;;) {
        msg.seq   = producer_seq++;
        msg.tick  = syn_port_get_tick_ms();
        msg.value = msg.seq * 7;  /* arbitrary payload */

        ok = syn_mailbox_post(&cross_core_mbox, &msg);

        if (ok) {
            SYN_LOG_I(TAG_C0, "POST seq=%lu val=%lu",
                      (unsigned long)msg.seq,
                      (unsigned long)msg.value);
        } else {
            SYN_LOG_W(TAG_C0, "MBOX FULL (overflow=%lu)",
                      (unsigned long)syn_mailbox_overflows(&cross_core_mbox));
        }

        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

/* ════════════════════════════════════════════════════════════════════════
 * CORE 1 — Consumer
 * ════════════════════════════════════════════════════════════════════════ */

#define TAG_C1 "C1"

static SYN_Sched sched1;
static SYN_Task  tasks1[2];
static SYN_LED   led;

static volatile uint32_t last_received_value = 0;

/* Consumer task: drain the mailbox and process messages */
static SYN_PT_Status consumer_task(SYN_PT *pt, SYN_Task *task)
{
    static DualCoreMsg rx;
    static uint16_t rate;
    static uint16_t prev_rate = 0;

    PT_BEGIN(pt);
    for (;;) {
        while (syn_mailbox_receive(&cross_core_mbox, &rx)) {
            last_received_value = rx.value;

            /* Adjust blink rate based on received value.
             * Update timing directly — don't call syn_led_blink()
             * which would reset the phase and force LED ON. */
            rate = 100 + (uint16_t)(rx.value % 400);
            led.on_ms  = rate;
            led.off_ms = rate;

            SYN_LOG_I(TAG_C1, "RECV seq=%lu val=%lu blink=%ums",
                      (unsigned long)rx.seq,
                      (unsigned long)rx.value,
                      (unsigned)rate);
        }

        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* LED update task on Core 1 */
static SYN_PT_Status led_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 25);
    }
    PT_END(pt);
}

/* ── Core 1 entry point ────────────────────────────────────────────────── */

static void core1_entry(void)
{
    /* Initialize Core 1 scheduler and run forever */
    syn_task_create(&tasks1[0], "consumer", consumer_task, 0, NULL);
    syn_task_create(&tasks1[1], "led",      led_task,      1, NULL);
    syn_sched_init(&sched1, tasks1, 2);

    SYN_LOG_I(TAG_C1, "Core 1 scheduler started");

    syn_sched_run_forever(&sched1);
}

#include <syntropic/cli/syn_cli.h>



static int cmd_bootloader(int argc, char *argv[])
{
    (void)argc; (void)argv;
    Serial.println("Rebooting to BOOTSEL...");
    Serial.flush();
    delay(100);
    rp2040.rebootToBootloader();
    return 0;
}

static const SYN_CLI_Command cli_commands[] = {
    { "bootloader", "Reboot into UF2 bootloader", cmd_bootloader },
};

static SYN_CLI cli;

void setup()
{
    /* USB serial */
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("=== SyntropicOS Dual-Core AMP Demo ===");
    Serial.println();

    /* Logging */
    syn_log_init(SYN_LOG_DEBUG);

    /* CLI */
    syn_cli_init(&cli, cli_commands, 1, "synos> ");
    syn_cli_set_echo(&cli, true);
    syn_cli_print_prompt(&cli);

    /* LED (owned by Core 1, but init here before launch) */
    syn_gpio_init(LED_BUILTIN, SYN_GPIO_OUTPUT);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Enable cross-core notification on mailbox post */
    syn_mailbox_set_notify(&cross_core_mbox, true);

    SYN_LOG_I(TAG_C0, "Launching Core 1...");

    /* Launch Core 1 */
    multicore_launch_core1(core1_entry);

    /* Core 0 scheduler setup */
    syn_task_create(&tasks0[0], "producer", producer_task, 0, NULL);
    syn_sched_init(&sched0, tasks0, 1);

    SYN_LOG_I(TAG_C0, "Core 0 scheduler started");
}

void loop()
{
    /* Feed CLI from serial input */
    while (Serial.available()) {
        syn_cli_process_char(&cli, (char)Serial.read());
    }

    syn_sched_run(&sched0);
}

