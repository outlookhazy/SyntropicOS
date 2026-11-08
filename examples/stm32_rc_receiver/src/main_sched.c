/**
 * @file main_sched.c
 * @brief STM32 RC Receiver & Digital ESC Example (`syn_sched` Task Variant).
 *
 * Demonstrates SBUS receiver decoding and DShot motor control managed via syn_sched:
 * - High-priority SBUS RX frame task
 * - Periodic 50 Hz DShot ESC output task with failsafe protection
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/output/syn_dshot.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/proto/syn_sbus.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#define TASK_SBUS_RX   0
#define TASK_DSHOT_OUT 1
#define TASK_COUNT     2

static SYN_Task       s_tasks[TASK_COUNT];
static SYN_Sched      s_sched;
static SYN_SBUS_Parser s_sbus;

static void stm32_send_dshot_dma(const SYN_DShot_Packet *packet)
{
    (void)packet;
}

static SYN_PT_Status task_sbus_rx_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        uint8_t byte = 0x0F;
        SYN_SBUS_Frame frame;
        (void)syn_sbus_parse_byte(&s_sbus, byte, &frame);
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_dshot_out_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        SYN_SBUS_Frame frame = s_sbus.last_frame;

        uint16_t dshot_val = 0;
        if (!frame.failsafe && !frame.frame_loss) {
            uint16_t us = syn_sbus_raw_to_us(frame.channels[2]);
            dshot_val   = syn_dshot_us_to_throttle(us);
        }

        SYN_DShot_Packet pkt;
        if (syn_dshot_encode(dshot_val, false, &pkt) == SYN_OK) {
            stm32_send_dshot_dma(&pkt);
        }

        PT_TASK_DELAY_MS(pt, task, 20); /* 50 Hz update */
    }

    PT_END(pt);
}

int main_sched(void)
{
    syn_sbus_init(&s_sbus);

    syn_task_create(&s_tasks[TASK_SBUS_RX], "SbusRX", task_sbus_rx_func, SYN_PRIO_HIGH, NULL);
    syn_task_create(&s_tasks[TASK_DSHOT_OUT], "DshotOut", task_dshot_out_func, SYN_PRIO_NORMAL,
                    NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
