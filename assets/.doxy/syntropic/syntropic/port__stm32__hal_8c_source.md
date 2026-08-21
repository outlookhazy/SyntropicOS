

# File port\_stm32\_hal.c

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**stm32\_hal**](dir_beb8b7258a6e20be6ee3b2db46ef4785.md) **>** [**port\_stm32\_hal.c**](port__stm32__hal_8c.md)

[Go to the documentation of this file](port__stm32__hal_8c.md)


```C++

#if !defined(ARDUINO) &&                                                                     \
    (defined(STM32F0xx) || defined(STM32F1xx) || defined(STM32F4xx) || defined(STM32F7xx) || \
     defined(STM32L4xx) || defined(STM32G0xx) || defined(STM32H7xx) || defined(STM32C0xx) || \
     defined(STM32U0xx) || defined(STM32F0) || defined(STM32F1) || defined(STM32F4) ||       \
     defined(STM32F7) || defined(STM32L4) || defined(STM32G0) || defined(STM32H7) ||         \
     defined(STM32C0) || defined(STM32U0))

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_uart.h"
#include "syntropic/util/syn_assert.h"

/* ── Assert Handler Fallback ────────────────────────────────────────────── */

SYN_WEAK SYN_NORETURN void syn_assert_failed(const char *file, int line)
{
    (void)file;
    (void)line;
    __disable_irq();
    for (;;) {
    }
}

/* ── STM32 HAL Headers ─────────────────────────────────────────────────── */
/* Adjust the include based on your target microcontroller family. */
#if defined(STM32F072xx) || defined(STM32F0) || defined(STM32F0xx)
#include "stm32f0xx_hal.h"
#elif defined(STM32F407xx) || defined(STM32F4) || defined(STM32F4xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F746xx) || defined(STM32F767xx) || defined(STM32F7) || defined(STM32F7xx)
#include "stm32f7xx_hal.h"
#elif defined(STM32F103xx) || defined(STM32F1) || defined(STM32F1xx)
#include "stm32f1xx_hal.h"
#elif defined(STM32L476xx) || defined(STM32L4) || defined(STM32L4xx)
#include "stm32l4xx_hal.h"
#elif defined(STM32G071xx) || defined(STM32G0) || defined(STM32G0xx)
#include "stm32g0xx_hal.h"
#elif defined(STM32H743xx) || defined(STM32H7) || defined(STM32H7xx)
#include "stm32h7xx_hal.h"
#elif defined(STM32C092xx) || defined(STM32C0) || defined(STM32C0xx)
#include "stm32c0xx_hal.h"
#elif defined(STM32U083xx) || defined(STM32U0) || defined(STM32U0xx)
#include "stm32u0xx_hal.h"
#else
/* Fallback: user can define this header or configure their include path */
#include "stm32_hal.h"
#endif

/* ── System Port ────────────────────────────────────────────────────────── */

static volatile uint32_t critical_nesting = 0;

void syn_port_enter_critical(void)
{
    __disable_irq();
    critical_nesting++;
}

void syn_port_exit_critical(void)
{
    if (critical_nesting > 0) {
        critical_nesting--;
        if (critical_nesting == 0) {
            __enable_irq();
        }
    }
}

uint32_t syn_port_get_tick_ms(void)
{
    return HAL_GetTick();
}

uint32_t syn_port_get_tick_us(void)
{
    uint32_t ms = HAL_GetTick();
    uint32_t load = SysTick->LOAD;
    if (load > 0) {
        uint32_t val = SysTick->VAL;
        uint32_t elapsed_cycles = load - val;
        uint32_t cycles_per_us = SystemCoreClock / 1000000U;
        if (cycles_per_us > 0) {
            return (ms * 1000U) + (elapsed_cycles / cycles_per_us);
        }
    }
    return ms * 1000U;
}

void syn_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void syn_port_system_reset(void)
{
    NVIC_SystemReset();
    for (;;)
        ;
}

/* ── GPIO Port ──────────────────────────────────────────────────────────── */

static GPIO_TypeDef *get_gpio_port(SYN_GPIO_Pin pin)
{
    uint8_t port_idx = SYN_GPIO_PIN_PORT(pin);
    switch (port_idx) {
#ifdef GPIOA
    case 0:
        return GPIOA;
#endif
#ifdef GPIOB
    case 1:
        return GPIOB;
#endif
#ifdef GPIOC
    case 2:
        return GPIOC;
#endif
#ifdef GPIOD
    case 3:
        return GPIOD;
#endif
#ifdef GPIOE
    case 4:
        return GPIOE;
#endif
#ifdef GPIOF
    case 5:
        return GPIOF;
#endif
#ifdef GPIOG
    case 6:
        return GPIOG;
#endif
#ifdef GPIOH
    case 7:
        return GPIOH;
#endif
#ifdef GPIOI
    case 8:
        return GPIOI;
#endif
    default:
        return NULL;
    }
}

static uint16_t get_gpio_pin_mask(SYN_GPIO_Pin pin)
{
    return (uint16_t)(1U << SYN_GPIO_PIN_NUM(pin));
}

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port)
        return SYN_INVALID_PARAM;

    /* Enable peripheral clock dynamically */
    uint8_t port_idx = SYN_GPIO_PIN_PORT(pin);
    switch (port_idx) {
#ifdef GPIOA
    case 0:
        __HAL_RCC_GPIOA_CLK_ENABLE();
        break;
#endif
#ifdef GPIOB
    case 1:
        __HAL_RCC_GPIOB_CLK_ENABLE();
        break;
#endif
#ifdef GPIOC
    case 2:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
#endif
#ifdef GPIOD
    case 3:
        __HAL_RCC_GPIOD_CLK_ENABLE();
        break;
#endif
#ifdef GPIOE
    case 4:
        __HAL_RCC_GPIOE_CLK_ENABLE();
        break;
#endif
#ifdef GPIOF
    case 5:
        __HAL_RCC_GPIOF_CLK_ENABLE();
        break;
#endif
#ifdef GPIOG
    case 6:
        __HAL_RCC_GPIOG_CLK_ENABLE();
        break;
#endif
#ifdef GPIOH
    case 7:
        __HAL_RCC_GPIOH_CLK_ENABLE();
        break;
#endif
#ifdef GPIOI
    case 8:
        __HAL_RCC_GPIOI_CLK_ENABLE();
        break;
#endif
    default:
        return SYN_INVALID_PARAM;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = get_gpio_pin_mask(pin);

    switch (mode) {
    case SYN_GPIO_INPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;
    case SYN_GPIO_OUTPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        break;
    case SYN_GPIO_INPUT_PULLUP:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;
    case SYN_GPIO_INPUT_PULLDOWN:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
    case SYN_GPIO_OUTPUT_OD:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        break;
    default:
        return SYN_NOT_IMPLEMENTED;
    }

    HAL_GPIO_Init(port, &GPIO_InitStruct);
    return SYN_OK;
}

SYN_Status syn_port_gpio_deinit(SYN_GPIO_Pin pin)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port)
        return SYN_INVALID_PARAM;
    HAL_GPIO_DeInit(port, get_gpio_pin_mask(pin));
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port)
        return SYN_INVALID_PARAM;
    HAL_GPIO_WritePin(port, get_gpio_pin_mask(pin),
                      (state == SYN_GPIO_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port)
        return SYN_GPIO_LOW;
    return (HAL_GPIO_ReadPin(port, get_gpio_pin_mask(pin)) == GPIO_PIN_SET) ? SYN_GPIO_HIGH
                                                                            : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port)
        return SYN_INVALID_PARAM;
    HAL_GPIO_TogglePin(port, get_gpio_pin_mask(pin));
    return SYN_OK;
}

/* ── UART Port ──────────────────────────────────────────────────────────── */

UART_HandleTypeDef *syn_port_uart_handles[6] = {NULL};

SYN_Status syn_port_stm32_register_uart(SYN_UARTInstance instance, void *huart)
{
    if (instance >= 6) {
        return SYN_INVALID_PARAM;
    }
    syn_port_uart_handles[instance] = (UART_HandleTypeDef *)huart;
    return SYN_OK;
}

static UART_HandleTypeDef *get_uart_handle(SYN_UARTInstance instance)
{
    if (instance < 6) {
        return syn_port_uart_handles[instance];
    }
    return NULL;
}

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    (void)baudrate;
    UART_HandleTypeDef *huart = get_uart_handle(instance);
    if (!huart) {
        return SYN_INVALID_PARAM;
    }
    return SYN_OK;
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    UART_HandleTypeDef *huart = get_uart_handle(instance);
    if (!huart) {
        return SYN_INVALID_PARAM;
    }
    HAL_StatusTypeDef status = HAL_UART_DeInit(huart);
    return (status == HAL_OK) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance, const uint8_t *data, size_t len,
                                  uint32_t timeout_ms)
{
    UART_HandleTypeDef *huart = get_uart_handle(instance);
    if (!huart)
        return SYN_INVALID_PARAM;

    HAL_StatusTypeDef status = HAL_UART_Transmit(huart, (uint8_t *)data, (uint16_t)len,
                                                 timeout_ms == 0 ? HAL_MAX_DELAY : timeout_ms);
    switch (status) {
    case HAL_OK:
        return SYN_OK;
    case HAL_TIMEOUT:
        return SYN_TIMEOUT;
    case HAL_BUSY:
        return SYN_BUSY;
    default:
        return SYN_ERROR;
    }
}

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance, uint8_t *data, size_t len,
                                 size_t *received, uint32_t timeout_ms)
{
    UART_HandleTypeDef *huart = get_uart_handle(instance);
    if (!huart)
        return SYN_INVALID_PARAM;

    size_t count = 0;
    uint32_t start_ms = syn_port_get_tick_ms();
    uint32_t per_byte_timeout = (timeout_ms > 0) ? 1 : 1;

    while (count < len) {
        /* Clear any overrun error before attempting receive */
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET) {
            __HAL_UART_CLEAR_OREFLAG(huart);
        }

        HAL_StatusTypeDef status = HAL_UART_Receive(huart, &data[count], 1, per_byte_timeout);
        if (status == HAL_OK) {
            count++;
        } else {
            /* No byte available within per_byte_timeout */
            if (timeout_ms > 0 && (syn_port_get_tick_ms() - start_ms) >= timeout_ms) {
                break;
            }
            if (count > 0) {
                /* We got at least one byte; return what we have */
                break;
            }
            break; /* No data, return immediately to let scheduler run */
        }
    }

    if (received)
        *received = count;
    return (count > 0) ? SYN_OK : SYN_TIMEOUT;
}

SYN_Status syn_port_uart_transmit_byte(SYN_UARTInstance instance, uint8_t byte)
{
    return syn_port_uart_transmit(instance, &byte, 1, 100);
}

SYN_Status syn_port_uart_receive_byte(SYN_UARTInstance instance, uint8_t *byte, uint32_t timeout_ms)
{
    size_t rec = 0;
    return syn_port_uart_receive(instance, byte, 1, &rec, timeout_ms);
}

/* ── Console serial port ────────────────────────────────────────────────── */

#include "syntropic/port/syn_port_serial.h"

#ifndef SYN_SERIAL_UART_INSTANCE
#define SYN_SERIAL_UART_INSTANCE 0
#endif

SYN_WEAK SYN_Status syn_port_serial_init(uint32_t baudrate)
{
    if (baudrate == 0)
        baudrate = 115200;
    return syn_port_uart_init(SYN_SERIAL_UART_INSTANCE, baudrate);
}

SYN_WEAK int syn_port_serial_write(const uint8_t *data, size_t len)
{
    SYN_Status s = syn_port_uart_transmit(SYN_SERIAL_UART_INSTANCE, data, len, 100);
    return (s == SYN_OK) ? (int)len : -1;
}

SYN_WEAK int syn_port_serial_read(uint8_t *buf, size_t max_len)
{
    size_t received = 0;
    SYN_Status s = syn_port_uart_receive(SYN_SERIAL_UART_INSTANCE, buf, max_len, &received, 0);
    if (s == SYN_TIMEOUT)
        return (int)received;
    if (s != SYN_OK)
        return -1;
    return (int)received;
}

/* ── Hardware Flash Port (STM32 HAL) ────────────────────────────────────── */

#include "syntropic/port/syn_port_flash.h"

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len)
{
    if (buf == NULL)
        return SYN_INVALID_PARAM;
    memcpy(buf, (const void *)addr, len);
    return SYN_OK;
}

SYN_Status syn_port_flash_erase(uint32_t addr)
{
    HAL_FLASH_Unlock();
#if defined(FLASH_TYPEERASE_SECTORS)
    FLASH_EraseInitTypeDef erase;
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Sector = 0; /* User sector index can be customized via port override */
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    uint32_t error = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error);
#elif defined(FLASH_TYPEERASE_PAGES)
    FLASH_EraseInitTypeDef erase;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = addr;
    erase.NbPages = 1;
    uint32_t error = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &error);
#else
    HAL_StatusTypeDef status = HAL_ERROR;
    (void)addr;
#endif
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len)
{
    if (buf == NULL)
        return SYN_INVALID_PARAM;
    const uint8_t *src = (const uint8_t *)buf;
    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_OK;
    for (size_t i = 0; i < len; i++) {
#if defined(FLASH_TYPEPROGRAM_BYTE)
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, src[i]);
#elif defined(FLASH_TYPEPROGRAM_DOUBLEWORD)
        if (i + 8 <= len && ((addr + i) % 8 == 0)) {
            uint64_t val = 0;
            memcpy(&val, &src[i], 8);
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, val);
            i += 7;
        } else {
            uint64_t val = 0xFFFFFFFFFFFFFFFFULL;
            size_t rem = len - i;
            memcpy(&val, &src[i], rem > 8 ? 8 : rem);
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, val);
            i += 7;
        }
#else
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, src[i]);
#endif
        if (status != HAL_OK)
            break;
    }
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? SYN_OK : SYN_ERROR;
}

uint32_t syn_port_flash_sector_size(uint32_t addr)
{
    (void)addr;
    return 128u * 1024u;
}

/* ── Hardware Crypto Port (STM32 CRYP) ─────────────────────────────────── */

#include "syntropic/port/syn_port_aes.h"

static void *g_hcryp = NULL;

SYN_Status syn_port_stm32_register_cryp(void *hcryp)
{
    g_hcryp = hcryp;
    return SYN_OK;
}

SYN_Status syn_port_aes_init(void)
{
#if defined(CRYP) || defined(AES)
    return (g_hcryp != NULL) ? SYN_OK : SYN_NOT_IMPLEMENTED;
#else
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_aes_encrypt_block(const uint8_t *round_keys, uint8_t nr, const uint8_t in[16],
                                      uint8_t out[16])
{
#if defined(CRYP) || defined(AES)
    if (g_hcryp == NULL || round_keys == NULL || in == NULL || out == NULL) {
        return SYN_NOT_IMPLEMENTED;
    }
#if defined(HAL_CRYP_MODULE_ENABLED)
    CRYP_HandleTypeDef *hcryp = (CRYP_HandleTypeDef *)g_hcryp;
    HAL_StatusTypeDef res = HAL_CRYP_AESECB_Encrypt(hcryp, (uint8_t *)in, 16U, out, 1000U);
    return (res == HAL_OK) ? SYN_OK : SYN_ERROR;
#else
    (void)nr;
    return SYN_NOT_IMPLEMENTED;
#endif
#else
    (void)round_keys;
    (void)nr;
    (void)in;
    (void)out;
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_aes_decrypt_block(const uint8_t *round_keys, uint8_t nr, const uint8_t in[16],
                                      uint8_t out[16])
{
#if defined(CRYP) || defined(AES)
    if (g_hcryp == NULL || round_keys == NULL || in == NULL || out == NULL) {
        return SYN_NOT_IMPLEMENTED;
    }
#if defined(HAL_CRYP_MODULE_ENABLED)
    CRYP_HandleTypeDef *hcryp = (CRYP_HandleTypeDef *)g_hcryp;
    HAL_StatusTypeDef res = HAL_CRYP_AESECB_Decrypt(hcryp, (uint8_t *)in, 16U, out, 1000U);
    return (res == HAL_OK) ? SYN_OK : SYN_ERROR;
#else
    (void)nr;
    return SYN_NOT_IMPLEMENTED;
#endif
#else
    (void)round_keys;
    (void)nr;
    (void)in;
    (void)out;
    return SYN_NOT_IMPLEMENTED;
#endif
}

SYN_Status syn_port_ghash_mult(const uint8_t x[16], const uint8_t h[16], uint8_t out[16])
{
    (void)x;
    (void)h;
    (void)out;
    return SYN_NOT_IMPLEMENTED;
}

#endif /* STM32 HAL */
```


