/**
 * @file WebsocketServer.ino
 * @brief SyntropicOS Non-Blocking WebSocket Server Example
 *
 * Demonstrates:
 *   - HTTP/1.1 Upgrade to WebSocket (Sec-WebSocket-Key -> SHA-1/Base64 Sec-WebSocket-Accept handshake)
 *   - Frame parsing: masked client text frames, auto Ping/Pong response, Close frame handling
 *   - Full-duplex WebSocket framing over TCP port 8080 on W5500 Ethernet
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test from any Web Browser (Chrome/Firefox/Edge DevTools Console):
 *   let ws = new WebSocket('ws://192.168.1.177:8080/ws');
 *   ws.onmessage = (e) => console.log('Received:', e.data);
 *   ws.send('Hello SyntropicOS!');
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_httpd.h>
#include <syntropic/net/syn_websocket.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    /* Safe fallback */
}
#endif

/* ── Network & Server Configuration ───────────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched            sched;
static SYN_Task             tasks[3];
static SYN_LED              status_led;
static SYN_Httpd            http_server;
static uint8_t              http_buf[512];
static SYN_WebsocketSession ws_session;

#define STATUS_LED_PIN 7  /* Pin 7 status LED */

/* ── Callbacks ─────────────────────────────────────────────────────────── */

/** Inbound WebSocket frame message handler */
static void on_ws_message(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx)
{
    (void)ctx;
    Serial.print(F("[WS RX] Opcode: 0x"));
    Serial.print(opcode, HEX);
    Serial.print(F(" | Payload ("));
    Serial.print(len);
    Serial.print(F("B): "));

    for (size_t i = 0; i < len; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    /* Echo message back with "ECHO: " prefix */
    char echo_buf[128];
    strcpy(echo_buf, "ECHO: ");
    size_t copy_len = len < 100 ? len : 100;
    memcpy(echo_buf + 6, payload, copy_len);
    echo_buf[6 + copy_len] = '\0';

    syn_websocket_send(&ws_session, 0x01, echo_buf, strlen(echo_buf));
}

/** HTTP /ws route handler — performs WebSocket Upgrade */
static void handle_ws_upgrade(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    Serial.println(F("[HTTP] Received WebSocket upgrade request on /ws!"));

    if (syn_websocket_upgrade(req, resp, &ws_session, on_ws_message, NULL) == SYN_OK) {
        Serial.println(F("[WS SUCCESS] Handshake complete! WebSocket active."));
    } else {
        Serial.println(F("[WS ERROR] Upgrade handshake failed!"));
    }
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

/** Task 2: HTTP Server Dispatcher Task */
static SYN_PT_Status httpd_task(SYN_PT *pt, SYN_Task *task)
{
    return syn_httpd_task(pt, task);
}

/** Task 3: WebSocket Polling Task */
static SYN_PT_Status websocket_task(SYN_PT *pt, SYN_Task *task)
{
    return syn_websocket_task(pt, task);
}

/** HTTP Route Table */
static const SYN_HttpdRoute http_routes[] = {
    { SYN_HTTP_GET, "/ws", handle_ws_upgrade, NULL }
};

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

    Serial.println(F("\r\n=== SyntropicOS Ethernet WebSocket Server Example ==="));

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

    /* 3. HTTPD Setup */
    syn_httpd_init(&http_server, 8080, http_routes, 1, http_buf, sizeof(http_buf));

    /* 4. Register Scheduler Tasks */
    syn_task_create(&tasks[0], "httpd", httpd_task,     1, &http_server);
    syn_task_create(&tasks[1], "ws",    websocket_task, 2, &ws_session);
    syn_task_create(&tasks[2], "blink", blink_task,     3, NULL);

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. WebSocket Server active on port 8080."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
