# Drivers

Hardware abstraction drivers. Each is guarded by a `SYN_USE_*` config switch.

## GPIO & Digital I/O

| Module | Header | Config | Description |
|---|---|---|---|
| GPIO | `drivers/syn_gpio.h` | `SYN_USE_GPIO` | Pin control: init, read, write, toggle, bulk operations. `SYN_GPIO_Pin` is a `uint16_t` whose meaning is platform-specific (port+pin pair, flat number, etc.). Modes: input, output, input with pull-up/down, open-drain. |
| EXTI | `drivers/syn_exti.h` | `SYN_USE_EXTI` | GPIO interrupt dispatcher with per-pin callback registration |

## Serial

| Module | Header | Config | Description |
|---|---|---|---|
| UART | `drivers/syn_uart.h` | `SYN_USE_UART` | Buffered UART with ISR feed. Uses `syn_ringbuf` for TX/RX buffers. Configurable buffer sizes (`SYN_UART_TX_BUF_SIZE`, `SYN_UART_RX_BUF_SIZE`) and max instances (`SYN_UART_MAX_INSTANCES`). |
| CAN | `drivers/syn_can.h` | `SYN_USE_CAN` | CAN bus driver interface: init, transmit, receive, hardware filtering |

## Analog

| Module | Header | Config | Description |
|---|---|---|---|
| ADC | `drivers/syn_adc.h` | `SYN_USE_ADC` | ADC abstraction with raw, millivolt, and percent read helpers |
| DAC | `drivers/syn_dac.h` | `SYN_USE_DAC` | DAC driver with raw, millivolt, and percent write helpers |

## Bus Interfaces

| Module | Header | Config | Description |
|---|---|---|---|
| Software I2C | `drivers/syn_soft_i2c.h` | Always available | Software bit-banged I2C master driver |
| Software SPI | `drivers/syn_soft_spi.h` | Always available | Software bit-banged SPI master driver |
| I2C Device | `drivers/syn_i2c_dev.h` | Always available | Register-level I2C helper (header-only) — read/write register sequences |
| SPI Device | `drivers/syn_spi_dev.h` | Always available | Register-level SPI helper (header-only) — read/write register sequences |

## Storage & Timing

| Module | Header | Config | Description |
|---|---|---|---|
| SD Card | `drivers/syn_sd.h` | `SYN_USE_SD` | SPI-mode SD card block driver: init, sector read/write/sync. Supports SDSC, SDHC, and SDXC. |
| RTC | `drivers/syn_rtc.h` | `SYN_USE_RTC` | Real-time clock driver: get/set datetime, Unix epoch conversion |

## Other

| Module | Header | Config | Description |
|---|---|---|---|
| 1-Wire | `drivers/syn_soft_onewire.h` | `SYN_USE_ONEWIRE` | Bit-bang 1-Wire master: reset, byte read/write, ROM search. Suitable for DS18B20 temperature sensors. |
| Hardware WDT | `system/syn_hwwdt.h` | `SYN_USE_HWWDT` | Hardware watchdog timer: init and feed. Complements the software task-level watchdog in `syn_watchdog`. |
