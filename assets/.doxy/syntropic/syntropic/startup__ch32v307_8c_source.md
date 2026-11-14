

# File startup\_ch32v307.c

[**File List**](files.md) **>** [**ch32v307**](dir_876381587f01eae594c70ce97acc1e04.md) **>** [**startup\_ch32v307.c**](startup__ch32v307_8c.md)

[Go to the documentation of this file](startup__ch32v307_8c.md)


```C++


#if (defined(CH32V307xx) || defined(CH32V307)) && !defined(ARDUINO)

#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int main(void);

void syn_port_system_init(void) __attribute__((weak));

void _start(void);

void Reset_Handler(void);

#if defined(__riscv)
#define SYN_ISR_ATTR __attribute__((interrupt("machine")))
#else
#define SYN_ISR_ATTR
#endif

SYN_ISR_ATTR void Default_Handler(void);

SYN_ISR_ATTR void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

SYN_ISR_ATTR void CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

#if defined(__riscv)
__attribute__((naked, section(".vector_table"), aligned(4), used)) void _start(void)
{
    __asm__ volatile(".option push\n\t"
                     ".option norvc\n\t"
                     "j Reset_Handler\n\t"
                     ".word 0\n\t"
                     ".word NMI_Handler\n\t"
                     ".word HardFault_Handler\n\t"
                     ".rept 8\n\t"
                     ".word 0\n\t"
                     ".endr\n\t"
                     ".word SysTick_Handler\n\t"
                     ".rept 23\n\t"
                     ".word 0\n\t"
                     ".endr\n\t"
                     ".word CAN1_RX0_IRQHandler\n\t"
                     ".rept 16\n\t"
                     ".word 0\n\t"
                     ".endr\n\t"
                     ".word USART1_IRQHandler\n\t"
                     ".option pop\n\t");
}
#else
__attribute__((section(".vector_table"), used)) void (*const vector_table[54])(void) = {
    [0] = Reset_Handler,    [2] = NMI_Handler,          [3] = HardFault_Handler,
    [12] = SysTick_Handler, [36] = CAN1_RX0_IRQHandler, [53] = USART1_IRQHandler,
};

void _start(void)
{
    Reset_Handler();
}
#endif

void syn_port_system_init(void)
{
}

void Reset_Handler(void)
{
#if defined(__riscv)
    /* Initialize stack pointer and vectored mtvec (MODE = 0b11) */
    __asm__ volatile("la sp, _estack\n\t"
                     "la t0, _start\n\t"
                     "ori t0, t0, 3\n\t"
                     "csrw mtvec, t0"
                     :
                     :
                     : "t0", "memory");
#endif

    /* Copy .data section from Flash to RAM */
    const uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while ((uintptr_t)dst < (uintptr_t)&_edata) {
        *dst++ = *src++;
    }

    /* Clear .bss section in RAM */
    dst = &_sbss;
    while ((uintptr_t)dst < (uintptr_t)&_ebss) {
        *dst++ = 0U;
    }

    /* Initialize platform tick timer */
    syn_port_system_init();

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

#endif /* (CH32V307xx || CH32V307) && !ARDUINO */
```


