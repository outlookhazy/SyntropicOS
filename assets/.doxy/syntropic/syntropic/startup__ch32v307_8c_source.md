

# File startup\_ch32v307.c

[**File List**](files.md) **>** [**ch32v307**](dir_876381587f01eae594c70ce97acc1e04.md) **>** [**startup\_ch32v307.c**](startup__ch32v307_8c.md)

[Go to the documentation of this file](startup__ch32v307_8c.md)


```C++

#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int main(void);

void Reset_Handler(void);

#if defined(__riscv)
#define SYN_ISR_ATTR __attribute__((interrupt("WCH-Interrupt-fast")))
#else
#define SYN_ISR_ATTR
#endif

SYN_ISR_ATTR void Default_Handler(void);

SYN_ISR_ATTR void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".vector_table"), used)) void (*const vector_table[])(void) = {
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SysTick_Handler,
    0,
    0,
    USART1_IRQHandler,
    CAN1_RX0_IRQHandler};

void Reset_Handler(void)
{
    /* Initialize Stack Pointer sp */
    __asm__ volatile("la sp, _estack");

    /* Copy .data section from Flash to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Clear .bss section in RAM */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0U;
    }

    /* Call application main */
    (void)main();

    /* Infinite loop if main returns */
    while (1) {
    }
}

SYN_ISR_ATTR void Default_Handler(void)
{
    while (1) {
    }
}
```


