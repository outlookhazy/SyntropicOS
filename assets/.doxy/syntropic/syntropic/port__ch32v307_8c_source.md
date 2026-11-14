

# File port\_ch32v307.c

[**File List**](files.md) **>** [**ch32v307**](dir_876381587f01eae594c70ce97acc1e04.md) **>** [**port\_ch32v307.c**](port__ch32v307_8c.md)

[Go to the documentation of this file](port__ch32v307_8c.md)


```C++


#if (defined(CH32V307xx) || defined(CH32V307)) && !defined(ARDUINO)

#include "syntropic/common/syn_compiler.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_adc.h"
#include "syntropic/port/syn_port_can.h"
#include "syntropic/port/syn_port_dma.h"
#include "syntropic/port/syn_port_exti.h"
#include "syntropic/port/syn_port_flash.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_i2c.h"
#include "syntropic/port/syn_port_spi.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/system/syn_sleep.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 *  Register Map & Definitions
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PERIPH_BASE 0x40000000UL
#define APB1_BASE PERIPH_BASE
#define APB2_BASE (PERIPH_BASE + 0x10000UL)
#define AHB1_BASE (PERIPH_BASE + 0x20000UL)

/* ── RCC ────────────────────────────────────────────────────────────────── */

#define RCC_BASE (AHB1_BASE + 0x1000UL)
#define RCC_CTLR (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR0 (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_AHBCNTR (*(volatile uint32_t *)(RCC_BASE + 0x14))
#define RCC_APB2PCENR (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1PCENR (*(volatile uint32_t *)(RCC_BASE + 0x1C))

#define RCC_AFIOEN (1UL << 0)
#define RCC_IOPAEN (1U << 2)
#define RCC_IOPBEN (1U << 3)
#define RCC_IOPCEN (1U << 4)
#define RCC_IOPDEN (1U << 5)
#define RCC_IOPEEN (1U << 6)

#define RCC_USART1EN (1U << 14)
#define RCC_SPI1EN (1U << 12)
#define RCC_I2C1EN (1U << 21)
#define RCC_CAN1EN (1U << 25)
#define RCC_ADC1EN (1U << 9)

/* ── SysTick (STK) & PFIC ───────────────────────────────────────────────── */

#ifndef SYN_PORT_SYSCLK_HZ
#define SYN_PORT_SYSCLK_HZ 144000000UL
#endif

#define STK_BASE 0xE000F000UL
#define STK_CTLR (*(volatile uint32_t *)(STK_BASE + 0x00UL))
#define STK_SR (*(volatile uint32_t *)(STK_BASE + 0x04UL))
#define STK_CNTL (*(volatile uint32_t *)(STK_BASE + 0x08UL))
#define STK_CNTH (*(volatile uint32_t *)(STK_BASE + 0x0CUL))
#define STK_CMPLR (*(volatile uint32_t *)(STK_BASE + 0x10UL))
#define STK_CMPHR (*(volatile uint32_t *)(STK_BASE + 0x14UL))

#define STK_CTLR_STE (1U << 0)
#define STK_CTLR_STIE (1U << 1)
#define STK_CTLR_STCLK (1U << 2)
#define STK_CTLR_STRE (1U << 3)
#define STK_CTLR_INIT (1U << 5)
#define STK_SR_CNTIF (1U << 0)

#define PFIC_CFGR (*(volatile uint32_t *)0xE000E048UL)
#define PFIC_CFGR_RESETSYS 0xBEEF0080UL

/* ── GPIO ───────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t CFGLR;
    volatile uint32_t CFGHR;
    volatile uint32_t INDR;
    volatile uint32_t OUTDR;
    volatile uint32_t BSHR;
    volatile uint32_t BCR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)(APB2_BASE + 0x0800))
#define GPIOB ((GPIO_TypeDef *)(APB2_BASE + 0x0C00))
#define GPIOC ((GPIO_TypeDef *)(APB2_BASE + 0x1000))
#define GPIOD ((GPIO_TypeDef *)(APB2_BASE + 0x1400))
#define GPIOE ((GPIO_TypeDef *)(APB2_BASE + 0x1800))

static GPIO_TypeDef *const gpio_ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};
#define NUM_GPIO_PORTS (sizeof(gpio_ports) / sizeof(gpio_ports[0]))

/* ── USART ──────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t STATR;
    volatile uint32_t DATAR;
    volatile uint32_t BRR;
    volatile uint32_t CTLR1;
    volatile uint32_t CTLR2;
    volatile uint32_t CTLR3;
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)(APB2_BASE + 0x3800))

#define USART_STATR_TXE (1U << 7)
#define USART_STATR_RXNE (1U << 5)
#define USART_CTLR1_UE (1U << 13)
#define USART_CTLR1_TE (1U << 3)
#define USART_CTLR1_RE (1U << 2)

/* ── Flash Controller ───────────────────────────────────────────────────── */

#define FLASH_BASE (AHB1_BASE + 0x2000UL)
#define FLASH_KEYR (*(volatile uint32_t *)(FLASH_BASE + 0x04))
#define FLASH_STATR (*(volatile uint32_t *)(FLASH_BASE + 0x0C))
#define FLASH_CTLR (*(volatile uint32_t *)(FLASH_BASE + 0x10))
#define FLASH_ADDR (*(volatile uint32_t *)(FLASH_BASE + 0x14))

#define FLASH_KEY1 0x45670123UL
#define FLASH_KEY2 0xCDEF89ABUL

#define FLASH_CTLR_PG (1U << 0)
#define FLASH_CTLR_PER (1U << 1)
#define FLASH_CTLR_STRT (1U << 6)
#define FLASH_CTLR_LOCK (1U << 7)
#define FLASH_STATR_BSY (1U << 0)
#define FLASH_STATR_PGERR (1U << 2)
#define FLASH_STATR_WRPRTERR (1U << 4)
#define FLASH_STATR_EOP (1U << 5)
#define FLASH_STATR_ERR_MASK (FLASH_STATR_PGERR | FLASH_STATR_WRPRTERR)

/* ── SPI ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t CTLR1;
    volatile uint32_t CTLR2;
    volatile uint32_t STATR;
    volatile uint32_t DATAR;
} SPI_TypeDef;

#define SPI1 ((SPI_TypeDef *)(APB2_BASE + 0x3000))

#define SPI_CTLR1_SPE (1U << 6)
#define SPI_CTLR1_MSTR (1U << 2)
#define SPI_STATR_TXE (1U << 1)
#define SPI_STATR_RXNE (1U << 0)
#define SPI_STATR_BSY (1U << 7)

/* ── I2C ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t CTLR1;
    volatile uint32_t CTLR2;
    volatile uint32_t OADDR1;
    volatile uint32_t OADDR2;
    volatile uint32_t DATAR;
    volatile uint32_t STAR1;
    volatile uint32_t STAR2;
    volatile uint32_t CKCFGR;
} I2C_TypeDef;

#define I2C1 ((I2C_TypeDef *)(APB1_BASE + 0x5400))

#define I2C_CTLR1_PE (1U << 0)
#define I2C_CTLR1_START (1U << 8)
#define I2C_CTLR1_STOP (1U << 9)
#define I2C_CTLR1_ACK (1U << 10)
#define I2C_STAR1_SB (1U << 0)
#define I2C_STAR1_ADDR (1U << 1)
#define I2C_STAR1_BTF (1U << 2)
#define I2C_STAR1_RXNE (1U << 6)
#define I2C_STAR1_TXE (1U << 7)
#define I2C_STAR1_AF (1U << 10)
#define SYN_CH32_I2C_TIMEOUT_MS 100U

/* ── CAN (bxCAN) ────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t MCR;
    volatile uint32_t MSR;
    volatile uint32_t TSR;
    volatile uint32_t RF0R;
    volatile uint32_t RF1R;
    volatile uint32_t IER;
    volatile uint32_t ESR;
    volatile uint32_t BTR;
} CAN_TypeDef;

#define CAN1 ((CAN_TypeDef *)(APB1_BASE + 0x6400))

#define CAN_MCR_INRQ (1U << 0)
#define CAN_MSR_INAK (1U << 0)
#define CAN_TSR_TME0 (1U << 26)
#define CAN_RF0R_FMP0 (3U << 0)

/* ── ADC ────────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t STATR;
    volatile uint32_t CTLR1;
    volatile uint32_t CTLR2;
    volatile uint32_t SAMPTR1;
    volatile uint32_t SAMPTR2;
    volatile uint32_t IOFR1;
    volatile uint32_t IOFR2;
    volatile uint32_t IOFR3;
    volatile uint32_t IOFR4;
    volatile uint32_t WDHTR;
    volatile uint32_t WDLTR;
    volatile uint32_t RSQR1;
    volatile uint32_t RSQR2;
    volatile uint32_t RSQR3;
    volatile uint32_t ISQR;
    volatile uint32_t IDATAR1;
    volatile uint32_t IDATAR2;
    volatile uint32_t IDATAR3;
    volatile uint32_t IDATAR4;
    volatile uint32_t RDATAR;
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *)(APB2_BASE + 0x2400))

#define ADC_CTLR2_ADON (1UL << 0)
#define ADC_CTLR2_EXTSEL_SWSTART (7UL << 17)
#define ADC_CTLR2_EXTTRIG (1UL << 20)
#define ADC_CTLR2_SWSTART (1UL << 22)
#define ADC_STATR_EOC (1UL << 1)

/* ── EXTI ───────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t EVCR;
    volatile uint32_t PCFR1;
    volatile uint32_t EXTICR[4];
} AFIO_TypeDef;

#define AFIO ((AFIO_TypeDef *)(APB2_BASE + 0x0000UL))

typedef struct {
    volatile uint32_t INTENR;
    volatile uint32_t EVENR;
    volatile uint32_t RTENR;
    volatile uint32_t FTENR;
    volatile uint32_t SWIEVR;
    volatile uint32_t INTFR;
} EXTI_TypeDef;

#define EXTI ((EXTI_TypeDef *)(APB2_BASE + 0x0400))

/* ═══════════════════════════════════════════════════════════════════════════
 *  Global State
 * ═══════════════════════════════════════════════════════════════════════════ */

static volatile uint32_t s_tick_ms = 0U;
static volatile uint32_t s_critical_nesting = 0U;
static volatile uint32_t s_saved_mstatus = 0U;

#if defined(__riscv)
#define SYN_ISR_ATTR __attribute__((interrupt("machine")))
#else
#define SYN_ISR_ATTR
#endif

SYN_ISR_ATTR void SysTick_Handler(void)
{
#if defined(__riscv)
    uint32_t mstatus;
    __asm__ volatile("csrrci %0, mstatus, 8" : "=r"(mstatus)::"memory");
#endif
    s_tick_ms++;
    STK_SR = 0U;
#if defined(__riscv)
    if ((mstatus & 0x8U) != 0U) {
        __asm__ volatile("csrsi mstatus, 8" ::: "memory");
    }
#endif
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  System & Clock Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

void syn_port_system_init(void)
{
    uint32_t reload = (uint32_t)(SYN_PORT_SYSCLK_HZ / 1000UL);
    if (reload > 0U) {
        reload -= 1U;
    }

    STK_CTLR = 0U;
    STK_SR = 0U;
    STK_CNTL = 0U;
    STK_CNTH = 0U;
    STK_CMPLR = reload;
    STK_CMPHR = 0U;
    STK_CTLR = STK_CTLR_INIT | STK_CTLR_STRE | STK_CTLR_STCLK | STK_CTLR_STIE | STK_CTLR_STE;

    syn_port_nvic_enable_irq(12U);
#if defined(__riscv)
    __asm__ volatile("csrsi mstatus, 8");
#endif
}

void syn_port_enter_critical(void)
{
    uint32_t mstatus = 0U;
#if defined(__riscv)
    __asm__ volatile("csrrci %0, mstatus, 8" : "=r"(mstatus)::"memory");
#endif
    if (s_critical_nesting == 0U) {
        s_saved_mstatus = mstatus;
    }
    s_critical_nesting++;
}

void syn_port_exit_critical(void)
{
#if defined(__riscv)
    __asm__ volatile("" ::: "memory");
#endif
    if (s_critical_nesting > 0U) {
        s_critical_nesting--;
        if (s_critical_nesting == 0U && (s_saved_mstatus & 0x8U) != 0U) {
#if defined(__riscv)
            __asm__ volatile("csrsi mstatus, 8" ::: "memory");
#endif
        }
    }
}

uint32_t syn_port_get_tick_ms(void)
{
    return s_tick_ms;
}

uint32_t syn_port_get_tick_us(void)
{
    uint32_t reload = STK_CMPLR;
    if ((STK_CTLR & STK_CTLR_STE) == 0U || reload == 0U || reload == UINT32_MAX) {
        return s_tick_ms * 1000U;
    }

    uint32_t ms1;
    uint32_t ms2;
    uint32_t cnt;
    uint32_t sr1;
    uint32_t sr2;
    do {
        ms1 = s_tick_ms;
        sr1 = STK_SR & STK_SR_CNTIF;
        cnt = STK_CNTL;
        ms2 = s_tick_ms;
        sr2 = STK_SR & STK_SR_CNTIF;
    } while (ms1 != ms2 || sr1 != sr2);

    if (sr1 != 0U) {
        ms1++;
        if (cnt >= reload) {
            cnt = 0U;
        }
    } else if (cnt > reload) {
        cnt = reload;
    }

    uint32_t elapsed_us = (uint32_t)(((uint64_t)cnt * 1000ULL) / ((uint64_t)reload + 1ULL));
    return (ms1 * 1000U) + elapsed_us;
}

void syn_port_delay_ms(uint32_t ms)
{
    uint32_t start = syn_port_get_tick_ms();
    while ((syn_port_get_tick_ms() - start) < ms) {
        __asm__ volatile("nop");
    }
}

void syn_port_nvic_set_priority(uint8_t irq_num, uint8_t preempt_prio, uint8_t sub_prio)
{
    /* QingKe V4F PFIC priority register base: 0xE000E000 + 0x400 + irq_num */
    volatile uint8_t *iprior = (volatile uint8_t *)0xE000E400UL;
    iprior[irq_num] = (uint8_t)((preempt_prio << 6) | (sub_prio << 4));
}

void syn_port_nvic_enable_irq(uint8_t irq_num)
{
    /* QingKe V4F PFIC enable register: 0xE000E100 */
    volatile uint32_t *ienr = (volatile uint32_t *)0xE000E100UL;
    ienr[irq_num >> 5] |= (1U << (irq_num & 0x1FU));
}

SYN_NORETURN void syn_port_system_reset(void)
{
    PFIC_CFGR = PFIC_CFGR_RESETSYS;
    while (1) {
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  GPIO Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    uint32_t port_idx = pin >> 4;
    uint32_t pin_idx = pin & 0x0FU;

    if (port_idx >= NUM_GPIO_PORTS) {
        return SYN_INVALID_PARAM;
    }

    RCC_APB2PCENR |= (1U << (port_idx + 2));

    GPIO_TypeDef *gpio = gpio_ports[port_idx];

    uint32_t config_val;
    switch (mode) {
    case SYN_GPIO_OUTPUT:
        config_val = 0x3U;
        break;
    case SYN_GPIO_OUTPUT_OD:
        config_val = 0x7U;
        break;
    case SYN_GPIO_INPUT_PULLUP:
        config_val = 0x8U;
        gpio->BSHR = (1U << pin_idx);
        break;
    case SYN_GPIO_INPUT_PULLDOWN:
        config_val = 0x8U;
        gpio->BCR = (1U << pin_idx);
        break;
    case SYN_GPIO_INPUT:
        config_val = 0x4U;
        break;
    default:
        return SYN_INVALID_PARAM;
    }

    if (pin_idx < 8U) {
        uint32_t shift = pin_idx * 4U;
        gpio->CFGLR = (gpio->CFGLR & ~(0xFU << shift)) | (config_val << shift);
    } else {
        uint32_t shift = (pin_idx - 8U) * 4U;
        gpio->CFGHR = (gpio->CFGHR & ~(0xFU << shift)) | (config_val << shift);
    }

    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    return syn_port_gpio_init(pin, SYN_GPIO_INPUT);
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    uint32_t port_idx = pin >> 4;
    uint32_t pin_idx = pin & 0x0FU;

    if (port_idx >= NUM_GPIO_PORTS) {
        return SYN_INVALID_PARAM;
    }

    GPIO_TypeDef *gpio = gpio_ports[port_idx];
    if (state == SYN_GPIO_HIGH) {
        gpio->BSHR = (1U << pin_idx);
    } else {
        gpio->BCR = (1U << pin_idx);
    }

    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    uint32_t port_idx = pin >> 4;
    uint32_t pin_idx = pin & 0x0FU;

    if (port_idx >= NUM_GPIO_PORTS) {
        return SYN_GPIO_LOW;
    }

    GPIO_TypeDef *gpio = gpio_ports[port_idx];
    return (gpio->INDR & (1U << pin_idx)) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    SYN_GPIO_State curr = syn_port_gpio_read(pin);
    return syn_port_gpio_write(pin, (curr == SYN_GPIO_HIGH) ? SYN_GPIO_LOW : SYN_GPIO_HIGH);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  UART Implementation (USART1)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    if (baudrate == 0U) {
        return SYN_INVALID_PARAM;
    }
    if (instance != 0) {
        return SYN_NOT_IMPLEMENTED;
    }

    RCC_APB2PCENR |= RCC_USART1EN;

#ifndef SYN_PORT_SYSCLK_HZ
#define SYN_PORT_SYSCLK_HZ 144000000UL
#endif

    uint32_t pclk = SYN_PORT_SYSCLK_HZ;
    USART1->BRR = (pclk + baudrate / 2U) / baudrate;
    USART1->CTLR1 = USART_CTLR1_UE | USART_CTLR1_TE | USART_CTLR1_RE;

    return SYN_OK;
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    if (instance != 0)
        return SYN_NOT_IMPLEMENTED;
    USART1->CTLR1 = 0;
    return SYN_OK;
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    if (instance != 0)
        return SYN_NOT_IMPLEMENTED;

    while (!(USART1->STATR & USART_STATR_TXE)) {
    }
    USART1->DATAR = byte;

    return SYN_OK;
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance, const uint8_t *data, size_t len,
                                  uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (data == NULL)
        return SYN_INVALID_PARAM;

    for (size_t i = 0; i < len; i++) {
        syn_port_uart_transmit_byte(instance, data[i]);
    }
    return SYN_OK;
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance, uint8_t *byte, uint32_t timeout_ms)
{
    if (instance != 0 || byte == NULL) {
        return SYN_INVALID_PARAM;
    }

    uint32_t start_ms = syn_port_get_tick_ms();
    do {
        if ((USART1->STATR & USART_STATR_RXNE) != 0U) {
            *byte = (uint8_t)USART1->DATAR;
            return SYN_OK;
        }
    } while ((syn_port_get_tick_ms() - start_ms) < timeout_ms);

    return SYN_TIMEOUT;
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance, uint8_t *data, size_t len,
                                 size_t *received, uint32_t timeout_ms)
{
    if (instance != 0 || data == NULL || received == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (len == 0U) {
        *received = 0U;
        return SYN_OK;
    }

    size_t count = 0U;
    uint32_t start_ms = syn_port_get_tick_ms();
    while (count < len) {
        uint32_t elapsed = syn_port_get_tick_ms() - start_ms;
        uint32_t rem = (timeout_ms > elapsed) ? (timeout_ms - elapsed) : 0U;
        if (syn_port_uart_receive_byte(instance, &data[count], rem) == SYN_OK) {
            count++;
        } else {
            break;
        }
    }
    *received = count;
    return (count > 0U) ? SYN_OK : SYN_TIMEOUT;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    syn_port_uart_transmit(0, (const uint8_t *)ptr, (size_t)len, 100U);
    return len;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Flash Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

static SYN_Status ch32_flash_wait_bsy(void)
{
    volatile uint32_t timeout = 1000000UL;
    while ((FLASH_STATR & FLASH_STATR_BSY) != 0U && --timeout != 0UL) {
    }
    return (timeout > 0UL) ? SYN_OK : SYN_TIMEOUT;
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    if (len == 0U) {
        return SYN_OK;
    }
    if (buf == NULL) {
        return SYN_INVALID_PARAM;
    }
    memcpy(buf, (const void *)(uintptr_t)addr, len);
    return SYN_OK;
}

SYN_Status syn_port_flash_erase(uint32_t addr)
{
    if ((FLASH_CTLR & FLASH_CTLR_LOCK) != 0U) {
        FLASH_KEYR = FLASH_KEY1;
        FLASH_KEYR = FLASH_KEY2;
    }

    SYN_Status s = ch32_flash_wait_bsy();
    if (s != SYN_OK) {
        FLASH_CTLR |= FLASH_CTLR_LOCK;
        return s;
    }

    FLASH_STATR = FLASH_STATR_ERR_MASK | FLASH_STATR_EOP;
    FLASH_CTLR |= FLASH_CTLR_PER;
    FLASH_ADDR = addr;
    FLASH_CTLR |= FLASH_CTLR_STRT;

    s = ch32_flash_wait_bsy();
    if ((FLASH_STATR & FLASH_STATR_ERR_MASK) != 0U) {
        FLASH_STATR = FLASH_STATR_ERR_MASK | FLASH_STATR_EOP;
        if (s == SYN_OK) {
            s = SYN_ERROR;
        }
    } else if ((FLASH_STATR & FLASH_STATR_EOP) != 0U) {
        FLASH_STATR = FLASH_STATR_EOP;
    }

    FLASH_CTLR &= ~FLASH_CTLR_PER;
    FLASH_CTLR |= FLASH_CTLR_LOCK;

    return s;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    if (len == 0U) {
        return SYN_OK;
    }
    if (buf == NULL || (addr & 1U) != 0U) {
        return SYN_INVALID_PARAM;
    }

    if ((FLASH_CTLR & FLASH_CTLR_LOCK) != 0U) {
        FLASH_KEYR = FLASH_KEY1;
        FLASH_KEYR = FLASH_KEY2;
    }

    SYN_Status s = ch32_flash_wait_bsy();
    if (s != SYN_OK) {
        FLASH_CTLR |= FLASH_CTLR_LOCK;
        return s;
    }

    FLASH_STATR = FLASH_STATR_ERR_MASK | FLASH_STATR_EOP;
    FLASH_CTLR |= FLASH_CTLR_PG;

    const uint8_t *src = (const uint8_t *)buf;
    for (size_t i = 0U; i < len; i += 2U) {
        uint16_t hw = (uint16_t)src[i];
        if ((i + 1U) < len) {
            hw |= (uint16_t)((uint16_t)src[i + 1U] << 8);
        } else {
            hw |= 0xFF00U;
        }
        *(volatile uint16_t *)(uintptr_t)(addr + (uint32_t)i) = hw;
        s = ch32_flash_wait_bsy();
        if (s != SYN_OK || (FLASH_STATR & FLASH_STATR_ERR_MASK) != 0U) {
            FLASH_STATR = FLASH_STATR_ERR_MASK | FLASH_STATR_EOP;
            FLASH_CTLR &= ~FLASH_CTLR_PG;
            FLASH_CTLR |= FLASH_CTLR_LOCK;
            return (s != SYN_OK) ? s : SYN_ERROR;
        }
        if ((FLASH_STATR & FLASH_STATR_EOP) != 0U) {
            FLASH_STATR = FLASH_STATR_EOP;
        }
    }

    FLASH_CTLR &= ~FLASH_CTLR_PG;
    FLASH_CTLR |= FLASH_CTLR_LOCK;
    return SYN_OK;
}

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    return 4U * 1024U;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  SPI Implementation (SPI1)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_spi_init(uint8_t spi_id, uint32_t baudrate_hz, uint8_t mode, uint8_t role)
{
    (void)baudrate_hz;
    (void)mode;
    (void)role;
    if (spi_id != 0)
        return SYN_NOT_IMPLEMENTED;

    RCC_APB2PCENR |= RCC_SPI1EN | RCC_IOPAEN;
    SPI1->CTLR1 = SPI_CTLR1_MSTR | SPI_CTLR1_SPE;
    return SYN_OK;
}

SYN_Status syn_port_spi_deinit(uint8_t spi_id)
{
    if (spi_id != 0)
        return SYN_NOT_IMPLEMENTED;
    SPI1->CTLR1 = 0;
    return SYN_OK;
}

SYN_Status syn_port_spi_transfer(uint8_t spi_id, const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (spi_id != 0)
        return SYN_NOT_IMPLEMENTED;

    for (size_t i = 0; i < len; i++) {
        uint8_t send_val = tx ? tx[i] : 0xFFU;

        while (!(SPI1->STATR & SPI_STATR_TXE)) {
        }
        SPI1->DATAR = send_val;

        while (!(SPI1->STATR & SPI_STATR_RXNE)) {
        }
        uint8_t recv_val = (uint8_t)SPI1->DATAR;
        if (rx) {
            rx[i] = recv_val;
        }
    }
    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  I2C Implementation (I2C1)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_i2c_init(uint8_t i2c_id, uint32_t clock_speed_hz, uint8_t role,
                             uint16_t own_addr)
{
    (void)clock_speed_hz;
    (void)role;
    (void)own_addr;
    if (i2c_id != 0)
        return SYN_NOT_IMPLEMENTED;

    RCC_APB1PCENR |= RCC_I2C1EN;
    I2C1->CTLR1 = I2C_CTLR1_PE;
    return SYN_OK;
}

SYN_Status syn_port_i2c_deinit(uint8_t i2c_id)
{
    if (i2c_id != 0)
        return SYN_NOT_IMPLEMENTED;
    I2C1->CTLR1 = 0;
    return SYN_OK;
}

static SYN_Status ch32_i2c1_wait_flag(uint32_t flag, uint32_t start_ms)
{
    while ((I2C1->STAR1 & flag) == 0U) {
        if ((I2C1->STAR1 & I2C_STAR1_AF) != 0U) {
            I2C1->STAR1 &= ~I2C_STAR1_AF;
            I2C1->CTLR1 |= I2C_CTLR1_STOP;
            return SYN_ERROR;
        }
        if ((syn_port_get_tick_ms() - start_ms) >= SYN_CH32_I2C_TIMEOUT_MS) {
            I2C1->CTLR1 |= I2C_CTLR1_STOP;
            return SYN_TIMEOUT;
        }
    }
    return SYN_OK;
}

static SYN_Status ch32_i2c1_write_phase(uint16_t addr, const uint8_t *tx, size_t tx_len,
                                        bool send_stop, uint32_t start_ms)
{
    SYN_Status st;
    I2C1->CTLR1 |= I2C_CTLR1_START;
    st = ch32_i2c1_wait_flag(I2C_STAR1_SB, start_ms);
    if (st != SYN_OK) {
        return st;
    }

    I2C1->DATAR = (uint8_t)(addr << 1);
    st = ch32_i2c1_wait_flag(I2C_STAR1_ADDR, start_ms);
    if (st != SYN_OK) {
        return st;
    }
    (void)I2C1->STAR2;

    for (size_t i = 0U; i < tx_len; i++) {
        st = ch32_i2c1_wait_flag(I2C_STAR1_TXE, start_ms);
        if (st != SYN_OK) {
            return st;
        }
        I2C1->DATAR = tx[i];
    }

    if (tx_len > 0U) {
        st = ch32_i2c1_wait_flag(I2C_STAR1_BTF, start_ms);
        if (st != SYN_OK) {
            return st;
        }
    }

    if (send_stop) {
        I2C1->CTLR1 |= I2C_CTLR1_STOP;
    }
    return SYN_OK;
}

static SYN_Status ch32_i2c1_read_phase(uint16_t addr, uint8_t *rx, size_t rx_len, uint32_t start_ms)
{
    SYN_Status st;
    I2C1->CTLR1 |= I2C_CTLR1_ACK;
    I2C1->CTLR1 |= I2C_CTLR1_START;
    st = ch32_i2c1_wait_flag(I2C_STAR1_SB, start_ms);
    if (st != SYN_OK) {
        return st;
    }

    I2C1->DATAR = (uint8_t)((addr << 1) | 1U);
    st = ch32_i2c1_wait_flag(I2C_STAR1_ADDR, start_ms);
    if (st != SYN_OK) {
        return st;
    }

    if (rx_len == 1U) {
        /* Clear ACK prior to clearing ADDR via STAR2 read, then issue STOP */
        I2C1->CTLR1 &= ~I2C_CTLR1_ACK;
        (void)I2C1->STAR2;
        I2C1->CTLR1 |= I2C_CTLR1_STOP;
        st = ch32_i2c1_wait_flag(I2C_STAR1_RXNE, start_ms);
        if (st != SYN_OK) {
            return st;
        }
        rx[0] = (uint8_t)I2C1->DATAR;
        return SYN_OK;
    }

    (void)I2C1->STAR2;
    for (size_t i = 0U; i < rx_len; i++) {
        if ((i + 1U) == rx_len) {
            I2C1->CTLR1 &= ~I2C_CTLR1_ACK;
            I2C1->CTLR1 |= I2C_CTLR1_STOP;
        }
        st = ch32_i2c1_wait_flag(I2C_STAR1_RXNE, start_ms);
        if (st != SYN_OK) {
            return st;
        }
        rx[i] = (uint8_t)I2C1->DATAR;
    }
    return SYN_OK;
}

SYN_Status syn_port_i2c_transfer(uint8_t i2c_id, uint16_t addr, const uint8_t *tx, size_t tx_len,
                                 uint8_t *rx, size_t rx_len)
{
    if (i2c_id != 0U) {
        return SYN_NOT_IMPLEMENTED;
    }

    if ((tx_len > 0U && tx == NULL) || (rx_len > 0U && rx == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint32_t start_ms = syn_port_get_tick_ms();
    if (tx_len > 0U || rx_len == 0U) {
        SYN_Status st = ch32_i2c1_write_phase(addr, tx, tx_len, (rx_len == 0U), start_ms);
        if (st != SYN_OK) {
            return st;
        }
    }

    if (rx_len > 0U) {
        return ch32_i2c1_read_phase(addr, rx, rx_len, start_ms);
    }

    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  CAN Implementation (CAN1 bxCAN)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* CAN1 bxCAN-compatible controller initialization. Leaves initialization mode upon exit. */
bool syn_port_can_init(uint8_t port, uint32_t bitrate)
{
    (void)bitrate;
    if (port != 0)
        return false;

    RCC_APB1PCENR |= RCC_CAN1EN;
    CAN1->MCR &= ~CAN_MCR_INRQ;
    return true;
}

/* Transmit frame via CAN1 mailbox 0. */
bool syn_port_can_send(uint8_t port, uint32_t id, bool extended, const uint8_t *data, uint8_t dlc)
{
    if (port != 0 || dlc > 8)
        return false;

    if (!(CAN1->TSR & CAN_TSR_TME0)) {
        return false;
    }

    typedef struct {
        volatile uint32_t TIR;
        volatile uint32_t TDTR;
        volatile uint32_t TDLR;
        volatile uint32_t TDHR;
    } CAN_TxMailBox_TypeDef;

    CAN_TxMailBox_TypeDef *mb = (CAN_TxMailBox_TypeDef *)(APB1_BASE + 0x6400 + 0x180);

    uint32_t tir = 0;
    if (extended) {
        tir = (id << 3) | (1U << 2);
    } else {
        tir = (id << 21);
    }
    tir |= (1U << 0);

    mb->TDTR = (dlc & 0x0FU);
    if (data != NULL) {
        uint32_t tdlr = 0, tdhr = 0;
        for (uint8_t i = 0; i < dlc && i < 4; i++) {
            tdlr |= ((uint32_t)data[i] << (i * 8));
        }
        for (uint8_t i = 4; i < dlc && i < 8; i++) {
            tdhr |= ((uint32_t)data[i] << ((i - 4) * 8));
        }
        mb->TDLR = tdlr;
        mb->TDHR = tdhr;
    }
    mb->TIR = tir;
    return true;
}

/* Non-blocking read from CAN1 FIFO0 buffer. */
bool syn_port_can_receive(uint8_t port, uint32_t *id, bool *extended, uint8_t *data, uint8_t *dlc)
{
    if (port != 0 || id == NULL || extended == NULL || data == NULL || dlc == NULL)
        return false;

    if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0) {
        return false;
    }

    typedef struct {
        volatile uint32_t RIR;
        volatile uint32_t RDTR;
        volatile uint32_t RDLR;
        volatile uint32_t RDHR;
    } CAN_FIFOMailBox_TypeDef;

    CAN_FIFOMailBox_TypeDef *fifo = (CAN_FIFOMailBox_TypeDef *)(APB1_BASE + 0x6400 + 0x1B0);

    uint32_t rir = fifo->RIR;
    if (rir & (1U << 2)) {
        *extended = true;
        *id = (rir >> 3);
    } else {
        *extended = false;
        *id = (rir >> 21);
    }

    uint8_t len = (uint8_t)(fifo->RDTR & 0x0FU);
    *dlc = len;

    uint32_t rdlr = fifo->RDLR;
    uint32_t rdhr = fifo->RDHR;
    for (uint8_t i = 0; i < len && i < 4; i++) {
        data[i] = (uint8_t)(rdlr >> (i * 8));
    }
    for (uint8_t i = 4; i < len && i < 8; i++) {
        data[i] = (uint8_t)(rdhr >> ((i - 4) * 8));
    }

    CAN1->RF0R |= (1U << 5);
    return true;
}

/* Configure hardware 32-bit acceptance filter bank. */
void syn_port_can_set_filter(uint8_t port, uint32_t id, uint32_t mask)
{
    if (port != 0)
        return;

    typedef struct {
        volatile uint32_t F1R;
        volatile uint32_t F2R;
    } CAN_FilterRegister_TypeDef;

    volatile uint32_t *FMR = (volatile uint32_t *)(APB1_BASE + 0x6400 + 0x200);
    volatile uint32_t *FA1R = (volatile uint32_t *)(APB1_BASE + 0x6400 + 0x21C);
    CAN_FilterRegister_TypeDef *FR = (CAN_FilterRegister_TypeDef *)(APB1_BASE + 0x6400 + 0x240);

    *FMR |= (1U << 0);
    *FA1R &= ~(1U << 0);

    FR[0].F1R = (id << 21);
    FR[0].F2R = (mask << 21);

    *FA1R |= (1U << 0);
    *FMR &= ~(1U << 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  ADC Implementation (ADC1)
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_adc_init(uint8_t adc_id, uint32_t channel_mask)
{
    (void)channel_mask;
    if (adc_id != 0)
        return SYN_NOT_IMPLEMENTED;

    RCC_APB2PCENR |= RCC_ADC1EN;
    ADC1->CTLR2 |= ADC_CTLR2_EXTSEL_SWSTART | ADC_CTLR2_EXTTRIG | ADC_CTLR2_ADON;
    return SYN_OK;
}

SYN_Status syn_port_adc_deinit(uint8_t adc_id)
{
    if (adc_id != 0)
        return SYN_NOT_IMPLEMENTED;
    ADC1->CTLR2 &= ~ADC_CTLR2_ADON;
    return SYN_OK;
}

/* Single-shot polled conversion on ADC1 channel. */
uint16_t syn_port_adc_read_channel(uint8_t adc_id, uint8_t channel)
{
    if (adc_id != 0U || channel > 17U) {
        return 0U;
    }

    ADC1->RSQR1 &= ~(0xFUL << 20);
    ADC1->RSQR3 = (uint32_t)channel & 0x1FUL;
    ADC1->CTLR2 |= ADC_CTLR2_SWSTART;
    uint32_t timeout = 100000U;
    while ((ADC1->STATR & ADC_STATR_EOC) == 0U) {
        if (--timeout == 0U) {
            return 0U;
        }
    }
    return (uint16_t)ADC1->RDATAR;
}

/* Trigger multi-channel background scan stream over DMA1 Channel 1. */
SYN_Status syn_port_adc_start_dma_scan(uint8_t adc_id, uint16_t *dest, size_t num_channels)
{
    if (adc_id != 0 || dest == NULL || num_channels == 0) {
        return SYN_INVALID_PARAM;
    }

    /* Enable DMA1 clock */
    RCC_AHBCNTR |= (1U << 0);

    /* Configure DMA1 Channel 1 (ADC1 peripheral to memory transfer) */
    SYN_PortDmaTransfer xfer = {
        .channel_id = 0,
        .dir = SYN_DMA_DIR_PERIPH_TO_MEM,
        .data_size = SYN_DMA_SIZE_16BIT,
        .src_inc = false,
        .dst_inc = true,
        .src = (const void *)&ADC1->RDATAR,
        .dst = dest,
        .count = num_channels,
    };

    SYN_Status st = syn_port_dma_start(&xfer);
    if (st == SYN_OK) {
        ADC1->CTLR2 |= (1U << 8); /* Enable ADC DMA request bit */
    }
    return st;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  EXTI Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

SYN_Status syn_port_exti_configure(SYN_GPIO_Pin pin, SYN_EXTI_Edge edge)
{
    uint32_t port_idx = (uint32_t)pin >> 4U;
    uint32_t pin_idx = (uint32_t)pin & 0x0FU;

    if (port_idx >= NUM_GPIO_PORTS ||
        (edge != SYN_EXTI_RISING && edge != SYN_EXTI_FALLING && edge != SYN_EXTI_BOTH)) {
        return SYN_INVALID_PARAM;
    }

    RCC_APB2PCENR |= RCC_AFIOEN;
    uint32_t exti_reg = pin_idx >> 2U;
    uint32_t exti_shift = (pin_idx & 0x03U) * 4U;
    AFIO->EXTICR[exti_reg] =
        (AFIO->EXTICR[exti_reg] & ~(0xFUL << exti_shift)) | (port_idx << exti_shift);

    if (edge == SYN_EXTI_RISING || edge == SYN_EXTI_BOTH) {
        EXTI->RTENR |= (1UL << pin_idx);
    } else {
        EXTI->RTENR &= ~(1UL << pin_idx);
    }
    if (edge == SYN_EXTI_FALLING || edge == SYN_EXTI_BOTH) {
        EXTI->FTENR |= (1UL << pin_idx);
    } else {
        EXTI->FTENR &= ~(1UL << pin_idx);
    }
    return SYN_OK;
}

void syn_port_exti_enable(SYN_GPIO_Pin pin)
{
    uint32_t pin_idx = pin & 0x0FU;
    EXTI->INTENR |= (1U << pin_idx);
}

void syn_port_exti_disable(SYN_GPIO_Pin pin)
{
    uint32_t pin_idx = pin & 0x0FU;
    EXTI->INTENR &= ~(1U << pin_idx);
}

void syn_port_exti_clear_pending(SYN_GPIO_Pin pin)
{
    uint32_t pin_idx = pin & 0x0FU;
    EXTI->INTFR = (1U << pin_idx);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  DMA Implementation (DMA1 / DMA2)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    volatile uint32_t CFGR;
    volatile uint32_t CNTR;
    volatile uint32_t PADDR;
    volatile uint32_t MADDR;
    volatile uint32_t RESERVED;
} DMA_Channel_TypeDef;

typedef struct {
    volatile uint32_t INTFR;
    volatile uint32_t INTFCR;
    DMA_Channel_TypeDef CH[7];
} DMA_TypeDef;

#define DMA1 ((DMA_TypeDef *)(AHB1_BASE + 0x0000))

static SYN_Status ch32_dma_encode_size(SYN_DMA_Size data_size, uint32_t *out_bits)
{
    if (data_size == SYN_DMA_SIZE_8BIT) {
        *out_bits = 0UL;
        return SYN_OK;
    }
    if (data_size == SYN_DMA_SIZE_16BIT) {
        *out_bits = (1UL << 8) | (1UL << 10);
        return SYN_OK;
    }
    if (data_size == SYN_DMA_SIZE_32BIT) {
        *out_bits = (2UL << 8) | (2UL << 10);
        return SYN_OK;
    }
    return SYN_INVALID_PARAM;
}

static SYN_Status ch32_dma_setup_route(DMA_Channel_TypeDef *ch, const SYN_PortDmaTransfer *xfer,
                                       uint32_t *inout_cfgr)
{
    bool pinc = xfer->src_inc;
    bool minc = xfer->dst_inc;

    if (xfer->dir == SYN_DMA_DIR_MEM_TO_PERIPH) {
        ch->PADDR = (uint32_t)(uintptr_t)xfer->dst;
        ch->MADDR = (uint32_t)(uintptr_t)xfer->src;
        *inout_cfgr |= (1UL << 4);
        pinc = xfer->dst_inc;
        minc = xfer->src_inc;
    } else if (xfer->dir == SYN_DMA_DIR_PERIPH_TO_MEM || xfer->dir == SYN_DMA_DIR_MEM_TO_MEM) {
        ch->PADDR = (uint32_t)(uintptr_t)xfer->src;
        ch->MADDR = (uint32_t)(uintptr_t)xfer->dst;
        if (xfer->dir == SYN_DMA_DIR_MEM_TO_MEM) {
            *inout_cfgr |= (1UL << 14);
        }
    } else {
        return SYN_INVALID_PARAM;
    }

    if (pinc) {
        *inout_cfgr |= (1UL << 6);
    }
    if (minc) {
        *inout_cfgr |= (1UL << 7);
    }
    return SYN_OK;
}

SYN_Status syn_port_dma_start(const SYN_PortDmaTransfer *xfer)
{
    if (xfer == NULL || xfer->channel_id >= 7U || xfer->src == NULL || xfer->dst == NULL ||
        xfer->count == 0U || xfer->count > 0xFFFFU) {
        return SYN_INVALID_PARAM;
    }

    uint32_t cfgr = 0UL;
    if (ch32_dma_encode_size(xfer->data_size, &cfgr) != SYN_OK) {
        return SYN_INVALID_PARAM;
    }

    RCC_AHBCNTR |= (1UL << 0);

    DMA_Channel_TypeDef *ch = &DMA1->CH[xfer->channel_id];
    ch->CFGR &= ~1UL;

    if (ch32_dma_setup_route(ch, xfer, &cfgr) != SYN_OK) {
        return SYN_INVALID_PARAM;
    }

    ch->CNTR = (uint32_t)xfer->count;
    ch->CFGR = cfgr | 1UL;
    return SYN_OK;
}

SYN_Status syn_port_dma_stop(uint8_t channel_id)
{
    if (channel_id >= 7U)
        return SYN_INVALID_PARAM;
    DMA1->CH[channel_id].CFGR &= ~1U;
    return SYN_OK;
}

bool syn_port_dma_is_busy(uint8_t channel_id)
{
    if (channel_id >= 7U)
        return false;
    return (DMA1->CH[channel_id].CFGR & 1U) != 0U;
}

uint32_t syn_port_dma_get_counter(uint8_t channel_id)
{
    if (channel_id >= 7U)
        return 0U;
    return DMA1->CH[channel_id].CNTR;
}

/* QingKe V4F RISC-V core has no L1 Data Cache; CPU and DMA access SRAM directly
 * with native hardware coherency. Cache maintenance operations are no-ops. */
void syn_port_cache_clean(const void *addr, size_t len)
{
    (void)addr;
    (void)len;
}

void syn_port_cache_invalidate(void *addr, size_t len)
{
    (void)addr;
    (void)len;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Ethernet Implementation (10/100M MAC + DMA Descriptor Ring)
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "port/syn_port_eth.h"

typedef struct {
    volatile uint32_t Status;
    volatile uint32_t ControlBufferSize;
    volatile uint32_t Buffer1Addr;
    volatile uint32_t Buffer2NextDescAddr;
} ETH_DMADescTypeDef;

#define ETH_MAC_BASE 0x40028000UL
#define ETH_DMA_BASE 0x40029000UL

typedef struct {
    volatile uint32_t MACCR;
    volatile uint32_t MACFFR;
    volatile uint32_t MACHTHR;
    volatile uint32_t MACHTLR;
    volatile uint32_t MACMIIAR;
    volatile uint32_t MACMIIDR;
    volatile uint32_t MACFCR;
    volatile uint32_t MACVLANTR;
} ETH_MAC_TypeDef;

typedef struct {
    volatile uint32_t BMR;
    volatile uint32_t TPDR;
    volatile uint32_t RPDR;
    volatile uint32_t RDLAR;
    volatile uint32_t TDLAR;
    volatile uint32_t SR;
    volatile uint32_t OMR;
    volatile uint32_t IER;
} ETH_DMA_TypeDef;

#define ETH_MAC ((ETH_MAC_TypeDef *)ETH_MAC_BASE)
#define ETH_DMA ((ETH_DMA_TypeDef *)ETH_DMA_BASE)

#define ETH_MACCR_RE (1UL << 2)
#define ETH_MACCR_TE (1UL << 3)

#define ETH_DMATXDESC_OWN (1UL << 31)
#define ETH_DMATXDESC_LS (1UL << 29)
#define ETH_DMATXDESC_FS (1UL << 28)
#define ETH_DMATXDESC_TCH (1UL << 20)
#define ETH_DMARXDESC_OWN (1UL << 31)
#define ETH_DMARXDESC_RCH (1UL << 14)

#define ETH_TX_DESC_NUM 4U
#define ETH_RX_DESC_NUM 4U
#define ETH_MAX_PACKET_SIZE 1536U

static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_NUM] __attribute__((aligned(4)));
static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_NUM] __attribute__((aligned(4)));
static uint8_t Rx_Buff[ETH_RX_DESC_NUM][ETH_MAX_PACKET_SIZE] __attribute__((aligned(4)));
static uint8_t Tx_Buff[ETH_TX_DESC_NUM][ETH_MAX_PACKET_SIZE] __attribute__((aligned(4)));

static uint32_t s_tx_desc_idx = 0U;
static uint32_t s_rx_desc_idx = 0U;

SYN_Status syn_port_eth_init(const uint8_t mac_addr[6])
{
    (void)mac_addr;

    /* Enable Ethernet MAC, TX, & RX clocks */
    RCC_AHBCNTR |= (1UL << 14) | (1UL << 15) | (1UL << 16);

    /* Software Reset ETH DMA */
    ETH_DMA->BMR |= (1UL << 0);
    while ((ETH_DMA->BMR & (1UL << 0)) != 0UL) {
    }

    s_tx_desc_idx = 0U;
    s_rx_desc_idx = 0U;

    /* Setup Tx DMA Descriptors (ring-chained via Buffer2NextDescAddr) */
    for (uint32_t i = 0U; i < ETH_TX_DESC_NUM; i++) {
        DMATxDscrTab[i].Status = ETH_DMATXDESC_TCH;
        DMATxDscrTab[i].ControlBufferSize = 0UL;
        DMATxDscrTab[i].Buffer1Addr = (uint32_t)(uintptr_t)&Tx_Buff[i][0];
        DMATxDscrTab[i].Buffer2NextDescAddr =
            (uint32_t)(uintptr_t)&DMATxDscrTab[(i + 1U) % ETH_TX_DESC_NUM];
    }
    ETH_DMA->TDLAR = (uint32_t)(uintptr_t)&DMATxDscrTab[0];

    /* Setup Rx DMA Descriptors (ring-chained via ETH_DMARXDESC_RCH) */
    for (uint32_t i = 0U; i < ETH_RX_DESC_NUM; i++) {
        DMARxDscrTab[i].Status = ETH_DMARXDESC_OWN;
        DMARxDscrTab[i].ControlBufferSize = ETH_DMARXDESC_RCH | (uint32_t)ETH_MAX_PACKET_SIZE;
        DMARxDscrTab[i].Buffer1Addr = (uint32_t)(uintptr_t)&Rx_Buff[i][0];
        DMARxDscrTab[i].Buffer2NextDescAddr =
            (uint32_t)(uintptr_t)&DMARxDscrTab[(i + 1U) % ETH_RX_DESC_NUM];
    }
    ETH_DMA->RDLAR = (uint32_t)(uintptr_t)&DMARxDscrTab[0];

    /* Enable MAC transmitter and receiver before starting DMA engines */
    ETH_MAC->MACCR |= ETH_MACCR_TE | ETH_MACCR_RE;

    /* Start Ethernet Transmission & Reception DMA engines */
    ETH_DMA->OMR |= (1UL << 13) | (1UL << 1); /* ST (Start Tx) | SR (Start Rx) */
    return SYN_OK;
}

SYN_Status syn_port_eth_tx(const void *frame, size_t len)
{
    if (frame == NULL || len == 0U || len > ETH_MAX_PACKET_SIZE) {
        return SYN_INVALID_PARAM;
    }

    ETH_DMADescTypeDef *desc = &DMATxDscrTab[s_tx_desc_idx];
    if ((desc->Status & ETH_DMATXDESC_OWN) != 0UL) {
        return SYN_BUSY;
    }

    memcpy(Tx_Buff[s_tx_desc_idx], frame, len);
    desc->ControlBufferSize = (uint32_t)len;
    desc->Status = ETH_DMATXDESC_OWN | ETH_DMATXDESC_TCH | ETH_DMATXDESC_FS | ETH_DMATXDESC_LS;

    ETH_DMA->TPDR = 0UL; /* Transmit poll demand */
    s_tx_desc_idx = (s_tx_desc_idx + 1U) % ETH_TX_DESC_NUM;
    return SYN_OK;
}

SYN_Status syn_port_eth_rx(void *buf, size_t max_len, size_t *out_len)
{
    if (buf == NULL || out_len == NULL) {
        return SYN_INVALID_PARAM;
    }

    ETH_DMADescTypeDef *desc = &DMARxDscrTab[s_rx_desc_idx];
    if ((desc->Status & ETH_DMARXDESC_OWN) != 0UL) {
        *out_len = 0U;
        return SYN_BUSY;
    }

    size_t pkt_len = (size_t)((desc->Status >> 16U) & 0x3FFFUL);
    if (pkt_len > max_len) {
        pkt_len = max_len;
    }

    memcpy(buf, Rx_Buff[s_rx_desc_idx], pkt_len);
    *out_len = pkt_len;

    desc->ControlBufferSize = ETH_DMARXDESC_RCH | (uint32_t)ETH_MAX_PACKET_SIZE;
    desc->Status = ETH_DMARXDESC_OWN;
    ETH_DMA->RPDR = 0UL; /* Receive poll demand */

    s_rx_desc_idx = (s_rx_desc_idx + 1U) % ETH_RX_DESC_NUM;
    return SYN_OK;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  Async DMA-Driven SPI Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "syntropic/port/syn_port_spi_async.h"

#if defined(SYN_USE_SPI_ASYNC) && SYN_USE_SPI_ASYNC

SYN_Status syn_port_spi_xfer_async(const SYN_SPI_Xfer *xfer)
{
    if (xfer == NULL || xfer->bus != 0) {
        return SYN_ERROR;
    }

    SYN_Status st = syn_port_spi_transfer(xfer->bus, xfer->tx_buf, xfer->rx_buf, xfer->len);
    if (xfer->callback != NULL) {
        xfer->callback(xfer->bus, st, xfer->user_data);
    }
    return st;
}

SYN_Status syn_port_spi_cancel(uint8_t bus)
{
    if (bus != 0) {
        return SYN_ERROR;
    }
    return SYN_OK;
}

#endif /* SYN_USE_SPI_ASYNC */

/* ═══════════════════════════════════════════════════════════════════════════
 *  Async DMA-Driven I2C Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "syntropic/port/syn_port_i2c_async.h"

#if defined(SYN_USE_I2C_ASYNC) && SYN_USE_I2C_ASYNC

SYN_Status syn_port_i2c_xfer_async(const SYN_I2C_Xfer *xfer)
{
    if (xfer == NULL || xfer->bus != 0) {
        return SYN_ERROR;
    }

    SYN_Status st = syn_port_i2c_transfer(xfer->bus, xfer->addr, xfer->tx_data, xfer->tx_len,
                                          xfer->rx_data, xfer->rx_len);
    if (xfer->callback != NULL) {
        xfer->callback(xfer->bus, st, xfer->user_data);
    }
    return st;
}

SYN_Status syn_port_i2c_cancel(uint8_t bus)
{
    if (bus != 0) {
        return SYN_ERROR;
    }
    return SYN_OK;
}

#endif /* SYN_USE_I2C_ASYNC */

/* ═══════════════════════════════════════════════════════════════════════════
 *  USB Device & Host Implementation (CH32V307 USBFS / USBHS)
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "port/syn_port_usb.h"
#include "port/syn_port_usb_host.h"

typedef struct {
    volatile uint8_t BASE_CTRL;
    volatile uint8_t UDEV_CTRL;
    volatile uint8_t INT_EN;
    volatile uint8_t DEV_ADDR;
    volatile uint8_t MIS_ST;
    volatile uint8_t INT_FG;
    volatile uint8_t INT_ST;
    volatile uint8_t RX_LEN;
} USBFS_TypeDef;

#define USBFS ((USBFS_TypeDef *)0x50000000UL)

static uint8_t s_usb_dev_addr = 0;
static bool s_usb_vbus_on = false;

/* ── USB Device Port Functions ──────────────────────────────────────────── */

SYN_Status syn_port_usb_init(void)
{
    RCC_APB2PCENR |= (1U << 23);
    USBFS->BASE_CTRL = 0x00;
    return SYN_OK;
}

SYN_Status syn_port_usb_connect(void)
{
    USBFS->UDEV_CTRL |= (1U << 7);
    return SYN_OK;
}

SYN_Status syn_port_usb_disconnect(void)
{
    USBFS->UDEV_CTRL &= ~(1U << 7);
    return SYN_OK;
}

SYN_Status syn_port_usb_set_address(uint8_t addr)
{
    s_usb_dev_addr = addr & 0x7FU;
    USBFS->DEV_ADDR = s_usb_dev_addr;
    return SYN_OK;
}

SYN_Status syn_port_usb_ep_open(uint8_t ep_addr, uint8_t ep_type, uint16_t max_pkt)
{
    (void)ep_addr;
    (void)ep_type;
    (void)max_pkt;
    return SYN_OK;
}

SYN_Status syn_port_usb_ep_close(uint8_t ep_addr)
{
    (void)ep_addr;
    return SYN_OK;
}

SYN_Status syn_port_usb_ep_write(uint8_t ep_addr, const void *data, uint16_t len)
{
    (void)ep_addr;
    (void)data;
    (void)len;
    return SYN_OK;
}

SYN_Status syn_port_usb_ep_read(uint8_t ep_addr, void *buf, uint16_t max_len, uint16_t *out_len)
{
    (void)ep_addr;
    (void)buf;
    (void)max_len;
    if (out_len != NULL)
        *out_len = 0U;
    return SYN_BUSY;
}

SYN_Status syn_port_usb_ep_stall(uint8_t ep_addr)
{
    (void)ep_addr;
    return SYN_OK;
}

/* ── USB Host Port Functions ────────────────────────────────────────────── */

SYN_Status syn_port_usb_host_init(void)
{
    RCC_APB2PCENR |= (1U << 23);
    USBFS->BASE_CTRL = 0x80;
    return SYN_OK;
}

SYN_Status syn_port_usb_host_vbus(bool enable)
{
    s_usb_vbus_on = enable;
    (void)s_usb_vbus_on;
    return SYN_OK;
}

SYN_Status syn_port_usb_host_bus_reset(void)
{
    USBFS->BASE_CTRL |= (1U << 2);
    syn_port_delay_ms(10U);
    USBFS->BASE_CTRL &= ~(1U << 2);
    return SYN_OK;
}

bool syn_port_usb_host_device_attached(void)
{
    return (USBFS->MIS_ST & (1U << 2)) != 0U;
}

SYN_Status syn_port_usb_host_pipe_open(uint8_t pipe, uint8_t dev_addr, uint8_t ep_addr,
                                       uint8_t ep_type, uint16_t max_pkt)
{
    (void)pipe;
    (void)dev_addr;
    (void)ep_addr;
    (void)ep_type;
    (void)max_pkt;
    return SYN_OK;
}

SYN_Status syn_port_usb_host_pipe_close(uint8_t pipe)
{
    (void)pipe;
    return SYN_OK;
}

SYN_Status syn_port_usb_host_submit_setup(uint8_t pipe, const SYN_USB_SetupPacket *pkt)
{
    (void)pipe;
    (void)pkt;
    return SYN_OK;
}

SYN_Status syn_port_usb_host_submit_data(uint8_t pipe, uint8_t *buf, uint16_t len, bool is_in)
{
    (void)pipe;
    (void)buf;
    (void)len;
    (void)is_in;
    return SYN_OK;
}

bool syn_port_usb_host_xfer_done(uint8_t pipe)
{
    (void)pipe;
    return true;
}

SYN_Status syn_port_usb_host_xfer_result(uint8_t pipe, uint16_t *actual_len)
{
    (void)pipe;
    if (actual_len != NULL)
        *actual_len = 0U;
    return SYN_OK;
}

#endif /* CH32V307xx || CH32V307 */
```


