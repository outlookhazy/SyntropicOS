# Porting Guide

SyntropicOS isolates hardware-specific code behind a thin C99 **port layer**. Portable modules under `src/syntropic/` never touch MCU registers directly; instead, they call `syn_port_*` functions declared in `src/syntropic/port/` (and `src/port/` for Ethernet/USB MAC contracts).

If your target is already covered by a [Reference Port](#reference-ports), add that port `.c` file to your build instead of writing one from scratch.

---

## Port Header Contracts

### Core & Peripheral Port Headers (`src/syntropic/port/`)

| Port Header | Functions (`syn_port_*`) | Required When |
|---|---|---|
| `syn_port_system.h` | `enter_critical`, `exit_critical`, `get_tick_ms`, `get_tick_us`, `delay_ms`, `nvic_set_priority`, `nvic_enable_irq`, `system_reset` (+ `sleep_until` when `SYN_USE_TICKLESS=1`) | Always (core scheduler, timers, critical sections) |
| `syn_port_spinlock.h` | `spinlock_acquire`, `spinlock_release`, `spinlock_try_acquire`, `core_id`, `ipc_notify` | `SYN_USE_MULTICORE=1` (AMP dual-core) |
| `syn_port_serial.h` | `serial_init`, `serial_write`, `serial_read` | Console logging (`syn_log_init(NULL, ...)`) or `syn_cli` |
| `syn_port_gpio.h` | `gpio_init`, `gpio_deinit`, `gpio_write`, `gpio_read`, `gpio_toggle` | GPIO, buttons, LEDs, soft PWM, keypad, bit-bang buses |
| `syn_port_exti.h` | `exti_configure`, `exti_enable`, `exti_disable`, `exti_clear_pending` | `syn_exti` GPIO edge interrupts |
| `syn_port_uart.h` | `uart_init`, `uart_deinit`, `uart_transmit`, `uart_receive`, byte & TXE IRQ helpers | `syn_uart` peripheral serial buses (Modbus, NMEA, LIN, etc.) |
| `syn_port_i2c.h` | `i2c_init`, `i2c_deinit`, `i2c_transfer` | Synchronous I2C master/device drivers (`syn_i2c`, sensors, OLED) |
| `syn_port_i2c_async.h` | `i2c_xfer_async`, `i2c_cancel` | `SYN_USE_I2C_ASYNC=1` (`syn_i2c_queue`) |
| `syn_port_spi.h` | `spi_init`, `spi_deinit`, `spi_transfer` | Synchronous SPI drivers (`syn_spi`, SD card, RFID, CAN/Eth MACs) |
| `syn_port_spi_async.h` | `spi_cs_assert`, `spi_cs_deassert`, `spi_xfer_async`, `spi_cancel` | `SYN_USE_SPI_ASYNC=1` (`syn_spi_queue`) |
| `syn_port_can.h` | `can_init`, `can_send`, `can_receive`, `can_set_filter` | `syn_can` and CAN protocols (CANopen, J1939, ISO-TP, UDS, DeviceNet) |
| `syn_port_adc.h` | `adc_init`, `adc_deinit`, `adc_read_channel`, `adc_start_dma_scan` | `syn_adc`, analog joystick |
| `syn_port_dac.h` | `dac_init`, `dac_write`, `dac_resolution`, `dac_reference_mv` | `syn_dac` analog output |
| `syn_port_comp.h` | `comp_init`, `comp_read`, `comp_enable` | `syn_comp` analog comparator |
| `syn_port_pwm.h` | `pwm_init`, `pwm_set_duty`, `pwm_set_duty_raw`, `pwm_enable`, `pwm_set_freq` | `syn_pwm`, `syn_servo`, `syn_dc_motor`, `syn_buzzer` |
| `syn_port_hrpwm.h` | `hrpwm_init`, `hrpwm_set_duty_q16`, `hrpwm_set_deadtime_ns`, `hrpwm_set_phase_deg`, `hrpwm_enable_fault`, `hrpwm_enable` | `syn_hrpwm` digital power / motor stage |
| `syn_port_dma.h` | `dma_start`, `dma_stop`, `dma_is_busy`, `dma_get_counter`, `cache_clean`, `cache_invalidate` | `SYN_USE_DMA=1` (`syn_dma`) |
| `syn_port_rtc.h` | `rtc_init`, `rtc_get`, `rtc_set`, `rtc_set_drift_ppm`, `rtc_set_alarm` | `syn_rtc` calendar & alarm driver |
| `syn_port_hpclock.h` | `hpclock_lsb_ptr`, `hpclock_freq_hz` | `syn_hpclock` 64-bit high-precision hardware counter |
| `syn_port_flash.h` | `flash_erase`, `flash_read`, `flash_write`, `flash_sector_size` | `syn_param`, `syn_settings`, `syn_lfs`, `syn_errlog`, `syn_coredump`, `syn_ota` |
| `syn_port_wdt.h` | `wdt_init`, `wdt_feed` | `syn_hwwdt` hardware watchdog |
| `syn_port_aes.h` | `aes_init`, `aes_encrypt_block`, `aes_decrypt_block`, `ghash_mult` | `SYN_USE_PORT_AES=1` hardware crypto offload |
| `syn_port_ble_hci.h` | `ble_hci_tx`, `ble_hci_reset` | `syn_ble` HCI H:4 UART / SPI controller transport |
| `syn_port_socket.h` | `sock_connect*`, `sock_send*`, `sock_recv`, `sock_listen`, `sock_accept`, `sock_close`, `udp_*` | Host/OS socket bridge (`port_posix_socket.c` or external TCP/IP offload) |

### MAC & USB Controller Port Headers (`src/port/`)

| Port Header | Functions (`syn_port_*`) | Required When |
|---|---|---|
| `port/syn_port_eth.h` | `eth_init`, `eth_tx`, `eth_rx` | Native zero-heap Ethernet II stack (`syn_eth`, `syn_tcp`, `syn_udp`, `syn_netcfg`) |
| `port/syn_port_usb.h` | `usb_init`, `usb_connect`, `usb_disconnect`, `usb_set_address`, `usb_ep_*` | USB 2.0 Device stack (`syn_usb`, CDC, HID, MIDI, MSC) |
| `port/syn_port_usb_host.h` | `usb_host_init`, `usb_host_vbus`, `usb_host_bus_reset`, `usb_host_pipe_*`, `usb_host_submit_*` | USB 2.0 Host stack (`syn_usb_host`, `syn_usb_host_cdc`) |

---

## System Port Example (Nested Critical Sections & Tick Source)

`syn_port_enter_critical()` and `syn_port_exit_critical()` **must support nesting** and **must preserve the caller's interrupt mask state** (so calling a critical section from inside an ISR does not prematurely unmask interrupts on exit). Complete Cortex-M / STM32 HAL pattern:

```c
#include "syntropic/port/syn_port_system.h"
#include "stm32f4xx_hal.h"

static volatile uint32_t s_crit_depth = 0U;
static volatile uint32_t s_saved_primask = 0U;

void syn_port_enter_critical(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (s_crit_depth == 0U) {
        s_saved_primask = primask;
    }
    s_crit_depth++;
}

void syn_port_exit_critical(void) {
    SYN_COMPILER_BARRIER();
    if (s_crit_depth > 0U) {
        s_crit_depth--;
        if (s_crit_depth == 0U && (s_saved_primask & 1U) == 0U) {
            __enable_irq();
        }
    }
}

uint32_t syn_port_get_tick_ms(void) {
    return HAL_GetTick();
}

uint32_t syn_port_get_tick_us(void) {
    return HAL_GetTick() * 1000U; /* Or interpolate SysTick->VAL; see port_stm32_hal.c */
}

void syn_port_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

SYN_NORETURN void syn_port_system_reset(void) {
    NVIC_SystemReset();
    for (;;) {}
}
```

---

## Weak Stubs (`syn_port_stubs.c`)

Compile `src/syntropic/port_stubs/syn_port_stubs.c` (or link `syn_stubs` in CMake / `$(SYN_STUB_SRCS)` in Make) alongside your port file. Every `syn_port_*` symbol has a `SYN_WEAK` fallback that invokes `syn_assert_failed(__FILE__, __LINE__)`. Your strong definitions override the stubs automatically, while any unimplemented port function you accidentally call traps deterministically at runtime instead of failing to link unused modules.

```cmake
target_link_libraries(your_target PRIVATE syntropic syn_stubs)
```

---

## Reference Ports

SyntropicOS ships production port implementations under `src/port/`:

| Platform | File | Description |
|---|---|---|
| **STM32F4 (Bare-Metal)** | `src/port/stm32f4/port_stm32f4.c` | Direct CMSIS register access, zero HAL dependency |
| **STM32 (Cube HAL)** | `src/port/stm32_hal/port_stm32_hal.c` | Portable across STM32F0/F1/F4/F7/G0/H7/L4/C0/U0 families |
| **WCH CH32V307 (RISC-V)** | `src/port/ch32v307/port_ch32v307.c` | QingKe V4F RISC-V bare-metal port + startup & linker script |
| **ESP32 (ESP-IDF)** | `src/port/esp32/port_esp32.c` | ESP-IDF GPIO, UART, SPI, I2C, ADC, timers, and flash port |
| **RP2040 / RP2350** | `src/port/rp2040/port_rp2040.c` | Raspberry Pi Pico SDK peripheral and system port |
| **RP2040 Multicore** | `src/port/rp2040/port_rp2040_multicore.c` | Hardware SIO spinlock & FIFO IPC port for AMP dual-core |
| **Arduino** | `src/port/arduino/port_arduino.cpp` | Cross-board Arduino C++ SDK port |
| **Arduino Multicore** | `src/port/arduino/port_arduino_multicore.cpp` | Arduino-Pico hardware spinlock & dual-core IPC port |
| **POSIX Sockets** | `src/port/posix/port_posix_socket.c` | Non-blocking BSD/POSIX TCP & UDP socket port for Linux/macOS |
