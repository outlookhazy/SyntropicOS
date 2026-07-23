/**
 * SyntropicOS — Data Logger & Signal Processing
 *
 * Demonstrates structured data acquisition and digital signal processing:
 *   - Dual-channel ADC driver with 4× oversampling (syn_adc)
 *   - Exponential Moving Average (EMA) noise reduction filter (syn_filter)
 *   - Running signal statistics — min, max, mean, variance (syn_signal)
 *   - Interactive Serial CLI (syn_cli)
 *   - Multi-task cooperative scheduler (syn_sched)
 *
 * Open Serial Monitor at 115200 baud. Type "stats" to view channel statistics.
 * Connect potentiometers to analog pins A0 and A1 (or leave floating).
 *
 * Documentation & Related Features:
 *   - DSP & Filters Guide:  https://outlookhazy.github.io/SyntropicOS/modules/dsp/
 *   - DSP API Ref:          https://outlookhazy.github.io/SyntropicOS/syntropic/group__syn__dsp/
 *   - ADC Driver Docs:      https://outlookhazy.github.io/SyntropicOS/modules/drivers/ (syn_adc)
 *   - Biquad & Median:      See syn_biquad.h for IIR filtering and syn_filter_median.h for spike removal
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/drivers/syn_adc.h>
#include <syntropic/dsp/syn_filter.h>
#include <syntropic/dsp/syn_signal.h>
#include <syntropic/output/syn_led.h>
#include <string.h>
#include <stdlib.h>

#define TAG "datalogger"

static SYN_CLI       cli;
static SYN_Sched     sched;
static SYN_Task      tasks[3];
static SYN_LED       led;

/* ADC Objects — 2 Channels */
static SYN_ADC       adc[2];
static SYN_FilterEMA ema[2];
static int32_t       stats_buf[2][8];
static SYN_Signal    signal_stats[2];
static int16_t       raw_val[2];
static int16_t       filt_val[2];

/* ── CLI Commands ─────────────────────────────────────────────────────── */

static int cmd_adc(int argc, char *argv[])
{
    if (argc < 2) {
        syn_cli_printf(&cli, "Usage: adc <0|1>\r\n");
        return 1;
    }
    int ch = atoi(argv[1]);
    if (ch < 0 || ch > 1) {
        syn_cli_printf(&cli, "Error: Channel must be 0 or 1\r\n");
        return 1;
    }
    syn_cli_printf(&cli, "A%d: raw=%d filtered=%d\r\n", ch, raw_val[ch], filt_val[ch]);
    return 0;
}

static int cmd_stats(int argc, char *argv[])
{
    (void)argc; (void)argv;
    syn_cli_printf(&cli, "--- Channel Statistics ---\r\n");
    for (int i = 0; i < 2; i++) {
        int32_t var = syn_signal_variance_q16(&signal_stats[i]);
        syn_cli_printf(&cli, "A%d: min=%ld max=%ld mean=%ld var=%ld.%03ld\r\n",
                       i,
                       (long)syn_signal_min(&signal_stats[i]),
                       (long)syn_signal_max(&signal_stats[i]),
                       (long)syn_signal_mean(&signal_stats[i]),
                       (long)(var >> 16),
                       (long)(((var & 0xFFFF) * 1000) >> 16));
    }
    return 0;
}

static int cmd_reset(int argc, char *argv[])
{
    (void)argc; (void)argv;
    for (int i = 0; i < 2; i++) {
        syn_signal_init(&signal_stats[i], stats_buf[i], 8);
    }
    syn_cli_printf(&cli, "Signal statistics reset for all channels.\r\n");
    return 0;
}

static const SYN_CLI_Command commands[] = {
    { "adc",   "Read current raw & filtered ADC value (adc 0 or adc 1)", cmd_adc },
    { "stats", "Print running signal statistics (min, max, mean, var)", cmd_stats },
    { "reset", "Reset accumulated signal statistics",                   cmd_reset },
};

/* ── Platform Hooks ───────────────────────────────────────────────────── */

extern "C" void syn_assert_failed(const char *f, int l) { (void)f; (void)l; for(;;); }

/* ── Tasks ────────────────────────────────────────────────────────────── */

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
        int r = syn_port_serial_read(&ch, 1);
        if (r > 0) {
            syn_cli_process_char(&cli, (char)ch);
            PT_YIELD(pt);
        } else {
            PT_TASK_DELAY_MS(pt, task, 20);
        }
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
        syn_adc_read(&adc[0]);
        raw_val[0]  = (int16_t)syn_adc_raw(&adc[0]);
        filt_val[0] = (int16_t)syn_adc_filtered(&adc[0]);

        PT_YIELD(pt); /* Yield to give other tasks CPU time between channels */

        /* Sample Channel 1 */
        syn_adc_read(&adc[1]);
        raw_val[1]  = (int16_t)syn_adc_raw(&adc[1]);
        filt_val[1] = (int16_t)syn_adc_filtered(&adc[1]);

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

/* ── Entry Point ──────────────────────────────────────────────────────── */

void setup()
{
    syn_port_serial_init(115200);
    syn_led_init(&led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&led, 500, 500);

    /* Initialize ADC channels with 4× oversampling, EMA filter, and statistics */
    for (int i = 0; i < 2; i++) {
        syn_signal_init(&signal_stats[i], stats_buf[i], 8);
        syn_filter_ema_init(&ema[i], 64); /* alpha = 64/256 = 0.25 */

        SYN_ADC_Config cfg = {
            .channel         = (uint8_t)i,
            .oversample      = 4,
            .filter          = &ema[i],
            .filter_type     = SYN_ADC_FILTER_EMA,
            .cal_offset      = 0,
            .cal_scale       = 1,
            .cal_scale_shift = 0
        };
        syn_adc_init(&adc[i], &cfg);
        syn_adc_set_stats(&adc[i], &signal_stats[i]);
    }

    syn_log_init(SYN_LOG_INFO);
    SYN_LOG_I(TAG, "DataLogger Pipeline Ready");

    syn_cli_init(&cli, commands, sizeof(commands) / sizeof(commands[0]), "> ");
    syn_cli_printf(&cli, "\r\n--- SyntropicOS Data Logger Demo ---\r\n");
    syn_cli_print_prompt(&cli);

    syn_task_create(&tasks[0], "blink", blink_task, 2, NULL);
    syn_task_create(&tasks[1], "cli",   cli_task,   1, NULL);
    syn_task_create(&tasks[2], "adc",   adc_task,   0, NULL);
    syn_sched_init(&sched, tasks, 3);
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
