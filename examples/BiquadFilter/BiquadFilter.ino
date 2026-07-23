/**
 * @file BiquadFilter.ino
 * @brief SyntropicOS Fixed-Point Q16.16 Biquad IIR Filter Example
 *
 * Demonstrates:
 *   - Fixed-point Butterworth lowpass biquad filter (syn_biquad.h)
 *   - Real-time sampling of analog pin A15 at 100 Hz
 *   - Attenuation of high-frequency noise without floating-point math
 *   - Direct compatibility with Arduino Serial Plotter (Raw vs Filtered)
 *   - Non-blocking scheduler execution with status LED blinking
 *
 * How to Test:
 *   1. Connect Arduino Mega 2560 on USB (COM23 at 115200 baud).
 *   2. Open Serial Plotter or Serial Monitor at 115200 baud.
 *   3. Observe smooth lowpass-filtered response vs raw noisy ADC readings on A15.
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/dsp/syn_biquad.h>
#include <syntropic/output/syn_led.h>

#define STATUS_LED_PIN 7
#define ADC_PIN        A15

#define SAMPLE_RATE_HZ 100
#define CUTOFF_FREQ_HZ 5

/* ── Filter & OS Objects ────────────────────────────────────────────────── */

static SYN_FilterBiquad lp_filter;
static SYN_Sched        sched;
static SYN_Task         tasks[2];
static SYN_LED          status_led;

/* ── Filter Task (Priority 1, 10ms = 100 Hz) ────────────────────────────── */

static SYN_PT_Status filter_task(SYN_PT *pt, SYN_Task *task)
{
    static int16_t raw_val;
    static q16_t   raw_q16;
    static q16_t   filt_q16;
    static int16_t filt_val;
    static uint8_t print_divider = 0;

    PT_BEGIN(pt);
    for (;;) {
        /* 1. Read raw ADC sample (0 - 1023) */
        raw_val = analogRead(ADC_PIN);

        /* 2. Convert to Q16.16 fixed-point */
        raw_q16 = Q16_FROM_INT(raw_val);

        /* 3. Apply Q16.16 Butterworth Lowpass Biquad Filter */
        filt_q16 = syn_filter_biquad_update(&lp_filter, raw_q16);

        /* 4. Convert back to integer (0 - 1023 range) */
        filt_val = (int16_t)Q16_TO_INT(filt_q16);

        /* Clamp output for plot safety */
        if (filt_val < 0)    filt_val = 0;
        if (filt_val > 1023) filt_val = 1023;

        /* Print Serial Plotter formatted string every 5 samples (50ms) */
        print_divider++;
        if (print_divider >= 5) {
            print_divider = 0;
            Serial.print(F("Raw:"));
            Serial.print(raw_val);
            Serial.print(F(" Filtered:"));
            Serial.println(filt_val);
            Serial.flush();
        }

        /* 100 Hz sampling period (10ms) */
        PT_TASK_DELAY_MS(pt, task, 10);
    }
    PT_END(pt);
}

/* ── LED Task (Priority 2) ─────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* ── Arduino Setup & Loop ───────────────────────────────────────────────── */

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS Biquad IIR Filter Example ==="));
    Serial.print(F("Configuring 2nd-order Butterworth Lowpass: Cutoff="));
    Serial.print(CUTOFF_FREQ_HZ);
    Serial.print(F(" Hz @ SampleRate="));
    Serial.print(SAMPLE_RATE_HZ);
    Serial.println(F(" Hz"));

    /* Initialize 5 Hz Lowpass Filter at 100 Hz sampling rate */
    syn_filter_biquad_lowpass(&lp_filter,
                              Q16_FROM_INT(CUTOFF_FREQ_HZ),
                              Q16_FROM_INT(SAMPLE_RATE_HZ));

    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    syn_task_create(&tasks[0], "filter", filter_task, 1, NULL);
    syn_task_create(&tasks[1], "blink",  blink_task,  2, NULL);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Scheduler Running. (Plotter format: Raw:XXX Filtered:YYY)"));
}

void loop()
{
    syn_sched_run(&sched);
    delay(1);
}
