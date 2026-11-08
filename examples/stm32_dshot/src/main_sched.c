/**
 * @file main_sched.c
 * @brief STM32 DShot Digital ESC Example (`syn_sched` Task Variant).
 *
 * Demonstrates periodic DShot ESC command encoding and motor telemetry monitoring
 * managed via syn_sched protothreads:
 * - Task 0: High-frequency 1 kHz DShot frame output task
 * - Task 1: Periodic ESC status & telemetry monitor task
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/output/syn_dshot.h"
#include "syntropic/output/syn_dshot_telemetry.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#define TASK_DSHOT_TX 0
#define TASK_TELEMETRY 1
#define TASK_COUNT    2

static SYN_Task       s_tasks[TASK_COUNT];
static SYN_Sched      s_sched;
static uint16_t       s_throttle_cmd = 0;

static void stm32_dshot_send_frame_dma(uint16_t raw_frame)
{
    (void)raw_frame;
}

static SYN_PT_Status task_dshot_tx_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        SYN_DShot_Packet packet;
        if (syn_dshot_encode(s_throttle_cmd, false, &packet) == SYN_OK) {
            stm32_dshot_send_frame_dma(packet.raw_frame);
        }

        PT_TASK_DELAY_MS(pt, task, 1); /* 1 kHz DShot rate */
    }

    PT_END(pt);
}

static SYN_PT_Status task_telemetry_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        uint32_t sample_gcr = 0xAA555;
        SYN_DShot_Telemetry telem;
        if (syn_dshot_parse_telemetry(sample_gcr, 7, &telem) == SYN_OK) {
            uint32_t erpm = telem.erpm;
            (void)erpm;
        }

        PT_TASK_DELAY_MS(pt, task, 50); /* 20 Hz telemetry task */
    }

    PT_END(pt);
}

int main_sched(void)
{
    syn_task_create(&s_tasks[TASK_DSHOT_TX], "DShotTx", task_dshot_tx_func, SYN_PRIO_CRITICAL,
                    NULL);
    syn_task_create(&s_tasks[TASK_TELEMETRY], "Telem", task_telemetry_func, SYN_PRIO_NORMAL, NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
