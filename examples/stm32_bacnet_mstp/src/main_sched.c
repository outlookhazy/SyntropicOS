/**
 * @file main_sched.c
 * @brief STM32 BACnet MS/TP Sensor Node Example (`syn_sched` Cooperative Task Scheduler Variant).
 *
 * Demonstrates BACnet MS/TP protocol processing task + 2s periodic sensor COV telemetry task
 * scheduled cleanly via priority task queues (syn_sched).
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/proto/syn_bacnet.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

extern void bacnet_app_init(void);
extern void bacnet_app_poll(void);
extern void bacnet_app_update_sensors(void);

#define TASK_BACNET_POLL 0
#define TASK_BACNET_COV  1
#define TASK_COUNT       2

static SYN_Task  s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

static SYN_PT_Status task_bacnet_poll_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        bacnet_app_poll();
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_bacnet_cov_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        bacnet_app_update_sensors();
        PT_TASK_DELAY_MS(pt, task, 2000);
    }

    PT_END(pt);
}

int main_sched(void)
{
    bacnet_app_init();

    syn_task_create(&s_tasks[TASK_BACNET_POLL], "BACnetPoll", task_bacnet_poll_func, SYN_PRIO_HIGH,
                    NULL);
    syn_task_create(&s_tasks[TASK_BACNET_COV], "BACnetCOV", task_bacnet_cov_func, SYN_PRIO_NORMAL,
                    NULL);

    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
