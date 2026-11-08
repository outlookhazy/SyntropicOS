/**
 * @file main_sched.c
 * @brief STM32 PPM RC Receiver Example (`syn_sched` Task Variant).
 *
 * Demonstrates PPM frame ingestion and periodic channel telemetry processing
 * managed via syn_sched protothreads:
 * - Task 0: High-frequency PPM pulse processing task
 * - Task 1: Periodic 50 Hz RC channel output & flight control task
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/input/syn_ppm.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#define TASK_PPM_RX     0
#define TASK_CONTROL    1
#define TASK_COUNT      2

static SYN_Task        s_tasks[TASK_COUNT];
static SYN_Sched       s_sched;
static SYN_PPM_Decoder  s_ppm;

static SYN_PT_Status task_ppm_rx_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        /* Ingest incoming timer capture pulse width */
        uint16_t pulse_us = 1500;
        (void)syn_ppm_process_pulse(&s_ppm, pulse_us);
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_control_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        if (s_ppm.in_frame || s_ppm.frames_received > 0) {
            uint16_t roll     = syn_ppm_get_channel(&s_ppm, 0);
            uint16_t pitch    = syn_ppm_get_channel(&s_ppm, 1);
            uint16_t throttle = syn_ppm_get_channel(&s_ppm, 2);
            uint16_t yaw      = syn_ppm_get_channel(&s_ppm, 3);
            (void)roll;
            (void)pitch;
            (void)throttle;
            (void)yaw;
        }

        PT_TASK_DELAY_MS(pt, task, 20); /* 50 Hz update */
    }

    PT_END(pt);
}

int main_sched(void)
{
    syn_ppm_init(&s_ppm);

    syn_task_create(&s_tasks[TASK_PPM_RX], "PpmRx", task_ppm_rx_func, SYN_PRIO_HIGH, NULL);
    syn_task_create(&s_tasks[TASK_CONTROL], "Control", task_control_func, SYN_PRIO_NORMAL, NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
