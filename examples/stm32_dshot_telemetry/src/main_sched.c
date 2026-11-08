/**
 * @file main_sched.c
 * @brief STM32 BDShot Telemetry Example (`syn_sched` Task Variant).
 *
 * Demonstrates BDShot telemetry frame ingestion and periodic motor speed logging
 * managed via syn_sched protothreads:
 * - Task 0: High-frequency telemetry pulse / GCR bitstream ingestion task
 * - Task 1: Periodic 20 Hz motor telemetry & RPM monitoring task
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/output/syn_dshot_telemetry.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#define TASK_GCR_RX   0
#define TASK_TELEM_LOG 1
#define TASK_COUNT    2

#define MOTOR_POLE_PAIRS 7U

static SYN_Task            s_tasks[TASK_COUNT];
static SYN_Sched           s_sched;
static SYN_DShot_Telemetry s_latest_telemetry;

static SYN_PT_Status task_gcr_rx_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        uint32_t raw_gcr = 0xAA555;
        (void)syn_dshot_parse_telemetry(raw_gcr, MOTOR_POLE_PAIRS, &s_latest_telemetry);
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_telem_log_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        if (s_latest_telemetry.valid) {
            uint32_t rpm  = s_latest_telemetry.rpm;
            uint32_t erpm = s_latest_telemetry.erpm;
            (void)rpm;
            (void)erpm;
        }

        PT_TASK_DELAY_MS(pt, task, 50); /* 20 Hz monitoring task */
    }

    PT_END(pt);
}

int main_sched(void)
{
    syn_task_create(&s_tasks[TASK_GCR_RX], "GcrRx", task_gcr_rx_func, SYN_PRIO_HIGH, NULL);
    syn_task_create(&s_tasks[TASK_TELEM_LOG], "TelemLog", task_telem_log_func, SYN_PRIO_NORMAL,
                    NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
