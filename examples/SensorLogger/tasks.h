/**
 * @file tasks.h
 * @brief Protothread Task Definitions for SensorLogger Example.
 *
 * Encapsulates background tasks (LED blink, CLI input, and ADC sampling)
 * so that SensorLogger.ino (Tab 1) remains clean and focused on CLI setup and configuration.
 */

#ifndef SENSOR_LOGGER_TASKS_H
#define SENSOR_LOGGER_TASKS_H

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/drivers/syn_adc.h>
#include <syntropic/dsp/syn_filter.h>
#include <syntropic/dsp/syn_signal.h>
#include <syntropic/output/syn_led.h>

extern SYN_CLI    cli;
extern SYN_LED    led;
extern SYN_ADC    adc[2];
extern int16_t    raw_val[2];
extern int16_t    filt_val[2];
extern SYN_Signal signal_stats[2];

static SYN_Sched sched;
static SYN_Task  tasks[3];

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

static SYN_PT_Status cli_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t ch;
    PT_BEGIN(pt);
    for (;;) {
        while (syn_port_serial_read(&ch, 1) > 0) {
            syn_cli_process_char(&cli, (char)ch);
            PT_CONTINUE_IF_NEEDED(pt, task);
        }
        PT_TASK_DELAY_MS(pt, task, 20);
    }
    PT_END(pt);
}

/* Data Acquisition Task — samples A0 and A1 every 200ms */
static SYN_PT_Status adc_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t log_timer = 0;
    PT_BEGIN(pt);
    for (;;) {
        /* Sample Channel 0 */
        raw_val[0]  = (int16_t)syn_adc_read_raw(&adc[0], 0);
        filt_val[0] = (int16_t)syn_filter_ema_update(&ema[0], raw_val[0]);
        syn_signal_push(&signal_stats[0], (int32_t)filt_val[0]);

        PT_CONTINUE(pt); /* Yield between channels while acquisition continues */

        /* Sample Channel 1 */
        raw_val[1]  = (int16_t)syn_adc_read_raw(&adc[1], 1);
        filt_val[1] = (int16_t)syn_filter_ema_update(&ema[1], raw_val[1]);
        syn_signal_push(&signal_stats[1], (int32_t)filt_val[1]);

        log_timer += 200;
        if (log_timer >= 1000) {
            log_timer = 0;
            syn_cli_printf(&cli, "[DATA] A0_filt=%d A1_filt=%d (A0_mean=%ld A1_mean=%ld)\r\n",
                           filt_val[0], filt_val[1],
                           (long)syn_signal_mean(&signal_stats[0]),
                           (long)syn_signal_mean(&signal_stats[1]));
        }

        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

inline void sys_start_tasks(void)
{
    syn_task_create(&tasks[0], "blink", blink_task, SYN_PRIO_NORMAL,   NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   SYN_PRIO_HIGH,     NULL);
    syn_task_create(&tasks[2], "adc",   adc_task,   SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 3);
}

inline void sys_run_scheduler(void)
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}

#endif /* SENSOR_LOGGER_TASKS_H */
