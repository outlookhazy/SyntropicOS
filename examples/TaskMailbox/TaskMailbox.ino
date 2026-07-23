/**
 * @file TaskMailbox.ino
 * @brief SyntropicOS Lock-Free Inter-Task Mailbox Messaging Example
 *
 * Demonstrates:
 *   - Lock-free Single-Producer Single-Consumer (SPSC) queue (syn_mailbox.h)
 *   - Producer task generating ADC sensor payloads and posting to mailbox
 *   - Consumer task retrieving and processing messages asynchronously
 *   - Cooperative multi-tasking with LED status indicator
 *
 * How to Test:
 *   1. Connect Arduino Mega 2560 to USB (COM23 at 115200 baud).
 *   2. Connect a wire or signal to analog pin A15 (optional).
 *   3. Open Serial Monitor at 115200 baud.
 *   4. Observe producer posting sequence events and consumer receiving/processing.
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/sched/syn_mailbox.h>
#include <syntropic/output/syn_led.h>

#define STATUS_LED_PIN 7
#define ADC_PIN        A15

/* ── Message Payload Structure ─────────────────────────────────────────── */

typedef struct {
    uint32_t seq;         /**< Sequence counter */
    uint32_t timestamp;   /**< Production timestamp in ms */
    uint16_t adc_val;     /**< Raw ADC sensor sample */
} SensorEvent;

/* Define static SPSC mailbox with capacity of 8 messages */
SYN_MAILBOX_DEFINE(sensor_mailbox, SensorEvent, 8);

/* ── OS Objects ─────────────────────────────────────────────────────────── */

static SYN_Sched sched;
static SYN_Task  tasks[3];
static SYN_LED   status_led;

/* ── Producer Task (Priority 1) ─────────────────────────────────────────── */

static SYN_PT_Status producer_task(SYN_PT *pt, SYN_Task *task)
{
    static uint32_t seq = 0;
    static SensorEvent evt;
    static bool posted;

    PT_BEGIN(pt);
    for (;;) {
        seq++;
        evt.seq       = seq;
        evt.timestamp = syn_port_get_tick_ms();
        evt.adc_val   = analogRead(ADC_PIN);

        posted = syn_mailbox_post(&sensor_mailbox, &evt);

        if (posted) {
            Serial.print(F("[PRODUCER] Posted event #"));
            Serial.print(evt.seq);
            Serial.print(F(" (ADC: "));
            Serial.print(evt.adc_val);
            Serial.print(F(", pending: "));
            Serial.print(syn_mailbox_pending(&sensor_mailbox));
            Serial.println(F(")"));
        } else {
            Serial.print(F("[PRODUCER] MAILBOX FULL! Dropped event #"));
            Serial.print(evt.seq);
            Serial.print(F(" (overflows: "));
            Serial.print(syn_mailbox_overflows(&sensor_mailbox));
            Serial.println(F(")"));
        }
        Serial.flush();

        PT_TASK_DELAY_MS(pt, task, 300);
    }
    PT_END(pt);
}

/* ── Consumer Task (Priority 2) ─────────────────────────────────────────── */

static SYN_PT_Status consumer_task(SYN_PT *pt, SYN_Task *task)
{
    static SensorEvent rx_evt;
    static uint32_t rx_count = 0;

    PT_BEGIN(pt);
    for (;;) {
        while (syn_mailbox_receive(&sensor_mailbox, &rx_evt)) {
            rx_count++;
            Serial.print(F("  [CONSUMER] Received #"));
            Serial.print(rx_evt.seq);
            Serial.print(F(" | Latency: "));
            Serial.print(syn_port_get_tick_ms() - rx_evt.timestamp);
            Serial.print(F("ms | ADC Raw: "));
            Serial.print(rx_evt.adc_val);
            Serial.print(F(" | Total Rx: "));
            Serial.println(rx_count);
            Serial.flush();
        }

        /* Check for pending items every 100ms */
        PT_TASK_DELAY_MS(pt, task, 100);
    }
    PT_END(pt);
}

/* ── LED Task (Priority 3) ─────────────────────────────────────────────── */

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

    Serial.println(F("\r\n=== SyntropicOS Task Mailbox Example ==="));

    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    syn_task_create(&tasks[0], "prod",  producer_task, 1, NULL);
    syn_task_create(&tasks[1], "cons",  consumer_task, 2, NULL);
    syn_task_create(&tasks[2], "blink", blink_task,    3, NULL);

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. Producer (300ms) -> Consumer (100ms)."));
}

void loop()
{
    syn_sched_run(&sched);
    delay(1);
}
