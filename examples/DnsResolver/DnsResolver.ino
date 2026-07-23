/**
 * @file DnsResolver.ino
 * @brief SyntropicOS Non-Blocking DNS Resolver & mDNS Responder Example
 *
 * Demonstrates:
 *   - Non-blocking UDP DNS resolution (syn_dns_resolve_task) over Ethernet (W5500)
 *   - Resolving hostnames ("example.com") to IPv4 addresses
 *   - Running an mDNS responder (syn_mdns_task) to advertise "syntropic-mega.local"
 *   - Running concurrently with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Ping mDNS Hostname: ping syntropic-mega.local
 *   2. View Serial Logs:   Open Serial Monitor at 115200 baud to observe 'example.com' resolution
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_dns.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    Serial.print(F("\r\n[BAD ISR DETECTED!]\r\n"));
    Serial.flush();
    delay(100);
}
#endif

/* ── Network & DNS Configuration ─────────────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

static const SYN_SockAddr dns_server_addr = {
    .ip   = { 8, 8, 8, 8 }, /* Google Public DNS Server */
    .port = 53
};

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched       sched;
static SYN_Task        tasks[3];
static SYN_LED         status_led;
static SYN_DnsResolver dns_resolver;
static SYN_Mdns        mdns_responder;
static SYN_SockAddr    resolved_addr;

#define STATUS_LED_PIN 7  /* Pin 7 (avoids Pin 13 SPI SCK header conflict) */

static bool query_started = false;

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

/** Task 2: DNS Query Controller & Status Logger */
static SYN_PT_Status dns_control_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 3000);
        if (!query_started) {
            Serial.println(F("[DNS CLIENT] Querying DNS server for 'example.com'..."));
            
            memset(&dns_resolver, 0, sizeof(dns_resolver));
            dns_resolver.dns_server = &dns_server_addr;
            dns_resolver.hostname   = "example.com";
            dns_resolver.addr_out   = &resolved_addr;
            dns_resolver.timeout_ms = 5000;
            dns_resolver.status     = SYN_BUSY;

            syn_task_create(&tasks[0], "dns", syn_dns_resolve_task, 1, &dns_resolver);
            query_started = true;
        } else {
            if (dns_resolver.status == SYN_OK) {
                Serial.print(F("[DNS SUCCESS] Resolved 'example.com' -> "));
                Serial.print(resolved_addr.ip[0]); Serial.print(F("."));
                Serial.print(resolved_addr.ip[1]); Serial.print(F("."));
                Serial.print(resolved_addr.ip[2]); Serial.print(F("."));
                Serial.println(resolved_addr.ip[3]);
            } else if (dns_resolver.status == SYN_TIMEOUT) {
                Serial.println(F("[DNS TIMEOUT] Query timed out. Retrying..."));
                query_started = false;
            } else if (dns_resolver.status == SYN_ERROR) {
                Serial.println(F("[DNS ERROR] Resolution failed. Retrying..."));
                query_started = false;
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

    Serial.println(F("\r\n=== SyntropicOS Ethernet DNS / mDNS Example ==="));

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

    /* 3. mDNS Responder Init (Advertising "syntropic-mega.local") */
    uint8_t local_ip_bytes[4] = { ip[0], ip[1], ip[2], ip[3] };
    syn_mdns_init(&mdns_responder, "syntropic-mega", local_ip_bytes);

    /* 4. Register Tasks */
    syn_task_create(&tasks[0], "mdns",   syn_mdns_task,        1, &mdns_responder);
    syn_task_create(&tasks[1], "blink",  blink_task,           2, NULL);
    syn_task_create(&tasks[2], "ctrl",   dns_control_task,     3, NULL);

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. mDNS Responder active."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
