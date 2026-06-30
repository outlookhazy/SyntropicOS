# Porting Guide

SyntropicOS separates hardware-specific code into a **port layer** — a set of functions you implement for your specific MCU. Each port function is declared in a header under `src/syntropic/port/`.

## Port Interfaces

| Port Header | Functions | When Needed |
|---|---|---|
| `syn_port_system.h` | `get_tick_ms`, `delay_ms`, `enter_critical`, `exit_critical`, `system_reset` | Always |
| `syn_port_spinlock.h` | `spinlock_acquire`, `spinlock_release`, `spinlock_try_acquire`, `core_id`, `ipc_notify` | If using multicore (AMP) |
| `syn_port_gpio.h` | `init`, `deinit`, `write`, `read`, `toggle` | If using GPIO, buttons, LEDs, motors |
| `syn_port_uart.h` | `init`, `deinit`, `transmit`, `receive`, byte variants | If using UART, CLI, logging |
| `syn_port_spi.h` | `init`, `deinit`, `transfer`, `cs_assert`, `cs_deassert` | If using SPI devices |
| `syn_port_i2c.h` | `init`, `deinit`, `write`, `read`, `write_read` | If using I2C devices |
| `syn_port_flash.h` | `erase`, `read`, `write`, `sector_size` | If using parameter store or LittleFS |
| `syn_port_adc.h` | `init`, `read`, `resolution`, `reference_mv` | If using ADC driver |
| `syn_port_dac.h` | `init`, `write_raw`, `write_mv` | If using DAC driver |
| `syn_port_pwm.h` | `init`, `set_duty`, `set_frequency` | If using servo, soft PWM |
| `syn_port_rtc.h` | `get_datetime`, `set_datetime`, `get_epoch` | If using RTC driver |
| `syn_port_exti.h` | `configure`, `enable`, `disable`, `clear_pending` | If using GPIO interrupts |
| `syn_port_can.h` | `init`, `transmit`, `receive`, `filter` | If using CAN bus |
| `syn_port_wdt.h` | `init`, `feed` | If using hardware watchdog |
| `syn_port_socket.h` | `connect`, `send`, `recv`, `listen`, `accept`, `close`, UDP variants | If using network stack |

## System Port (Required)

The system port is needed by almost every module. Here's a complete STM32 HAL example:

```c
#include "syntropic/port/syn_port_system.h"
#include "stm32f4xx_hal.h"

uint32_t syn_port_get_tick_ms(void) {
    return HAL_GetTick();
}

void syn_port_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

static volatile uint32_t critical_nesting = 0;

void syn_port_enter_critical(void) {
    __disable_irq();
    critical_nesting++;
}

void syn_port_exit_critical(void) {
    if (critical_nesting > 0) {
        critical_nesting--;
        if (critical_nesting == 0) {
            __enable_irq();
        }
    }
}

SYN_NORETURN void syn_port_system_reset(void) {
    NVIC_SystemReset();
    while (1) {}
}
```

## Weak Stubs

Compile `src/syntropic/port_stubs/syn_port_stubs.c` into your project. Every port function has a weak stub that calls `syn_assert_failed()` — if you forget to implement a port function, you'll get a clear runtime assertion instead of silent misbehavior or a cryptic linker error.

```cmake
target_link_libraries(your_target PRIVATE syn_stubs)
```

## Reference Ports

SyntropicOS ships with complete port implementations for several platforms:

| Platform | File | Description |
|---|---|---|
| STM32F4 (bare-metal) | `src/port/stm32f4/port_stm32f4.c` | Direct register access, no HAL dependency |
| STM32 HAL | `src/port/stm32_hal/port_stm32_hal.c` | Uses STM32Cube HAL for portability across STM32 families |
| ESP32 (ESP-IDF) | `src/port/esp32/port_esp32.c` | ESP-IDF based port |
| RP2040/RP2350 | `src/port/rp2040/port_rp2040.c` | Raspberry Pi Pico SDK based port |
| RP2040 Multicore | `src/port/rp2040/port_rp2040_multicore.c` | RP2040/RP2350 multicore hardware spinlock port |
| Arduino | `src/port/arduino/port_arduino.cpp` | Arduino C++ SDK based port |
| Arduino Multicore | `src/port/arduino/port_arduino_multicore.cpp` | Arduino RP2040 multicore hardware spinlock port |

These can be used directly or as a reference when writing your own port.
