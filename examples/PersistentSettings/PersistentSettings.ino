/**
 * @file PersistentSettings.ino
 * @brief SyntropicOS Non-Volatile Wear-Leveled Storage Example
 *
 * Demonstrates:
 *   - Wear-leveled Flash/EEPROM persistent parameter storage (syn_param / syn_settings)
 *   - Automatic CRC-16 integrity verification and corruption detection
 *   - Automatic load-or-default initialization on system startup
 *   - Auto-incrementing boot counter persisting across hardware reboots and power cycles
 *   - Concurrent execution with scheduler tasks (status LED)
 *
 * How to Test:
 *   1. Open Serial Monitor at 115200 baud
 *   2. Press the Reset button or power-cycle the board multiple times
 *   3. Observe the boot counter incrementing sequentially across reboots (Boot #1 -> Boot #2 -> Boot #3)
 */

#include <SyntropicOS.h>
#include <syntropic/sched/syn_sched.h>
#include <syntropic/port/syn_port_serial.h>
#include <syntropic/output/syn_led.h>
#include <syntropic/storage/syn_settings.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
ISR(BADISR_vect) {
    /* Safe fallback */
}
#endif

/* ── Application Settings Structure ─────────────────────────────────────── */

typedef struct {
    uint32_t boot_count;
    uint16_t blink_rate_ms;
    char     device_name[32];
} AppConfig;

static const AppConfig default_config = {
    0,                      /* boot_count */
    200,                    /* blink_rate_ms */
    "SyntropicOS-Mega"      /* device_name */
};

static AppConfig    app_config;
static SYN_Settings settings_store;

/* ── Global Scheduler Tasks ────────────────────────────────────────────── */

static SYN_Sched sched;
static SYN_Task  tasks[2];
static SYN_LED   status_led;

#define STATUS_LED_PIN 7  /* Pin 7 status LED */
#define FLASH_BASE_ADDR 0  /* Start of EEPROM / Flash storage region */

/* ── Callbacks ─────────────────────────────────────────────────────────── */

static void on_settings_changed(void *data, void *ctx)
{
    (void)ctx;
    AppConfig *cfg = (AppConfig *)data;
    Serial.print(F("[STORAGE CALLBACK] Settings updated & written to flash! Boot Count: "));
    Serial.println(cfg->boot_count);
}

/* ── Protothread Tasks ─────────────────────────────────────────────────── */

/** Task 1: Blink Status LED */
static SYN_PT_Status blink_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        syn_led_update(&status_led);
        PT_TASK_DELAY_MS(pt, task, app_config.blink_rate_ms);
    }
    PT_END(pt);
}

/** Task 2: Config Management & Periodic Storage Sync Task */
static SYN_PT_Status config_task(SYN_PT *pt, SYN_Task *task)
{
    PT_BEGIN(pt);
    for (;;) {
        PT_TASK_DELAY_MS(pt, task, 5000);
        Serial.print(F("[STORAGE REPORT] Current Boot Count: "));
        Serial.print(app_config.boot_count);
        Serial.print(F(" | CRC-16 Checksum: 0x"));
        Serial.println(syn_settings_checksum(&settings_store), HEX);
    }
    PT_END(pt);
}

/* ── Setup & Main Loop ─────────────────────────────────────────────────── */

void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\r\n=== SyntropicOS Wear-Leveled Storage Example ==="));

    /* 1. Initialize Persistent Settings Engine */
    /* Base Address 0, 2 Sectors for Wear Leveling */
    SYN_Status st = syn_settings_init(&settings_store, FLASH_BASE_ADDR, 2,
                                      &app_config, sizeof(app_config), &default_config);
    if (st == SYN_OK) {
        Serial.println(F("[STORAGE SUCCESS] Flash settings initialized!"));
    } else {
        Serial.println(F("[STORAGE WARNING] Default settings applied!"));
    }

    syn_settings_on_change(&settings_store, on_settings_changed, NULL);

    /* 2. Increment Boot Counter & Save */
    app_config.boot_count++;
    Serial.print(F("--> SYSTEM BOOT #"));
    Serial.println(app_config.boot_count);
    Serial.print(F("--> Device Name: "));
    Serial.println(app_config.device_name);

    syn_settings_save(&settings_store);

    /* 3. Initialize LED */
    syn_led_init(&status_led, STATUS_LED_PIN, SYN_LED_ACTIVE_HIGH);
    syn_led_blink(&status_led, app_config.blink_rate_ms, app_config.blink_rate_ms);

    /* 4. Register Scheduler Tasks */
    syn_task_create(&tasks[0], "config", config_task, 1, NULL);
    syn_task_create(&tasks[1], "blink",  blink_task,  2, NULL);

    syn_sched_init(&sched, tasks, 2);

    Serial.println(F("SyntropicOS Scheduler Running. Persistent storage active."));
}

void loop()
{
    if (!syn_sched_run(&sched)) {
        syn_port_delay_ms(1);
    }
}
