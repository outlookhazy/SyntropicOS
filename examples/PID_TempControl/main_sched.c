/**
 * @file main_sched.c
 * @brief PID Temperature Controller Example (`syn_sched` Cooperative Task Scheduler Variant).
 *
 * Demonstrates 100 Hz PID control loop task + 1 Hz telemetry task scheduled
 * cleanly via priority protothread task queues (syn_sched).
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "syntropic/control/syn_pid.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/sched/syn_sched.h"

static SYN_PID s_pid;
static int32_t s_setpoint_temp = 50;
static int32_t s_current_temp  = 20;

#define TASK_PID_CONTROL 0
#define TASK_TELEMETRY   1
#define TASK_COUNT       2

static SYN_Task  s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

static SYN_PT_Status task_pid_control_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    static uint32_t last_ms = 0;

    PT_BEGIN(pt);

    while (1) {
        uint32_t now_ms = syn_port_get_tick_ms();
        uint32_t dt_ms  = (last_ms == 0) ? 10 : (now_ms - last_ms);
        last_ms         = now_ms;

        int32_t output = syn_pid_update(&s_pid, s_setpoint_temp, s_current_temp, dt_ms);
        s_current_temp += (output >> 4);

        PT_TASK_DELAY_MS(pt, task, 10);
    }

    PT_END(pt);
}

static SYN_PT_Status task_telemetry_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        printf("Temp: %ld C, Setpoint: %ld C, Output: %ld%%\n",
               (long)s_current_temp, (long)s_setpoint_temp, (long)s_pid.output);
        PT_TASK_DELAY_MS(pt, task, 1000);
    }

    PT_END(pt);
}

int main_sched(void)
{
    SYN_PID_Config cfg = SYN_PID_GAINS(2.0f, 0.5f, 0.1f, 256, 0, 100);
    syn_pid_init(&s_pid, &cfg);

    syn_task_create(&s_tasks[TASK_PID_CONTROL], "PidControl", task_pid_control_func, SYN_PRIO_HIGH,
                    NULL);
    syn_task_create(&s_tasks[TASK_TELEMETRY], "Telemetry", task_telemetry_func, SYN_PRIO_NORMAL,
                    NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
