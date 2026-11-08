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

## 8. Sensor Interface Drivers (`sensor/*.h`)

SyntropicOS provides zero-allocation drivers for common industrial and embedded sensors:

| Sensor Header | Target Hardware | Description |
|---|---|---|
| `sensor/syn_powermon.h` | INA219 / INA226 | Bus voltage, shunt current, and power monitoring |
| `sensor/syn_climate.h` | BME280 / DHT22 / SHT30 | Temperature, relative humidity, and barometric pressure |
| `sensor/syn_distance.h` | HC-SR04 / VL53L0X | Ultrasonic pulse timing and ToF laser distance |
| `sensor/syn_scale.h` | HX711 | 24-bit ADC load cell weight measurement and tare calibration |
| `sensor/syn_lux.h` | BH1750 / TSL2561 | Ambient light lux intensity reading |
| `sensor/syn_biometric.h` | MAX30102 | PPG optical pulse oximeter and heart rate monitoring |

---

## 9. USB 2.0 Device Core & Class Drivers (`drivers/syn_usb.h`, `syn_usb_cdc.h`, `syn_usb_hid.h`)

Provides a zero-heap USB 2.0 device core supporting pluggable class driver registration (CDC ACM, HID), automatic configuration descriptor table assembly, and protothread coroutine integration.

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

Provides a zero-heap USB 2.0 Host core engine with tick-driven enumeration state machine (attach detection, bus reset, descriptor reads, `SET_ADDRESS`, `SET_CONFIGURATION`), interface probing against registered host class drivers, and non-blocking protothread coroutines.

```c
#include <syntropic/drivers/syn_usb_host.h>
#include <syntropic/drivers/syn_usb_host_cdc.h>

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

## 11. Asynchronous I2C & SPI Transaction Queues (`drivers/syn_i2c_queue.h`, `drivers/syn_spi_queue.h`)

Provides non-blocking transaction queue managers for I2C and SPI peripherals supporting multi-client request queuing, automatic Chip Select (CS) GPIO toggling, target device parameter switching (clock rate and SPI mode), and non-blocking completion callbacks.

```c
#include <syntropic/drivers/syn_i2c_queue.h>
#include <syntropic/drivers/syn_spi_queue.h>

static SYN_SPI_Queue spi_q;

static void on_spi_done(uint8_t bus, SYN_Status result, void *user_data) {
    // Process transfer result
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





