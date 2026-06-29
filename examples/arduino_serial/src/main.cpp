#include <Arduino.h>
#include "syntropic/syntropic.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/sched/syn_timer.h"
#include "syntropic/control/syn_pid.h"
#include "syntropic/dsp/syn_filter.h"
#include "syntropic/dsp/syn_signal.h"
#include "syntropic/util/syn_fsm.h"
#include "syntropic/drivers/syn_adc.h"
#include "syntropic/output/syn_led.h"
#include <string.h>
#include <stdlib.h>

#define TAG "main"
#define LED_PIN 13

static SYN_CLI cli;
static SYN_Sched sched;
static SYN_Task tasks[3];

static SYN_LED status_led;

static SYN_ADC my_adc_a0;
static SYN_ADC my_adc_a1;

/* Global state for background measurements of both ADCs */
static int16_t last_raw_val_a0 = 0;
static int16_t last_filtered_val_a0 = 0;
static int16_t last_raw_val_a1 = 0;
static int16_t last_filtered_val_a1 = 0;

/* ── Digital Filters ────────────────────────────────────────────────────── */
static SYN_FilterEMA ema_filter_a0;
static SYN_FilterEMA ema_filter_a1;

/* ── Signal Statistics ──────────────────────────────────────────────────── */
static int32_t stats_buf_a0[8];
static int32_t stats_buf_a1[8];
static SYN_Signal my_signal_a0;
static SYN_Signal my_signal_a1;

static int cmd_adc(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: adc <0|1>\r\n");
        return 1;
    }

    int channel = atoi(argv[1]);
    if (channel == 0) {
        int32_t min = syn_signal_min(&my_signal_a0);
        int32_t max = syn_signal_max(&my_signal_a0);
        int32_t mean = syn_signal_mean(&my_signal_a0);
        int32_t var_q16 = syn_signal_variance_q16(&my_signal_a0);
        int32_t var_int = var_q16 >> 16;
        int32_t var_frac = ((var_q16 & 0xFFFF) * 1000) >> 16;

        syn_cli_printf(&cli, "A0: s:%d,r:%d,f:%d,min:%ld,max:%ld,mean:%ld,var:%ld.%03ld\r\n",
                       (int)syn_signal_count(&my_signal_a0), last_raw_val_a0, last_filtered_val_a0,
                       min, max, mean, var_int, var_frac);
    } else if (channel == 1) {
        int32_t min = syn_signal_min(&my_signal_a1);
        int32_t max = syn_signal_max(&my_signal_a1);
        int32_t mean = syn_signal_mean(&my_signal_a1);
        int32_t var_q16 = syn_signal_variance_q16(&my_signal_a1);
        int32_t var_int = var_q16 >> 16;
        int32_t var_frac = ((var_q16 & 0xFFFF) * 1000) >> 16;

        syn_cli_printf(&cli, "A1: s:%d,r:%d,f:%d,min:%ld,max:%ld,mean:%ld,var:%ld.%03ld\r\n",
                       (int)syn_signal_count(&my_signal_a1), last_raw_val_a1, last_filtered_val_a1,
                       min, max, mean, var_int, var_frac);
    } else {
        syn_cli_printf(&cli, "err:%d\r\n", channel);
        return 1;
    }
    return 0;
}

/* ── Finite State Machine Test ──────────────────────────────────────────── */
static SYN_FSM my_fsm;
static bool fsm_initialized = false;
static const char* state_names[] = { "IDLE", "RUN", "FAULT" };

static void on_transition_to_running(void *ctx) {
    (void)ctx;
    SYN_LOG_I(TAG, "FSM:RUN");
}

static void on_transition_to_fault(void *ctx) {
    (void)ctx;
    SYN_LOG_W(TAG, "FSM:FAULT");
}

static void on_transition_to_idle(void *ctx) {
    (void)ctx;
    SYN_LOG_I(TAG, "FSM:IDLE");
}

static const SYN_FSM_Transition transitions[] = {
    { 0, 0, 1, NULL, on_transition_to_running }, // IDLE -> START -> RUNNING
    { 1, 1, 0, NULL, on_transition_to_idle },    // RUNNING -> STOP -> IDLE
    { 1, 2, 2, NULL, on_transition_to_fault },   // RUNNING -> FAULT -> FAULT
    { 2, 1, 0, NULL, on_transition_to_idle },    // FAULT -> STOP -> IDLE
    SYN_FSM_END
};

static int cmd_fsm(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "fsm <start|stop|fault>\r\n");
        return 1;
    }
    if (!fsm_initialized) {
        syn_fsm_init(&my_fsm, transitions, 0, "fsm");
        syn_fsm_set_state_names(&my_fsm, state_names);
        fsm_initialized = true;
    }

    SYN_FSM_Event event = -1;
    if (strcmp(argv[1], "start") == 0) event = 0;
    else if (strcmp(argv[1], "stop") == 0) event = 1;
    else if (strcmp(argv[1], "fault") == 0) event = 2;
    else {
        syn_cli_printf(&cli, "err:%s\r\n", argv[1]);
        return 1;
    }

    bool transitioned = syn_fsm_dispatch(&my_fsm, event);
    syn_cli_printf(&cli, "fsm:%s,%s,%s\r\n", 
                   argv[1], transitioned ? "OK" : "IGNORED", state_names[syn_fsm_state(&my_fsm)]);
    return 0;
}

/* ── LED Control CLI Command ────────────────────────────────────────────── */
static int cmd_led(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "led <on|off|blink|flash|sos>\r\n");
        return 1;
    }
    if (strcmp(argv[1], "on") == 0) {
        syn_led_on(&status_led);
        syn_cli_printf(&cli, "LED:ON\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        syn_led_off(&status_led);
        syn_cli_printf(&cli, "LED:OFF\r\n");
    } else if (strcmp(argv[1], "blink") == 0) {
        syn_led_blink(&status_led, 200, 200);
        syn_cli_printf(&cli, "LED:BLINK\r\n");
    } else if (strcmp(argv[1], "flash") == 0) {
        syn_led_flash(&status_led, 100, 100, 5);
        syn_cli_printf(&cli, "LED:FLASH\r\n");
    } else if (strcmp(argv[1], "sos") == 0) {
        syn_led_pattern(&status_led, "... --- ... |", 150);
        syn_cli_printf(&cli, "LED:SOS\r\n");
    } else {
        syn_cli_printf(&cli, "err:%s\r\n", argv[1]);
        return 1;
    }
    return 0;
}

/* CLI command table */
static const SYN_CLI_Command commands[] = {
    { "led", "led <cmd>", cmd_led },
    { "adc", "adc <0|1>", cmd_adc },
    { "fsm", "fsm <e>", cmd_fsm }
};

extern "C" void syn_assert_failed(const char *file, int line)
{
    (void)file;
    (void)line;
    for (;;);
}

/* putchar callback for CLI */
static void cli_putchar(char ch)
{
    syn_port_uart_transmit_byte(0, (uint8_t)ch);
}

/* Logging output callback */
static void log_output(const char *str, size_t len)
{
    syn_port_uart_transmit(0, (const uint8_t *)str, len, 100);
}

/* Task 3: Background ADC sampling task with cooperative yields (runs every 200ms) */
static SYN_PT_Status adc_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        /* Sample ADC Channel 0 (A0) */
        syn_adc_read(&my_adc_a0);
        last_raw_val_a0 = (int16_t)syn_adc_raw(&my_adc_a0);
        last_filtered_val_a0 = (int16_t)syn_adc_filtered(&my_adc_a0);

        /* Yield cooperatively to show protothread scheduling capability */
        PT_YIELD(pt);

        /* Sample ADC Channel 1 (A1) */
        syn_adc_read(&my_adc_a1);
        last_raw_val_a1 = (int16_t)syn_adc_raw(&my_adc_a1);
        last_filtered_val_a1 = (int16_t)syn_adc_filtered(&my_adc_a1);

        /* Delay task for 200ms */
        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

/* Task 1: LED Pattern Update Task (runs every 50ms) */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* Task 2: Polling task to process CLI characters from serial port */
static SYN_PT_Status cli_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t ch;
    size_t rec;
    SYN_Status status;
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        rec = 0;
        status = syn_port_uart_receive(0, &ch, 1, &rec, 1);
        if (status == SYN_OK && rec > 0) {
            syn_cli_process_char(&cli, (char)ch);
        }
        PT_YIELD(pt);
    }
    PT_END(pt);
}

void setup()
{
    /* Initialize ports */
    syn_port_uart_init(0, 115200);

    /* Initialize Status LED */
    syn_led_init(&status_led, LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 500, 500); // Default blinking heartbeat

    /* Initialize math, filters, and ADC structures at startup */
    syn_signal_init(&my_signal_a0, stats_buf_a0, 8);
    syn_signal_init(&my_signal_a1, stats_buf_a1, 8);
    syn_filter_ema_init(&ema_filter_a0, 64); // alpha = 64 (0.25)
    syn_filter_ema_init(&ema_filter_a1, 64);

    /* Initialize ADC on Channel 0 (A0) with 4x oversampling and native EMA filter */
    SYN_ADC_Config adc_cfg_a0 = {
        .channel = 0,
        .oversample = 4,
        .filter = &ema_filter_a0,
        .filter_type = SYN_ADC_FILTER_EMA,
        .cal_offset = 0,
        .cal_scale = 1,
        .cal_scale_shift = 0
    };
    syn_adc_init(&my_adc_a0, &adc_cfg_a0);
    syn_adc_set_stats(&my_adc_a0, &my_signal_a0); // Bind stats window

    /* Initialize ADC on Channel 1 (A1) with 4x oversampling and native EMA filter */
    SYN_ADC_Config adc_cfg_a1 = {
        .channel = 1,
        .oversample = 4,
        .filter = &ema_filter_a1,
        .filter_type = SYN_ADC_FILTER_EMA,
        .cal_offset = 0,
        .cal_scale = 1,
        .cal_scale_shift = 0
    };
    syn_adc_init(&my_adc_a1, &adc_cfg_a1);
    syn_adc_set_stats(&my_adc_a1, &my_signal_a1); // Bind stats window

    /* Initialize Logging */
    syn_log_init(log_output, SYN_LOG_INFO);
    SYN_LOG_I(TAG, "boot");

    /* Initialize CLI */
    syn_cli_init(&cli, commands, sizeof(commands) / sizeof(commands[0]), cli_putchar, "> ");
    syn_cli_set_scheduler(&sched);

    /* Print CLI banner & prompt */
    syn_cli_printf(&cli, "\r\n--- SyntropicOS ---\r\n");
    syn_cli_print_prompt(&cli);

    /* Create tasks */
    syn_task_create(&tasks[0], "blink", blink_task, 2, NULL);
    syn_task_create(&tasks[1], "cli", cli_task, 1, NULL);
    syn_task_create(&tasks[2], "adc", adc_task, 0, NULL);

    /* Initialize and start scheduler */
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    /* Execute scheduler tick */
    syn_sched_run(&sched);
}
