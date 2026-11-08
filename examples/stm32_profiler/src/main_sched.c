/**
 * @file main_sched.c
 * @brief STM32 Protothread Task Scheduler & CPU Profiler Integration Example (`syn_sched` Variant).
 *
 * Demonstrates the SyntropicOS cooperative protothread scheduler (syn_sched)
 * working hand-in-hand with the CPU load profiler (syn_profiler) on STM32 targets.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "syntropic/debug/syn_profiler.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"

#define TASK_SENSOR   0
#define TASK_CONTROL  1
#define TASK_MONITOR  2
#define TASK_COUNT    3

static SYN_Task s_tasks[TASK_COUNT];
static SYN_ProfileEntry s_profile_entries[TASK_COUNT];
static SYN_Profiler s_profiler;

static void stm32_uart_send_string(const char *str)
{
    while (*str) {
        (void)str++;
    }
}

static void print_profiler_output(const char *str)
{
    stm32_uart_send_string(str);
}

static SYN_PT_Status task_sensor_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        syn_profiler_task_begin(&s_profiler, TASK_SENSOR);
        syn_port_delay_ms(1);
        syn_profiler_task_end(&s_profiler, TASK_SENSOR);
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_control_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);

    while (1) {
        syn_profiler_task_begin(&s_profiler, TASK_CONTROL);
        syn_port_delay_ms(1);
        syn_profiler_task_end(&s_profiler, TASK_CONTROL);
        PT_PASS(pt);
    }

    PT_END(pt);
}

static SYN_PT_Status task_monitor_func(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    static uint32_t last_report_ms = 0;

    PT_BEGIN(pt);

    last_report_ms = syn_port_get_tick_ms();

    while (1) {
        syn_profiler_task_begin(&s_profiler, TASK_MONITOR);
        uint32_t now_ms = syn_port_get_tick_ms();
        bool report_due = (now_ms - last_report_ms >= 1000u);
        syn_profiler_task_end(&s_profiler, TASK_MONITOR);

        if (report_due) {
            last_report_ms = now_ms;

            syn_profiler_update(&s_profiler);
            stm32_uart_send_string("\r\n=== Real-Time Task CPU Load Report ===\r\n");
            syn_profiler_dump(&s_profiler, print_profiler_output);
        }

        PT_PASS(pt);
    }

    PT_END(pt);
}

int main_sched(void)
{
    stm32_uart_send_string("SyntropicOS STM32 Scheduler & CPU Profiler Example Started\r\n");

    syn_profiler_init(&s_profiler, s_profile_entries, TASK_COUNT);

    syn_profiler_register(&s_profiler, TASK_SENSOR, "SensorTask");
    syn_profiler_register(&s_profiler, TASK_CONTROL, "ControlTask");
    syn_profiler_register(&s_profiler, TASK_MONITOR, "MonitorTask");

    syn_task_create(&s_tasks[TASK_SENSOR], "SensorTask", task_sensor_func, SYN_PRIO_HIGH, NULL);
    syn_task_create(&s_tasks[TASK_CONTROL], "ControlTask", task_control_func, SYN_PRIO_HIGH, NULL);
    syn_task_create(&s_tasks[TASK_MONITOR], "MonitorTask", task_monitor_func, SYN_PRIO_NORMAL,
                    NULL);

    SYN_Sched sched;
    syn_sched_init(&sched, s_tasks, TASK_COUNT);

    uint32_t start_ms = syn_port_get_tick_ms();
    for (uint32_t i = 0; i < 2000u; i++) {
        syn_sched_run(&sched);
        if (syn_port_get_tick_ms() - start_ms >= 1050u) {
            break;
        }
    }

    stm32_uart_send_string("\r\n=== STM32 Scheduler & CPU Profiler Demo Complete ===\r\n");
    return 0;
}
