/**
 * SyntropicOS — Sensor Logger
 *
 * Reads two analog inputs (A0, A1) with:
 *   - 4× oversampling for noise reduction
 *   - Exponential Moving Average (EMA) digital filter
 *   - Running statistics (min, max, mean, variance)
 *
 * Type "adc 0" or "adc 1" in the Serial Monitor to view live readings.
 * Connect potentiometers to A0/A1, or leave floating for demo values.
 *
 * Open Serial Monitor at 115200 baud.
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_uart.h>
#include <syntropic/drivers/syn_adc.h>
#include <syntropic/dsp/syn_filter.h>
#include <syntropic/dsp/syn_signal.h>
#include <syntropic/output/syn_led.h>
#include <string.h>
#include <stdlib.h>

static SYN_CLI   cli;
static SYN_Sched sched;
static SYN_Task  tasks[3];
static SYN_LED   led;

/* ADC objects — one per channel */
static SYN_ADC       adc[2];
static SYN_FilterEMA ema[2];
static int32_t       stats_buf[2][8];
static SYN_Signal    signal[2];
static int16_t       raw[2], filtered[2];

/* ── CLI: read ADC channel ────────────────────────────────────────────── */

static int cmd_adc(int argc, char *argv[])
{
    if (argc < 2) { syn_cli_printf(&cli, "Usage: adc <0|1>\r\n"); return 1; }
    int ch = atoi(argv[1]);
    if (ch < 0 || ch > 1) { syn_cli_printf(&cli, "Channel must be 0 or 1\r\n"); return 1; }

    int32_t v = syn_signal_variance_q16(&signal[ch]);
    syn_cli_printf(&cli,
        "A%d: raw=%d filtered=%d min=%ld max=%ld mean=%ld var=%ld.%03ld\r\n",
        ch, raw[ch], filtered[ch],
        syn_signal_min(&signal[ch]), syn_signal_max(&signal[ch]),
        syn_signal_mean(&signal[ch]), v >> 16, ((v & 0xFFFF) * 1000) >> 16);
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "adc", "Read ADC channel with statistics", cmd_adc },
};

/* ── Platform hooks ───────────────────────────────────────────────────── */

extern "C" void syn_assert_failed(const char *f, int l) { (void)f; (void)l; for(;;); }

/* ── Tasks ────────────────────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) { syn_led_update(&led); PT_TASK_DELAY_MS(pt, task, 50); }
    PT_END(pt);
}

static SYN_PT_Status cli_task(SYN_PT *pt, SYN_Task *task)
{
    uint8_t ch; size_t n; SYN_Status st; (void)task;
    PT_BEGIN(pt);
    for (;;) {
        n = 0; st = syn_port_uart_receive(0, &ch, 1, &n, 1);
        if (st == SYN_OK && n > 0) syn_cli_process_char(&cli, (char)ch);
        PT_YIELD(pt);
    }
    PT_END(pt);
}

/* ADC task — samples both channels every 200ms with a yield in between */
static SYN_PT_Status adc_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_adc_read(&adc[0]);
        raw[0]      = (int16_t)syn_adc_raw(&adc[0]);
        filtered[0] = (int16_t)syn_adc_filtered(&adc[0]);

        PT_YIELD(pt);   /* Let other tasks run between samples */

        syn_adc_read(&adc[1]);
        raw[1]      = (int16_t)syn_adc_raw(&adc[1]);
        filtered[1] = (int16_t)syn_adc_filtered(&adc[1]);

        PT_TASK_DELAY_MS(pt, task, 200);
    }
    PT_END(pt);
}

/* ── Setup ────────────────────────────────────────────────────────────── */

void setup()
{
    syn_port_uart_init(0, 115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Initialize filters and statistics for both channels */
    for (int i = 0; i < 2; i++) {
        syn_signal_init(&signal[i], stats_buf[i], 8);
        syn_filter_ema_init(&ema[i], 64);   /* alpha ≈ 0.25 */
        SYN_ADC_Config cfg = {
            .channel = (uint8_t)i, .oversample = 4,
            .filter = &ema[i], .filter_type = SYN_ADC_FILTER_EMA,
            .cal_offset = 0, .cal_scale = 1, .cal_scale_shift = 0
        };
        syn_adc_init(&adc[i], &cfg);
        syn_adc_set_stats(&adc[i], &signal[i]);
    }

    syn_log_init(SYN_LOG_INFO);
    syn_cli_init(&cli, commands, sizeof(commands)/sizeof(commands[0]),
                 "> ");
    syn_cli_set_scheduler(&sched);
    syn_cli_printf(&cli, "\r\n--- SyntropicOS Sensor Logger ---\r\n");
    syn_cli_print_prompt(&cli);

    syn_task_create(&tasks[0], "blink", blink_task, 2, NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   1, NULL);
    syn_task_create(&tasks[2], "adc",   adc_task,   0, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    syn_sched_run(&sched);
}
