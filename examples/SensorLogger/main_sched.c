/**
 * @file main_sched.c
 * @brief SensorLogger Example (`syn_sched` Cooperative Task Scheduler Variant).
 *
 * Demonstrates sensor data acquisition, EMA filtering, and CLI interactions
 * managed cleanly via the syn_sched task scheduler.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntropic/drivers/syn_adc.h"
#include "syntropic/dsp/syn_filter.h"
#include "syntropic/dsp/syn_signal.h"
#include "syntropic/output/syn_led.h"
#include "syntropic/port/syn_port_serial.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/sched/syn_sched.h"
#include "syntropic/cli/syn_cli.h"

static SYN_CLI       cli;
static SYN_LED       led;
static SYN_ADC       adc[2];
static SYN_FilterEMA ema[2];
static int32_t       stats_buf[2][8];
static SYN_Signal    signal_stats[2];
static int16_t       raw_val[2];
static int16_t       filt_val[2];

static SYN_Sched sched;
static SYN_Task  tasks[3];

static SYN_PT_Status task_led_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

static SYN_PT_Status task_cli_func(SYN_PT *pt, SYN_Task *task)
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

static SYN_PT_Status task_adc_func(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        for (int i = 0; i < 2; i++) {
            raw_val[i]  = (int16_t)syn_adc_read_raw(&adc[i], (uint8_t)i);
            filt_val[i] = (int16_t)syn_filter_ema_update(&ema[i], raw_val[i]);
            syn_signal_push(&signal_stats[i], (int32_t)filt_val[i]);
        }
        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

int main_sched(void)
{
    syn_port_serial_init(115200);
    syn_led_init(&led, 13, SYN_LED_ACTIVE_HIGH);

    for (int i = 0; i < 2; i++) {
        syn_signal_init(&signal_stats[i], stats_buf[i], 8);
        syn_filter_ema_init(&ema[i], 64);

        SYN_ADC_Config cfg = {
            .adc_id = 0,
            .channel_mask = (1u << i),
            .vref_mv = 3300,
            .use_dma = false
        };
        syn_adc_init(&adc[i], &cfg);
    }

    syn_task_create(&tasks[0], "led", task_led_func, SYN_PRIO_NORMAL, NULL);
    syn_task_create(&tasks[1], "cli", task_cli_func, SYN_PRIO_HIGH, NULL);
    syn_task_create(&tasks[2], "adc", task_adc_func, SYN_PRIO_CRITICAL, NULL);
    syn_sched_init(&sched, tasks, 3);

    while (1) {
        if (!syn_sched_run(&sched)) {
            syn_port_delay_ms(1);
        }
    }

    return 0;
}
