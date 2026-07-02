

# File port\_esp32.c

[**File List**](files.md) **>** [**esp32**](dir_5fe4984ab7fe7841faedbf54ff0b3c4c.md) **>** [**port\_esp32.c**](port__esp32_8c.md)

[Go to the documentation of this file](port__esp32_8c.md)


```C++

#if defined(ESP_PLATFORM) && !defined(ARDUINO)

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/uart.h"

/* ── System Port ────────────────────────────────────────────────────────── */

static portMUX_TYPE critical_mux = portMUX_INITIALIZER_UNLOCKED;

void syn_port_enter_critical(void)
{
    portENTER_CRITICAL(&critical_mux);
}

void syn_port_exit_critical(void)
{
    portEXIT_CRITICAL(&critical_mux);
}

uint32_t syn_port_get_tick_ms(void)
{
    // Returns millisecond ticks since boot
    return (uint32_t)(esp_timer_get_time() / 1000);
}

void syn_port_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void syn_port_system_reset(void)
{
    esp_restart();
    for (;;);
}

/* ── GPIO Port ──────────────────────────────────────────────────────────── */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.intr_type = GPIO_INTR_DISABLE;

    switch (mode) {
        case SYN_GPIO_INPUT:
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case SYN_GPIO_OUTPUT:
            io_conf.mode = GPIO_MODE_OUTPUT;
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
            io_conf.mode = GPIO_MODE_OUTPUT_OD;
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

/* ── UART Port ──────────────────────────────────────────────────────────── */

static uart_port_t get_uart_port(SYN_UARTInstance instance)
{
    if (instance == 0) return UART_NUM_0;
    if (instance == 1) return UART_NUM_1;
    if (instance == 2) return UART_NUM_2;
    return UART_NUM_MAX;
}

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX) return SYN_INVALID_PARAM;

    uart_config_t uart_config = {
        .baud_rate = (int)baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    if (uart_param_config(port, &uart_config) != ESP_OK) return SYN_INVALID_PARAM;
    // Install driver with standard RX/TX buffers (no event queue)
    if (uart_driver_install(port, 256, 256, 0, NULL, 0) != ESP_OK) return SYN_INVALID_PARAM;

    // Use default pin mapping for the port
    if (port == UART_NUM_0) {
        uart_set_pin(port, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    } else if (port == UART_NUM_1) {
        uart_set_pin(port, 10, 9, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); // Standard pins
    } else if (port == UART_NUM_2) {
        uart_set_pin(port, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); // Standard pins
    }

    return SYN_OK;
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX) return SYN_INVALID_PARAM;
    uart_driver_delete(port);
    return SYN_OK;
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                  const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX) return SYN_INVALID_PARAM;

    int written = uart_write_bytes(port, (const char *)data, len);
    if (written < 0) return SYN_INVALID_PARAM;

    // Wait for transmit completion
    esp_err_t err = uart_wait_tx_done(port, timeout_ms == 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
    return (err == ESP_OK) ? SYN_OK : SYN_TIMEOUT;
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                 uint8_t *data,
                                 size_t len,
                                 size_t *received,
                                 uint32_t timeout_ms)
{
    uart_port_t port = get_uart_port(instance);
    if (port == UART_NUM_MAX) return SYN_INVALID_PARAM;

    int read = uart_read_bytes(port, data, len, timeout_ms == 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
    if (read < 0) return SYN_TIMEOUT;

    if (received) *received = (size_t)read;
    return (read > 0) ? SYN_OK : SYN_TIMEOUT;
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    return syn_port_uart_transmit(instance, &byte, 1, 100);
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance, uint8_t *byte, uint32_t timeout_ms)
{
    size_t rec = 0;
    return syn_port_uart_receive(instance, byte, 1, &rec, timeout_ms);
}

#endif /* ESP_PLATFORM && !ARDUINO */
```


