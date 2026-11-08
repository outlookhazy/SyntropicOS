/**
 * @file main_sched.c
 * @brief STM32 EtherCAT Servo Drive Example (`syn_sched` Cooperative Task Scheduler Variant).
 *
 * Demonstrates 1 kHz EtherCAT process data task scheduled via priority task queues (syn_sched).
 */

#include <stdbool.h>
#include <stdint.h>

#include "syntropic/proto/syn_ethercat.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

extern void ethercat_app_init(void);
extern void ethercat_app_1khz_process(void);

#define TASK_ETHERCAT_1KHZ 0
#define TASK_COUNT         1

static SYN_Task  s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

static SYN_PT_Status task_ethercat_1khz_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);

    while (1) {
        ethercat_app_1khz_process();
        PT_TASK_DELAY_MS(pt, task, 1);
    }

    PT_END(pt);
}

int main_sched(void)
{
    ethercat_app_init();

    syn_task_create(&s_tasks[TASK_ETHERCAT_1KHZ], "EtherCAT1kHz", task_ethercat_1khz_func,
                    SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);

    while (1) {
        syn_sched_run(&s_sched);
    }

    return 0;
}
