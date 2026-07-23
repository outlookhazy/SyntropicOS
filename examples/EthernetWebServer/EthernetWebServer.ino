/**
 * @file EthernetWebServer.ino
 * @brief SyntropicOS Embedded HTTP Web Server & Live Dashboard Example
 *
 * Demonstrates:
 *   - SyntropicOS non-blocking protothread scheduler running concurrent tasks
 *   - Zero-allocation HTTP Web Server (syn_httpd) serving rich PROGMEM dashboard
 *   - In-place zero-allocation JSON parsing (syn_json_read) to dynamically configure hardware
 *   - Live REST API (/api/status, /api/config) dynamically changing LED blink frequency
 *   - Status LED blinking concurrently with network operations
 *
 * How to Test:
 *   1. Open Web Browser: http://192.168.1.177:8080/
 *   2. Query REST API:   curl http://192.168.1.177:8080/api/status
 *   3. Update Config:    curl -X POST http://192.168.1.177:8080/api/config -d '{"blink_rate_ms":100}'
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_httpd.h>
#include <syntropic/util/syn_json_read.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
ISR(BADISR_vect) {
    Serial.print(F("\r\n[BAD ISR DETECTED!]\r\n"));
    Serial.flush();
    delay(100);
}
#endif

/* ── Network Configuration ────────────────────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

/* ── Global Instance & Hardware State Declarations ──────────────────────── */

static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   status_led;
static SYN_Httpd http_server;
static uint8_t   http_work_buffer[512];

/* 
 * Pin 13 (LED_BUILTIN) is bridged to the SPI SCK clock line on standard Ethernet shields.
 * Pin 7 is used for the status LED to prevent SPI bus clock interference.
 */
#define STATUS_LED_PIN 7  

/* Dynamic Device State (configurable via POST /api/config) */
static char     dev_mode[16]  = "normal";
static uint16_t dev_blink_ms = 500;

/* ── Dashboard HTML stored in PROGMEM ───────────────────────────────────── */

static const char DASHBOARD_HTML[] PROGMEM =
"<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
"<title>SyntropicOS Dashboard</title><style>"
":root{--bg:#0f172a;--card:rgba(30,41,59,0.7);--accent:#38bdf8;--text:#f8fafc;--muted:#94a3b8;--green:#4ade80}"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);display:flex;justify-content:center;padding:20px;min-height:100vh}"
".container{width:100%;max-width:480px;display:flex;flex-direction:column;gap:16px}"
".header{display:flex;justify-content:space-between;align-items:center;padding:12px 0;border-bottom:1px solid #334155}"
".logo{font-size:1.2rem;font-weight:700;color:var(--accent)}"
".badge{display:inline-flex;align-items:center;gap:6px;background:rgba(74,222,128,0.1);color:var(--green);padding:4px 10px;border-radius:20px;font-size:0.75rem;font-weight:600}"
".dot{width:8px;height:8px;background:var(--green);border-radius:50%;box-shadow:0 0 8px var(--green)}"
".card{background:var(--card);backdrop-filter:blur(12px);border:1px solid rgba(255,255,255,0.1);border-radius:12px;padding:18px}"
".card h2{font-size:0.75rem;color:var(--muted);text-transform:uppercase;margin-bottom:8px;letter-spacing:0.5px}"
".metric{font-size:1.4rem;font-weight:700;color:var(--text)}"
".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}"
".btn{background:var(--accent);color:#0f172a;border:none;padding:10px 16px;border-radius:8px;font-weight:600;cursor:pointer;width:100%}"
".btn:hover{opacity:0.9}"
".log{background:#020617;font-family:monospace;font-size:0.75rem;padding:10px;border-radius:6px;color:var(--accent);margin-top:10px;word-break:break-all}"
"</style></head><body><div class=\"container\">"
"<div class=\"header\"><div class=\"logo\">⚡ SyntropicOS</div><div class=\"badge\"><span class=\"dot\"></span> LIVE</div></div>"
"<div class=\"grid\"><div class=\"card\"><h2>Active Mode</h2><div class=\"metric\" id=\"mode\">normal</div></div>"
"<div class=\"card\"><h2>LED Blink</h2><div class=\"metric\" id=\"rate\">500ms</div></div></div>"
"<div class=\"grid\"><div class=\"card\"><h2>Target MCU</h2><div class=\"metric\">AVR Mega</div></div>"
"<div class=\"card\"><h2>System Uptime</h2><div class=\"metric\" id=\"up\">0s</div></div></div>"
"<div class=\"card\"><h2>Hardware REST Control</h2>"
"<button class=\"btn\" onclick=\"sendCfg('turbo',100)\">Set Turbo Mode (100ms LED)</button>"
"<div style=\"height:8px\"></div>"
"<button class=\"btn\" style=\"background:#94a3b8\" onclick=\"sendCfg('normal',500)\">Reset Normal Mode (500ms LED)</button>"
"<div class=\"log\" id=\"log\">Click a button to update hardware config...</div></div></div>"
"<script>"
"async function poll(){try{let r=await fetch('/api/status');let d=await r.json();document.getElementById('up').innerText=Math.floor(d.uptime_ms/1000)+'s';document.getElementById('mode').innerText=d.mode;document.getElementById('rate').innerText=d.blink_ms+'ms';}catch(e){}}"
"async function sendCfg(m,s){try{let r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mode:m,sample_rate:s})});let d=await r.json();document.getElementById('log').innerText=JSON.stringify(d);poll();}catch(e){document.getElementById('log').innerText='Error: '+e;}}"
"setInterval(poll,1500);poll();"
"</script></body></html>";

/** Helper: Stream PROGMEM HTML string to HTTP Response */
static void send_progmem_html(SYN_HttpdResponse *resp, const char *pgm_html)
{
    size_t len = strlen_P(pgm_html);
    char clen[16];
    snprintf(clen, sizeof(clen), "%u", (unsigned int)len);

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "text/html");
    syn_httpd_header(resp, "Content-Length", clen);

    char chunk[64];
    size_t offset = 0;
    while (offset < len) {
        size_t chunk_len = len - offset;
        if (chunk_len > sizeof(chunk)) chunk_len = sizeof(chunk);
        memcpy_P(chunk, pgm_html + offset, chunk_len);
        syn_httpd_body(resp, chunk, chunk_len);
        offset += chunk_len;
    }
}

/* ── HTTP Handlers ──────────────────────────────────────────────────────── */

/** HTTP GET / — Root Dashboard Page */
static void handle_root_page(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    Serial.println(F("[HTTP SERVER] Serving / Dashboard Page"));
    Serial.flush();
    send_progmem_html(resp, DASHBOARD_HTML);
}

/** HTTP GET /api/status — System Metrics & Configuration Endpoint */
static void handle_api_status(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    Serial.println(F("[HTTP SERVER] Handling /api/status request"));
    Serial.flush();

    char json_buf[160];
    snprintf(json_buf, sizeof(json_buf),
        "{\"os\":\"SyntropicOS\",\"status\":\"ok\",\"mode\":\"%s\",\"blink_ms\":%u,\"uptime_ms\":%lu}",
        dev_mode, dev_blink_ms, syn_port_get_tick_ms());

    char clen_buf[16];
    snprintf(clen_buf, sizeof(clen_buf), "%u", (unsigned int)strlen(json_buf));

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_header(resp, "Content-Length", clen_buf);
    syn_httpd_body_str(resp, json_buf);
}

/** HTTP POST /api/config — Real Hardware Configuration Endpoint */
static void handle_api_config(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    Serial.println(F("[HTTP SERVER] Handling POST /api/config request"));
    Serial.flush();

    char body_buf[128];
    memset(body_buf, 0, sizeof(body_buf));
    int read_bytes = syn_httpd_read_body(req, resp, body_buf, sizeof(body_buf) - 1);

    if (read_bytes > 0) {
        SYN_JsonReader r;
        if (syn_json_parse(&r, body_buf, (size_t)read_bytes)) {
            char new_mode[16];
            if (syn_json_get_str(&r, "mode", new_mode, sizeof(new_mode))) {
                strncpy(dev_mode, new_mode, sizeof(dev_mode) - 1);
                dev_mode[sizeof(dev_mode) - 1] = '\0';
            }
            int32_t new_rate = 0;
            if (syn_json_get_int(&r, "sample_rate", &new_rate) && new_rate > 0) {
                dev_blink_ms = (uint16_t)new_rate;
                /* Dynamically update physical LED blink rate in real time! */
                syn_led_blink(&status_led, dev_blink_ms, dev_blink_ms);
            }
        }
    }

    char json_resp[192];
    snprintf(json_resp, sizeof(json_resp),
        "{\"status\":\"success\",\"applied_mode\":\"%s\",\"blink_rate_ms\":%u}",
        dev_mode, dev_blink_ms);

    char clen_buf[16];
    snprintf(clen_buf, sizeof(clen_buf), "%u", (unsigned int)strlen(json_resp));

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_header(resp, "Content-Length", clen_buf);
    syn_httpd_body_str(resp, json_resp);
}

/** HTTP Route Table */
static const SYN_HttpdRoute http_routes[] = {
    { SYN_HTTP_GET,  "/",           handle_root_page,  NULL },
    { SYN_HTTP_GET,  "/api/status", handle_api_status, NULL },
    { SYN_HTTP_POST, "/api/config", handle_api_config, NULL },
};

/* ── Protothread Tasks ─────────────────────────────────────────────────── */

/** Task 2: Blink Status LED concurrently with HTTP Server */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, 50);
    }
    PT_END(pt);
}

/* ── Setup & Main Loop ─────────────────────────────────────────────────── */

void setup()
{
    /* 1. Ensure Hardware SS pin is configured as OUTPUT HIGH for SPI Master mode */
#if defined(MEGA_SOFT_SPI) || defined(__AVR_ATmega2560__)
    pinMode(53, OUTPUT);
    digitalWrite(53, HIGH);
#endif

    pinMode(4, OUTPUT);  digitalWrite(4, HIGH);   /* Deselect SD Card CS if present */
    pinMode(10, OUTPUT); digitalWrite(10, HIGH);  /* Deselect Ethernet CS */

    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS Ethernet Web Server Example ==="));

    /* 2. Initialize Ethernet Shield (CS Pin 10) */
    Ethernet.init(10);
    while (W5100.init() == 0) {
        delay(100);
    }
    Ethernet.begin(mac, ip, dns, gateway, subnet);

    Serial.print(F("W5500 Chip: W")); Serial.println(W5100.getChip());
    Serial.print(F("Link Status: ")); Serial.println((int)Ethernet.linkStatus());
    Serial.print(F("IP Address: ")); Serial.println(Ethernet.localIP());

    /* 3. Initialize Status LED */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, dev_blink_ms, dev_blink_ms);

    /* 4. Initialize HTTP Web Server on Port 8080 */
    syn_httpd_init(&http_server, 8080, http_routes,
                   sizeof(http_routes) / sizeof(http_routes[0]),
                   http_work_buffer, sizeof(http_work_buffer));

    /* 5. Register Concurrent Scheduler Tasks */
    syn_task_create(&tasks[0], "httpd", syn_httpd_task, 1, &http_server);
    syn_task_create(&tasks[1], "blink", blink_task, 2, NULL);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Web Server running on Port 8080."));
}

void loop()
{
    /* Run SyntropicOS Cooperative Scheduler */
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
