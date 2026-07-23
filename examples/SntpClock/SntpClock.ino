/**
 * @file SntpClock.ino
 * @brief SyntropicOS Non-Blocking SNTP Time Sync + RTC Calendar Example
 *
 * Demonstrates:
 *   - Non-blocking UDP time synchronization (syn_sntp) over Ethernet (W5500)
 *   - Querying an NTP server on UDP port 123
 *   - Maintaining a local UTC epoch clock (syn_sntp_get_epoch_s)
 *   - Converting UTC epoch to calendar date/time via RTC driver (syn_rtc_from_epoch)
 *   - Continuous DSP Exponential Moving Average filtering (SYN_FilterEMA) for clock drift compensation
 *   - Running concurrently with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Connect Ethernet cable with internet/gateway access to 216.239.35.0 (Google NTP)
 *   2. Open Serial Monitor at 115200 baud to view live formatted UTC time & PPM drift logs
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_sntp.h>
#include <syntropic/drivers/syn_rtc.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    Serial.print(F("\r\n[BAD ISR DETECTED!]\r\n"));
    Serial.flush();
    delay(100);
}
#endif

/* ── Network & NTP Configuration ──────────────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

static const SYN_SockAddr ntp_server_addr = {
    .ip   = { 216, 239, 35, 0 }, /* Google Public NTP Server (time1.google.com) */
    .port = 123
};

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched sched;
static SYN_Task  tasks[3];
static SYN_LED   status_led;
static SYN_SNTP  sntp_client;

#define STATUS_LED_PIN 7  /* Pin 7 (avoids Pin 13 SPI SCK header conflict) */

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

/** Task 2: Print Synced UTC Time & Calendar Date Every 2 Seconds */
static SYN_PT_Status clock_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 2000);
        if (syn_sntp_is_synced(&sntp_client)) {
            uint32_t epoch = syn_sntp_get_epoch_s(&sntp_client);
            int32_t drift_ppm = syn_sntp_get_drift_ppm(&sntp_client);
            
            /* Apply calculated hardware clock drift to RTC driver */
            syn_rtc_set_drift_ppm(drift_ppm);

            /* Use syn_rtc driver to convert Epoch to Calendar Date/Time */
            SYN_RTC_DateTime dt;
            syn_rtc_from_epoch(epoch, &dt);
            syn_rtc_set(&dt); /* Sync hardware/software RTC */

            char buf[80];
            snprintf(buf, sizeof(buf),
                "[UTC CLOCK] %04u-%02u-%02u %02u:%02u:%02u (Epoch: %lu s | Drift: %ld PPM)",
                dt.year, dt.month, dt.day,
                dt.hour, dt.minute, dt.second,
                (unsigned long)epoch,
                (long)drift_ppm);

            Serial.println(buf);
        } else {
            Serial.println(F("[UTC CLOCK] Waiting for NTP sync..."));
        }
    }
    PT_END(pt);
}

/* ── Setup & Main Loop ─────────────────────────────────────────────────── */

void setup()
{
#if defined(MEGA_SOFT_SPI) || defined(__AVR_ATmega2560__)
    pinMode(53, OUTPUT); digitalWrite(53, HIGH);
#endif
    pinMode(4, OUTPUT);  digitalWrite(4, HIGH);
    pinMode(10, OUTPUT); digitalWrite(10, HIGH);

    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS Ethernet SNTP Clock + RTC Example ==="));

    /* 1. Network Hardware Init */
    Ethernet.init(10);
    while (W5100.init() == 0) {
        delay(100);
    }
    Ethernet.begin(mac, ip, dns, gateway, subnet);

    Serial.print(F("IP Address: ")); Serial.println(Ethernet.localIP());
    Serial.print(F("NTP Server Target: "));
    Serial.print(ntp_server_addr.ip[0]); Serial.print(F("."));
    Serial.print(ntp_server_addr.ip[1]); Serial.print(F("."));
    Serial.print(ntp_server_addr.ip[2]); Serial.print(F("."));
    Serial.print(ntp_server_addr.ip[3]); Serial.print(F(":"));
    Serial.println(ntp_server_addr.port);

    /* 2. LED Init */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    /* 3. RTC Init */
    syn_rtc_init();

    /* 4. SNTP Client Init (Re-sync every 10 seconds for testing) */
    syn_sntp_init(&sntp_client, &ntp_server_addr, 10);

    /* 5. Register Tasks */
    syn_task_create(&tasks[0], "sntp",  syn_sntp_task, 1, &sntp_client);
    syn_task_create(&tasks[1], "blink", blink_task,    2, NULL);
    syn_task_create(&tasks[2], "clock", clock_task,    3, NULL);

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. Querying NTP Server..."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
