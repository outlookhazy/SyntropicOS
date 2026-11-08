/**
 * @file main_sched.c
 * @brief STM32 CANopen DS301 Slave Example (`syn_sched` Cooperative Task Scheduler Variant).
 *
 * Demonstrates CANopen DS301 slave execution managed by syn_sched:
 * - High-priority CANopen protocol engine update task (10ms tick)
 * - Periodic 100ms TPDO telemetry transmission task
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "syntropic/proto/syn_canopen.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

extern void canopen_app_init(void);
extern void canopen_app_loop(uint32_t dt_ms);
extern void canopen_app_task_100ms(void);

#define TASK_CANOPEN_ENGINE 0
#define TASK_CANOPEN_TPDO   1
#define TASK_COUNT          2

static SYN_Task  s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

static SYN_PT_Status task_canopen_engine_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        canopen_app_loop(10);
        PT_TASK_DELAY_MS(pt, task, 10);
    }

    PT_END(pt);
}

static SYN_PT_Status task_canopen_tpdo_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        canopen_app_task_100ms();
        PT_TASK_DELAY_MS(pt, task, 100);
    }

    PT_END(pt);
}

int main_sched(void)
{
    canopen_app_init();

    syn_task_create(&s_tasks[TASK_CANOPEN_ENGINE], "CANopenEngine", task_canopen_engine_func,
                    SYN_PRIO_HIGH, NULL);
    syn_task_create(&s_tasks[TASK_CANOPEN_TPDO], "CANopenTPDO", task_canopen_tpdo_func,
                    SYN_PRIO_NORMAL, NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
