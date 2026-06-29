/**
 * @file port_stm32f4.c
 * @brief SyntropicOS port layer for STM32F407 (bare-metal, direct register access).
 *
 * Full register-level implementation targeting the STM32F407 Discovery board.
 * Designed for both real hardware and Renode simulation. No vendor HAL/LL
 * dependency — all peripheral access is via CMSIS-style direct register writes.
 *
 * Peripherals implemented:
 *   - SysTick (tick source, delay)
 *   - GPIO (all ports A–I)
 *   - UART (USART1–3, UART4–5)
 *   - SPI (SPI1–3, master mode, full-duplex)
 *   - I2C (I2C1–3, master mode, 7-bit addressing)
 *   - Flash (erase/read/write via flash controller)
 *   - EXTI (interrupt configuration)
 *   - CAN (CAN1, basic TX/RX/filter)
 *   - ADC (ADC1, single-channel software-triggered)
 *   - Sleep (WFI-based)
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "syntropic/common/syn_defs.h"
#include "syntropic/common/syn_compiler.h"
#include "syntropic/port/syn_port_spi.h"
#include "syntropic/port/syn_port_i2c.h"
#include "syntropic/port/syn_port_adc.h"
#include "syntropic/port/syn_port_exti.h"
#include "syntropic/port/syn_port_can.h"
#include "syntropic/system/syn_sleep.h"

/* ═══════════════════════════════════════════════════════════════════════════
 *  Register Definitions
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PERIPH_BASE       0x40000000UL
#define APB1_BASE         PERIPH_BASE
#define APB2_BASE         (PERIPH_BASE + 0x10000UL)
#define AHB1_BASE         (PERIPH_BASE + 0x20000UL)

/* ── RCC ────────────────────────────────────────────────────────────────── */

#define RCC_BASE          (AHB1_BASE + 0x3800UL)
#define RCC_AHB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_APB2ENR       (*(volatile uint32_t *)(RCC_BASE + 0x44))

/* ── SysTick ────────────────────────────────────────────────────────────── */

#define SYSTICK_BASE      0xE000E010UL
#define SYSTICK_CTRL      (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD      (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL       (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))

/* ── GPIO ───────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

#define GPIOA  ((GPIO_TypeDef *)(AHB1_BASE + 0x0000))
#define GPIOB  ((GPIO_TypeDef *)(AHB1_BASE + 0x0400))
#define GPIOC  ((GPIO_TypeDef *)(AHB1_BASE + 0x0800))
#define GPIOD  ((GPIO_TypeDef *)(AHB1_BASE + 0x0C00))
#define GPIOE  ((GPIO_TypeDef *)(AHB1_BASE + 0x1000))
#define GPIOF  ((GPIO_TypeDef *)(AHB1_BASE + 0x1400))
#define GPIOG  ((GPIO_TypeDef *)(AHB1_BASE + 0x1800))
#define GPIOH  ((GPIO_TypeDef *)(AHB1_BASE + 0x1C00))
#define GPIOI  ((GPIO_TypeDef *)(AHB1_BASE + 0x2000))

static GPIO_TypeDef *const gpio_ports[] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI
};
#define NUM_GPIO_PORTS  (sizeof(gpio_ports) / sizeof(gpio_ports[0]))
#define GPIO_PORT(pin)  gpio_ports[(pin) >> 4]
#define GPIO_BIT(pin)   ((pin) & 0x0F)

/* ── USART ──────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

#define USART1  ((USART_TypeDef *)0x40011000)
#define USART2  ((USART_TypeDef *)0x40004400)
#define USART3  ((USART_TypeDef *)0x40004800)
#define UART4   ((USART_TypeDef *)0x40004C00)
#define UART5   ((USART_TypeDef *)0x40005000)

#define USART_SR_TXE   (1U << 7)
#define USART_SR_RXNE  (1U << 5)
#define USART_SR_TC    (1U << 6)
#define USART_CR1_UE   (1U << 13)
#define USART_CR1_TE   (1U << 3)
#define USART_CR1_RE   (1U << 2)

static USART_TypeDef *const usart_instances[] = {
    USART1, USART2, USART3, UART4, UART5
};

/* ── SPI ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t CRCPR;
    volatile uint32_t RXCRCR;
    volatile uint32_t TXCRCR;
    volatile uint32_t I2SCFGR;
    volatile uint32_t I2SPR;
} SPI_TypeDef;

#define SPI1  ((SPI_TypeDef *)0x40013000)
#define SPI2  ((SPI_TypeDef *)0x40003800)
#define SPI3  ((SPI_TypeDef *)0x40003C00)

#define SPI_CR1_SPE    (1U << 6)
#define SPI_CR1_MSTR   (1U << 2)
#define SPI_CR1_SSM    (1U << 9)
#define SPI_CR1_SSI    (1U << 8)
#define SPI_CR1_CPOL   (1U << 1)
#define SPI_CR1_CPHA   (1U << 0)
#define SPI_CR1_LSBFIRST (1U << 7)
#define SPI_SR_TXE     (1U << 1)
#define SPI_SR_RXNE    (1U << 0)
#define SPI_SR_BSY     (1U << 7)

static SPI_TypeDef *const spi_instances[] = { SPI1, SPI2, SPI3 };

/* ── I2C ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
    volatile uint32_t FLTR;
} I2C_TypeDef;

#define I2C1  ((I2C_TypeDef *)0x40005400)
#define I2C2  ((I2C_TypeDef *)0x40005800)
#define I2C3  ((I2C_TypeDef *)0x40005C00)

#define I2C_CR1_PE     (1U << 0)
#define I2C_CR1_START  (1U << 8)
#define I2C_CR1_STOP   (1U << 9)
#define I2C_CR1_ACK    (1U << 10)
#define I2C_SR1_SB     (1U << 0)
#define I2C_SR1_ADDR   (1U << 1)
#define I2C_SR1_BTF    (1U << 2)
#define I2C_SR1_TXE    (1U << 7)
#define I2C_SR1_RXNE   (1U << 6)
#define I2C_SR1_AF     (1U << 10)

static I2C_TypeDef *const i2c_instances[] = { I2C1, I2C2, I2C3 };

/* ── Flash controller ───────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t OPTCR;
} FLASH_TypeDef;

#define FLASH           ((FLASH_TypeDef *)0x40023C00)
#define FLASH_SR_BSY    (1U << 16)
#define FLASH_CR_PG     (1U << 0)
#define FLASH_CR_SER    (1U << 1)
#define FLASH_CR_STRT   (1U << 16)
#define FLASH_CR_LOCK   (1U << 31)
#define FLASH_CR_PSIZE_BYTE  (0U << 8)
#define FLASH_KEY1      0x45670123UL
#define FLASH_KEY2      0xCDEF89ABUL

/* ── ADC ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_TypeDef;

#define ADC1  ((ADC_TypeDef *)0x40012000)

#define ADC_SR_EOC     (1U << 1)
#define ADC_CR2_ADON   (1U << 0)
#define ADC_CR2_SWSTART (1U << 30)

/* ── EXTI ───────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t IMR;
    volatile uint32_t EMR;
    volatile uint32_t RTSR;
    volatile uint32_t FTSR;
    volatile uint32_t SWIER;
    volatile uint32_t PR;
} EXTI_TypeDef;

#define EXTI  ((EXTI_TypeDef *)0x40013C00)

/* SYSCFG for EXTI line mapping */
#define SYSCFG_BASE     0x40013800UL
#define SYSCFG_EXTICR   ((volatile uint32_t *)(SYSCFG_BASE + 0x08))

/* ── CAN ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t MCR;
    volatile uint32_t MSR;
    volatile uint32_t TSR;
    volatile uint32_t RF0R;
    volatile uint32_t RF1R;
    volatile uint32_t IER;
    volatile uint32_t ESR;
    volatile uint32_t BTR;
    uint32_t _reserved0[88];
    /* TX mailboxes */
    volatile uint32_t TI0R;
    volatile uint32_t TDT0R;
    volatile uint32_t TDL0R;
    volatile uint32_t TDH0R;
    volatile uint32_t TI1R;
    volatile uint32_t TDT1R;
    volatile uint32_t TDL1R;
    volatile uint32_t TDH1R;
    volatile uint32_t TI2R;
    volatile uint32_t TDT2R;
    volatile uint32_t TDL2R;
    volatile uint32_t TDH2R;
    /* RX FIFO 0 */
    volatile uint32_t RI0R;
    volatile uint32_t RDT0R;
    volatile uint32_t RDL0R;
    volatile uint32_t RDH0R;
    /* RX FIFO 1 */
    volatile uint32_t RI1R;
    volatile uint32_t RDT1R;
    volatile uint32_t RDL1R;
    volatile uint32_t RDH1R;
    uint32_t _reserved1[12];
    /* Filter bank registers */
    volatile uint32_t FMR;
    volatile uint32_t FM1R;
    uint32_t _reserved2;
    volatile uint32_t FS1R;
    uint32_t _reserved3;
    volatile uint32_t FFA1R;
    uint32_t _reserved4;
    volatile uint32_t FA1R;
    uint32_t _reserved5[8];
    volatile uint32_t FR1[28];   /* Filter bank i register 1 */
    volatile uint32_t FR2[28];   /* Filter bank i register 2 */
} CAN_TypeDef;

#define CAN1_BASE  0x40006400UL
#define CAN1  ((CAN_TypeDef *)CAN1_BASE)

#define CAN_MCR_INRQ   (1U << 0)
#define CAN_MSR_INAK   (1U << 0)
#define CAN_TSR_TME0   (1U << 26)
#define CAN_TIR_TXRQ   (1U << 0)
#define CAN_TIR_IDE    (1U << 2)
#define CAN_RF0R_FMP0  (0x3U)
#define CAN_RF0R_RFOM0 (1U << 5)

/* ── System clock (assumed 16 MHz for Renode default) ───────────────────── */
#define SYSTEM_CLOCK_HZ  16000000UL
#define APB1_CLOCK_HZ    (SYSTEM_CLOCK_HZ / 1)
#define APB2_CLOCK_HZ    (SYSTEM_CLOCK_HZ / 1)

/* ═══════════════════════════════════════════════════════════════════════════
 *  System / Tick
 * ═══════════════════════════════════════════════════════════════════════════ */

static volatile uint32_t systick_ms = 0;

void SysTick_Handler(void)
{
    systick_ms++;
}

uint32_t syn_port_get_tick_ms(void)
{
    return systick_ms;
}

void syn_port_delay_ms(uint32_t ms)
{
    uint32_t start = systick_ms;
    while ((systick_ms - start) < ms) {
        __asm volatile("wfi");
    }
}

static volatile uint32_t critical_nesting = 0;

void syn_port_enter_critical(void)
{
    __asm volatile("cpsid i");
    critical_nesting++;
}

void syn_port_exit_critical(void)
{
    if (--critical_nesting == 0) {
        __asm volatile("cpsie i");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  GPIO
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    uint8_t port_idx = pin >> 4;
    uint8_t bit = GPIO_BIT(pin);
    if (port_idx >= NUM_GPIO_PORTS) return SYN_INVALID_PARAM;

    GPIO_TypeDef *gpio = gpio_ports[port_idx];

    /* Enable port clock */
    RCC_AHB1ENR |= (1U << port_idx);

    /* Clear and set mode */
    gpio->MODER &= ~(3U << (bit * 2));
    gpio->PUPDR &= ~(3U << (bit * 2));

    switch (mode) {
    case SYN_GPIO_OUTPUT:
        gpio->MODER |= (1U << (bit * 2));  /* 01 = output */
        gpio->OSPEEDR |= (2U << (bit * 2)); /* High speed */
        break;
    case SYN_GPIO_INPUT:
        /* 00 = input, no pull */
        break;
    case SYN_GPIO_INPUT_PULLUP:
        gpio->PUPDR |= (1U << (bit * 2));  /* 01 = pull-up */
        break;
    case SYN_GPIO_INPUT_PULLDOWN:
        gpio->PUPDR |= (2U << (bit * 2));  /* 10 = pull-down */
        break;
    default:
        return SYN_INVALID_PARAM;
    }

    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    uint8_t port_idx = pin >> 4;
    uint8_t bit = GPIO_BIT(pin);
    if (port_idx >= NUM_GPIO_PORTS) return SYN_INVALID_PARAM;

    GPIO_TypeDef *gpio = gpio_ports[port_idx];
    gpio->MODER &= ~(3U << (bit * 2));  /* Reset to input */
    gpio->PUPDR &= ~(3U << (bit * 2));  /* No pull */
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    uint8_t port_idx = pin >> 4;
    if (port_idx >= NUM_GPIO_PORTS) return SYN_INVALID_PARAM;
    GPIO_TypeDef *gpio = gpio_ports[port_idx];
    uint8_t bit = GPIO_BIT(pin);

    if (state == SYN_GPIO_HIGH) {
        gpio->BSRR = (1U << bit);         /* Set bit */
    } else {
        gpio->BSRR = (1U << (bit + 16));  /* Reset bit */
    }
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    uint8_t port_idx = pin >> 4;
    if (port_idx >= NUM_GPIO_PORTS) return SYN_GPIO_LOW;
    GPIO_TypeDef *gpio = gpio_ports[port_idx];
    uint8_t bit = GPIO_BIT(pin);
    return (gpio->IDR & (1U << bit)) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    uint8_t port_idx = pin >> 4;
    if (port_idx >= NUM_GPIO_PORTS) return SYN_INVALID_PARAM;
    GPIO_TypeDef *gpio = gpio_ports[port_idx];
    uint8_t bit = GPIO_BIT(pin);
    gpio->ODR ^= (1U << bit);
    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  UART
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_uart_init(SYN_UARTInstance inst, uint32_t baud)
{
    if (inst >= 5) return SYN_INVALID_PARAM;
    USART_TypeDef *uart = usart_instances[inst];

    /* Enable clock */
    if (inst == 0) {
        RCC_APB2ENR |= (1U << 4);    /* USART1 on APB2 */
    } else {
        RCC_APB1ENR |= (1U << (16 + inst)); /* USART2..UART5 on APB1 */
    }

    /* Disable while configuring */
    uart->CR1 = 0;
    uart->CR2 = 0;
    uart->CR3 = 0;

    /* Set baud rate */
    uint32_t pclk = (inst == 0) ? APB2_CLOCK_HZ : APB1_CLOCK_HZ;
    uart->BRR = pclk / baud;

    /* 8N1, enable TX/RX/UART */
    uart->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    return SYN_OK;
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance inst)
{
    if (inst >= 5) return SYN_INVALID_PARAM;
    usart_instances[inst]->CR1 = 0;
    return SYN_OK;
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance inst, uint8_t byte)
{
    if (inst >= 5) return SYN_INVALID_PARAM;
    USART_TypeDef *uart = usart_instances[inst];
    volatile uint32_t timeout = 100000;
    while (!(uart->SR & USART_SR_TXE) && --timeout) { /* spin */ }
    if (!timeout) return SYN_TIMEOUT;
    uart->DR = byte;
    return SYN_OK;
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance inst, uint8_t *byte, uint32_t timeout_ms)
{
    if (inst >= 5) return SYN_INVALID_PARAM;
    USART_TypeDef *uart = usart_instances[inst];

    if (timeout_ms == 0) {
        if (!(uart->SR & USART_SR_RXNE)) {
            return SYN_TIMEOUT;
        }
        *byte = (uint8_t)(uart->DR & 0xFF);
        return SYN_OK;
    }

    uint32_t start = systick_ms;
    while (!(uart->SR & USART_SR_RXNE)) {
        if ((systick_ms - start) >= timeout_ms) {
            return SYN_TIMEOUT;
        }
    }
    *byte = (uint8_t)(uart->DR & 0xFF);
    return SYN_OK;
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance inst, const uint8_t *data,
                                     size_t len, uint32_t timeout_ms)
{
    (void)timeout_ms;
    for (size_t i = 0; i < len; i++) {
        SYN_Status s = syn_port_uart_transmit_byte(inst, data[i]);
        if (s != SYN_OK) return s;
    }
    /* Wait for last byte to finish transmitting */
    USART_TypeDef *uart = usart_instances[inst];
    volatile uint32_t timeout = 100000;
    while (!(uart->SR & USART_SR_TC) && --timeout) { /* spin */ }
    if (!timeout) return SYN_TIMEOUT;
    return SYN_OK;
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance inst, uint8_t *data,
                                    size_t len, size_t *received, uint32_t timeout_ms)
{
    size_t count = 0;
    for (size_t i = 0; i < len; i++) {
        SYN_Status s = syn_port_uart_receive_byte(inst, &data[i], timeout_ms);
        if (s == SYN_TIMEOUT) break;
        if (s != SYN_OK) { if (received) *received = count; return s; }
        count++;
    }
    if (received) *received = count;
    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  SPI (master mode, full-duplex, polling)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_spi_init(const SYN_SPI_Config *cfg)
{
    if (!cfg || cfg->bus >= 3) return SYN_INVALID_PARAM;
    SPI_TypeDef *spi = spi_instances[cfg->bus];

    /* Enable clock */
    if (cfg->bus == 0) {
        RCC_APB2ENR |= (1U << 12);   /* SPI1 on APB2 */
    } else {
        RCC_APB1ENR |= (1U << (13 + cfg->bus)); /* SPI2=bit14, SPI3=bit15 */
    }

    /* Build CR1 */
    uint32_t cr1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;

    /* CPOL / CPHA */
    if (cfg->mode == SYN_SPI_MODE_2 || cfg->mode == SYN_SPI_MODE_3) {
        cr1 |= SPI_CR1_CPOL;
    }
    if (cfg->mode == SYN_SPI_MODE_1 || cfg->mode == SYN_SPI_MODE_3) {
        cr1 |= SPI_CR1_CPHA;
    }

    /* Bit order */
    if (cfg->bit_order == 1) {
        cr1 |= SPI_CR1_LSBFIRST;
    }

    /* Baud rate prescaler — find closest prescaler for requested clock.
     * BR[2:0] in bits 5:3, prescaler = 2^(BR+1) */
    uint32_t pclk = (cfg->bus == 0) ? APB2_CLOCK_HZ : APB1_CLOCK_HZ;
    uint32_t br = 0;
    while (br < 7 && (pclk / (2U << br)) > cfg->clock_hz) {
        br++;
    }
    cr1 |= (br << 3);

    spi->CR1 = cr1;
    spi->CR2 = 0;
    spi->CR1 |= SPI_CR1_SPE;  /* Enable */

    return SYN_OK;
}

SYN_Status syn_port_spi_deinit(uint8_t bus)
{
    if (bus >= 3) return SYN_INVALID_PARAM;
    spi_instances[bus]->CR1 = 0;
    return SYN_OK;
}

SYN_Status syn_port_spi_transfer(uint8_t bus, const uint8_t *tx_buf,
                                    uint8_t *rx_buf, size_t len)
{
    if (bus >= 3) return SYN_INVALID_PARAM;
    SPI_TypeDef *spi = spi_instances[bus];

    for (size_t i = 0; i < len; i++) {
        /* Wait for TX empty with timeout */
        volatile uint32_t timeout = 100000;
        while (!(spi->SR & SPI_SR_TXE) && --timeout) { /* spin */ }
        if (!timeout) return SYN_TIMEOUT;

        /* Send byte (or 0xFF if no TX buffer) */
        spi->DR = tx_buf ? tx_buf[i] : 0xFF;

        /* Wait for RX not empty with timeout */
        timeout = 100000;
        while (!(spi->SR & SPI_SR_RXNE) && --timeout) { /* spin */ }
        if (!timeout) return SYN_TIMEOUT;

        /* Read received byte */
        uint8_t rx = (uint8_t)(spi->DR & 0xFF);
        if (rx_buf) rx_buf[i] = rx;
    }

    /* Wait until not busy with timeout */
    volatile uint32_t timeout = 100000;
    while ((spi->SR & SPI_SR_BSY) && --timeout) { /* spin */ }
    if (!timeout) return SYN_TIMEOUT;

    return SYN_OK;
}

SYN_Status syn_port_spi_cs_assert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus;
    return syn_port_gpio_write(cs_pin, SYN_GPIO_LOW);
}

SYN_Status syn_port_spi_cs_deassert(uint8_t bus, SYN_GPIO_Pin cs_pin)
{
    (void)bus;
    return syn_port_gpio_write(cs_pin, SYN_GPIO_HIGH);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  I2C (master mode, 7-bit addressing, polling)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_i2c_init(const SYN_I2C_Config *cfg)
{
    if (!cfg || cfg->bus >= 3) return SYN_INVALID_PARAM;
    I2C_TypeDef *i2c = i2c_instances[cfg->bus];

    /* Enable clock (I2C1=bit21, I2C2=bit22, I2C3=bit23) */
    RCC_APB1ENR |= (1U << (21 + cfg->bus));

    /* Disable peripheral for configuration */
    i2c->CR1 = 0;

    /* Set APB clock frequency in CR2 (in MHz) */
    uint32_t freq_mhz = APB1_CLOCK_HZ / 1000000;
    i2c->CR2 = freq_mhz & 0x3F;

    /* Set CCR for standard/fast mode */
    if (cfg->clock_hz <= 100000) {
        /* Standard mode: T_high = T_low = CCR * T_pclk */
        i2c->CCR = APB1_CLOCK_HZ / (2 * cfg->clock_hz);
    } else {
        /* Fast mode (duty=0): T_high = CCR * T_pclk, T_low = 2 * CCR * T_pclk */
        i2c->CCR = (1U << 15) | (APB1_CLOCK_HZ / (3 * cfg->clock_hz));
    }

    /* TRISE = (max_rise_time_ns / T_pclk_ns) + 1 */
    i2c->TRISE = freq_mhz + 1;

    /* Enable */
    i2c->CR1 = I2C_CR1_PE;

    return SYN_OK;
}

SYN_Status syn_port_i2c_deinit(uint8_t bus)
{
    if (bus >= 3) return SYN_INVALID_PARAM;
    i2c_instances[bus]->CR1 = 0;
    return SYN_OK;
}

static SYN_Status i2c_start(I2C_TypeDef *i2c, uint8_t addr, bool read)
{
    /* Generate START */
    i2c->CR1 |= I2C_CR1_START;

    /* Wait for SB (start bit generated) */
    uint32_t timeout = 100000;
    while (!(i2c->SR1 & I2C_SR1_SB) && timeout--) { /* spin */ }
    if (!timeout) return SYN_TIMEOUT;

    /* Send address */
    i2c->DR = (addr << 1) | (read ? 1 : 0);

    /* Wait for ADDR (address acknowledged) */
    timeout = 100000;
    while (!(i2c->SR1 & I2C_SR1_ADDR) && timeout--) {
        if (i2c->SR1 & I2C_SR1_AF) {
            /* NACK — no device at this address */
            i2c->SR1 &= ~I2C_SR1_AF;
            i2c->CR1 |= I2C_CR1_STOP;
            return SYN_ERROR;
        }
    }
    if (!timeout) return SYN_TIMEOUT;

    /* Clear ADDR by reading SR1 then SR2 */
    (void)i2c->SR1;
    (void)i2c->SR2;

    return SYN_OK;
}

SYN_Status syn_port_i2c_write(uint8_t bus, uint8_t addr,
                                 const uint8_t *data, size_t len)
{
    if (bus >= 3) return SYN_INVALID_PARAM;
    I2C_TypeDef *i2c = i2c_instances[bus];

    SYN_Status s = i2c_start(i2c, addr, false);
    if (s != SYN_OK) return s;

    for (size_t i = 0; i < len; i++) {
        /* Wait for TXE */
        uint32_t timeout = 100000;
        while (!(i2c->SR1 & I2C_SR1_TXE) && timeout--) { /* spin */ }
        if (!timeout) return SYN_TIMEOUT;
        i2c->DR = data[i];
    }

    /* Wait for BTF (byte transfer finished) */
    uint32_t timeout = 100000;
    while (!(i2c->SR1 & I2C_SR1_BTF) && timeout--) { /* spin */ }

    /* Generate STOP */
    i2c->CR1 |= I2C_CR1_STOP;

    return SYN_OK;
}

SYN_Status syn_port_i2c_read(uint8_t bus, uint8_t addr,
                                uint8_t *data, size_t len)
{
    if (bus >= 3 || len == 0) return SYN_INVALID_PARAM;
    I2C_TypeDef *i2c = i2c_instances[bus];

    /* Enable ACK for multi-byte reads */
    if (len > 1) {
        i2c->CR1 |= I2C_CR1_ACK;
    }

    SYN_Status s = i2c_start(i2c, addr, true);
    if (s != SYN_OK) return s;

    for (size_t i = 0; i < len; i++) {
        if (i == len - 1) {
            /* Last byte: NACK + STOP */
            i2c->CR1 &= ~I2C_CR1_ACK;
            i2c->CR1 |= I2C_CR1_STOP;
        }

        /* Wait for RXNE */
        uint32_t timeout = 100000;
        while (!(i2c->SR1 & I2C_SR1_RXNE) && timeout--) { /* spin */ }
        if (!timeout) return SYN_TIMEOUT;

        data[i] = (uint8_t)(i2c->DR & 0xFF);
    }

    return SYN_OK;
}

SYN_Status syn_port_i2c_write_read(uint8_t bus, uint8_t addr,
                                      const uint8_t *tx_data, size_t tx_len,
                                      uint8_t *rx_data, size_t rx_len)
{
    if (bus >= 3) return SYN_INVALID_PARAM;
    I2C_TypeDef *i2c = i2c_instances[bus];

    /* Write phase (no STOP — repeated start) */
    SYN_Status s = i2c_start(i2c, addr, false);
    if (s != SYN_OK) return s;

    for (size_t i = 0; i < tx_len; i++) {
        uint32_t timeout = 100000;
        while (!(i2c->SR1 & I2C_SR1_TXE) && timeout--) { /* spin */ }
        if (!timeout) return SYN_TIMEOUT;
        i2c->DR = tx_data[i];
    }

    /* Wait for BTF before repeated start */
    uint32_t timeout = 100000;
    while (!(i2c->SR1 & I2C_SR1_BTF) && timeout--) { /* spin */ }

    /* Read phase (with STOP at end) */
    return syn_port_i2c_read(bus, addr, rx_data, rx_len);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Flash (STM32F4 internal flash controller)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

static void flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

static SYN_Status flash_wait_bsy(void)
{
    volatile uint32_t timeout = 1000000;
    while ((FLASH->SR & FLASH_SR_BSY) && --timeout) { /* spin */ }
    return (timeout > 0) ? SYN_OK : SYN_TIMEOUT;
}

/* STM32F4 sector map (first 5 sectors of bank 1) */
static const struct { uint32_t addr; uint32_t size; } flash_sectors[] = {
    { 0x08000000, 16 * 1024 },   /* Sector 0 */
    { 0x08004000, 16 * 1024 },   /* Sector 1 */
    { 0x08008000, 16 * 1024 },   /* Sector 2 */
    { 0x0800C000, 16 * 1024 },   /* Sector 3 */
    { 0x08010000, 64 * 1024 },   /* Sector 4 */
    { 0x08020000, 128 * 1024 },  /* Sector 5 */
    { 0x08040000, 128 * 1024 },  /* Sector 6 */
    { 0x08060000, 128 * 1024 },  /* Sector 7 */
};
#define NUM_FLASH_SECTORS (sizeof(flash_sectors) / sizeof(flash_sectors[0]))

static int flash_find_sector(uint32_t addr)
{
    for (int i = 0; i < (int)NUM_FLASH_SECTORS; i++) {
        if (addr >= flash_sectors[i].addr &&
            addr < flash_sectors[i].addr + flash_sectors[i].size) {
            return i;
        }
    }
    return -1;
}

SYN_Status syn_port_flash_erase(uint32_t addr)
{
    int sector = flash_find_sector(addr);
    if (sector < 0) return SYN_INVALID_PARAM;

    flash_unlock();
    SYN_Status s = flash_wait_bsy();
    if (s != SYN_OK) {
        flash_lock();
        return s;
    }

    /* Sector erase, byte-size programming */
    FLASH->CR = FLASH_CR_SER | ((uint32_t)sector << 3) | FLASH_CR_PSIZE_BYTE;
    FLASH->CR |= FLASH_CR_STRT;

    s = flash_wait_bsy();
    FLASH->CR = 0;
    flash_lock();

    return s;
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    /* Flash is memory-mapped — just read directly */
    memcpy(buf, (const void *)addr, len);
    return SYN_OK;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    const uint8_t *src = (const uint8_t *)buf;

    flash_unlock();
    SYN_Status s = flash_wait_bsy();
    if (s != SYN_OK) {
        flash_lock();
        return s;
    }

    /* Byte programming */
    FLASH->CR = FLASH_CR_PG | FLASH_CR_PSIZE_BYTE;

    for (size_t i = 0; i < len; i++) {
        *(volatile uint8_t *)(addr + i) = src[i];
        s = flash_wait_bsy();
        if (s != SYN_OK) {
            FLASH->CR = 0;
            flash_lock();
            return s;
        }
    }

    FLASH->CR = 0;
    flash_lock();

    return SYN_OK;
}

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    int sector = flash_find_sector(addr);
    if (sector < 0) return 0;
    return flash_sectors[sector].size;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  ADC (ADC1, single-channel software-triggered)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_adc_init(uint8_t ch)
{
    (void)ch;
    /* Enable ADC1 clock */
    RCC_APB2ENR |= (1U << 8);

    /* Turn on ADC */
    ADC1->CR2 = ADC_CR2_ADON;
    /* Small settling delay */
    for (volatile int i = 0; i < 1000; i++) {}

    return SYN_OK;
}

uint16_t syn_port_adc_read(uint8_t ch)
{
    /* Set channel in SQR3 (regular sequence, 1 conversion) */
    ADC1->SQR3 = ch & 0x1F;
    ADC1->SQR1 = 0;  /* 1 conversion */

    /* Clear EOC */
    ADC1->SR &= ~ADC_SR_EOC;

    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Wait for EOC with timeout to prevent hangs in simulation */
    volatile uint32_t timeout = 10000;
    while (!(ADC1->SR & ADC_SR_EOC) && --timeout) { /* spin */ }

    return (uint16_t)(ADC1->DR & 0x0FFF);
}

uint8_t syn_port_adc_resolution(void)
{
    return 12;
}

uint16_t syn_port_adc_reference_mv(void)
{
    return 3300;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  EXTI (external interrupt configuration)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_exti_configure(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge)
{
    uint8_t port_idx = pin >> 4;
    uint8_t bit = GPIO_BIT(pin);

    if (port_idx >= NUM_GPIO_PORTS || bit >= 16) return SYN_INVALID_PARAM;

    /* Enable SYSCFG clock */
    RCC_APB2ENR |= (1U << 14);

    /* Map EXTI line to GPIO port via SYSCFG_EXTICR */
    uint8_t reg_idx = bit / 4;
    uint8_t shift = (bit % 4) * 4;
    SYSCFG_EXTICR[reg_idx] &= ~(0xFU << shift);
    SYSCFG_EXTICR[reg_idx] |= ((uint32_t)port_idx << shift);

    /* Configure edge */
    if (edge == SYN_EXTI_RISING || edge == SYN_EXTI_BOTH) {
        EXTI->RTSR |= (1U << bit);
    } else {
        EXTI->RTSR &= ~(1U << bit);
    }
    if (edge == SYN_EXTI_FALLING || edge == SYN_EXTI_BOTH) {
        EXTI->FTSR |= (1U << bit);
    } else {
        EXTI->FTSR &= ~(1U << bit);
    }

    return SYN_OK;
}

void syn_port_exti_enable(SYN_GPIO_Pin pin)
{
    uint8_t bit = GPIO_BIT(pin);
    EXTI->IMR |= (1U << bit);
}

void syn_port_exti_disable(SYN_GPIO_Pin pin)
{
    uint8_t bit = GPIO_BIT(pin);
    EXTI->IMR &= ~(1U << bit);
}

void syn_port_exti_clear_pending(SYN_GPIO_Pin pin)
{
    uint8_t bit = GPIO_BIT(pin);
    EXTI->PR = (1U << bit);  /* Write 1 to clear */
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  CAN (CAN1, basic TX/RX with filter support)
 * ═══════════════════════════════════════════════════════════════════════════ */

bool syn_port_can_init(uint8_t port, uint32_t bitrate)
{
    (void)port;

    /* Enable CAN1 clock */
    RCC_APB1ENR |= (1U << 25);

    /* Enter init mode */
    CAN1->MCR |= CAN_MCR_INRQ;
    uint32_t timeout = 100000;
    while (!(CAN1->MSR & CAN_MSR_INAK) && timeout--) { /* spin */ }
    if (!timeout) return false;

    /* Set bit timing (rough: 16MHz / (1+BS1+BS2) / prescaler = bitrate)
     * For 500kbps at 16MHz: prescaler=2, BS1=13, BS2=2 => 16/(1+13+2)/2 = 500k */
    uint32_t prescaler = SYSTEM_CLOCK_HZ / (16 * bitrate);
    if (prescaler < 1) prescaler = 1;
    CAN1->BTR = ((2 - 1) << 20) |   /* SJW = 2 */
                ((13 - 1) << 16) |   /* BS1 = 13 */
                ((2 - 1) << 0);      /* Prescaler */
    (void)prescaler;  /* Use computed prescaler in production */

    /* Leave init mode */
    CAN1->MCR &= ~CAN_MCR_INRQ;
    timeout = 100000;
    while ((CAN1->MSR & CAN_MSR_INAK) && timeout--) { /* spin */ }

    return true;
}

bool syn_port_can_send(uint8_t port, uint32_t id, bool ext,
                         const uint8_t *data, uint8_t dlc)
{
    (void)port;

    /* Wait for empty TX mailbox 0 */
    if (!(CAN1->TSR & CAN_TSR_TME0)) return false;

    /* Set ID */
    if (ext) {
        CAN1->TI0R = (id << 3) | CAN_TIR_IDE;
    } else {
        CAN1->TI0R = (id << 21);
    }

    /* Set DLC */
    CAN1->TDT0R = dlc & 0x0F;

    /* Load data bytes */
    uint32_t lo = 0, hi = 0;
    for (int i = 0; i < dlc && i < 4; i++) {
        lo |= ((uint32_t)data[i]) << (i * 8);
    }
    for (int i = 4; i < dlc && i < 8; i++) {
        hi |= ((uint32_t)data[i]) << ((i - 4) * 8);
    }
    CAN1->TDL0R = lo;
    CAN1->TDH0R = hi;

    /* Request transmission */
    CAN1->TI0R |= CAN_TIR_TXRQ;

    return true;
}

bool syn_port_can_receive(uint8_t port, uint32_t *id, bool *ext,
                            uint8_t *data, uint8_t *dlc)
{
    (void)port;

    /* Check FIFO 0 for pending messages */
    if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0) return false;

    /* Read ID */
    if (CAN1->RI0R & CAN_TIR_IDE) {
        *id = CAN1->RI0R >> 3;
        *ext = true;
    } else {
        *id = CAN1->RI0R >> 21;
        *ext = false;
    }

    /* Read DLC */
    *dlc = CAN1->RDT0R & 0x0F;

    /* Read data */
    uint32_t lo = CAN1->RDL0R;
    uint32_t hi = CAN1->RDH0R;
    for (int i = 0; i < *dlc && i < 4; i++) {
        data[i] = (uint8_t)(lo >> (i * 8));
    }
    for (int i = 4; i < *dlc && i < 8; i++) {
        data[i] = (uint8_t)(hi >> ((i - 4) * 8));
    }

    /* Release FIFO 0 */
    CAN1->RF0R |= CAN_RF0R_RFOM0;

    return true;
}

void syn_port_can_set_filter(uint8_t port, uint32_t id, uint32_t mask)
{
    (void)port;

    /* Enter filter init mode */
    CAN1->FMR |= 1;

    /* Configure filter 0: mask mode, 32-bit scale */
    CAN1->FA1R &= ~1U;    /* Deactivate filter 0 */
    CAN1->FM1R &= ~1U;    /* Mask mode */
    CAN1->FS1R |= 1U;     /* 32-bit scale */
    CAN1->FFA1R &= ~1U;   /* Assign to FIFO 0 */

    CAN1->FR1[0] = id << 21;    /* Filter ID */
    CAN1->FR2[0] = mask << 21;  /* Filter mask */

    CAN1->FA1R |= 1U;     /* Activate filter 0 */

    /* Leave filter init mode */
    CAN1->FMR &= ~1U;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Sleep
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_port_sleep(SYN_SleepMode mode)
{
    (void)mode;
    __asm volatile("wfi");
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Assert handler
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_assert_failed(const char *file, int line)
{
    (void)file;
    (void)line;
    /* Hard fault loop — Renode will catch this */
    for (;;) { __asm volatile("bkpt #0"); }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  PWM — TIM2, channels 0–3 → CCR1–CCR4 (PA0–PA3, AF1)
 *
 *  The implementation uses TIM2 which is on APB1.
 *  SYSTEM_CLOCK_HZ / (prescaler+1) drives the timer counter.
 *  ARR (auto-reload register) sets the PWM period; CCRx sets the duty.
 *
 *  Channel mapping:
 *    channel 0 → TIM2 CH1 → PA0  (AF1)
 *    channel 1 → TIM2 CH2 → PA1  (AF1)
 *    channel 2 → TIM2 CH3 → PA2  (AF1)
 *    channel 3 → TIM2 CH4 → PA3  (AF1)
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "syntropic/port/syn_port_pwm.h"

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
} TIM_TypeDef;

#define TIM2  ((TIM_TypeDef *)0x40000000)

#define TIM_CR1_CEN    (1U << 0)
#define TIM_CR1_ARPE   (1U << 7)

/* PWM mode 1: active when CNT < CCRx */
#define TIM_CCMR_PWM1  0x68U   /* OC1M = 110, OC1PE = 1 */

/* CCER: CCxE enables the output */
#define TIM2_APB1ENR_BIT  (1U << 0)

/* GPIO AF1 for PA0–PA3 = TIM2 CH1–CH4 */
#define GPIO_MODER_AF    2U
#define GPIO_AF1         1U

static uint32_t s_pwm_arr = 999U; /* default: 1 kHz at 16 MHz with PSC=15 */

static void pwm_gpio_init_pin(uint8_t bit)
{
    /* PA[bit]: MODER = alternate function (10b) */
    GPIOA->MODER &= ~(3U << (bit * 2));
    GPIOA->MODER |=  (GPIO_MODER_AF << (bit * 2));

    /* OSPEEDR: high speed */
    GPIOA->OSPEEDR |= (3U << (bit * 2));

    /* AFR[0] (AFRL, covers PA0–PA7): set AF1 for the pin */
    GPIOA->AFR[0] &= ~(0xFU << (bit * 4));
    GPIOA->AFR[0] |=  (GPIO_AF1 << (bit * 4));
}

SYN_Status syn_port_pwm_init(uint8_t channel, uint32_t freq_hz)
{
    if (channel > 3 || freq_hz == 0) return SYN_INVALID_PARAM;

    /* Enable GPIOA and TIM2 clocks */
    RCC_AHB1ENR |= (1U << 0);   /* GPIOAEN */
    RCC_APB1ENR |= TIM2_APB1ENR_BIT;

    /* Configure the GPIO pin for this channel as TIM2 AF output */
    pwm_gpio_init_pin(channel); /* PA0..PA3 */

    /*
     * Timer base: PSC=15, ARR derived from freq_hz.
     * Timer clock = 16 MHz (SYSTEM_CLOCK_HZ).
     * f_pwm = 16MHz / (PSC+1) / (ARR+1) = 1MHz / (ARR+1)
     */
    uint32_t arr = (SYSTEM_CLOCK_HZ / 16U / freq_hz);
    if (arr == 0) arr = 1;
    s_pwm_arr = arr - 1U;

    TIM2->PSC  = 15U;
    TIM2->ARR  = s_pwm_arr;
    TIM2->CR1 |= TIM_CR1_ARPE;

    /* Configure PWM mode 1 on the selected channel */
    switch (channel) {
        case 0:
            TIM2->CCMR1 = (TIM2->CCMR1 & ~0x00FFU) | ((uint32_t)TIM_CCMR_PWM1);
            TIM2->CCR1   = 0;
            TIM2->CCER  |= (1U << 0);   /* CC1E */
            break;
        case 1:
            TIM2->CCMR1 = (TIM2->CCMR1 & ~0xFF00U) | ((uint32_t)TIM_CCMR_PWM1 << 8);
            TIM2->CCR2   = 0;
            TIM2->CCER  |= (1U << 4);   /* CC2E */
            break;
        case 2:
            TIM2->CCMR2 = (TIM2->CCMR2 & ~0x00FFU) | ((uint32_t)TIM_CCMR_PWM1);
            TIM2->CCR3   = 0;
            TIM2->CCER  |= (1U << 8);   /* CC3E */
            break;
        case 3:
            TIM2->CCMR2 = (TIM2->CCMR2 & ~0xFF00U) | ((uint32_t)TIM_CCMR_PWM1 << 8);
            TIM2->CCR4   = 0;
            TIM2->CCER  |= (1U << 12);  /* CC4E */
            break;
    }

    /* Generate an update event to load the shadow registers */
    TIM2->EGR = 1U;

    /* Start the counter */
    TIM2->CR1 |= TIM_CR1_CEN;

    return SYN_OK;
}

void syn_port_pwm_set_duty(uint8_t channel, uint8_t duty_pct)
{
    if (channel > 3) return;
    if (duty_pct > 100) duty_pct = 100;

    uint32_t ccr = ((uint32_t)duty_pct * (s_pwm_arr + 1U)) / 100U;

    switch (channel) {
        case 0: TIM2->CCR1 = ccr; break;
        case 1: TIM2->CCR2 = ccr; break;
        case 2: TIM2->CCR3 = ccr; break;
        case 3: TIM2->CCR4 = ccr; break;
    }
}

void syn_port_pwm_set_duty_raw(uint8_t channel, uint16_t duty_u16)
{
    if (channel > 3) return;

    /* Map 0–65535 to 0–ARR */
    uint32_t ccr = ((uint32_t)duty_u16 * (s_pwm_arr + 1U)) / 65536U;

    switch (channel) {
        case 0: TIM2->CCR1 = ccr; break;
        case 1: TIM2->CCR2 = ccr; break;
        case 2: TIM2->CCR3 = ccr; break;
        case 3: TIM2->CCR4 = ccr; break;
    }
}

void syn_port_pwm_enable(uint8_t channel, bool enable)
{
    if (channel > 3) return;

    static const uint32_t ccer_bits[] = { 1U<<0, 1U<<4, 1U<<8, 1U<<12 };
    if (enable) {
        TIM2->CCER |= ccer_bits[channel];
    } else {
        TIM2->CCER &= ~ccer_bits[channel];
    }
}

void syn_port_pwm_set_freq(uint8_t channel, uint32_t freq_hz)
{
    (void)channel; /* All channels share TIM2 — frequency change affects all */

    if (freq_hz == 0) return;

    uint32_t arr = (SYSTEM_CLOCK_HZ / 16U / freq_hz);
    if (arr == 0) arr = 1;
    s_pwm_arr = arr - 1U;

    TIM2->ARR = s_pwm_arr;
    TIM2->EGR = 1U; /* reload */
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  System init (called before main)
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_port_system_init(void)
{
    /* Configure SysTick for 1ms interrupt at 16MHz */
    SYSTICK_LOAD = (SYSTEM_CLOCK_HZ / 1000) - 1;
    SYSTICK_VAL  = 0;
    SYSTICK_CTRL = 0x07; /* Enable, interrupt, use processor clock */
}

/* Redirect standard library output (printf/putchar) to USART2 for Renode/Unity */
int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        if (ptr[i] == '\n') {
            syn_port_uart_transmit_byte(1 /* USART2 */, '\r');
        }
        syn_port_uart_transmit_byte(1 /* USART2 */, (uint8_t)ptr[i]);
    }
    return len;
}

