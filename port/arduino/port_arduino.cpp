/**
 * @file port_arduino.cpp
 * @brief SyntropicOS port layer for the Arduino C++ SDK.
 *
 * Implements the system, GPIO, and UART interfaces by wrapping standard
 * Arduino C++ SDK functions. Compatible with AVR, SAMD, and other Arduino boards.
 */

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/port/syn_port_adc.h"

#include <Arduino.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/wdt.h>
#endif

/* ── System Port ────────────────────────────────────────────────────────── */

static uint32_t critical_nesting = 0;
static uint32_t critical_saved_state = 0;

void syn_port_enter_critical(void)
{
    if (critical_nesting == 0) {
#if defined(ARDUINO_ARCH_AVR)
        /* Save SREG (which contains the global interrupt enable bit I) */
        critical_saved_state = SREG;
        noInterrupts();
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM) || defined(CORE_TEENSY)
        /* Save PRIMASK (bit 0 = 1 means interrupts already disabled) */
        critical_saved_state = __get_PRIMASK();
        __disable_irq();
#else
        critical_saved_state = 0;
        noInterrupts();
#endif
    }
    critical_nesting++;
}

void syn_port_exit_critical(void)
{
    if (critical_nesting > 0) {
        critical_nesting--;
        if (critical_nesting == 0) {
#if defined(ARDUINO_ARCH_AVR)
            /* Restore SREG — only re-enables interrupts if they were on before */
            SREG = (uint8_t)critical_saved_state;
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM) || defined(CORE_TEENSY)
            /* Only re-enable if PRIMASK showed interrupts were enabled on entry */
            if ((critical_saved_state & 1U) == 0) {
                __enable_irq();
            }
#else
            interrupts();
#endif
        }
    }
}

uint32_t syn_port_get_tick_ms(void)
{
    return millis();
}

void syn_port_delay_ms(uint32_t ms)
{
    delay(ms);
}

void syn_port_system_reset(void)
{
#if defined(ARDUINO_ARCH_AVR)
    // Setup watchdog for a short reset timeout, then loop forever
    wdt_enable(WDTO_15MS);
    for (;;);
#elif defined(ARDUINO_ARCH_SAMD)
    NVIC_SystemReset();
#else
    // Fallback: jump to reset vector (address 0)
    ((void (*)(void))0)();
    for (;;);
#endif
}

/* ── GPIO Port ──────────────────────────────────────────────────────────── */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    switch (mode) {
        case SYN_GPIO_INPUT:
            pinMode(pin, INPUT);
            break;
        case SYN_GPIO_OUTPUT:
            pinMode(pin, OUTPUT);
            break;
        case SYN_GPIO_INPUT_PULLUP:
            pinMode(pin, INPUT_PULLUP);
            break;
#if defined(INPUT_PULLDOWN)
        case SYN_GPIO_INPUT_PULLDOWN:
            pinMode(pin, INPUT_PULLDOWN);
            break;
#endif
        default:
            return SYN_NOT_IMPLEMENTED;
    }
    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    pinMode(pin, INPUT);
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    digitalWrite(pin, (state == SYN_GPIO_HIGH) ? HIGH : LOW);
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    return (digitalRead(pin) == HIGH) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    int current = digitalRead(pin);
    digitalWrite(pin, (current == HIGH) ? LOW : HIGH);
    return SYN_OK;
}

/* ── UART Port ──────────────────────────────────────────────────────────── */

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY)
static HardwareSerial* get_serial_instance(SYN_UARTInstance instance)
{
    if (instance == 0) return &Serial;
#if defined(UBRR1H) || defined(ARDUINO_ARCH_SAMD)
    else if (instance == 1) return &Serial1;
#endif
    return NULL;
}
#endif

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY)
    HardwareSerial* serial = get_serial_instance(instance);
    if (!serial) return SYN_INVALID_PARAM;
    serial->begin(baudrate);
    return SYN_OK;
#else
    if (instance == 0) {
        Serial.begin(baudrate);
        return SYN_OK;
    }
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY)
    HardwareSerial* serial = get_serial_instance(instance);
    if (!serial) return SYN_INVALID_PARAM;
    serial->end();
    return SYN_OK;
#else
    if (instance == 0) {
        Serial.end();
        return SYN_OK;
    }
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                  const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms)
{
    (void)timeout_ms; // Timeout handled synchronously by serial block write
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY)
    HardwareSerial* serial = get_serial_instance(instance);
    if (!serial) return SYN_INVALID_PARAM;
    serial->write(data, len);
    return SYN_OK;
#else
    if (instance != 0) return SYN_INVALID_PARAM;
    Serial.write(data, len);
    return SYN_OK;
#endif
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                 uint8_t *data,
                                 size_t len,
                                 size_t *received,
                                 uint32_t timeout_ms)
{
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY)
    HardwareSerial* serial = get_serial_instance(instance);
    if (!serial) return SYN_INVALID_PARAM;
    
    serial->setTimeout(timeout_ms == 0 ? 1000 : timeout_ms);
    size_t count = serial->readBytes(data, len);
    if (received) *received = count;
    
    return (count > 0) ? SYN_OK : SYN_TIMEOUT;
#else
    if (instance != 0) return SYN_INVALID_PARAM;
    Serial.setTimeout(timeout_ms == 0 ? 1000 : timeout_ms);
    size_t count = Serial.readBytes(data, len);
    if (received) *received = count;
    return (count > 0) ? SYN_OK : SYN_TIMEOUT;
#endif
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    return syn_port_uart_transmit(instance, &byte, 1, 0);
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance, uint8_t *byte, uint32_t timeout_ms)
{
    size_t rec = 0;
    return syn_port_uart_receive(instance, byte, 1, &rec, timeout_ms);
}

/* ── ADC Port ───────────────────────────────────────────────────────────── */

SYN_Status syn_port_adc_init(uint8_t channel)
{
    if (channel < 6) {
        pinMode(A0 + channel, INPUT);
    }
    return SYN_OK;
}

uint16_t syn_port_adc_read(uint8_t channel)
{
    return analogRead(channel);
}

uint8_t syn_port_adc_resolution(void)
{
    return 10;
}

uint16_t syn_port_adc_reference_mv(void)
{
    return 5000;
}
