/**
 * @file port_stm32_hal.c
 * @brief SyntropicOS port layer for STM32 microcontrollers using STM32Cube HAL.
 *
 * Implements the system, GPIO, and UART interfaces by wrapping the standard
 * STM32Cube HAL peripheral APIs.
 */

#include "syntropic/common/syn_defs.h"
#include "syntropic/port/syn_port_system.h"
#include "syntropic/port/syn_port_gpio.h"
#include "syntropic/port/syn_port_uart.h"

/* ── STM32 HAL Headers ─────────────────────────────────────────────────── */
/* Adjust the include based on your target microcontroller family. */
#if defined(STM32F407xx) || defined(STM32F4) || defined(STM32F4xx)
  #include "stm32f4xx_hal.h"
#elif defined(STM32F103xx) || defined(STM32F1) || defined(STM32F1xx)
  #include "stm32f1xx_hal.h"
#elif defined(STM32L476xx) || defined(STM32L4) || defined(STM32L4xx)
  #include "stm32l4xx_hal.h"
#elif defined(STM32G071xx) || defined(STM32G0) || defined(STM32G0xx)
  #include "stm32g0xx_hal.h"
#elif defined(STM32H743xx) || defined(STM32H7) || defined(STM32H7xx)
  #include "stm32h7xx_hal.h"
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

void syn_port_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void syn_port_system_reset(void)
{
    NVIC_SystemReset();
    for (;;);
}

/* ── GPIO Port ──────────────────────────────────────────────────────────── */

/**
 * @brief Map flat SYN_GPIO_Pin index to STM32 GPIO port.
 *
 * Encodes port as (pin >> 4):
 *   0 -> GPIOA, 1 -> GPIOB, 2 -> GPIOC, etc.
 */
static GPIO_TypeDef* get_gpio_port(SYN_GPIO_Pin pin)
{
    uint8_t port_idx = pin >> 4;
    switch (port_idx) {
#ifdef GPIOA
        case 0: return GPIOA;
#endif
#ifdef GPIOB
        case 1: return GPIOB;
#endif
#ifdef GPIOC
        case 2: return GPIOC;
#endif
#ifdef GPIOD
        case 3: return GPIOD;
#endif
#ifdef GPIOE
        case 4: return GPIOE;
#endif
#ifdef GPIOF
        case 5: return GPIOF;
#endif
#ifdef GPIOG
        case 6: return GPIOG;
#endif
#ifdef GPIOH
        case 7: return GPIOH;
#endif
#ifdef GPIOI
        case 8: return GPIOI;
#endif
        default: return NULL;
    }
}

/**
 * @brief Get the standard STM32 HAL GPIO Pin mask.
 */
static uint16_t get_gpio_pin_mask(SYN_GPIO_Pin pin)
{
    return (uint16_t)(1U << (pin & 0x0F));
}

SYN_Status syn_port_gpio_init(SYN_GPIO_Pin pin, SYN_GPIO_Mode mode)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port) return SYN_INVALID_PARAM;

    /* Enable peripheral clock dynamically */
    uint8_t port_idx = pin >> 4;
    switch (port_idx) {
#ifdef GPIOA
        case 0: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
#endif
#ifdef GPIOB
        case 1: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
#endif
#ifdef GPIOC
        case 2: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
#endif
#ifdef GPIOD
        case 3: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
#endif
#ifdef GPIOE
        case 4: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
#endif
#ifdef GPIOF
        case 5: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
#endif
#ifdef GPIOG
        case 6: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
#endif
#ifdef GPIOH
        case 7: __HAL_RCC_GPIOH_CLK_ENABLE(); break;
#endif
#ifdef GPIOI
        case 8: __HAL_RCC_GPIOI_CLK_ENABLE(); break;
#endif
        default: return SYN_INVALID_PARAM;
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
    if (!port) return SYN_INVALID_PARAM;
    HAL_GPIO_DeInit(port, get_gpio_pin_mask(pin));
    return SYN_OK;
}

SYN_Status syn_port_gpio_write(SYN_GPIO_Pin pin, SYN_GPIO_State state)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port) return SYN_INVALID_PARAM;
    HAL_GPIO_WritePin(port, get_gpio_pin_mask(pin), (state == SYN_GPIO_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SYN_OK;
}

SYN_GPIO_State syn_port_gpio_read(SYN_GPIO_Pin pin)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port) return SYN_GPIO_LOW;
    return (HAL_GPIO_ReadPin(port, get_gpio_pin_mask(pin)) == GPIO_PIN_SET) ? SYN_GPIO_HIGH : SYN_GPIO_LOW;
}

SYN_Status syn_port_gpio_toggle(SYN_GPIO_Pin pin)
{
    GPIO_TypeDef *port = get_gpio_port(pin);
    if (!port) return SYN_INVALID_PARAM;
    HAL_GPIO_TogglePin(port, get_gpio_pin_mask(pin));
    return SYN_OK;
}

/* ── UART Port ──────────────────────────────────────────────────────────── */

/**
 * @brief Handle pointers for UART instances.
 *
 * To integrate with CubeMX-generated code, register the UART handles in
 * this array during system initialization:
 * @code
 *   extern UART_HandleTypeDef huart1;
 *   syn_port_uart_handles[0] = &huart1;
 * @endcode
 */
UART_HandleTypeDef* syn_port_uart_handles[6] = {NULL};

static UART_HandleTypeDef* get_uart_handle(SYN_UARTInstance instance)
{
    if (instance < 6) {
        return syn_port_uart_handles[instance];
    }
    return NULL;
}

SYN_Status syn_port_uart_init(SYN_UARTInstance instance, uint32_t baudrate)
{
    (void)baudrate;
    // Initialization is typically performed by STM32CubeMX auto-generated code
    // (e.g. MX_USART1_UART_Init()).
    // This function verifies that the handle is correctly registered.
    UART_HandleTypeDef* huart = get_uart_handle(instance);
    if (!huart) {
        return SYN_INVALID_PARAM;
    }
    return SYN_OK;
}

SYN_Status syn_port_uart_deinit(SYN_UARTInstance instance)
{
    UART_HandleTypeDef* huart = get_uart_handle(instance);
    if (!huart) {
        return SYN_INVALID_PARAM;
    }
    HAL_StatusTypeDef status = HAL_UART_DeInit(huart);
    return (status == HAL_OK) ? SYN_OK : SYN_ERROR;
}

SYN_Status syn_port_uart_transmit(SYN_UARTInstance instance,
                                  const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms)
{
    UART_HandleTypeDef* huart = get_uart_handle(instance);
    if (!huart) return SYN_INVALID_PARAM;

    HAL_StatusTypeDef status = HAL_UART_Transmit(huart, (uint8_t *)data, (uint16_t)len, timeout_ms == 0 ? HAL_MAX_DELAY : timeout_ms);
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

SYN_Status syn_port_uart_receive(SYN_UARTInstance instance,
                                 uint8_t *data,
                                 size_t len,
                                 size_t *received,
                                 uint32_t timeout_ms)
{
    UART_HandleTypeDef* huart = get_uart_handle(instance);
    if (!huart) return SYN_INVALID_PARAM;

    // HAL_UART_Receive is blocking. On timeout, we calculate how many bytes
    // were received by reading RxXferSize - RxXferCount from the handle.
    HAL_StatusTypeDef status = HAL_UART_Receive(huart, data, (uint16_t)len, timeout_ms == 0 ? HAL_MAX_DELAY : timeout_ms);

    size_t count = (size_t)(huart->RxXferSize - huart->RxXferCount);
    if (received) {
        *received = count;
    }

    if (status == HAL_OK) {
        return SYN_OK;
    } else if (status == HAL_TIMEOUT) {
        return (count > 0) ? SYN_OK : SYN_TIMEOUT;
    } else if (status == HAL_BUSY) {
        return SYN_BUSY;
    } else {
        return SYN_ERROR;
    }
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
