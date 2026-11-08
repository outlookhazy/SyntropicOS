/**
 * @file sys_init.h
 * @brief Background OS Scheduler & Infrastructure for Arduino Examples (Tab 2).
 *
 * This header encapsulates the SyntropicOS cooperative scheduler (syn_sched),
 * protothread background tasks (PT_BEGIN / PT_END), serial port setup,
 * and system logging so that the main sketch (.ino) remains clean and readable.
 *
 * Usage in Custom Projects:
 *   - Call sys_init(update_callback) in setup() to start the background engine.
 *   - Call sys_run() in loop() to yield CPU to background OS tasks.
 */

#ifndef SYS_INIT_H
#define SYS_INIT_H

#include <SyntropicOS.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/sched/syn_sched.h>

/* Background Scheduler & Task Descriptor Storage */
static SYN_Sched sys_sched;
static SYN_Task sys_tasks[1];

/* Function pointer to user's background update routine (e.g. updating buttons/sensors) */
typedef void (*SysBgTaskFn)(void);
static SysBgTaskFn g_bg_fn = NULL;

/**
 * @brief Protothread task loop. Runs periodically every 10ms in the background.
 */
static SYN_PT_Status sys_bg_pt(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        if (g_bg_fn != NULL) {
            g_bg_fn();
        }
        PT_TASK_DELAY_MS(pt, task, 10);
    }
    PT_END(pt);
}

/**
 * @brief Initialize Serial Port (115200 baud), Logging, and OS Scheduler.
 * @param bg_fn Optional background update callback (e.g. button state updates).
 */
inline void sys_init(SysBgTaskFn bg_fn)
{
    syn_port_serial_init(115200);
    syn_log_init(SYN_LOG_INFO);
    g_bg_fn = bg_fn;

    syn_task_create(&sys_tasks[0], "sys_bg", sys_bg_pt, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sys_sched, sys_tasks, 1);
}

/**
 * @brief Run the cooperative scheduler. Call inside loop().
 */
inline void sys_run(void)
{
    if (!syn_sched_run(&sys_sched)) {
        syn_port_delay_ms(1);
    }
}

#endif /* SYS_INIT_H */
