/**
 * @file startup_stm32f407.c
 * @brief Minimal startup code for STM32F407 (Cortex-M4).
 *
 * Provides vector table, Reset_Handler, and default interrupt handlers.
 * Designed for bare-metal (no RTOS, no vendor HAL).
 */

#include <stdint.h>
#include <string.h>

/* Symbols defined by linker script */
extern uint32_t _sidata;   /* Start of .data initializers in flash */
extern uint32_t _sdata;    /* Start of .data in SRAM */
extern uint32_t _edata;    /* End of .data in SRAM */
extern uint32_t _sbss;     /* Start of .bss */
extern uint32_t _ebss;     /* End of .bss */
extern uint32_t _estack;   /* Top of stack */

extern int main(void);
extern void syn_port_system_init(void);
extern void SysTick_Handler(void);

/* Default handler — infinite loop for unhandled interrupts */
void Default_Handler(void)
{
    for (;;) { __asm volatile("bkpt #0"); }
}

/* Weak alias all handlers to Default_Handler */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
/* SysTick_Handler is defined in port_stm32f4.c — not weak */

/* Reset handler — C runtime init */
void Reset_Handler(void)
{
    /* Copy .data from flash to SRAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while ((uintptr_t)dst < (uintptr_t)&_edata) {
        *dst++ = *src++;
    }

    /* Zero .bss */
    dst = &_sbss;
    while ((uintptr_t)dst < (uintptr_t)&_ebss) {
        *dst++ = 0;
    }

    /* Platform init (SysTick, clocks) */
    syn_port_system_init();

    /* Jump to main */
    main();

    /* If main returns, halt */
    for (;;) { __asm volatile("bkpt #0"); }
}

/* Vector table — placed at 0x08000000 by linker script */
__attribute__((section(".isr_vector"), used))
void (*const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),   /* Initial SP */
    Reset_Handler,                 /* Reset */
    NMI_Handler,                   /* NMI */
    HardFault_Handler,             /* Hard Fault */
    MemManage_Handler,             /* Mem Manage */
    BusFault_Handler,              /* Bus Fault */
    UsageFault_Handler,            /* Usage Fault */
    0, 0, 0, 0,                    /* Reserved */
    SVC_Handler,                   /* SVCall */
    DebugMon_Handler,              /* Debug Monitor */
    0,                             /* Reserved */
    PendSV_Handler,                /* PendSV */
    SysTick_Handler,               /* SysTick */
};
