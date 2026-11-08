/**
 * @file main.c
 * @brief STM32 Severity-Filtered Logging Example (syn_log over UART).
 *
 * Demonstrates how to wire SyntropicOS `syn_log` to STM32 UART for
 * tagged, color-coded, timestamped logging and binary hex dumping.
 */

#include "syntropic/log/syn_log.h"
#include "syntropic/syntropic.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>
#include <string.h>

#define TAG "SYSTEM"

extern UART_HandleTypeDef huart2; /* USART2 (PA2 TX / PA3 RX) at 115200 8N1 */

/* ── STM32 UART Logging Output Backend ─────────────────────────────────── */

/**
 * @brief Logging backend callback passed to syn_log_init().
 * Transmits formatted log strings directly out the STM32 UART port.
 */
static void stm32_log_output(const char *str, size_t len)
{
    HAL_UART_Transmit(&huart2, (const uint8_t *)str, (uint16_t)len, 100);
}

/* ── Periodic Logging Task ──────────────────────────────────────────────── */

static SYN_PT_Status log_demo_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t counter = 0;
    static uint8_t sample_packet[] = {0x68, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x16};

    PT_BEGIN(pt);

    for (;;) {
        counter++;

        /* 1. Trace log (very fine-grained execution flow) */
        SYN_LOG_T(TAG, "Task tick iteration = %lu", (unsigned long)counter);

        /* 2. Debug log with variables */
        SYN_LOG_D(TAG, "ADC raw sample channel 0 = %u", 2048 + (uint16_t)(counter & 0xFF));

        /* 3. Informational log */
        SYN_LOG_I(TAG, "Heartbeat active, uptime = %lu ms", (unsigned long)syn_port_get_tick_ms());

        /* 4. Warning log */
        if (counter % 3 == 0) {
            SYN_LOG_W(TAG, "High CPU load detected on scheduled task");
        }

        /* 5. Error log */
        if (counter % 5 == 0) {
            SYN_LOG_E(TAG, "Sensor I2C bus NACK timeout error (-1)");
        }

        /* 6. Binary Hex Dump log */
        if (counter % 4 == 0) {
            syn_log_hexdump(TAG, sample_packet, sizeof(sample_packet));
        }

        /* Non-blocking delay for 1500ms */
        PT_TASK_DELAY_MS(pt, task, 1500);
    }

    PT_END(pt);
}

/* ── Main Application Entry Point ───────────────────────────────────────── */

int main(void)
{
    HAL_Init();
    /* MCU Clock & USART2 (115200 8N1) initialization here */

    /* Step 1: Initialize syn_log with custom STM32 UART output callback and minimum level */
    syn_log_init(stm32_log_output, SYN_LOG_TRACE);

    /* Log startup message */
    SYN_LOG_I(TAG, "SyntropicOS syn_log console initialized on USART2 @ 115200 baud");

    /* Step 2: Initialize scheduler & task */
    static SYN_Task tasks[1];
    static SYN_Sched sched;

    syn_task_create(&tasks[0], "log_demo", log_demo_task, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 1);

    /* Step 3: Run cooperative kernel loop */
    syn_sched_run_forever(&sched);
}
