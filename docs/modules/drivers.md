# Hardware Peripheral Drivers

SyntropicOS provides portable hardware abstraction drivers for GPIO, UART, ADC, DAC, CAN, SPI, I2C, RTC, and DMA. Every driver is guarded by compile-time configuration switches (`SYN_USE_*`).

---

## Technical Specifications

| Feature | Specification |
|---|---|
| **Port Interface** | Hardware-independent wrapper calling `syn_port_*` interfaces. |
| **ISR Safety** | UART and DMA drivers use lock-free SPSC ring buffers for safe ISR-to-task transfers. |
| **Memory Allocation** | **100% Static / Zero Heap**. All driver instances are caller-owned structures. |

---

## Driver Dataflow Pipeline (UART + DMA Example)

```mermaid
flowchart LR
    HW["Hardware Peripheral (UART / ADC)"] -->|ISR / DMA Interrupt| RingBuf["SPSC Ring Buffer (syn_ringbuf)"]
    RingBuf -->|syn_uart_read| Task["Cooperative Protothread Task"]
    Task --> Processing["Process Byte Stream"]
```

---

## 1. GPIO & Digital I/O (`drivers/syn_gpio.h`)

Provides pin initialization, reading, writing, toggling, and mode configuration (Input, Output, Pull-Up, Pull-Down, Open-Drain).

```c
#include <syntropic/drivers/syn_gpio.h>

void gpio_demo(void) {
    // Initialize pin 13 as Output
    syn_gpio_init(13, SYN_GPIO_OUTPUT);
    
    // Toggle pin state
    syn_gpio_toggle(13);
    
    // Read input level
    SYN_GPIO_State state = syn_gpio_read(12);
}
```

---

## 2. Buffered Serial UART (`drivers/syn_uart.h`)

Buffered UART driver using lock-free SPSC ring buffers for high-speed RX/TX without data loss.

```c
#include <syntropic/drivers/syn_uart.h>

static SYN_UART uart;

void uart_setup(void) {
    // Initialize UART 1 at 115200 8N1 (TX/RX ring buffers embedded in SYN_UART)
    syn_uart_init(&uart, 1, 115200);
}

void USART1_IRQHandler(void) {
    // ISR feed: safe to call from interrupt context
    uint8_t rx_byte = (uint8_t)USART1->DR;
    syn_uart_rx_isr_feed(&uart, rx_byte);

    // ISR flush: drain TX ring buffer asynchronously
    syn_uart_tx_isr_flush(&uart);
}

void uart_send_nonblocking(const uint8_t *data, size_t len) {
    // Non-blocking bulk write into TX ring buffer
    if (syn_uart_write_async(&uart, data, len) == SYN_OK) {
        // Wait until tx_rb is empty AND hardware shift register is clear (RS485 DE safety)
        // syn_uart_tx_complete(&uart)
    }
}
```

---

## 3. Analog-to-Digital Converter (`drivers/syn_adc.h`)

Provides single-shot 12-bit raw conversion, calibrated millivolt reading (`mV`), and multi-channel SRAM background scanning via DMA.

```c
#include <syntropic/drivers/syn_adc.h>

static SYN_ADC adc0;

void adc_setup(void) {
    SYN_ADC_Config cfg = {
        .adc_id = 0,
        .channel_mask = (1U << 0),
        .vref_mv = 3300,
        .use_dma = false,
    };
    syn_adc_init(&adc0, &cfg);
}

void read_voltage(void) {
    uint16_t raw = syn_adc_read_raw(&adc0, 0);
    uint32_t millivolts = syn_adc_read_mv(&adc0, 0);
    printf("Channel 0: raw=%u, %lu mV\n", (unsigned int)raw, (unsigned long)millivolts);
}
```

---

## 4. DMA Transaction Engine (`drivers/syn_dma.h`)

Bare-metal safe DMA transaction engine featuring address alignment verification, D-cache invalidation, and atomic busy protection.

```c
#include <syntropic/drivers/syn_dma.h>

static SYN_DMA dma;

void on_dma_complete(SYN_DMA *dma_inst, SYN_DMA_Event event, void *ctx) {
    (void)dma_inst;
    (void)ctx;
    if ((event & SYN_DMA_EVENT_COMPLETE) != 0) {
        printf("DMA Transfer Complete!\n");
    }
}

void start_dma_transfer(const uint32_t *src, uint32_t *dst, size_t count) {
    SYN_DMA_Config cfg = {
        .channel_id = 0,
        .dir = SYN_DMA_DIR_MEM_TO_MEM,
        .data_size = SYN_DMA_SIZE_32BIT,
        .src_inc = true,
        .dst_inc = true,
        .callback = on_dma_complete,
        .user_ctx = NULL,
    };
    syn_dma_init(&dma, &cfg);
    syn_dma_start(&dma, (const void *)src, (void *)dst, count);
}
```

---

## 5. Controller Area Network (`drivers/syn_can.h`)

Provides CAN 2.0A/B frame transmission, reception filtering, and mailbox queuing.

```c
#include <syntropic/drivers/syn_can.h>

static SYN_CAN can;

void can_demo(void) {
    syn_can_init(&can, 0, 500000);

    SYN_CAN_Frame frame = {
        .id = 0x123,
        .extended = false,
        .dlc = 4,
        .data = {0x01, 0x02, 0x03, 0x04}
    };
    syn_can_send(&can, &frame);
}
```

---

## 6. External Interrupt Controller (`drivers/syn_exti.h`)

Configures pin edge-triggered interrupts (rising, falling, both) with ISR callback dispatch.

```c
#include <syntropic/drivers/syn_exti.h>

void on_pin_interrupt(SYN_GPIO_Pin pin, void *ctx) {
    (void)pin;
    (void)ctx;
    // Process ISR trigger
}

void exti_setup(void) {
    syn_exti_init();
    syn_exti_register(5, SYN_EXTI_RISING, on_pin_interrupt, NULL);
    syn_exti_enable(5);
}
```

---

## 7. Shift Register & I/O Expander (`drivers/syn_shiftreg.h` & `drivers/syn_ioexp.h`)

Supports 74HC595 output expansion, 74HC165 input reading, and MCP23017 / PCF8574 I2C GPIO expanders.

```c
#include <syntropic/drivers/syn_shiftreg.h>
#include <syntropic/drivers/syn_ioexp.h>

static SYN_ShiftRegOut sr;
static SYN_IOExp ioexp;

void expander_demo(void) {
    // 74HC595 Shift register write (1 cascaded chip on pins 2=DS, 3=SHCP, 4=STCP)
    syn_shiftreg_out_init(&sr, 2, 3, 4, 1);
    syn_shiftreg_out_write_byte(&sr, 0, 0xAA);
    syn_shiftreg_out_flush(&sr);

    // MCP23017 I2C GPIO Expander write (SCL=6, SDA=7, addr=0x20, pin 4 HIGH)
    syn_ioexp_init(&ioexp, 6, 7, 0x20, SYN_IOEXP_MCP23017);
    syn_ioexp_set_pin_mode(&ioexp, 4, SYN_GPIO_OUTPUT);
    syn_ioexp_write_pin(&ioexp, 4, SYN_GPIO_HIGH);
}
```

---

## 8. Sensor Framework, Attitude Fusion & Sensor Drivers (`sensor/*.h`)

SyntropicOS provides a periodic sensor polling framework, Q16.16 IMU attitude fusion, and zero-allocation drivers for industrial and environmental sensors:

| Sensor Header | Config Switch | Target Hardware / Scope | Description |
|---|---|---|---|
| `sensor/syn_sensor.h` | `SYN_USE_SENSOR` | Generic Polling Engine | Periodic read → moving-average / EMA filter → threshold & hysteresis → event callback (`SYN_Sensor`, `syn_sensor_init`, `syn_sensor_set_interval`) |
| `sensor/syn_sensor_fusion.h` | Always available | 6-DOF / 9-DOF IMU AHRS | Q16.16 fixed-point Complementary / Mahony attitude filter producing `SYN_Quaternion` and `SYN_EulerAngles` (`syn_sensor_fusion_update`, `syn_sensor_fusion_update_9dof`) |
| `sensor/syn_powermon.h` | `SYN_USE_POWERMON` | INA219 / INA226 / INA3221 | Bus voltage, shunt current, and power monitoring (`SYN_PowerMon`, `syn_powermon_feed_raw`) |
| `sensor/syn_climate.h` | `SYN_USE_CLIMATE` | SHT3x / BME280 / HTU21D / DHT22 | Temperature (`°C` / `°F`), relative humidity, and barometric pressure (`SYN_Climate`, `syn_climate_feed_sample`) |
| `sensor/syn_distance.h` | `SYN_USE_DISTANCE` | HC-SR04 / VL53L0X / Sharp IR | Ultrasonic pulse timing, ToF laser distance, and proximity thresholds (`SYN_Distance`, `syn_distance_get_mm`) |
| `sensor/syn_scale.h` | `SYN_USE_SCALE` | HX711 / NAU7802 | 24-bit ADC load cell weight measurement and tare calibration (`SYN_Scale`, `syn_scale_tare`, `syn_scale_get_grams`) |
| `sensor/syn_lux.h` | `SYN_USE_LUX` | BH1750 / TSL2561 / TCS34725 | Ambient illuminance (lux) and RGBC correlated color temperature (`SYN_Lux`, `syn_lux_get_color_temp_k`) |
| `sensor/syn_biometric.h` | `SYN_USE_BIOMETRIC` | MAX30102 / MAX30100 | Optical PPG heart-rate (BPM), SpO2 estimation, and finger presence detection (`SYN_Biometric`, `syn_biometric_feed_samples`) |

---

## 9. USB 2.0 Device Core & Class Drivers (`drivers/syn_usb.h`, `syn_usb_cdc.h`, `syn_usb_hid.h`)

Provides a zero-heap USB 2.0 device core supporting pluggable class driver registration, automatic configuration descriptor assembly, and protothread coroutine integration across CDC ACM, HID (including boot keyboard and mouse helpers), USB MIDI, Mass Storage (MSC Bulk-Only Transport), and `SYN_Transport` stream bindings.

| Header | Config Switch | Class / Role | Key Types & Functions |
|---|---|---|---|
| `drivers/syn_usb.h` | Always available | USB 2.0 Device Core | `SYN_USB_Device`, `SYN_USB_ClassDriver`, `syn_usb_init`, `syn_usb_register_class`, `syn_usb_alloc_ep` |
| `drivers/syn_usb_cdc.h` | `SYN_USE_USB_CDC` | CDC ACM Virtual COM Port | `SYN_USB_CDC`, `syn_usb_cdc_init`, `syn_usb_cdc_register`, `syn_usb_cdc_write`, `syn_usb_cdc_read` |
| `drivers/syn_transport_usb_cdc.h` | Always available | `SYN_Transport` Binding | `syn_transport_from_usb_cdc` — wraps `SYN_USB_CDC` as a `SYN_Transport` for CLI, RPC, or protocol engines |
| `drivers/syn_usb_hid.h` | Always available | Generic USB HID | `SYN_USB_HID`, `syn_usb_hid_init`, `syn_usb_hid_register`, `syn_usb_hid_send_report`, `syn_usb_hid_read_report` |
| `drivers/syn_usb_hid_keyboard.h` | Always available | HID Boot Keyboard Helper | `SYN_USB_HID_KeyboardReport`, `syn_usb_hid_keyboard_press`, `syn_usb_hid_keyboard_release`, `syn_usb_hid_keyboard_send` |
| `drivers/syn_usb_hid_mouse.h` | Always available | HID Boot Mouse Helper | `SYN_USB_HID_MouseReport`, `syn_usb_hid_mouse_move`, `syn_usb_hid_mouse_click`, `syn_usb_hid_mouse_send` |
| `drivers/syn_usb_midi.h` | `SYN_USE_USB_MIDI` | USB Audio/MIDI 1.0 | `SYN_USB_MIDI`, `syn_usb_midi_init`, `syn_usb_midi_register`, `syn_usb_midi_send_note_on`, `syn_usb_midi_send_cc` |
| `drivers/syn_usb_msc.h` | `SYN_USE_USB_MSC` | Mass Storage Class (BOT/SCSI) | `SYN_USB_MSC`, `SYN_USB_MSC_Media`, `syn_usb_msc_init`, `syn_usb_msc_set_media`, `syn_usb_msc_register` |

```c
#include <syntropic/drivers/syn_usb.h>
#include <syntropic/drivers/syn_usb_cdc.h>
#include <syntropic/drivers/syn_usb_hid.h>

static const uint8_t dev_desc[18] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0xFE, 0xCA, 0xEF, 0xBE, 0x00, 0x01, 0x01, 0x02,
    0x00, 0x01
};

static SYN_USB_Device usb_dev;
static SYN_USB_CDC    usb_cdc;

void usb_setup(void) {
    syn_usb_init(&usb_dev, dev_desc);
    syn_usb_cdc_init(&usb_cdc);
    syn_usb_cdc_register(&usb_dev, &usb_cdc);
}
```

---

## 10. USB 2.0 Host Core & Host CDC Class Driver (`drivers/syn_usb_host.h`, `syn_usb_host_cdc.h`)

Provides a zero-heap USB 2.0 Host core engine with a tick-driven enumeration state machine (attach detection, bus reset, descriptor reads, `SET_ADDRESS`, `SET_CONFIGURATION`), interface probing against registered host class drivers (`SYN_USB_HostClassDriver`), and `drivers/syn_transport_usb_host_cdc.h` (`syn_transport_from_usb_host_cdc`) to bind an attached USB CDC device directly to any `SYN_Transport` consumer.

```c
#include <syntropic/drivers/syn_usb_host.h>
#include <syntropic/drivers/syn_usb_host_cdc.h>
#include <syntropic/drivers/syn_transport_usb_host_cdc.h>

static SYN_USB_Host    host;
static SYN_USB_HostCDC host_cdc;

void host_setup(void) {
    syn_usb_host_init(&host);
    syn_usb_host_cdc_init(&host_cdc);
    syn_usb_host_cdc_register(&host, &host_cdc);
}

void host_task(void) {
    /* Called each scheduler tick */
    syn_usb_host_process(&host);
}
```

---

## 11. I2C, SPI & 1-Wire Bus Drivers (`syn_i2c*`, `syn_spi*`, `syn_soft_*`)

SyntropicOS provides synchronous Master/Slave bus drivers, thin register-access device wrappers, non-blocking multi-client transaction queues, and GPIO bit-banged bus masters for pin-constrained targets:

| Header | Config Switch | Description |
|---|---|---|
| `drivers/syn_i2c.h` | `SYN_USE_I2C` | Hardware-decoupled I2C Master/Slave driver (`SYN_I2C`, `syn_i2c_init`, `syn_i2c_transfer`, `syn_i2c_read_reg`, `syn_i2c_write_reg`) |
| `drivers/syn_i2c_dev.h` | Always available | Thin 8/16-bit register helper bound to `(bus, addr)` (`SYN_I2CDev`, `syn_i2c_dev_read8`, `syn_i2c_dev_write8`, `syn_i2c_dev_read16_be`) |
| `drivers/syn_i2c_queue.h` | `SYN_USE_I2C` | Non-blocking multi-client I2C transaction queue (`SYN_I2C_Queue`, `syn_i2c_queue_enqueue`, `syn_i2c_queue_process`) |
| `drivers/syn_soft_i2c.h` | `SYN_USE_SOFT_I2C` | Bit-banged open-drain I2C master over any two GPIO pins (`SYN_SoftI2C`, `syn_soft_i2c_init`, `syn_soft_i2c_write`, `syn_soft_i2c_read`) |
| `drivers/syn_spi.h` | `SYN_USE_SPI` | Hardware-decoupled full-duplex SPI Master/Slave driver (`SYN_SPI`, `syn_spi_init`, `syn_spi_transfer`, `syn_spi_write`, `syn_spi_read`) |
| `drivers/syn_spi_dev.h` | Always available | Thin SPI peripheral register helper with automatic CS control (`SYN_SPIDev`, `syn_spi_dev_read8`, `syn_spi_dev_read_burst`) |
| `drivers/syn_spi_queue.h` | `SYN_USE_SPI` | Non-blocking SPI transaction queue with per-device CS pin, clock rate, and mode switching (`SYN_SPI_Queue`, `syn_spi_queue_enqueue`) |
| `drivers/syn_soft_spi.h` | `SYN_USE_SOFT_SPI` | Bit-banged SPI master supporting modes 0–3 (`SYN_SoftSPI`, `syn_soft_spi_init`, `syn_soft_spi_transfer_bulk`) |
| `drivers/syn_soft_onewire.h` | `SYN_USE_ONEWIRE` | Bit-banged Dallas/Maxim 1-Wire master for DS18B20 and ROM-ID devices (`SYN_SoftOneWire`, `syn_soft_onewire_reset`, `syn_soft_onewire_write_rom`) |

```c
#include <syntropic/drivers/syn_i2c_queue.h>
#include <syntropic/drivers/syn_spi_queue.h>

static SYN_SPI_Queue spi_q;

static void on_spi_done(uint8_t bus, SYN_Status result, void *user_data) {
    (void)bus;
    (void)result;
    (void)user_data;
}

void spi_queue_demo(void) {
    syn_spi_queue_init(&spi_q, 0);

    uint8_t tx[4] = {0x9F, 0x00, 0x00, 0x00};
    uint8_t rx[4] = {0};

    SYN_SPI_Transaction xfer = {
        .bus = 0,
        .cs_pin = 10,
        .mode = SYN_SPI_MODE_0,
        .baudrate_hz = 1000000,
        .keep_cs_active = false,
        .tx_data = tx,
        .rx_data = rx,
        .len = 4,
        .callback = on_spi_done,
        .user_data = NULL
    };

    syn_spi_queue_enqueue(&spi_q, &xfer);
}
```

---

## 12. PWM, High-Resolution PWM, DAC & Analog Comparator

| Header | Config Switch | Description |
|---|---|---|
| `drivers/syn_pwm.h` | `SYN_USE_PWM` | Hardware PWM output driver (`SYN_PWM`, `syn_pwm_init`, `syn_pwm_set_duty`, `syn_pwm_set_duty_raw`, `syn_pwm_set_freq`, `syn_pwm_enable`) |
| `drivers/syn_hrpwm.h` | `SYN_USE_HRPWM` | High-resolution power-stage & motion PWM with sub-nanosecond duty, complementary dead-time (`ns`), and phase-shift control (`SYN_HRPWM`, `syn_hrpwm_set_duty_q16`, `syn_hrpwm_set_deadtime_ns`, `syn_hrpwm_set_phase_deg`) |
| `drivers/syn_dac.h` | `SYN_USE_DAC` | Digital-to-Analog Converter driver supporting raw counts, calibrated millivolts, and percentage output (`SYN_DAC`, `syn_dac_init`, `syn_dac_write_raw`, `syn_dac_write_mv`, `syn_dac_write_percent`) |
| `drivers/syn_comp.h` | `SYN_USE_COMP` | High-speed analog comparator driver for zero-crossing, over-current, and window threshold detection (`SYN_COMP`, `syn_comp_init`, `syn_comp_read`, `syn_comp_enable`) |

---

## 13. Clocks, Time Discipline, SD Block Storage & RFID

| Header | Config Switch | Description |
|---|---|---|
| `drivers/syn_rtc.h` | `SYN_USE_RTC` | Real-Time Clock calendar read/write, validity check, and Unix epoch conversion (`syn_rtc_init`, `syn_rtc_get`, `syn_rtc_set`, `syn_rtc_to_epoch`) |
| `drivers/syn_hpclock.h` | `SYN_USE_HPCLOCK` | 64-bit cycle-accurate monotonic timestamps resolved from CPU cycle counter (`DWT->CYCCNT` / `mcycle`) (`SYN_HPTimestamp`, `syn_hpclock_resolve`, `syn_hpclock_ticks_to_ns`) |
| `drivers/syn_timesync.h` | `SYN_USE_TIMESYNC` | PPS-disciplined UTC clock synchronization combining RTC/SNTP/GNSS coarse epoch with `syn_hpclock` nanosecond interpolation (`SYN_TimeSync`, `syn_timesync_bind_pps`, `syn_timesync_resolve_utc`, `syn_timesync_to_epoch_ns`) |
| `drivers/syn_sd.h` | `SYN_USE_SD` | SPI-mode SD / SDHC / SDXC 512-byte sector block driver backing `syn_fat` and `syn_lfs` (`SYN_SD`, `syn_sd_init`, `syn_sd_read`, `syn_sd_write`, `syn_sd_sectors`) |
| `drivers/syn_rfid.h` | `SYN_USE_RFID` | Contactless card & tag UID tracker supporting MFRC522 (13.56 MHz), PN532 NFC, and RDM6300 (125 kHz) (`SYN_RFID`, `syn_rfid_init`, `syn_rfid_feed_card`, `syn_rfid_is_card_present`) |





