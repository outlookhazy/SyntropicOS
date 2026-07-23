/**
 * @file GpsNmeaParser.ino
 * @brief SyntropicOS Non-Blocking NMEA 0183 GPS Streaming Parser Example
 *
 * Demonstrates:
 *   - Zero-allocation streaming NMEA 0183 GPS sentence parsing (syn_nmea)
 *   - Parsing $GPRMC & $GPGGA sentences with XOR checksum validation
 *   - Extracting microdegree coordinates, altitude, UTC timestamp, satellite count, and fix status
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Connect NMEA GPS module (e.g. NEO-6M / NEO-M8N) to UART (115200 baud)
 *   2. Open Serial Monitor at 115200 baud to observe live parsed coordinates and satellite status logs
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/proto/syn_nmea.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    /* Safe fallback */
}
#endif

/* ── Global Declarations ───────────────────────────────────────────────── */

static SYN_NmeaParser nmea_parser;
static SYN_Sched      sched;
static SYN_Task       tasks[2];
static SYN_LED        status_led;

#define STATUS_LED_PIN 7  /* Pin 7 status LED */

/* ── Callbacks ─────────────────────────────────────────────────────────── */

/** Callback triggered when a valid NMEA sentence (RMC / GGA) is parsed */
static void on_gps_fix(const SYN_NmeaData *data, const char *sentence_type, void *ctx)
{
    (void)ctx;
    Serial.print(F("[GPS FIX - "));
    Serial.print(sentence_type);
    Serial.print(F("] Time: "));
    Serial.print(data->utc_time);
    Serial.print(F(" | Lat: "));
    Serial.print((float)data->lat_microdeg / 1000000.0, 6);
    Serial.print(F(" | Lon: "));
    Serial.print((float)data->lon_microdeg / 1000000.0, 6);
    Serial.print(F(" | Alt: "));
    Serial.print((float)data->alt_mm / 1000.0, 1);
    Serial.print(F("m | Sats: "));
    Serial.print(data->satellites);
    Serial.print(F(" | Valid: "));
    Serial.println(data->valid ? F("YES") : F("NO"));
}

/* ── Protothread Tasks ─────────────────────────────────────────────────── */

/** Task 1: Blink Status LED */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/** Task 2: GPS Serial Receiver & Parser Task */
static SYN_PT_Status gps_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        /* Read available bytes from Serial UART */
        while (Serial.available()) {
            char ch = (char)Serial.read();
            syn_nmea_feed_char(&nmea_parser, ch);
        }
        PT_DEFER(pt, task);
    }
    PT_END(pt);
}

/* ── Setup & Main Loop ─────────────────────────────────────────────────── */

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS NMEA 0183 GPS Streaming Parser Example ==="));

    /* 1. Initialize NMEA Parser */
    syn_nmea_init(&nmea_parser, on_gps_fix, NULL);

    /* 2. Initialize LED */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    /* 3. Register Scheduler Tasks */
    syn_task_create(&tasks[0], "gps",   gps_task,   1, NULL);
    syn_task_create(&tasks[1], "blink", blink_task, 2, NULL);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Scheduler Running. NMEA GPS Parser active."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
