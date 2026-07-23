/**
 * @file FftSpectrumAnalyzer.ino
 * @brief SyntropicOS Fixed-Point Radix-2 FFT Spectrum Analyzer Example
 *
 * Demonstrates:
 *   - 32-point Q16.16 Radix-2 Decimation-in-Time Fast Fourier Transform (syn_dsp_fft)
 *   - Live ADC sampling with DC offset removal
 *   - ASCII power spectrum visualization over Serial
 *   - Cooperative multi-tasking via SyntropicOS scheduler
 *
 * How to Test:
 *   1. Connect a wire to pin A15 (touch it or attach a signal)
 *   2. Open Serial Monitor at 115200 baud
 *   3. Observe live frequency bin magnitude analysis updated every ~500ms
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/dsp/syn_fft.h>

#define FFT_SIZE 32
#define SAMPLING_FREQ_HZ 1000  /* 1000 Hz sampling rate -> 31.25 Hz resolution per bin */
#define ADC_PIN A15

static q16_t real_buf[FFT_SIZE];
static q16_t imag_buf[FFT_SIZE];

static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   status_led;

#define STATUS_LED_PIN 7

/* ── Protothread Tasks ─────────────────────────────────────────────────── */

static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/**
 * FFT spectrum analyzer task.
 *
 * NOTE: All local variables that persist across PT_TASK_DELAY_MS must be
 * declared static. Protothread yield/delay macros expand to a switch/case
 * Duff's device that resumes execution by jumping past variable declarations,
 * leaving stack locals uninitialized on re-entry.
 */
static SYN_PT_Status fft_task(SYN_PT *pt, SYN_Task *task)
{
    /* All locals must be static — protothreads resume via switch/case */
    static int16_t raw_adc[FFT_SIZE];
    static int32_t sum;
    static int16_t dc_offset;
    static uint16_t i;
    static uint16_t k;
    static uint16_t max_bin;
    static int32_t  max_mag;
    static int32_t  re, im, mag;
    static uint16_t freq_x10;
    static uint8_t  bar_len;

    PT_BEGIN(pt);
    for (;;) {
        /* 1. Sample 32 ADC readings from pin A15 at ~1000 Hz */
        sum = 0;
        for (i = 0; i < FFT_SIZE; i++) {
            raw_adc[i] = analogRead(ADC_PIN);
            sum += raw_adc[i];
            delay(1);
        }

        /* 2. Compute mean and subtract DC offset — pure integer Q16 */
        dc_offset = (int16_t)(sum / FFT_SIZE);
        for (i = 0; i < FFT_SIZE; i++) {
            /* Scale centered value into Q16: (sample - dc) / 512 * 65536 = (sample - dc) * 128 */
            real_buf[i] = (q16_t)(raw_adc[i] - dc_offset) * 128;
            imag_buf[i] = 0;
        }

        /* 3. Execute Q16.16 Radix-2 FFT */
        syn_dsp_fft(real_buf, imag_buf, FFT_SIZE);

        /* 4. Output Magnitude Spectrum over Serial — pure integer math */
        Serial.println(F("\r\n=== Live Hardware ADC (A15) Q16.16 FFT Spectrum ==="));
        Serial.print(F("[ADC A15] DC Baseline: "));
        Serial.print(dc_offset);
        Serial.println(F(" / 1023"));

        max_bin = 0;
        max_mag = 0;

        for (k = 1; k < FFT_SIZE / 2; k++) {
            /* |Re| + |Im| magnitude approximation (no sqrt needed) */
            re = real_buf[k] < 0 ? -real_buf[k] : real_buf[k];
            im = imag_buf[k] < 0 ? -imag_buf[k] : imag_buf[k];
            mag = (re + im) >> 8;  /* Scale down for display */

            if (mag > max_mag) {
                max_mag = mag;
                max_bin = k;
            }

            /* Frequency label: k * (Fs / N), printed as fixed point */
            freq_x10 = (uint16_t)((uint32_t)k * SAMPLING_FREQ_HZ * 10 / FFT_SIZE);
            Serial.print(F("[BIN "));
            if (k < 10) Serial.print(F("0"));
            Serial.print(k);
            Serial.print(F("] "));
            Serial.print(freq_x10 / 10);
            Serial.print('.');
            Serial.print(freq_x10 % 10);
            Serial.print(F(" Hz | Mag: "));
            Serial.print(mag);
            Serial.print(F(" | "));

            bar_len = (uint8_t)(mag / 8);
            if (bar_len > 40) bar_len = 40;
            for (uint8_t b = 0; b < bar_len; b++) Serial.print('#');
            Serial.println();
        }

        freq_x10 = (uint16_t)((uint32_t)max_bin * SAMPLING_FREQ_HZ * 10 / FFT_SIZE);
        Serial.print(F("--> LIVE PEAK NOISE DETECTED: Bin "));
        Serial.print(max_bin);
        Serial.print(F(" ("));
        Serial.print(freq_x10 / 10);
        Serial.print('.');
        Serial.print(freq_x10 % 10);
        Serial.println(F(" Hz)"));
        Serial.flush();

        PT_TASK_DELAY_MS(pt, task, 500);
    }
    PT_END(pt);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS FFT Spectrum Analyzer Example ==="));

    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    syn_task_create(&tasks[0], "fft",   fft_task,   1, NULL);
    syn_task_create(&tasks[1], "blink", blink_task, 2, NULL);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Scheduler Running. FFT Analyzer active."));
}

void loop()
{
    syn_sched_run(&sched);
    delay(1);
}
