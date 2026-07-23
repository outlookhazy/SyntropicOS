/**
 * @file HttpClient.ino
 * @brief SyntropicOS Non-Blocking HTTP Client Example
 *
 * Demonstrates:
 *   - Outbound HTTP/1.1 GET/POST requests over Ethernet (W5500)
 *   - Streaming response body chunks via zero-allocation callback (on_http_body)
 *   - HTTP status code parsing, Content-Length & Chunked transfer encoding support
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Start local HTTP server on 192.168.1.100:8080 (e.g. python -m http.server 8080)
 *   2. Open Serial Monitor at 115200 baud to observe HTTP response status and streaming body chunks
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_http.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    /* Safe fallback */
}
#endif

/* ── Network & Target Server Configuration ───────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

static const char *target_host = "192.168.1.100";
static uint16_t    target_port = 8080;
static const char *target_path = "/";

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched      sched;
static SYN_Task       tasks[2];
static SYN_LED        status_led;
static SYN_HttpClient http_client;
static uint8_t        work_buf[256];

#define STATUS_LED_PIN 7  /* Pin 7 status LED */

/* ── Callbacks ─────────────────────────────────────────────────────────── */

/** Streaming HTTP body chunk callback */
static bool on_http_body(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    Serial.print(F("[HTTP BODY CHUNK ("));
    Serial.print(len);
    Serial.print(F("B)]: "));

    for (size_t i = 0; i < len; i++) {
        Serial.print((char)data[i]);
    }
    Serial.println();

    return true; /* Continue reading remaining body */
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

/** Task 2: HTTP Client Task */
static SYN_PT_Status http_task(SYN_PT *pt, SYN_Task *task)
{
    static SYN_PT child_pt;
    PT_BEGIN(pt);
    for (;;) {
        Serial.println(F("\r\n[HTTP CLIENT] Initializing GET request..."));
        syn_http_client_init(&http_client, "GET", target_host, target_port, target_path,
                             NULL, NULL, 0, NULL, 0,
                             on_http_body, NULL, work_buf, sizeof(work_buf));

        /* Execute non-blocking HTTP request task */
        PT_INIT(&child_pt);
        PT_SPAWN(pt, &child_pt, syn_http_client_task(&child_pt, task));

        if (http_client.status == SYN_OK) {
            Serial.print(F("[HTTP SUCCESS] Status Code: "));
            Serial.println(http_client.resp.status_code);
        } else if (http_client.status == SYN_TIMEOUT) {
            Serial.println(F("[HTTP TIMEOUT] Request timed out!"));
        } else {
            Serial.println(F("[HTTP ERROR] Request execution failed!"));
        }

        /* Wait 10 seconds before next query */
        PT_TASK_DELAY_MS(pt, task, 10000);
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

    Serial.println(F("\r\n=== SyntropicOS Ethernet HTTP Client Example ==="));

    /* 1. Network Hardware Init */
    Ethernet.init(10);
    while (W5100.init() == 0) {
        delay(100);
    }
    Ethernet.begin(mac, ip, dns, gateway, subnet);

    Serial.print(F("IP Address: ")); Serial.println(Ethernet.localIP());

    /* 2. LED Init */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    /* 3. Register Scheduler Tasks */
    syn_task_create(&tasks[0], "http",  http_task,  1, &http_client);
    syn_task_create(&tasks[1], "blink", blink_task, 2, NULL);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Scheduler Running. HTTP Client active."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
