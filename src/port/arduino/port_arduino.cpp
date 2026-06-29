/**
 * @file port_arduino.cpp
 * @brief SyntropicOS port layer for the Arduino C++ SDK.
 *
 * Implements the system, GPIO, and UART interfaces by wrapping standard
 * Arduino C++ SDK functions. Compatible with AVR, SAMD, and other Arduino boards.
 */

#if defined(ARDUINO)

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

/* ── PWM Port ───────────────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_pwm.h"

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

/* ── I2C Port ───────────────────────────────────────────────────────────── */

#if __has_include(<Wire.h>)

#include <Wire.h>
#include "syntropic/port/syn_port_i2c.h"

SYN_Status syn_port_i2c_init(const SYN_I2C_Config *cfg)
{
    if (!cfg) return SYN_INVALID_PARAM;
    /* Bus 0 = Wire. Secondary buses not universally available. */
    if (cfg->bus != 0) return SYN_NOT_IMPLEMENTED;
    Wire.begin();
    Wire.setClock(cfg->clock_hz);
    return SYN_OK;
}

SYN_Status syn_port_i2c_deinit(uint8_t bus)
{
    if (bus != 0) return SYN_NOT_IMPLEMENTED;
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP32) || \
    defined(ARDUINO_ARCH_MEGAAVR) || defined(ARDUINO_ARCH_MBED)
    Wire.end();
#endif
    /* AVR Wire has no end() — just return OK */
    return SYN_OK;
}

SYN_Status syn_port_i2c_write(uint8_t bus, uint8_t addr,
                                const uint8_t *data, size_t len)
{
    if (bus != 0) return SYN_NOT_IMPLEMENTED;
    Wire.beginTransmission(addr);
    Wire.write(data, len);
    uint8_t result = Wire.endTransmission();
    return (result == 0) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_i2c_read(uint8_t bus, uint8_t addr,
                               uint8_t *data, size_t len)
{
    if (bus != 0) return SYN_NOT_IMPLEMENTED;
    size_t received = Wire.requestFrom(addr, (uint8_t)len);
    for (size_t i = 0; i < received && i < len; i++) {
        data[i] = (uint8_t)Wire.read();
    }
    return (received == len) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_i2c_write_read(uint8_t bus, uint8_t addr,
                                     const uint8_t *tx_data, size_t tx_len,
                                     uint8_t *rx_data, size_t rx_len)
{
    if (bus != 0) return SYN_NOT_IMPLEMENTED;
    Wire.beginTransmission(addr);
    Wire.write(tx_data, tx_len);
    uint8_t result = Wire.endTransmission(false);  /* No STOP — repeated start */
    if (result != 0) return SYN_ERROR;

    size_t received = Wire.requestFrom(addr, (uint8_t)rx_len);
    for (size_t i = 0; i < received && i < rx_len; i++) {
        rx_data[i] = (uint8_t)Wire.read();
    }
    return (received == rx_len) ? SYN_OK : SYN_ERROR;
}

#endif /* Wire.h */

/* ── SPI Port ───────────────────────────────────────────────────────────── */

#if __has_include(<SPI.h>)

#include <SPI.h>
#include "syntropic/port/syn_port_spi.h"

static SPISettings spi_settings;

SYN_Status syn_port_spi_init(const SYN_SPI_Config *cfg)
{
    if (!cfg || cfg->bus != 0) return SYN_INVALID_PARAM;

    uint8_t mode;
    switch (cfg->mode) {
        case SYN_SPI_MODE_0: mode = SPI_MODE0; break;
        case SYN_SPI_MODE_1: mode = SPI_MODE1; break;
        case SYN_SPI_MODE_2: mode = SPI_MODE2; break;
        case SYN_SPI_MODE_3: mode = SPI_MODE3; break;
        default:             mode = SPI_MODE0; break;
    }

    uint8_t order = (cfg->bit_order == 1) ? LSBFIRST : MSBFIRST;
    spi_settings = SPISettings(cfg->clock_hz, order, mode);
    SPI.begin();
    return SYN_OK;
}

SYN_Status syn_port_spi_deinit(uint8_t bus)
{
    if (bus != 0) return SYN_NOT_IMPLEMENTED;
    SPI.end();
    return SYN_OK;
}

SYN_Status syn_port_spi_transfer(uint8_t bus,
                                   const uint8_t *tx_buf,
                                   uint8_t *rx_buf,
                                   size_t len)
{
    if (bus != 0) return SYN_NOT_IMPLEMENTED;
    SPI.beginTransaction(spi_settings);
    for (size_t i = 0; i < len; i++) {
        uint8_t tx_byte = tx_buf ? tx_buf[i] : 0xFF;
        uint8_t rx_byte = SPI.transfer(tx_byte);
        if (rx_buf) rx_buf[i] = rx_byte;
    }
    SPI.endTransaction();
    return SYN_OK;
}

SYN_Status syn_port_spi_cs_assert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus;
    digitalWrite(cs_pin, LOW);
    return SYN_OK;
}

SYN_Status syn_port_spi_cs_deassert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus;
    digitalWrite(cs_pin, HIGH);
    return SYN_OK;
}

#endif /* SPI.h */

/* ── EXTI Port (GPIO Interrupts) ────────────────────────────────────────── */

#if defined(digitalPinToInterrupt)

#include "syntropic/port/syn_port_exti.h"
#include "syntropic/drivers/syn_exti.h"

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
                    exti_modes[slot]);
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

#endif /* digitalPinToInterrupt */

/* ── Hardware Watchdog Port ─────────────────────────────────────────────── */

#include "syntropic/port/syn_port_wdt.h"

#if defined(ARDUINO_ARCH_AVR)

#include <avr/wdt.h>

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
    noInterrupts();
    _PROTECTED_WRITE(WDT.CTRLA, period);
    interrupts();
    return SYN_OK;
}

void syn_port_wdt_feed(void)
{
    __asm__ __volatile__("wdr");
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

#endif /* ARDUINO */

