/**
 * @file port_esp32.c
 * @brief SyntropicOS port layer for ESP32 (ESP-IDF).
 *
 * Implements the system, GPIO, and UART interfaces by wrapping the standard
 * Espressif ESP-IDF drivers and FreeRTOS timing functions.
 */

#if defined(ESP_PLATFORM) && !defined(ARDUINO)

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "syntropic/common/syn_compiler.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_uart.h"

static const char *TAG = "syn_port";

/* ── System Port ────────────────────────────────────────────────────────── */

static portMUX_TYPE s_crit_mux = portMUX_INITIALIZER_UNLOCKED;

void syn_port_enter_critical(void)
{
    portENTER_CRITICAL_SAFE(&s_crit_mux);
}

void syn_port_exit_critical(void)
{
    portEXIT_CRITICAL_SAFE(&s_crit_mux);
}

uint32_t syn_port_get_tick_ms(void)
{
    // Returns millisecond ticks since boot
    return (uint32_t)(esp_timer_get_time() / 1000);
}

uint32_t syn_port_get_tick_us(void)
{
    // Returns microsecond ticks since boot directly from esp_timer hardware
    return (uint32_t)esp_timer_get_time();
}

/**
 * @brief Convert milliseconds to FreeRTOS ticks, clamping non-zero sub-tick durations to 1 tick.
 *
 * @param ms Duration in milliseconds.
 * @return FreeRTOS tick count (at least 1 when ms > 0).
 */
static TickType_t esp32_ms_to_ticks(uint32_t ms)
{
    if (ms == 0U) {
        return 0U;
    }
    TickType_t ticks = pdMS_TO_TICKS(ms);
    return (ticks == 0U) ? (TickType_t)1U : ticks;
}

void syn_port_delay_ms(uint32_t ms)
{
    if (ms == 0U) {
        return;
    }
    vTaskDelay(esp32_ms_to_ticks(ms));
}

SYN_NORETURN void syn_port_system_reset(void)
{
    esp_restart();
    for (;;) {
    }
}

SYN_NORETURN void syn_assert_failed(const char *file, int line)
{
    ESP_LOGE(TAG, "ASSERT FAILED at %s:%d", file, line);
    abort();
}

/* ── Random Port ────────────────────────────────────────────────────────── */

SYN_Status syn_port_random_fill(void *buf, size_t len)
{
    esp_fill_random(buf, len);
    return SYN_OK;
}

/* ── GPIO Port ──────────────────────────────────────────────────────────── */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    if ((uint32_t)pin >= 64U) {
        return SYN_INVALID_PARAM;
    }

    gpio_config_t io_conf = {0};
    io_conf.pin_bit_mask = (1ULL << (uint32_t)pin);
    io_conf.intr_type = GPIO_INTR_DISABLE;

    switch (mode) {
    case SYN_GPIO_INPUT:
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    case SYN_GPIO_OUTPUT:
        io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    case SYN_GPIO_INPUT_PULLUP:
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    case SYN_GPIO_INPUT_PULLDOWN:
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        break;
    case SYN_GPIO_OUTPUT_OD:
        io_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    default:
        return SYN_NOT_IMPLEMENTED;
    }

    esp_err_t err = gpio_config(&io_conf);
    return (err == ESP_OK) ? SYN_OK : SYN_INVALID_PARAM;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    gpio_reset_pin(pin);
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    esp_err_t err = gpio_set_level(pin, (state == SYN_GPIO_HIGH) ? 1 : 0);
    return (err == ESP_OK) ? SYN_OK : SYN_INVALID_PARAM;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    return gpio_get_level(pin) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    int current = gpio_get_level(pin);
    esp_err_t err = gpio_set_level(pin, current ? 0 : 1);
    return (err == ESP_OK) ? SYN_OK : SYN_INVALID_PARAM;
}

/* ── UART Port (weak — override for chips with USB Serial JTAG) ─────── */

#include "soc/soc_caps.h"

static uart_port_t get_uart_port(SYN_UARTInstance instance)
{
    if (instance == 0)
        return UART_NUM_0;
#if SOC_UART_HP_NUM > 1
    if (instance == 1)
        return UART_NUM_1;
#endif
#if SOC_UART_HP_NUM > 2
    if (instance == 2)
        return UART_NUM_2;
#endif
    return UART_NUM_MAX;
}

SYN_WEAK SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX || baudrate == 0U || baudrate > 0x7FFFFFFFU) {
        return SYN_INVALID_PARAM;
    }

    uart_config_t uart_config = {
        .baud_rate = (int)baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if (uart_param_config(port, &uart_config) != ESP_OK)
        return SYN_INVALID_PARAM;
    if (uart_driver_install(port, 256, 256, 0, NULL, 0) != ESP_OK)
        return SYN_INVALID_PARAM;

    /* Use default pin mapping */
    uart_set_pin(port, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    return SYN_OK;
}

SYN_WEAK SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX)
        return SYN_INVALID_PARAM;
    uart_driver_delete(port);
    return SYN_OK;
}

SYN_WEAK SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance, const uint8_t *data,
                                           size_t len, uint32_t timeout_ms)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX || (len > 0U && data == NULL)) {
        return SYN_INVALID_PARAM;
    }
    if (len == 0U) {
        return SYN_OK;
    }

    uint32_t start_ms = syn_port_get_tick_ms();
    size_t offset = 0U;
    while (offset < len) {
        size_t chunk = ((len - offset) > 128U) ? 128U : (len - offset);
        int written = uart_write_bytes(port, (const char *)&data[offset], chunk);
        if (written < 0) {
            return SYN_INVALID_PARAM;
        }
        TickType_t wait_ticks = portMAX_DELAY;
        if (timeout_ms > 0U) {
            uint32_t elapsed = syn_port_get_tick_ms() - start_ms;
            if (elapsed >= timeout_ms) {
                return SYN_TIMEOUT;
            }
            wait_ticks = esp32_ms_to_ticks(timeout_ms - elapsed);
        }
        if (uart_wait_tx_done(port, wait_ticks) != ESP_OK) {
            return SYN_TIMEOUT;
        }
        offset += (size_t)written;
    }
    return SYN_OK;
}

SYN_WEAK SYN_Status syn_port_uart_receive(SYN_UARTInstance instance, uint8_t *data, size_t len,
                                          size_t *received, uint32_t timeout_ms)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX || (len > 0U && data == NULL)) {
        return SYN_INVALID_PARAM;
    }
    if (len == 0U) {
        if (received != NULL) {
            *received = 0U;
        }
        return SYN_OK;
    }

    TickType_t wait_ticks = (timeout_ms == 0U) ? 0U : esp32_ms_to_ticks(timeout_ms);
    int read = uart_read_bytes(port, data, (uint32_t)len, wait_ticks);
    if (read < 0) {
        return SYN_TIMEOUT;
    }

    if (received != NULL) {
        *received = (size_t)read;
    }
    return (read > 0) ? SYN_OK : SYN_TIMEOUT;
}

SYN_WEAK SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    return syn_port_uart_transmit(instance, &byte, 1, 100);
}

SYN_WEAK SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance, uint8_t *byte,
                                               uint32_t timeout_ms)
{
    size_t rec = 0;
    return syn_port_uart_receive(instance, byte, 1, &rec, timeout_ms);
}

/* ── Console serial port (weak — override for USB CDC chips) ──────────── */

#include "syntropic/port/syn_port_serial.h"

#ifndef SYN_SERIAL_UART_INSTANCE
#define SYN_SERIAL_UART_INSTANCE 0
#endif

SYN_WEAK SYN_Status syn_port_serial_init(uint32_t baudrate)
{
    if (baudrate == 0)
        baudrate = 115200;
    return syn_port_uart_init(SYN_SERIAL_UART_INSTANCE, baudrate);
}

SYN_WEAK int syn_port_serial_write(const uint8_t *data, size_t len)
{
    SYN_Status s = syn_port_uart_transmit(SYN_SERIAL_UART_INSTANCE, data, len, 100);
    return (s == SYN_OK) ? (int)len : -1;
}

SYN_WEAK int syn_port_serial_read(uint8_t *buf, size_t max_len)
{
    uart_port_t port = get_uart_port(SYN_SERIAL_UART_INSTANCE);
    if (port == UART_NUM_MAX || buf == NULL) {
        return -1;
    }
    if (max_len == 0U) {
        return 0;
    }
    int n = uart_read_bytes(port, buf, max_len, 0);
    return (n < 0) ? -1 : n;
}

/* ── Hardware Crypto Port (ESP32 AES) ──────────────────────────────────── */

#include "syntropic/port/syn_port_aes.h"
#if defined(SOC_AES_SUPPORTED) || defined(CONFIG_IDF_TARGET_ESP32) ||           \
    defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || \
    defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
#include "aes/esp_aes.h"
#endif

SYN_Status syn_port_aes_init(void)
{
#if defined(SOC_AES_SUPPORTED) || defined(CONFIG_IDF_TARGET_ESP32) ||           \
    defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || \
    defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    return SYN_OK;
#else
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_aes_encrypt_block(const uint8_t *round_keys, uint8_t nr, const uint8_t in[16],
                                      uint8_t out[16])
{
#if defined(SOC_AES_SUPPORTED) || defined(CONFIG_IDF_TARGET_ESP32) ||           \
    defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || \
    defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    if (round_keys == NULL || in == NULL || out == NULL) {
        return SYN_NOT_IMPLEMENTED;
    }
    if (nr != 10U && nr != 12U && nr != 14U) {
        return SYN_INVALID_PARAM;
    }
    esp_aes_context ctx;
    esp_aes_init(&ctx);
    uint32_t key_bits = (nr == 10U) ? 128U : ((nr == 12U) ? 192U : 256U);
    if (esp_aes_setkey_enc(&ctx, round_keys, key_bits) != 0) {
        esp_aes_free(&ctx);
        return SYN_ERROR;
    }
    int res = esp_aes_crypt_ecb(&ctx, ESP_AES_ENCRYPT, in, out);
    esp_aes_free(&ctx);
    return (res == 0) ? SYN_OK : SYN_ERROR;
#else
    (void)round_keys;
    (void)nr;
    (void)in;
    (void)out;
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_aes_decrypt_block(const uint8_t *round_keys, uint8_t nr, const uint8_t in[16],
                                      uint8_t out[16])
{
#if defined(SOC_AES_SUPPORTED) || defined(CONFIG_IDF_TARGET_ESP32) ||           \
    defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || \
    defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
    if (round_keys == NULL || in == NULL || out == NULL) {
        return SYN_NOT_IMPLEMENTED;
    }
    if (nr != 10U && nr != 12U && nr != 14U) {
        return SYN_INVALID_PARAM;
    }
    esp_aes_context ctx;
    esp_aes_init(&ctx);
    uint32_t key_bits = (nr == 10U) ? 128U : ((nr == 12U) ? 192U : 256U);
    if (esp_aes_setkey_dec(&ctx, round_keys, key_bits) != 0) {
        esp_aes_free(&ctx);
        return SYN_ERROR;
    }
    int res = esp_aes_crypt_ecb(&ctx, ESP_AES_DECRYPT, in, out);
    esp_aes_free(&ctx);
    return (res == 0) ? SYN_OK : SYN_ERROR;
#else
    (void)round_keys;
    (void)nr;
    (void)in;
    (void)out;
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_ghash_mult(const uint8_t x[16], const uint8_t h[16], uint8_t out[16])
{
    (void)x;
    (void)h;
    (void)out;
    return SYN_NOT_IMPLEMENTED;
}

#endif /* ESP_PLATFORM && !ARDUINO */
