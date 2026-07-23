/**
 * @file EthernetWebServer.ino
 * @brief SyntropicOS Embedded HTTP Web Server Example
 *
 * Demonstrates how to run the SyntropicOS cooperative scheduler and zero-allocation
 * HTTP Web Server (syn_httpd) on Arduino platforms equipped with an Ethernet shield
 * (W5500, W5100, etc.).
 *
 * Features demonstrated:
 *   - SyntropicOS non-blocking protothread scheduler (syn_sched)
 *   - Route-based HTTP Web Server with HTML dashboard and JSON API endpoint
 *   - LED status blinking task running concurrently with network operations
 */

#include <SPI.h>
#include <Ethernet.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_httpd.h>

/* ── Network Configuration ────────────────────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   status_led;
static SYN_Httpd http_server;
static uint8_t   http_work_buffer[512];

#define STATUS_LED_PIN 12  /* Pin 12 (avoids Pin 13 SPI SCK header conflict) */

/* ── HTTP Handlers ──────────────────────────────────────────────────────── */

/** HTTP GET / — Root Dashboard Page */
static void handle_root_page(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "text/html");
    syn_httpd_body_str(resp,
        "<!DOCTYPE html><html><head><title>SyntropicOS Web Server</title>"
        "<style>"
        "body{font-family:sans-serif;background:#121212;color:#eee;padding:2rem;}"
        "h1{color:#4caf50;}"
        ".card{background:#1e1e1e;padding:1.5rem;border-radius:8px;max-width:500px;}"
        "</style></head><body><div class='card'>"
        "<h1>SyntropicOS Web Server</h1>"
        "<p>Cooperative protothread scheduler running over Ethernet.</p>"
        "<p><a href='/api/status' style='color:#4caf50;'>View JSON Status API</a></p>"
        "</div></body></html>"
    );
}

/** HTTP GET /api/status — System Metrics Endpoint */
static void handle_api_status(const SYN_HttpdRequest *req, SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");

    char json_buf[128];
    snprintf(json_buf, sizeof(json_buf),
        "{\"os\":\"SyntropicOS\",\"status\":\"ok\",\"uptime_ms\":%lu}",
        syn_port_get_tick_ms());
    syn_httpd_body_str(resp, json_buf);
}

/** HTTP Route Table */
static const SYN_HttpdRoute http_routes[] = {
    { SYN_HTTP_GET, "/",           handle_root_page,  NULL },
    { SYN_HTTP_GET, "/api/status", handle_api_status, NULL },
};

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
    Ethernet.begin(mac, ip, dns, gateway, subnet);

    Serial.print(F("IP Address: "));
    Serial.println(Ethernet.localIP());

    /* 3. Initialize Status LED */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 500, 500);

    /* 4. Initialize HTTP Web Server on Port 80 */
    syn_httpd_init(&http_server, 80, http_routes,
                   sizeof(http_routes) / sizeof(http_routes[0]),
                   http_work_buffer, sizeof(http_work_buffer));

    /* 5. Create Scheduler Tasks */
    syn_task_create(&tasks[0], "blink", blink_task,     1, NULL);
    syn_task_create(&tasks[1], "httpd", syn_httpd_task, 2, &http_server);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Web Server running on Port 80."));
}

void loop()
{
    /* Run SyntropicOS Cooperative Scheduler */
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
