/**
 * SyntropicOS -- Bare-Metal Pico SDK Dual-Core AMP Example
 *
 * Demonstrates Asymmetric Multi-Processing on the RP2040 using the
 * native Pico SDK port (no Arduino dependency):
 *
 *   Core 0: Producer -- increments a counter and posts messages
 *           to a cross-core mailbox every 200 ms.
 *   Core 1: Consumer -- receives mailbox messages, adjusts the LED
 *           blink rate, and logs to USB serial.
 *
 * Cross-core communication uses the SPSC mailbox with acquire/release
 * memory barriers.  USB serial output is protected by a hardware
 * spinlock to prevent garbled output from concurrent cores.
 *
 * Build:
 *   mkdir build && cd build
 *   cmake -G Ninja -DPICO_BOARD=pico ..
 *   cmake --build .
 *
 * Flash:
 *   Copy pico_dual_core.uf2 to the Pico in BOOTSEL mode, or type
 *   "bootloader" at the synos> prompt to reboot into BOOTSEL.
 *
 * Board: Raspberry Pi Pico (rp2040)
 */

#include "syn_config.h"

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_spinlock.h"
#include "syntropic/common/syn_barrier.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/sched/syn_mailbox.h"
#include "syntropic/output/syn_led.h"
#include "syntropic/log/syn_log.h"
#include "syntropic/util/syn_spinlock.h"
#include "syntropic/cli/syn_cli.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include <stdio.h>
#include <string.h>

/* -- Pin Definitions ---------------------------------------------------- */

#define LED_PIN  25

/* -- Cross-core mailbox ------------------------------------------------- */

typedef struct {
    uint32_t seq;        /* Sequence number */
    uint32_t tick;       /* Timestamp from producer */
    uint32_t value;      /* Payload value */
} DualCoreMsg;

SYN_MAILBOX_DEFINE(cross_core_mbox, DualCoreMsg, 8);

/* -- Spinlock-protected serial output ----------------------------------- */

static void serial_output(const char *str, size_t len)
{
    syn_port_spinlock_acquire(SYN_SPINLOCK_UART);
    fwrite(str, 1, len, stdout);
    fflush(stdout);
    syn_port_spinlock_release(SYN_SPINLOCK_UART);
}

/* ======================================================================
 * CORE 0 -- Producer
 * ====================================================================== */

#define TAG_C0 "C0"

static SYN_Sched sched0;
static SYN_Task  tasks0[1];
static uint32_t  producer_seq = 0;

static DualCoreMsg producer_msg;
static bool        producer_ok;

static SYN_PT_Status producer_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        producer_msg.seq   = producer_seq++;
        producer_msg.tick  = syn_port_get_tick_ms();
        producer_msg.value = producer_msg.seq * 7;  /* arbitrary payload */

        producer_ok = syn_mailbox_post(&cross_core_mbox, &producer_msg);

        if (producer_ok) {
            SYN_LOG_I(TAG_C0, "POST seq=%lu val=%lu",
                      (unsigned long)producer_msg.seq,
                      (unsigned long)producer_msg.value);
        } else {
            SYN_LOG_W(TAG_C0, "MBOX FULL (overflow=%lu)",
                      (unsigned long)syn_mailbox_overflows(&cross_core_mbox));
        }

        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

/* ======================================================================
 * CORE 1 -- Consumer
 * ====================================================================== */

#define TAG_C1 "C1"

static SYN_Sched sched1;
static SYN_Task  tasks1[2];
static SYN_LED   led;

static volatile uint32_t last_received_value = 0;

static DualCoreMsg consumer_rx;
static uint16_t    consumer_rate;

static SYN_PT_Status consumer_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        while (syn_mailbox_receive(&cross_core_mbox, &consumer_rx)) {
            last_received_value = consumer_rx.value;

            /* Adjust blink rate based on received value.
             * Update timing directly -- don't call syn_led_blink()
             * which would reset the phase and force LED ON. */
            consumer_rate = 100 + (uint16_t)(consumer_rx.value % 400);
            led.on_ms  = consumer_rate;
            led.off_ms = consumer_rate;

            SYN_LOG_I(TAG_C1, "RECV seq=%lu val=%lu blink=%ums",
                      (unsigned long)consumer_rx.seq,
                      (unsigned long)consumer_rx.value,
                      (unsigned)consumer_rate);
        }

        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

static SYN_PT_Status led_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 25);
    }
    PT_END(pt);
}

/* -- Core 1 entry point ------------------------------------------------- */

static void core1_entry(void)
{
    syn_task_create(&tasks1[0], "consumer", consumer_task, 0, NULL);
    syn_task_create(&tasks1[1], "led",      led_task,      1, NULL);
    syn_sched_init(&sched1, tasks1, 2);

    SYN_LOG_I(TAG_C1, "Core 1 scheduler started");

    syn_sched_run_forever(&sched1);
}

/* ======================================================================
 * CLI
 * ====================================================================== */

static void cli_putchar(char ch)
{
    syn_port_spinlock_acquire(SYN_SPINLOCK_UART);
    putchar(ch);
    fflush(stdout);
    syn_port_spinlock_release(SYN_SPINLOCK_UART);
}

static int cmd_bootloader(int argc, char *argv[])
{
    (void)argc; (void)argv;
    printf("Rebooting to BOOTSEL...\n");
    fflush(stdout);
    sleep_ms(100);
    reset_usb_boot(0, 0);
    return 0;  /* unreachable */
}

static const SYN_CLI_Command cli_commands[] = {
    { "bootloader", "Reboot into UF2 bootloader", cmd_bootloader },
};

static SYN_CLI cli;

/* ======================================================================
 * main() -- runs on Core 0
 * ====================================================================== */

int main(void)
{
    stdio_init_all();
    sleep_ms(1500);

    printf("\n=== SyntropicOS Bare-Metal Dual-Core AMP Example ===\n\n");

    /* Logging (spinlock-protected output) */
    syn_log_init(serial_output, SYN_LOG_DEBUG);

    /* CLI */
    syn_cli_init(&cli, cli_commands, 1, cli_putchar, "synos> ");
    syn_cli_set_echo(&cli, true);

    /* LED (owned by Core 1, but init here before launch) */
    syn_led_init(&led, LED_PIN, SYN_LED_ACTIVE_HIGH);
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

    syn_cli_print_prompt(&cli);

    for (;;) {
        /* Feed CLI from USB serial */
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            syn_cli_process_char(&cli, (char)c);
        }

        syn_sched_run(&sched0);
    }

    return 0;
}
