/**
 * @file main_sched.c
 * @brief STM32 6-Step BLDC Motor Commutation Example (`syn_sched` Task Scheduler Variant).
 *
 * Demonstrates 6-step trapezoidal BLDC motor commutation managed cleanly via syn_sched:
 * - High-priority Hall sensor commutation task
 * - Periodic 100 Hz closed-loop speed PID control task
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/motor/syn_bldc_6step.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

static SYN_BLDC_6Step s_bldc;

#define TASK_HALL_COMMUTATION 0
#define TASK_SPEED_PID        1
#define TASK_COUNT            2

static SYN_Task  s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

static void stm32_apply_phase_gates(const SYN_BLDC_PhaseOutputs *gates)
{
    (void)gates;
}

static uint8_t stm32_read_hall_sensors(void)
{
    return 0b101;
}

static SYN_PT_Status task_hall_commutation_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        uint8_t hall = stm32_read_hall_sensors();
        SYN_BLDC_PhaseOutputs gates;
        if (syn_bldc_6step_set_hall(&s_bldc, hall, &gates) == SYN_OK) {
            stm32_apply_phase_gates(&gates);
        }
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_speed_pid_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        uint32_t now_ms = syn_port_get_tick_ms();
        syn_bldc_6step_update_speed(&s_bldc, now_ms, 1500); /* Target 1500 RPM */
        PT_TASK_DELAY_MS(pt, task, 10);                     /* 100 Hz PID loop */
    }

    PT_END(pt);
}

int main_sched(void)
{
    SYN_BLDC_Config cfg = {
        .pole_pairs    = 4,
        .pwm_frequency = 20000
    };
    syn_bldc_6step_init(&s_bldc, &cfg);
    syn_bldc_6step_set_duty(&s_bldc, 500);
    s_bldc.speed_pid_active = true;
    syn_bldc_6step_start(&s_bldc);

    syn_task_create(&s_tasks[TASK_HALL_COMMUTATION], "HallCommutation", task_hall_commutation_func,
                    SYN_PRIO_CRITICAL, NULL);
    syn_task_create(&s_tasks[TASK_SPEED_PID], "SpeedPID", task_speed_pid_func, SYN_PRIO_HIGH,
                    NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
