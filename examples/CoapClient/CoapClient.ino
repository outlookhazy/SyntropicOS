/**
 * @file CoapClient.ino
 * @brief SyntropicOS Non-Blocking CoAP Client (RFC 7252) Example
 *
 * Demonstrates:
 *   - Non-blocking CoAP GET/POST requests over UDP Ethernet (W5500)
 *   - CoAP message building, token matching, and URI option encoding
 *   - Exponential backoff retransmissions for CON (Confirmable) messages
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Start CoAP Server on 192.168.1.100:5683 (e.g. aiocoap-fileserver / libcoap / Node-RED)
 *   2. Open Serial Monitor at 115200 baud to view incoming 2.05 Content response payloads
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_coap.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    Serial.print(F("\r\n[BAD ISR DETECTED!]\r\n"));
    Serial.flush();
    delay(100);
}
#endif

/* ── Network & CoAP Server Configuration ───────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

static const SYN_SockAddr coap_server_addr = {
    .ip   = { 192, 168, 1, 100 }, /* Standard CoAP Server IP */
    .port = 5683
};

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched       sched;
static SYN_Task        tasks[3];
static SYN_LED         status_led;
static SYN_CoapRequest coap_req;
static SYN_CoapMsg     req_hdr;
static SYN_CoapOption  req_opts[2];

#define STATUS_LED_PIN 7  /* Pin 7 (avoids Pin 13 SPI SCK header conflict) */

static bool req_active = false;
static uint16_t msg_counter = 1;

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

/** Task 2: CoAP Request Controller & Response Logger */
static SYN_PT_Status coap_control_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 3000);
        if (!req_active) {
            Serial.println(F("[COAP CLIENT] Sending CON GET /telemetry..."));

            memset(&coap_req, 0, sizeof(coap_req));
            memset(&req_hdr, 0, sizeof(req_hdr));

            req_hdr.type      = COAP_TYPE_CON;
            req_hdr.code      = COAP_CODE_GET;
            req_hdr.msg_id    = msg_counter++;
            req_hdr.token_len = 2;
            req_hdr.token[0]  = 0xCA;
            req_hdr.token[1]  = 0xFE;

            req_opts[0].num = COAP_OPT_URI_PATH;
            req_opts[0].val = (const uint8_t *)"telemetry";
            req_opts[0].len = 9;

            coap_req.req_options      = req_opts;
            coap_req.req_option_count = 1;

            syn_coap_request_init(&coap_req, &coap_server_addr, &req_hdr, 2000, 3);
            PT_INIT(&tasks[0].pt);
            tasks[0].state = SYN_TASK_READY;
            req_active = true;
        } else {
            if (coap_req.status == SYN_OK) {
                Serial.print(F("[COAP SUCCESS] Response Code: 0x"));
                Serial.println(coap_req.resp_msg.code, HEX);
                if (coap_req.resp_msg.payload && coap_req.resp_msg.payload_len > 0) {
                    Serial.print(F("[COAP PAYLOAD] "));
                    for (size_t i = 0; i < coap_req.resp_msg.payload_len; i++) {
                        Serial.print((char)coap_req.resp_msg.payload[i]);
                    }
                    Serial.println();
                }
                req_active = false;
            } else if (coap_req.status == SYN_TIMEOUT) {
                Serial.println(F("[COAP TIMEOUT] Retries exhausted. Retrying..."));
                req_active = false;
            } else if (coap_req.status == SYN_ERROR) {
                Serial.println(F("[COAP ERROR] Transmission error. Retrying..."));
                req_active = false;
            }
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

    Serial.println(F("\r\n=== SyntropicOS Ethernet CoAP Client Example ==="));

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

    /* 3. Initial Request Setup & Task Registration */
    req_hdr.type      = COAP_TYPE_CON;
    req_hdr.code      = COAP_CODE_GET;
    req_hdr.msg_id    = 1;
    req_hdr.token_len = 2;
    req_hdr.token[0]  = 0xCA;
    req_hdr.token[1]  = 0xFE;

    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"telemetry";
    req_opts[0].len = 9;

    coap_req.req_options      = req_opts;
    coap_req.req_option_count = 1;

    syn_coap_request_init(&coap_req, &coap_server_addr, &req_hdr, 2000, 3);

    syn_task_create(&tasks[0], "coap",  syn_coap_request_task, 1, &coap_req);
    syn_task_create(&tasks[1], "blink", blink_task,            2, NULL);
    syn_task_create(&tasks[2], "ctrl",  coap_control_task,     3, NULL);

    /* Pause coap task until ctrl task triggers it */
    tasks[0].state = SYN_TASK_SUSPENDED;

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. CoAP Client active."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
