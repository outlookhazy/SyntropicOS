/**
 * @file port_rp2040.c
 * @brief SyntropicOS port layer for Raspberry Pi RP2040/RP2350 (Pico SDK).
 *
 * Implements the system, GPIO, and UART interfaces by wrapping the official
 * Raspberry Pi Pico SDK peripheral APIs.
 */

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_uart.h"

#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"

/* ── System Port ────────────────────────────────────────────────────────── */

static uint32_t critical_status = 0;
static uint32_t critical_nesting = 0;

void syn_port_enter_critical(void)
{
    uint32_t status = save_and_disable_interrupts();
    if (critical_nesting == 0) {
        critical_status = status;
    }
    critical_nesting++;
}

void syn_port_exit_critical(void)
{
    if (critical_nesting > 0) {
        critical_nesting--;
        if (critical_nesting == 0) {
            restore_interrupts(critical_status);
        }
    }
}

uint32_t syn_port_get_tick_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

void syn_port_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

void syn_port_system_reset(void)
{
    // Reboot immediately using watchdog register control
    watchdog_reboot(0, 0, 0);
    for (;;);
}

/* ── GPIO Port ──────────────────────────────────────────────────────────── */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    gpio_init(pin);
    switch (mode) {
        case SYN_GPIO_INPUT:
            gpio_set_dir(pin, GPIO_IN);
            break;
        case SYN_GPIO_OUTPUT:
            gpio_set_dir(pin, GPIO_OUT);
            break;
        case SYN_GPIO_INPUT_PULLUP:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;
        case SYN_GPIO_INPUT_PULLDOWN:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
            break;
        default:
            return SYN_NOT_IMPLEMENTED;
    }
    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    gpio_deinit(pin);
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    gpio_put(pin, (state == SYN_GPIO_HIGH) ? 1 : 0);
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    return gpio_get(pin) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    gpio_put(pin, !gpio_get(pin));
    return SYN_OK;
}

/* ── UART Port ──────────────────────────────────────────────────────────── */

static uart_inst_t* get_uart_instance(SYN_UARTInstance instance)
{
    if (instance == 0) return uart0;
    if (instance == 1) return uart1;
    return NULL;
}

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    uart_inst_t* uart = get_uart_instance(instance);
    if (!uart) return SYN_INVALID_PARAM;
    
    uart_init(uart, baudrate);
    
    // Set standard GPIO function pins for the UART instance
    if (uart == uart0) {
        gpio_set_function(0, GPIO_FUNC_UART); // TX
        gpio_set_function(1, GPIO_FUNC_UART); // RX
    } else {
        gpio_set_function(8, GPIO_FUNC_UART); // TX
        gpio_set_function(9, GPIO_FUNC_UART); // RX
    }
    
    return SYN_OK;
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    uart_inst_t* uart = get_uart_instance(instance);
    if (!uart) return SYN_INVALID_PARAM;
    uart_deinit(uart);
    return SYN_OK;
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                  const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms)
{
    uart_inst_t* uart = get_uart_instance(instance);
    if (!uart) return SYN_INVALID_PARAM;
    (void)timeout_ms; // Timeout ignored (SDK blocking transmit does not timeout)

    uart_write_blocking(uart, data, len);
    return SYN_OK;
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                 uint8_t *data,
                                 size_t len,
                                 size_t *received,
                                 uint32_t timeout_ms)
{
    uart_inst_t* uart = get_uart_instance(instance);
    if (!uart) return SYN_INVALID_PARAM;

    size_t count = 0;
    uint32_t start_ms = syn_port_get_tick_ms();

    while (count < len) {
        if (uart_is_readable(uart)) {
            data[count++] = uart_getc(uart);
        } else {
            if (timeout_ms > 0 && (syn_port_get_tick_ms() - start_ms) >= timeout_ms) {
                break;
            }
            // Yield CPU slightly
            best_effort_wfe_or_timeout(make_timeout_time_us(100));
        }
    }

    if (received) *received = count;
    return (count > 0) ? SYN_OK : SYN_TIMEOUT;
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    uart_inst_t* uart = get_uart_instance(instance);
    if (!uart) return SYN_INVALID_PARAM;
    uart_putc(uart, (char)byte);
    return SYN_OK;
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance, uint8_t *byte, uint32_t timeout_ms)
{
    uart_inst_t* uart = get_uart_instance(instance);
    if (!uart) return SYN_INVALID_PARAM;

    uint32_t start_ms = syn_port_get_tick_ms();
    while (!uart_is_readable(uart)) {
        if (timeout_ms > 0 && (syn_port_get_tick_ms() - start_ms) >= timeout_ms) {
            return SYN_TIMEOUT;
        }
    }
    *byte = (uint8_t)uart_getc(uart);
    return SYN_OK;
}
