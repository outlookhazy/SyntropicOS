/**
 * @file main.c
 * @brief Demonstration application for WCH CH32V307V-EVT-R1 using SyntropicOS.
 *
 * Configures board peripherals and runs cooperative protothread tasks
 * for LED blinking, ADC telemetry, and CAN packet transmission.
 */

#include "board_evt.h"
#include "syntropic/port/syn_port_adc.h"
#include "syntropic/port/syn_port_can.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/syntropic.h"

#include <stdio.h>

/* ── Scheduler & Task Array ─────────────────────────────────────────────── */

#define NUM_TASKS 3

static SYN_Task g_tasks[NUM_TASKS];
static SYN_Sched g_sched;

static uint32_t s_blink_counter = 0;
static uint32_t s_can_tx_counter = 0;

/* ── Protothread Task 1: Onboard LED Blinker ────────────────────────────── */

static SYN_PT_Status task_blink_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        /* Toggle CH32V307V-EVT-R1 onboard LEDs (PA15 & PB4) */
        board_led_toggle(BOARD_LED1_PIN);
        board_led_toggle(BOARD_LED2_PIN);

        s_blink_counter++;

        /* Non-blocking 250 ms delay via SyntropicOS Protothread macro */
        PT_TASK_DELAY_MS(pt, task, 250);
    }

    PT_END(pt);
}

/* ── Protothread Task 2: System & Analog Telemetry Stream ───────────────── */

static SYN_PT_Status task_telemetry_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        /* Read ADC1 Channel 1 (PA1) analog voltage */
        uint16_t adc_raw = syn_port_adc_read_channel(0, 1);

        /* Read KEY1 button status (PA0) */
        bool key1_pressed = board_button_read(BOARD_KEY1_PIN);

        printf("[TELEMETRY %5lu ms] Cycles: %lu | ADC1(PA1): %u raw (%1.2f V) | KEY1: %s\r\n",
               (unsigned long)syn_port_get_tick_ms(), (unsigned long)s_blink_counter,
               (unsigned int)adc_raw, (double)adc_raw * 3.3 / 4095.0,
               key1_pressed ? "PRESSED" : "RELEASED");

        /* Non-blocking 1000 ms delay via SyntropicOS Protothread macro */
        PT_TASK_DELAY_MS(pt, task, 1000);
    }

    PT_END(pt);
}

/* ── Protothread Task 3: CAN1 Bus Packet Broadcast ─────────────────────── */

static SYN_PT_Status task_can_fn(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        /* Construct 8-byte CAN 2.0B diagnostic frame */
        uint8_t payload[8];
        uint32_t tick = syn_port_get_tick_ms();

        payload[0] = 0x07U;                                 /* DLC length */
        payload[1] = 0x62U;                                 /* ReadDataByIdentifier Response */
        payload[2] = 0xF1U;                                 /* DID High Byte */
        payload[3] = 0x90U;                                 /* DID Low Byte */
        payload[4] = (uint8_t)(tick >> 24);                /* Timestamp byte 3 */
        payload[5] = (uint8_t)(tick >> 16);                /* Timestamp byte 2 */
        payload[6] = (uint8_t)(tick >> 8);                 /* Timestamp byte 1 */
        payload[7] = (uint8_t)(s_can_tx_counter++ & 0xFF); /* Packet counter */

        syn_port_can_send(0, 0x7E8U, false, payload, 8U);

        /* Non-blocking 500 ms delay via SyntropicOS Protothread macro */
        PT_TASK_DELAY_MS(pt, task, 500);
    }

    PT_END(pt);
}

/* ── Main Application Entry ─────────────────────────────────────────────── */

int main(void)
{
    /* 1. Initialize CH32V307V-EVT-R1 Board Hardware & Peripherals */
    board_init();

    printf("\r\n===================================================================\r\n");
    printf("  SyntropicOS Demo - WCH CH32V307V-EVT-R1 Board\r\n");
    printf("  Target Core: QingKe V4F RISC-V @ 144 MHz (256KB Flash / 64KB SRAM)\r\n");
    printf("===================================================================\r\n\r\n");

    /* 2. Create Scheduler Tasks with Protothread Functions */
    syn_task_create(&g_tasks[0], "blink", task_blink_fn, SYN_PRIO_LOW, NULL);
    syn_task_create(&g_tasks[1], "telemetry", task_telemetry_fn, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&g_tasks[2], "can_tx", task_can_fn, SYN_PRIO_HIGH, NULL);

    /* 3. Initialize Scheduler with Task Array */
    syn_sched_init(&g_sched, g_tasks, NUM_TASKS);

    /* 4. Run Cooperative Scheduler Loop */
    while (1) {
        syn_sched_run(&g_sched);
    }

    return 0;
}
