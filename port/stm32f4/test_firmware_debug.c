/**
 * @file test_firmware_debug.c
 * @brief Absolute minimal UART output to debug Renode STM32F4 simulation.
 *
 * Just writes "HELLO\n" to USART2 and halts.
 * If this doesn't produce output, the issue is in UART/RCC initialization.
 */

#include <stdint.h>

/* Register addresses */
#define RCC_APB1ENR  (*(volatile uint32_t *)0x40023840)
#define GPIOA_MODER  (*(volatile uint32_t *)0x40020000)
#define GPIOA_AFRLO  (*(volatile uint32_t *)0x40020020)
#define RCC_AHB1ENR  (*(volatile uint32_t *)0x40023830)

#define USART2_SR   (*(volatile uint32_t *)0x40004400)
#define USART2_DR   (*(volatile uint32_t *)0x40004404)
#define USART2_BRR  (*(volatile uint32_t *)0x40004408)
#define USART2_CR1  (*(volatile uint32_t *)0x4000440C)

void setUp(void) {}
void tearDown(void) {}

static void uart_putc(char c)
{
    /* Wait for TXE */
    while (!(USART2_SR & (1U << 7))) {}
    USART2_DR = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

int main(void)
{
    /* Enable GPIOA clock (for USART2 TX = PA2) */
    RCC_AHB1ENR |= (1U << 0);

    /* PA2 = AF7 (USART2 TX) */
    GPIOA_MODER &= ~(3U << (2 * 2));
    GPIOA_MODER |= (2U << (2 * 2));    /* AF mode */
    GPIOA_AFRLO &= ~(0xFU << (2 * 4));
    GPIOA_AFRLO |= (7U << (2 * 4));    /* AF7 = USART2 */

    /* Enable USART2 clock */
    RCC_APB1ENR |= (1U << 17);

    /* Configure USART2: 115200 baud at 16MHz */
    USART2_CR1 = 0;
    USART2_BRR = 16000000 / 115200;    /* ~139 */
    USART2_CR1 = (1U << 13) |          /* UE */
                 (1U << 3);            /* TE */

    uart_puts("HELLO FROM STM32F4\r\n");
    uart_puts("UART WORKS\r\n");

    for (;;) { __asm volatile("bkpt #0"); }
}
