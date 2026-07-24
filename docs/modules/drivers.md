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

## DMA & Acceleration

| Module | Header | Config | Description |
|---|---|---|---|
| DMA Engine | `drivers/syn_dma.h` | `SYN_USE_DMA` | Bare-metal safe DMA transaction engine with address alignment verification, D-cache coherency invalidation, and atomic busy protection |
| DMA Port | `port/syn_port_dma.h` | `SYN_USE_DMA` | Portable hardware DMA channel abstraction with completion callbacks |

Provides `init`, `start`, `stop`, `busy`, and `remaining` operations. The completion callback fires from ISR context — use `syn_workqueue_post()` to defer heavy processing to the main context.

```c
static void on_dma_done(uint8_t ch, SYN_Status result, void *ctx) {
    syn_workqueue_post(&wq, process_adc_data, ctx);
}

SYN_DMA_Config cfg = {
    .channel   = 0,
    .direction = SYN_DMA_PERIPH_TO_MEM,
    .width     = SYN_DMA_WIDTH_16,
    .src_incr  = false,   // Peripheral register (fixed)
    .dst_incr  = true,    // Memory buffer (incrementing)
    .callback  = on_dma_done,
    .user_data = &adc_buf,
};
syn_port_dma_init(&cfg);
syn_port_dma_start(0, &ADC_DR, adc_buf, 256);
```

## Async I2C / SPI

| Module | Header | Config | Description |
|---|---|---|---|
| Async I2C | `port/syn_port_i2c_async.h` | `SYN_USE_I2C_ASYNC` | Non-blocking I2C transactions with completion callback |
| Async SPI | `port/syn_port_spi_async.h` | `SYN_USE_SPI_ASYNC` | Non-blocking SPI transfers with completion callback |

Callback-based async alternatives to the existing blocking port APIs. The blocking `syn_port_i2c_*` and `syn_port_spi_*` functions remain unchanged — async is a separate opt-in.

```c
static void on_i2c_done(uint8_t bus, SYN_Status result, void *ctx) {
    // Data is ready in rx_data
}

uint8_t reg = 0xD0;
uint8_t chip_id;
SYN_I2C_Xfer xfer = {
    .bus = 0, .addr = 0x76,
    .tx_data = &reg, .tx_len = 1,
    .rx_data = &chip_id, .rx_len = 1,
    .callback = on_i2c_done,
};
syn_port_i2c_xfer_async(&xfer);
```

The transfer descriptor must remain valid until the callback fires. Callbacks fire from ISR context — use the workqueue for non-trivial processing.
