/**
 * @file startup_ch32v307.c
 * @brief Bare-metal startup code and vector table for WCH CH32V307VCT6 (QingKe V4F RISC-V).
 */

#include <stdint.h>

/** @brief Start address of initialized data section in Flash. */
extern uint32_t _sidata;
/** @brief Start address of initialized data section in RAM. */
extern uint32_t _sdata;
/** @brief End address of initialized data section in RAM. */
extern uint32_t _edata;
/** @brief Start address of uninitialized data section in RAM. */
extern uint32_t _sbss;
/** @brief End address of uninitialized data section in RAM. */
extern uint32_t _ebss;
/** @brief Initial top of stack address in RAM. */
extern uint32_t _estack;

/**
 * @brief Main application entry point function declaration.
 * @return Exit status code.
 */
extern int main(void);

/**
 * @brief Platform system initialization hook invoked before main().
 */
void syn_port_system_init(void) __attribute__((weak));

/**
 * @brief Naked hardware reset entry and QingKe V4F vector table root.
 */
void _start(void);

/**
 * @brief MCU Reset handler — initializes memory sections and jumps to main().
 */
void Reset_Handler(void);

#if defined(__riscv)
#define SYN_ISR_ATTR __attribute__((interrupt("machine")))
#else
#define SYN_ISR_ATTR
#endif

/**
 * @brief Default unhandled exception/interrupt catch handler.
 */
SYN_ISR_ATTR void Default_Handler(void);

/**
 * @brief Non-Maskable Interrupt (NMI) handler.
 */
SYN_ISR_ATTR void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));

/**
 * @brief HardFault exception handler.
 */
SYN_ISR_ATTR void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));

/**
 * @brief SysTick timer interrupt handler.
 */
SYN_ISR_ATTR void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/**
 * @brief USART1 serial communication interrupt handler.
 */
SYN_ISR_ATTR void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/**
 * @brief CAN1 receive mailbox 0 interrupt handler.
 */
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
/**
 * @brief Host analysis fallback vector table matching QingKe V4F slot indices.
 */
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
