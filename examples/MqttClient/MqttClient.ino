/**
 * @file MqttClient.ino
 * @brief SyntropicOS Non-Blocking MQTT Client Example
 *
 * Demonstrates:
 *   - Connecting non-blockingly to an MQTT broker over Ethernet (W5500)
 *   - Subscribing to topics ("syntropic/cmd") with callback processing
 *   - Periodically publishing JSON telemetry ("syntropic/telemetry")
 *   - Running concurrently with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Subscribe Telemetry: mosquitto_sub -h 192.168.1.100 -t "syntropic/telemetry"
 *   2. Send Commands:      mosquitto_pub -h 192.168.1.100 -t "syntropic/cmd" -m "LED_TURBO"
 */

#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/net/syn_mqtt.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    Serial.print(F("\r\n[BAD ISR DETECTED!]\r\n"));
    Serial.flush();
    delay(100);
}
#endif

/* ── Network & MQTT Configuration ────────────────────────────────────────── */

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static IPAddress ip(192, 168, 1, 177);
static IPAddress dns(192, 168, 1, 1);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);

static const char BROKER_IP[]   = "192.168.1.100"; /* Standard LAN MQTT broker IP (e.g. Mosquitto / Home Assistant) */
static const uint16_t BROKER_PORT = 1883;

/* ── Global Instance Declarations ───────────────────────────────────────── */

static SYN_Sched      sched;
static SYN_Task       tasks[3];
static SYN_LED        status_led;
static SYN_MqttClient mqtt_client;

static uint8_t mqtt_rx_buf[256];
static uint8_t mqtt_tx_buf[256];

#define STATUS_LED_PIN 7  /* Pin 7 (avoids Pin 13 SPI SCK header conflict) */

static bool has_subscribed = false;

/* ── MQTT Callbacks & Handlers ──────────────────────────────────────────── */

static void on_mqtt_message(const char *topic, const uint8_t *payload, size_t len, void *ctx)
{
    (void)ctx;
    Serial.print(F("[MQTT RX CALLBACK] Topic: "));
    Serial.print(topic);
    Serial.print(F(" | Payload ("));
    Serial.print(len);
    Serial.print(F("B): "));
    for (size_t i = 0; i < len; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.flush();
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

/** Task 2: Periodic Telemetry Publisher & Subscription Manager */
static SYN_PT_Status pub_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 1500);
        if (mqtt_client.state == SYN_MQTT_CONNECTED) {
            if (!has_subscribed) {
                Serial.println(F("[MQTT SUB] Subscribing to syntropic/cmd..."));
                syn_mqtt_subscribe(&mqtt_client, "syntropic/cmd", 0);
                has_subscribed = true;
            }
            char payload[128];
            snprintf(payload, sizeof(payload),
                "{\"device\":\"Mega2560\",\"uptime_ms\":%lu}",
                syn_port_get_tick_ms());
            Serial.print(F("[MQTT PUB] Publishing telemetry: "));
            Serial.println(payload);
            syn_mqtt_publish(&mqtt_client, "syntropic/telemetry", payload, strlen(payload), 0, false);
        } else {
            has_subscribed = false;
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

    Serial.println(F("\r\n=== SyntropicOS Ethernet MQTT Client Example ==="));

    /* 1. Network Hardware Init */
    Ethernet.init(10);
    while (W5100.init() == 0) {
        delay(100);
    }
    Ethernet.begin(mac, ip, dns, gateway, subnet);

    Serial.print(F("IP Address: ")); Serial.println(Ethernet.localIP());
    Serial.print(F("MQTT Broker Target: ")); Serial.print(BROKER_IP); Serial.print(F(":")); Serial.println(BROKER_PORT);

    /* 2. LED Init */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, 200, 200);

    /* 3. MQTT Client Init */
    syn_mqtt_init(&mqtt_client, BROKER_IP, BROKER_PORT,
                  "Mega2560_Client", NULL, NULL, 60,
                  mqtt_rx_buf, sizeof(mqtt_rx_buf),
                  mqtt_tx_buf, sizeof(mqtt_tx_buf));

    mqtt_client.on_message = on_mqtt_message;

    /* 4. Register Tasks (Distinct priorities: Prio 1 > Prio 2 > Prio 3) */
    syn_task_create(&tasks[0], "mqtt",  syn_mqtt_task, 1, &mqtt_client);
    syn_task_create(&tasks[1], "blink", blink_task,    2, NULL);
    syn_task_create(&tasks[2], "pub",   pub_task,      3, NULL);

    syn_sched_init(&sched, tasks, 3);

    Serial.println(F("SyntropicOS Scheduler Running. Connecting to MQTT Broker..."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
