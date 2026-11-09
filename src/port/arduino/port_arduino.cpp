/**
 * @file port_arduino.cpp
 * @brief SyntropicOS port layer for the Arduino C++ SDK.
 *
 * Implements the system, GPIO, and UART interfaces by wrapping standard
 * Arduino C++ SDK functions. Compatible with AVR, SAMD, and other Arduino boards.
 */

#if defined(ARDUINO)

#include "syntropic/common/syn_compiler.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/port/syn_port_adc.h"
#include "syntropic/drivers/syn_rtc.h"
#include "syntropic/util/syn_assert.h"

#include <Arduino.h>

#ifndef __placement_new_implicitly_defined
  inline void* operator new(size_t, void* ptr) { return ptr; }
#endif

#if defined(ARDUINO_ARCH_AVR)
#include <avr/wdt.h>
#elif defined(ARDUINO_ARCH_RP2040)
#include "hardware/sync.h"
#include "hardware/watchdog.h"
#include "pico/platform.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

/* ── System Port ────────────────────────────────────────────────────────── */

#if defined(ARDUINO_ARCH_ESP32)
static portMUX_TYPE s_crit_mux = portMUX_INITIALIZER_UNLOCKED;
#elif defined(ARDUINO_ARCH_RP2040)
static volatile uint32_t s_crit_depth[2] = {0U, 0U};
static volatile uint32_t s_saved_irq[2] = {0U, 0U};
#else
static volatile uint32_t s_crit_depth = 0U;
static volatile uint32_t s_saved_irq = 0U;
#endif

extern "C" {

SYN_WEAK SYN_NORETURN void syn_assert_failed(const char *file, int line)
{
    Serial.print(F("\r\n[ASSERT FAILED] "));
    Serial.print(file);
    Serial.print(F(":"));
    Serial.println(line);
    Serial.flush();
    delay(100);
    syn_port_system_reset();
}

void syn_port_enter_critical(void)
{
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL_SAFE(&s_crit_mux);
#elif defined(ARDUINO_ARCH_RP2040)
    uint32_t status = save_and_disable_interrupts();
    uint32_t core = get_core_num();
    if (s_crit_depth[core] == 0U) {
        s_saved_irq[core] = status;
    }
    s_crit_depth[core]++;
#elif defined(ARDUINO_ARCH_AVR) || defined(__AVR__)
    uint8_t sreg = SREG;
    noInterrupts();
    if (s_crit_depth == 0U) {
        s_saved_irq = sreg;
    }
    s_crit_depth++;
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM) || defined(CORE_TEENSY) || \
    defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_ARCH_MBED) || \
    defined(ARDUINO_ARCH_RENESAS) || defined(__ARM_ARCH_ISA_THUMB) || defined(__arm__) || \
    defined(__thumb__)
    uint32_t primask = 0U;
    __asm__ volatile("mrs %0, primask" : "=r"(primask));
    __asm__ volatile("cpsid i" ::: "memory");
    if (s_crit_depth == 0U) {
        s_saved_irq = primask;
    }
    s_crit_depth++;
#elif defined(__riscv)
    uint32_t mstatus = 0U;
    __asm__ volatile("csrrci %0, mstatus, 8" : "=r"(mstatus) :: "memory");
    if (s_crit_depth == 0U) {
        s_saved_irq = mstatus;
    }
    s_crit_depth++;
#elif defined(ARDUINO_ARCH_ESP8266) || defined(__XTENSA__)
    uint32_t ps = xt_rsil(15);
    if (s_crit_depth == 0U) {
        s_saved_irq = ps;
    }
    s_crit_depth++;
#else
    noInterrupts();
    s_crit_depth++;
#endif
}

void syn_port_exit_critical(void)
{
    SYN_COMPILER_BARRIER();
#if defined(ARDUINO_ARCH_ESP32)
    portEXIT_CRITICAL_SAFE(&s_crit_mux);
#elif defined(ARDUINO_ARCH_RP2040)
    uint32_t core = get_core_num();
    if (s_crit_depth[core] > 0U) {
        s_crit_depth[core]--;
        if (s_crit_depth[core] == 0U) {
            restore_interrupts(s_saved_irq[core]);
        }
    }
#elif defined(ARDUINO_ARCH_AVR) || defined(__AVR__)
    if (s_crit_depth > 0U) {
        s_crit_depth--;
        if (s_crit_depth == 0U && (s_saved_irq & 0x80U) != 0U) {
            interrupts();
        }
    }
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM) || defined(CORE_TEENSY) || \
    defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_NRF52) || defined(ARDUINO_ARCH_MBED) || \
    defined(ARDUINO_ARCH_RENESAS) || defined(__ARM_ARCH_ISA_THUMB) || defined(__arm__) || \
    defined(__thumb__)
    if (s_crit_depth > 0U) {
        s_crit_depth--;
        if (s_crit_depth == 0U && (s_saved_irq & 1U) == 0U) {
            __asm__ volatile("cpsie i" ::: "memory");
        }
    }
#elif defined(__riscv)
    __asm__ volatile("" ::: "memory");
    if (s_crit_depth > 0U) {
        s_crit_depth--;
        if (s_crit_depth == 0U && (s_saved_irq & 0x8U) != 0U) {
            __asm__ volatile("csrsi mstatus, 8" ::: "memory");
        }
    }
#elif defined(ARDUINO_ARCH_ESP8266) || defined(__XTENSA__)
    if (s_crit_depth > 0U) {
        s_crit_depth--;
        if (s_crit_depth == 0U) {
            xt_wsr_ps(s_saved_irq);
        }
    }
#else
    (void)s_saved_irq;
    if (s_crit_depth > 0U) {
        s_crit_depth--;
        if (s_crit_depth == 0U) {
            interrupts();
        }
    }
#endif
}

uint32_t syn_port_get_tick_ms(void)
{
    return millis();
}

uint32_t syn_port_get_tick_us(void)
{
    return micros();
}

void syn_port_delay_ms(uint32_t ms)
{
    delay(ms);
}

SYN_NORETURN void syn_port_system_reset(void)
{
#if defined(ARDUINO_ARCH_AVR)
    wdt_enable(WDTO_15MS);
    for (;;) {
    }
#elif defined(ARDUINO_ARCH_RP2040)
    watchdog_reboot(0, 0, 0);
    for (;;) {
    }
#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    ESP.restart();
    for (;;) {
    }
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_NRF52)
    NVIC_SystemReset();
    for (;;) {
    }
#else
    ((void (*)(void))0)();
    for (;;) {
    }
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

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_RP2040)
static HardwareSerial* get_serial_instance(SYN_UARTInstance instance)
{
    if (instance == 0) return &Serial;
#if defined(UBRR1H) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_RP2040)
    else if (instance == 1) return &Serial1;
#endif
#if defined(ARDUINO_ARCH_RP2040)
    else if (instance == 2) return &Serial2;
#endif
    return NULL;
}
#endif

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_RP2040)
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
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_RP2040)
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
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_RP2040)
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
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_RP2040)
    HardwareSerial* serial = get_serial_instance(instance);
    if (!serial) return SYN_INVALID_PARAM;
    if (serial->available() == 0 && timeout_ms == 0) {
        if (received) *received = 0;
        return SYN_TIMEOUT;
    }
    serial->setTimeout(timeout_ms);
    size_t count = serial->readBytes(data, len);
    if (received) *received = count;
    
    return (count > 0) ? SYN_OK : SYN_TIMEOUT;
#else
    if (instance != 0) return SYN_INVALID_PARAM;
    if (Serial.available() == 0 && timeout_ms == 0) {
        if (received) *received = 0;
        return SYN_TIMEOUT;
    }
    Serial.setTimeout(timeout_ms);
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

SYN_Status syn_port_adc_init(uint8_t adc_id, uint32_t channel_mask)
{
    if (adc_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
#if defined(ARDUINO_ARCH_RP2040)
    for (uint8_t ch = 0U; ch <= 3U; ch++) {
        if ((channel_mask & (1UL << ch)) != 0UL) {
            pinMode(26U + ch, INPUT);
        }
    }
#elif defined(A0)
    for (uint8_t ch = 0U; ch < 6U; ch++) {
        if ((channel_mask & (1UL << ch)) != 0UL) {
            pinMode(A0 + ch, INPUT);
        }
    }
#else
    (void)channel_mask;
#endif
    return SYN_OK;
}

SYN_Status syn_port_adc_deinit(uint8_t adc_id)
{
    if (adc_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
    return SYN_OK;
}

uint16_t syn_port_adc_read_channel(uint8_t adc_id, uint8_t channel)
{
    if (adc_id != 0U) {
        return 0U;
    }
#if defined(ARDUINO_ARCH_RP2040)
    if (channel <= 3U) {
        return (uint16_t)analogRead(26U + channel);
    }
    if (channel == 4U) {
        return (uint16_t)analogReadTemp();
    }
    return 0U;
#elif defined(A0)
    return (uint16_t)analogRead(A0 + channel);
#else
    return (uint16_t)analogRead(channel);
#endif
}

SYN_Status syn_port_adc_start_dma_scan(uint8_t adc_id, uint16_t *dest, size_t num_channels)
{
    if (adc_id != 0U || dest == NULL || num_channels == 0U) {
        return SYN_INVALID_PARAM;
    }
    return SYN_NOT_IMPLEMENTED;
}

} /* extern "C" */

/* ── PWM Port ───────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_pwm.h"

extern "C" {

/*
 * Arduino analogWrite() is 8-bit (0–255). The 16-bit raw duty interface
 * maps down to 8 bits. Frequency control is not available through the
 * standard Arduino SDK on most boards.
 */

#ifndef SYN_ARDUINO_PWM_MAX_CHANNELS
  #define SYN_ARDUINO_PWM_MAX_CHANNELS  12
#endif

static struct {
    uint8_t  pin;
    uint8_t  duty;      /* 0–255 */
    bool     enabled;
} pwm_channels[SYN_ARDUINO_PWM_MAX_CHANNELS];

SYN_Status syn_port_pwm_init(uint8_t channel, uint32_t freq_hz)
{
    (void)freq_hz;  /* Arduino SDK does not expose frequency control */
    if (channel >= SYN_ARDUINO_PWM_MAX_CHANNELS) return SYN_INVALID_PARAM;
    pinMode(channel, OUTPUT);
    pwm_channels[channel].pin     = channel;
    pwm_channels[channel].duty    = 0;
    pwm_channels[channel].enabled = true;
    analogWrite(channel, 0);
    return SYN_OK;
}

void syn_port_pwm_set_duty(uint8_t channel, uint8_t duty_pct)
{
    if (channel >= SYN_ARDUINO_PWM_MAX_CHANNELS) return;
    uint8_t val = (uint8_t)((uint16_t)duty_pct * 255 / 100);
    pwm_channels[channel].duty = val;
    if (pwm_channels[channel].enabled) {
        analogWrite(channel, val);
    }
}

void syn_port_pwm_set_duty_raw(uint8_t channel, uint16_t duty_u16)
{
    if (channel >= SYN_ARDUINO_PWM_MAX_CHANNELS) return;
    uint8_t val = (uint8_t)(duty_u16 >> 8);  /* Map 0–65535 → 0–255 */
    pwm_channels[channel].duty = val;
    if (pwm_channels[channel].enabled) {
        analogWrite(channel, val);
    }
}

void syn_port_pwm_enable(uint8_t channel, bool enable)
{
    if (channel >= SYN_ARDUINO_PWM_MAX_CHANNELS) return;
    pwm_channels[channel].enabled = enable;
    if (enable) {
        analogWrite(channel, pwm_channels[channel].duty);
    } else {
        analogWrite(channel, 0);
    }
}

void syn_port_pwm_set_freq(uint8_t channel, uint32_t freq_hz)
{
    /* Not available through standard Arduino SDK */
    (void)channel;
    (void)freq_hz;
}

} /* extern "C" */

/* ── I2C Port ───────────────────────────────────────────────────────────── */

#if __has_include(<Wire.h>)

#include <Wire.h>
#include "syntropic/port/syn_port_i2c.h"

extern "C" {

SYN_Status syn_port_i2c_init(uint8_t i2c_id, uint32_t clock_speed_hz, uint8_t role,
                             uint16_t own_addr)
{
    if (i2c_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
    if (clock_speed_hz == 0U) {
        return SYN_INVALID_PARAM;
    }
    if (role == 0U) {
        (void)own_addr;
        Wire.begin();
        Wire.setClock(clock_speed_hz);
    } else {
        Wire.begin((uint8_t)(own_addr & 0x7FU));
    }
    return SYN_OK;
}

SYN_Status syn_port_i2c_deinit(uint8_t i2c_id)
{
    if (i2c_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP32) || \
    defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_MBED) || \
    defined(ARDUINO_ARCH_RP2040)
    Wire.end();
#endif
    return SYN_OK;
}

SYN_Status syn_port_i2c_transfer(uint8_t i2c_id, uint16_t addr, const uint8_t *tx, size_t tx_len,
                                 uint8_t *rx, size_t rx_len)
{
    if (i2c_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
    if (addr > 0x7FU || rx_len > 255U || (tx_len > 0U && tx == NULL) ||
        (rx_len > 0U && rx == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t addr7 = (uint8_t)addr;
    if (tx_len > 0U || rx_len == 0U) {
        Wire.beginTransmission(addr7);
        if (tx_len > 0U) {
            Wire.write(tx, tx_len);
        }
        bool send_stop = (rx_len == 0U);
        uint8_t result = Wire.endTransmission(send_stop);
        if (result != 0U) {
            return SYN_ERROR;
        }
    }

    if (rx_len > 0U) {
        size_t received = (size_t)Wire.requestFrom(addr7, (uint8_t)rx_len);
        for (size_t i = 0U; i < received && i < rx_len; i++) {
            rx[i] = (uint8_t)Wire.read();
        }
        if (received != rx_len) {
            return SYN_ERROR;
        }
    }

    return SYN_OK;
}

} /* extern "C" */

#endif /* Wire.h */

/* ── SPI Port ───────────────────────────────────────────────────────────── */

#if __has_include(<SPI.h>)

#include <SPI.h>
#include "syntropic/port/syn_port_spi.h"

static SPISettings spi_settings;

extern "C" {

SYN_Status syn_port_spi_init(uint8_t spi_id, uint32_t baudrate_hz, uint8_t mode, uint8_t role)
{
    if (spi_id != 0U || role != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
    if (baudrate_hz == 0U || mode > 3U) {
        return SYN_INVALID_PARAM;
    }

    uint8_t spi_mode;
    switch (mode) {
        case 1U: spi_mode = SPI_MODE1; break;
        case 2U: spi_mode = SPI_MODE2; break;
        case 3U: spi_mode = SPI_MODE3; break;
        default: spi_mode = SPI_MODE0; break;
    }

    spi_settings = SPISettings(baudrate_hz, MSBFIRST, spi_mode);
    SPI.begin();
    return SYN_OK;
}

SYN_Status syn_port_spi_deinit(uint8_t spi_id)
{
    if (spi_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
    SPI.end();
    return SYN_OK;
}

SYN_Status syn_port_spi_transfer(uint8_t spi_id, const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (spi_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }
    if (len > 0U && tx == NULL && rx == NULL) {
        return SYN_INVALID_PARAM;
    }
    SPI.beginTransaction(spi_settings);
    for (size_t i = 0U; i < len; i++) {
        uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFFU;
        uint8_t rx_byte = SPI.transfer(tx_byte);
        if (rx != NULL) {
            rx[i] = rx_byte;
        }
    }
    SPI.endTransaction();
    return SYN_OK;
}

} /* extern "C" */

#endif /* SPI.h */

/* ── EXTI Port (GPIO Interrupts) ────────────────────────────────────────── */

#if defined(digitalPinToInterrupt)

#include "syntropic/port/syn_port_exti.h"
#include "syntropic/drivers/syn_exti.h"

extern "C" {

/*
 * Arduino's attachInterrupt() requires a plain function pointer — no context
 * parameter. We use a small table of static trampoline functions, each bound
 * to a pin number.  Up to SYN_ARDUINO_EXTI_SLOTS pins can have interrupts.
 */

#ifndef SYN_ARDUINO_EXTI_SLOTS
  #define SYN_ARDUINO_EXTI_SLOTS  6
#endif

static SYN_GPIO_Pin exti_pins[SYN_ARDUINO_EXTI_SLOTS];
static uint8_t      exti_modes[SYN_ARDUINO_EXTI_SLOTS];
#if defined(ARDUINO_ARCH_RP2040)
/* arduino-pico uses PinStatus enum instead of uint8_t for interrupt modes */
static PinStatus exti_pin_status[SYN_ARDUINO_EXTI_SLOTS];
#endif
static uint8_t      exti_slot_count = 0;

/* Generate trampoline ISRs — each calls syn_exti_irq_handler with its pin */
#define EXTI_TRAMPOLINE(n) \
    static void exti_isr_##n(void) { syn_exti_irq_handler(exti_pins[n]); }

EXTI_TRAMPOLINE(0)
EXTI_TRAMPOLINE(1)
EXTI_TRAMPOLINE(2)
EXTI_TRAMPOLINE(3)
EXTI_TRAMPOLINE(4)
EXTI_TRAMPOLINE(5)

typedef void (*ISR_Func)(void);
static const ISR_Func exti_trampolines[SYN_ARDUINO_EXTI_SLOTS] = {
    exti_isr_0, exti_isr_1, exti_isr_2,
    exti_isr_3, exti_isr_4, exti_isr_5,
};

static int8_t exti_find_slot(SYN_GPIO_Pin pin)
{
    for (uint8_t i = 0; i < exti_slot_count; i++) {
        if (exti_pins[i] == pin) return (int8_t)i;
    }
    return -1;
}

SYN_Status syn_port_exti_configure(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge)
{
    int8_t slot = exti_find_slot(pin);
    if (slot < 0) {
        if (exti_slot_count >= SYN_ARDUINO_EXTI_SLOTS) return SYN_ERROR;
        slot = (int8_t)exti_slot_count++;
        exti_pins[slot] = pin;
    }

    /* Verify the pin supports hardware interrupts */
    if (digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT) {
        return SYN_NOT_IMPLEMENTED;
    }

    switch (edge) {
        case SYN_EXTI_RISING:  exti_modes[slot] = RISING;  break;
        case SYN_EXTI_FALLING: exti_modes[slot] = FALLING; break;
        case SYN_EXTI_BOTH:    exti_modes[slot] = CHANGE;  break;
        default:               exti_modes[slot] = CHANGE;  break;
    }
#if defined(ARDUINO_ARCH_RP2040)
    exti_pin_status[slot] = (PinStatus)exti_modes[slot];
#endif

    /* Configure pin as input with pull-up */
    pinMode(pin, INPUT_PULLUP);
    return SYN_OK;
}

void syn_port_exti_enable(SYN_GPIO_Pin pin)
{
    int8_t slot = exti_find_slot(pin);
    if (slot < 0) return;
    attachInterrupt(digitalPinToInterrupt(pin),
                    exti_trampolines[slot],
#if defined(ARDUINO_ARCH_RP2040)
                    exti_pin_status[slot]);
#else
                    exti_modes[slot]);
#endif
}

void syn_port_exti_disable(SYN_GPIO_Pin pin)
{
    if (digitalPinToInterrupt(pin) == NOT_AN_INTERRUPT) return;
    detachInterrupt(digitalPinToInterrupt(pin));
}

void syn_port_exti_clear_pending(SYN_GPIO_Pin pin)
{
    /* Arduino SDK does not expose pending interrupt flags */
    (void)pin;
}

} /* extern "C" */

#endif /* digitalPinToInterrupt */

/* ── Hardware Watchdog Port ─────────────────────────────────────────────── */

#include "syntropic/port/syn_port_wdt.h"
#if defined(ARDUINO_ARCH_AVR)
#include <avr/wdt.h>
#endif

extern "C" {

#if defined(ARDUINO_ARCH_AVR)

SYN_Status syn_port_wdt_init(uint32_t timeout_ms)
{
    uint8_t wdt_val;
    if      (timeout_ms <= 15)   wdt_val = WDTO_15MS;
    else if (timeout_ms <= 30)   wdt_val = WDTO_30MS;
    else if (timeout_ms <= 60)   wdt_val = WDTO_60MS;
    else if (timeout_ms <= 120)  wdt_val = WDTO_120MS;
    else if (timeout_ms <= 250)  wdt_val = WDTO_250MS;
    else if (timeout_ms <= 500)  wdt_val = WDTO_500MS;
    else if (timeout_ms <= 1000) wdt_val = WDTO_1S;
    else if (timeout_ms <= 2000) wdt_val = WDTO_2S;
#if defined(WDTO_4S)
    else if (timeout_ms <= 4000) wdt_val = WDTO_4S;
#endif
#if defined(WDTO_8S)
    else                         wdt_val = WDTO_8S;
#endif
#if !defined(WDTO_4S)
    else                         wdt_val = WDTO_2S;
#endif
    wdt_enable(wdt_val);
    return SYN_OK;
}

void syn_port_wdt_feed(void)
{
    wdt_reset();
}

#elif defined(ARDUINO_ARCH_MEGAAVR)

/*
 * megaAVR (ATmega4809 / Nano Every) uses a different WDT register layout.
 * The WDT is controlled via WDT.CTRLA with a period bitfield.
 */

SYN_Status syn_port_wdt_init(uint32_t timeout_ms)
{
    uint8_t period;
    if      (timeout_ms <= 8)    period = 0x01;  /* 8ms    */
    else if (timeout_ms <= 16)   period = 0x02;  /* 16ms   */
    else if (timeout_ms <= 31)   period = 0x03;  /* 32ms   */
    else if (timeout_ms <= 63)   period = 0x04;  /* 64ms   */
    else if (timeout_ms <= 125)  period = 0x05;  /* 128ms  */
    else if (timeout_ms <= 250)  period = 0x06;  /* 256ms  */
    else if (timeout_ms <= 500)  period = 0x07;  /* 512ms  */
    else if (timeout_ms <= 1000) period = 0x08;  /* 1024ms */
    else if (timeout_ms <= 2000) period = 0x09;  /* 2048ms */
    else if (timeout_ms <= 4000) period = 0x0A;  /* 4096ms */
    else                         period = 0x0B;  /* 8192ms */

    /* CCP unlock + set period in WDT.CTRLA */
    syn_port_enter_critical();
    _PROTECTED_WRITE(WDT.CTRLA, period);
    syn_port_exit_critical();
    return SYN_OK;
}

void syn_port_wdt_feed(void)
{
    __asm__ __volatile__("wdr");
}

#elif defined(ARDUINO_ARCH_RP2040)

/*
 * RP2040 hardware watchdog — supports timeouts up to ~8.3 seconds.
 * The Pico SDK watchdog_enable() starts the countdown; watchdog_update()
 * (aliased to "feed") resets it.
 */

SYN_Status syn_port_wdt_init(uint32_t timeout_ms)
{
    /* Clamp to RP2040 max (~8300ms). Second arg: pause_on_debug. */
    if (timeout_ms > 8300) timeout_ms = 8300;
    watchdog_enable(timeout_ms, true);
    return SYN_OK;
}

void syn_port_wdt_feed(void)
{
    watchdog_update();
}

#else

SYN_Status syn_port_wdt_init(uint32_t timeout_ms)
{
    (void)timeout_ms;
    return SYN_NOT_IMPLEMENTED;
}

void syn_port_wdt_feed(void)
{
    /* No-op on unsupported architectures */
}

#endif /* WDT arch selection */

} /* extern "C" */

/* ── Console serial port (Arduino Serial) ──────────────────────────────── */

#include "syntropic/port/syn_port_serial.h"

extern "C" {

SYN_Status syn_port_serial_init(uint32_t baudrate)
{
    if (baudrate == 0) baudrate = 115200;
    Serial.begin(baudrate);
    return SYN_OK;
}

int syn_port_serial_write(const uint8_t *data, size_t len)
{
    return (int)Serial.write(data, len);
}

int syn_port_serial_read(uint8_t *buf, size_t max_len)
{
    size_t count = 0;
    while (count < max_len && Serial.available() > 0) {
        int ch = Serial.read();
        if (ch < 0) break;
        buf[count++] = (uint8_t)ch;
    }
    return (int)count;
}

} /* extern "C" */

/* ── Socket Port (Arduino Ethernet / W5500 / W5100 / ENC28J60) ───────────── */

#ifndef SYN_ARDUINO_HAS_ETHERNET
  #if __has_include(<Ethernet.h>)
    #define SYN_ARDUINO_HAS_ETHERNET 1
  #else
    #define SYN_ARDUINO_HAS_ETHERNET 0
  #endif
#endif

#if SYN_ARDUINO_HAS_ETHERNET

#include <Ethernet.h>
#include <utility/w5100.h>
#include <EthernetUdp.h>
#include "syntropic/port/syn_port_socket.h"


enum SynSockType {
    SYN_SOCK_FREE = 0,
    SYN_SOCK_TCP_CLIENT,
    SYN_SOCK_TCP_LISTEN,
    SYN_SOCK_UDP
};

struct SynArduinoSocketSlot {
    SynSockType type;
    uint16_t port;
    uint8_t hw_sock;
    uint8_t pending_accept_hw;
    int pending_pkt_len;
    EthernetServer *server;
    EthernetUDP *udp;
};

#ifndef SYN_ARDUINO_MAX_SOCKETS
  #define SYN_ARDUINO_MAX_SOCKETS 4
#endif

static SynArduinoSocketSlot s_arduino_sockets[SYN_ARDUINO_MAX_SOCKETS];
static uint8_t s_srv_buf[SYN_ARDUINO_MAX_SOCKETS][sizeof(EthernetServer)]
    __attribute__((aligned(sizeof(void *))));
static uint8_t s_udp_buf[SYN_ARDUINO_MAX_SOCKETS][sizeof(EthernetUDP)]
    __attribute__((aligned(sizeof(void *))));

static bool valid_arduino_sock(SYN_Socket sock, SynSockType type) {
    return (sock >= 0 && sock < SYN_ARDUINO_MAX_SOCKETS &&
            s_arduino_sockets[sock].type == type);
}

static int alloc_socket_slot(SynSockType type) {
    for (int i = 0; i < SYN_ARDUINO_MAX_SOCKETS; i++) {
        if (s_arduino_sockets[i].type == SYN_SOCK_FREE) {
            s_arduino_sockets[i].type = type;
            s_arduino_sockets[i].port = 0;
            s_arduino_sockets[i].hw_sock = MAX_SOCK_NUM;
            s_arduino_sockets[i].pending_accept_hw = MAX_SOCK_NUM;
            s_arduino_sockets[i].pending_pkt_len = 0;
            s_arduino_sockets[i].server = NULL;
            s_arduino_sockets[i].udp = NULL;
            return i;
        }
    }
    return SYN_SOCKET_INVALID;
}


extern "C" {

SYN_Socket syn_port_sock_connect(const SYN_SockAddr *addr) {
    if (!addr) return SYN_SOCKET_INVALID;
    int slot = alloc_socket_slot(SYN_SOCK_TCP_CLIENT);
    if (slot == SYN_SOCKET_INVALID) return SYN_SOCKET_INVALID;

    EthernetClient c;
    IPAddress target_ip(addr->ip[0], addr->ip[1], addr->ip[2], addr->ip[3]);
    if (c.connect(target_ip, addr->port)) {
        s_arduino_sockets[slot].hw_sock = c.getSocketNumber();
        return slot;
    }

    s_arduino_sockets[slot].type = SYN_SOCK_FREE;
    return SYN_SOCKET_INVALID;
}

SYN_Socket syn_port_sock_connect_host(const char *host, uint16_t port) {
    if (!host) return SYN_SOCKET_INVALID;
    int slot = alloc_socket_slot(SYN_SOCK_TCP_CLIENT);
    if (slot == SYN_SOCKET_INVALID) return SYN_SOCKET_INVALID;

    EthernetClient c;
    if (c.connect(host, port)) {
        s_arduino_sockets[slot].hw_sock = c.getSocketNumber();
        return slot;
    }

    s_arduino_sockets[slot].type = SYN_SOCK_FREE;
    return SYN_SOCKET_INVALID;
}

int syn_port_sock_send(SYN_Socket sock, const void *data, size_t len) {
    if (!valid_arduino_sock(sock, SYN_SOCK_TCP_CLIENT)) {
        return -1;
    }
    uint8_t hw = s_arduino_sockets[sock].hw_sock;
    if (hw >= MAX_SOCK_NUM) return -1;

    EthernetClient c(hw);
    return (int)c.write((const uint8_t *)data, len);
}

int syn_port_sock_send_all(SYN_Socket sock, const void *data, size_t len) {
    if (!valid_arduino_sock(sock, SYN_SOCK_TCP_CLIENT)) {
        return -1;
    }
    uint8_t hw = s_arduino_sockets[sock].hw_sock;
    if (hw >= MAX_SOCK_NUM) return -1;

    EthernetClient c(hw);
    size_t written = c.write((const uint8_t *)data, len);
    return (written == len) ? (int)len : -1;
}

int syn_port_sock_recv(SYN_Socket sock, void *buf, size_t max_len, uint32_t timeout_ms) {
    if (!valid_arduino_sock(sock, SYN_SOCK_TCP_CLIENT) || (buf == NULL && max_len > 0U)) {
        return -1;
    }
    uint8_t hw = s_arduino_sockets[sock].hw_sock;
    if (hw >= MAX_SOCK_NUM) return 0;

    EthernetClient c(hw);
    uint32_t start = millis();
    do {
        int avail = c.available();
        if (avail > 0) {
            size_t to_read = (size_t)avail < max_len ? (size_t)avail : max_len;
            return c.read((uint8_t *)buf, to_read);
        }

        if (!c.connected()) {
            return 0; /* Connection closed by peer */
        }

        if (timeout_ms == 0U) {
            break;
        }
        delay(1);
    } while ((millis() - start) < timeout_ms);

    return -1; /* Connection alive, but no data available yet (WOULDBLOCK) */
}

bool syn_port_sock_readable(SYN_Socket sock) {
    if (sock < 0 || sock >= SYN_ARDUINO_MAX_SOCKETS) {
        return false;
    }
    if (s_arduino_sockets[sock].type == SYN_SOCK_TCP_CLIENT) {
        uint8_t hw = s_arduino_sockets[sock].hw_sock;
        if (hw >= MAX_SOCK_NUM) return false;
        EthernetClient c(hw);
        return (c.available() > 0) || !c.connected();
    }
    if (s_arduino_sockets[sock].type == SYN_SOCK_TCP_LISTEN) {
        if (s_arduino_sockets[sock].pending_accept_hw < MAX_SOCK_NUM) {
            return true;
        }
        EthernetServer *srv = s_arduino_sockets[sock].server;
        if (!srv) return false;
        EthernetClient client = srv->accept();
        if (client) {
            s_arduino_sockets[sock].pending_accept_hw = client.getSocketNumber();
            return true;
        }
    }
    return false;
}

SYN_Socket syn_port_sock_listen(uint16_t port, int backlog) {
    (void)backlog;
    if (port == 0U) {
        return SYN_SOCKET_INVALID;
    }
    int slot = alloc_socket_slot(SYN_SOCK_TCP_LISTEN);
    if (slot == SYN_SOCKET_INVALID) {
        return SYN_SOCKET_INVALID;
    }

    new (s_srv_buf[slot]) EthernetServer(port);
    s_arduino_sockets[slot].server = (EthernetServer *)s_srv_buf[slot];
    s_arduino_sockets[slot].port = port;
    s_arduino_sockets[slot].server->begin();

    return slot;
}

SYN_Socket syn_port_sock_accept(SYN_Socket listener, uint32_t timeout_ms) {
    if (!valid_arduino_sock(listener, SYN_SOCK_TCP_LISTEN)) {
        return SYN_SOCKET_INVALID;
    }
    EthernetServer *srv = s_arduino_sockets[listener].server;
    if (!srv) return SYN_SOCKET_INVALID;

    uint8_t hw_sock = MAX_SOCK_NUM;
    if (s_arduino_sockets[listener].pending_accept_hw < MAX_SOCK_NUM) {
        hw_sock = s_arduino_sockets[listener].pending_accept_hw;
        s_arduino_sockets[listener].pending_accept_hw = MAX_SOCK_NUM;
    } else {
        uint32_t start = millis();
        do {
            EthernetClient client = srv->accept();
            if (client) {
                hw_sock = client.getSocketNumber();
                break;
            }
            if (timeout_ms == 0U) {
                break;
            }
            delay(1);
        } while ((millis() - start) < timeout_ms);
    }

    if (hw_sock < MAX_SOCK_NUM) {
        int slot = alloc_socket_slot(SYN_SOCK_TCP_CLIENT);
        if (slot == SYN_SOCKET_INVALID) {
            EthernetClient(hw_sock).stop();
            return SYN_SOCKET_INVALID;
        }

        s_arduino_sockets[slot].hw_sock = hw_sock;

        return slot;
    }

    return SYN_SOCKET_INVALID;
}

SYN_Socket syn_port_udp_open(uint16_t port) {
    int slot = alloc_socket_slot(SYN_SOCK_UDP);
    if (slot == SYN_SOCKET_INVALID) return SYN_SOCKET_INVALID;

    new (s_udp_buf[slot]) EthernetUDP();
    s_arduino_sockets[slot].udp = (EthernetUDP *)s_udp_buf[slot];

    s_arduino_sockets[slot].port = port;
    s_arduino_sockets[slot].pending_pkt_len = 0;

    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    int res = s_arduino_sockets[slot].udp->begin(port);
    SPI.endTransaction();

    if (res == 1) {
        return slot;
    }
    s_arduino_sockets[slot].udp = NULL;
    s_arduino_sockets[slot].type = SYN_SOCK_FREE;
    return SYN_SOCKET_INVALID;
}

int syn_port_udp_sendto(SYN_Socket sock, const void *data, size_t len, const SYN_SockAddr *to) {
    if (!valid_arduino_sock(sock, SYN_SOCK_UDP) || !to) {
        return -1;
    }
    EthernetUDP *u = s_arduino_sockets[sock].udp;
    if (!u) return -1;

    IPAddress ip(to->ip[0], to->ip[1], to->ip[2], to->ip[3]);
    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    if (u->beginPacket(ip, to->port) != 1) {
        SPI.endTransaction();
        delay(10);
        SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
        if (u->beginPacket(ip, to->port) != 1) {
            SPI.endTransaction();
            return -1;
        }
    }
    u->write((const uint8_t *)data, len);
    int res = u->endPacket();
    SPI.endTransaction();
    return res == 1 ? (int)len : -1;
}

bool syn_port_udp_readable(SYN_Socket sock) {
    if (!valid_arduino_sock(sock, SYN_SOCK_UDP)) {
        return false;
    }
    EthernetUDP *u = s_arduino_sockets[sock].udp;
    if (!u) return false;

    if (s_arduino_sockets[sock].pending_pkt_len > 0) {
        return true;
    }

    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    int packetSize = u->parsePacket();
    SPI.endTransaction();
    if (packetSize > 0) {
        s_arduino_sockets[sock].pending_pkt_len = packetSize;
        return true;
    }
    return false;
}

int syn_port_udp_recvfrom(SYN_Socket sock, void *buf, size_t max_len, SYN_SockAddr *from, uint32_t timeout_ms) {
    if (!valid_arduino_sock(sock, SYN_SOCK_UDP)) {
        return -1;
    }
    EthernetUDP *u = s_arduino_sockets[sock].udp;
    if (!u) return -1;

    int packetSize = s_arduino_sockets[sock].pending_pkt_len;
    s_arduino_sockets[sock].pending_pkt_len = 0;

    if (packetSize <= 0) {
        uint32_t start = millis();
        do {
            SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
            packetSize = u->parsePacket();
            SPI.endTransaction();
            if (packetSize > 0) break;
            if (timeout_ms == 0) return 0; /* Non-blocking poll */
            delay(1);
        } while ((millis() - start) < timeout_ms);
    }

    if (packetSize <= 0) return -1;

    size_t to_read = (size_t)packetSize < max_len ? (size_t)packetSize : max_len;

    SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
    int read_bytes = u->read((uint8_t *)buf, to_read);
    IPAddress remote = u->remoteIP();
    uint16_t remote_port = u->remotePort();
    SPI.endTransaction();

    if (read_bytes > 0 && from) {
        from->ip[0] = remote[0];
        from->ip[1] = remote[1];
        from->ip[2] = remote[2];
        from->ip[3] = remote[3];
        from->port = remote_port;
    }
    return read_bytes;
}

SYN_Status syn_port_udp_join_multicast(SYN_Socket sock, const char *multicast_ip) {
    if (!valid_arduino_sock(sock, SYN_SOCK_UDP) || !s_arduino_sockets[sock].udp) {
        return SYN_INVALID_PARAM;
    }
    uint8_t m_ip[4];
    if (sscanf(multicast_ip, "%hhu.%hhu.%hhu.%hhu", &m_ip[0], &m_ip[1], &m_ip[2], &m_ip[3]) != 4) {
        return SYN_INVALID_PARAM;
    }
    IPAddress mcast(m_ip[0], m_ip[1], m_ip[2], m_ip[3]);
    uint16_t local_port = s_arduino_sockets[sock].port;

    s_arduino_sockets[sock].pending_pkt_len = 0;
    s_arduino_sockets[sock].udp->stop();
    if (s_arduino_sockets[sock].udp->beginMulticast(mcast, local_port) == 1) {
        return SYN_OK;
    }
    return SYN_ERROR;
}

void syn_port_sock_close(SYN_Socket sock) {
    if (sock < 0 || sock >= SYN_ARDUINO_MAX_SOCKETS) return;

    if (s_arduino_sockets[sock].type == SYN_SOCK_TCP_CLIENT) {
        uint8_t hw_sock = s_arduino_sockets[sock].hw_sock;
        if (hw_sock < MAX_SOCK_NUM) {
            EthernetClient c(hw_sock);
            c.flush();
            c.stop();
            for (int i = 0; i < SYN_ARDUINO_MAX_SOCKETS; i++) {
                if (s_arduino_sockets[i].type == SYN_SOCK_TCP_LISTEN &&
                    s_arduino_sockets[i].server != NULL) {
                    s_arduino_sockets[i].server->begin();
                }
            }
        }
        s_arduino_sockets[sock].hw_sock = MAX_SOCK_NUM;
    } else if (s_arduino_sockets[sock].type == SYN_SOCK_TCP_LISTEN) {
        uint16_t listen_port = s_arduino_sockets[sock].port;
        if (s_arduino_sockets[sock].pending_accept_hw < MAX_SOCK_NUM) {
            EthernetClient(s_arduino_sockets[sock].pending_accept_hw).stop();
            s_arduino_sockets[sock].pending_accept_hw = MAX_SOCK_NUM;
        }
        s_arduino_sockets[sock].server = NULL;
        s_arduino_sockets[sock].port = 0;
        s_arduino_sockets[sock].type = SYN_SOCK_FREE;

        bool same_port_active = false;
        for (int i = 0; i < SYN_ARDUINO_MAX_SOCKETS; i++) {
            if (s_arduino_sockets[i].type == SYN_SOCK_TCP_LISTEN &&
                s_arduino_sockets[i].port == listen_port) {
                same_port_active = true;
                break;
            }
        }
        if (!same_port_active) {
            SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
            for (uint8_t s = 0; s < MAX_SOCK_NUM; s++) {
                if (W5100.readSnSR(s) == SnSR::LISTEN) {
                    W5100.execCmdSn(s, Sock_CLOSE);
                }
            }
            SPI.endTransaction();
            for (int i = 0; i < SYN_ARDUINO_MAX_SOCKETS; i++) {
                if (s_arduino_sockets[i].type == SYN_SOCK_TCP_LISTEN &&
                    s_arduino_sockets[i].server != NULL) {
                    s_arduino_sockets[i].server->begin();
                }
            }
        }
        return;
    } else if (s_arduino_sockets[sock].type == SYN_SOCK_UDP) {
        s_arduino_sockets[sock].pending_pkt_len = 0;
        if (s_arduino_sockets[sock].udp) {
            s_arduino_sockets[sock].udp->stop();
            s_arduino_sockets[sock].udp = NULL;
        }
    }
    s_arduino_sockets[sock].type = SYN_SOCK_FREE;
}

} /* extern "C" */

#else /* !SYN_ARDUINO_HAS_ETHERNET */

#include "syntropic/port/syn_port_socket.h"

extern "C" {

SYN_WEAK bool syn_port_sock_readable(SYN_Socket sock)
{
    (void)sock;
    return false;
}

SYN_WEAK bool syn_port_udp_readable(SYN_Socket sock)
{
    (void)sock;
    return false;
}

} /* extern "C" */

#endif /* SYN_ARDUINO_HAS_ETHERNET */

/* ── RTC Port (Software RTC implementation on Arduino) ─────────────────── */

extern "C" {

static uint32_t s_rtc_base_epoch = 0;
static uint32_t s_rtc_base_ms = 0;

SYN_Status syn_port_rtc_init(void)
{
    s_rtc_base_epoch = 0;
    s_rtc_base_ms = 0;
    return SYN_OK;
}

SYN_Status syn_port_rtc_set(const SYN_RTC_DateTime *dt)
{
    if (!dt) return SYN_INVALID_PARAM;
    s_rtc_base_epoch = syn_rtc_to_epoch(dt);
    s_rtc_base_ms = millis();
    return SYN_OK;
}

SYN_Status syn_port_rtc_get(SYN_RTC_DateTime *dt)
{
    if (!dt) return SYN_INVALID_PARAM;
    if (s_rtc_base_epoch == 0) return SYN_ERROR;

    uint32_t elapsed_s = (millis() - s_rtc_base_ms) / 1000u;
    syn_rtc_from_epoch(s_rtc_base_epoch + elapsed_s, dt);
    return SYN_OK;
}

} /* extern "C" */

/* ── Flash / EEPROM Port Implementation ────────────────────────────────────── */

#include "syntropic/port/syn_port_flash.h"
#if defined(ARDUINO_ARCH_AVR)
#include <avr/eeprom.h>
#endif

extern "C" {

#define SYN_ARDUINO_SECTOR_SIZE  64  /* 64-byte sector pages */

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    return SYN_ARDUINO_SECTOR_SIZE;
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    if (!buf) return SYN_INVALID_PARAM;
#if defined(ARDUINO_ARCH_AVR)
    eeprom_read_block(buf, (const void *)(uintptr_t)addr, len);
    return SYN_OK;
#else
    memset(buf, 0xFF, len);
    return SYN_OK;
#endif
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    if (!buf) return SYN_INVALID_PARAM;
#if defined(ARDUINO_ARCH_AVR)
    eeprom_update_block(buf, (void *)(uintptr_t)addr, len);
    return SYN_OK;
#else
    return SYN_OK;
#endif
}

SYN_Status syn_port_flash_erase(uint32_t addr)
{
#if defined(ARDUINO_ARCH_AVR)
    uint8_t erased[SYN_ARDUINO_SECTOR_SIZE];
    memset(erased, 0xFF, sizeof(erased));
    eeprom_update_block(erased, (void *)(uintptr_t)addr, SYN_ARDUINO_SECTOR_SIZE);
    return SYN_OK;
#else
    return SYN_OK;
#endif
}

} /* extern "C" */

#endif /* ARDUINO */


