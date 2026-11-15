

# File startup\_stm32f407.c

[**File List**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**stm32f4**](dir_110bdd47148ae3321ee97f39905b089a.md) **>** [**startup\_stm32f407.c**](startup__stm32f407_8c.md)

[Go to the documentation of this file](startup__stm32f407_8c.md)


```C++


#if defined(STM32F407xx) && !defined(ARDUINO)

#include <stdint.h>
#include <string.h>

/* Symbols defined by linker script */
extern uint32_t _sidata; /* Start of .data initializers in flash */
extern uint32_t _sdata;  /* Start of .data in SRAM */
extern uint32_t _edata;  /* End of .data in SRAM */
extern uint32_t _sbss;   /* Start of .bss */
extern uint32_t _ebss;   /* End of .bss */
extern uint32_t _estack; /* Top of stack */

extern int main(void);
extern void syn_port_system_init(void);
extern void SysTick_Handler(void);

/* Default handler — infinite loop for unhandled interrupts */
void Default_Handler(void)
{
    for (;;) {
        __asm volatile("bkpt #0");
    }
}

/* Weak alias all handlers to Default_Handler */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
/* SysTick_Handler is defined in port_stm32f4.c — not weak */

/* External peripheral IRQ handlers (STM32F407xx positions 0..81) */
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_FS_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_BRK_TIM12_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_UP_TIM13_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FSMC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SDIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ETH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ETH_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_RX1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_SCE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_FS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_EP1_OUT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_EP1_IN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DCMI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRYP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void HASH_RNG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/* Reset handler — C runtime init */
void Reset_Handler(void)
{
    /* Copy .data from flash to SRAM */
    const uint32_t *src = &_sidata;
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
    for (;;) {
        __asm volatile("bkpt #0");
    }
}

/* Vector table — placed at 0x08000000 by linker script (16 core + 82 external = 98 entries) */
__attribute__((section(".isr_vector"), used)) void (*const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),    /*  -16: Initial SP */
    Reset_Handler,                 /*  -15: Reset */
    NMI_Handler,                   /*  -14: NMI */
    HardFault_Handler,             /*  -13: Hard Fault */
    MemManage_Handler,             /*  -12: Mem Manage */
    BusFault_Handler,              /*  -11: Bus Fault */
    UsageFault_Handler,            /*  -10: Usage Fault */
    0,                             /*   -9: Reserved */
    0,                             /*   -8: Reserved */
    0,                             /*   -7: Reserved */
    0,                             /*   -6: Reserved */
    SVC_Handler,                   /*   -5: SVCall */
    DebugMon_Handler,              /*   -4: Debug Monitor */
    0,                             /*   -3: Reserved */
    PendSV_Handler,                /*   -2: PendSV */
    SysTick_Handler,               /*   -1: SysTick */
    WWDG_IRQHandler,               /*    0: Window WatchDog */
    PVD_IRQHandler,                /*    1: PVD through EXTI Line detection */
    TAMP_STAMP_IRQHandler,         /*    2: Tamper and TimeStamps through EXTI line */
    RTC_WKUP_IRQHandler,           /*    3: RTC Wakeup through EXTI line */
    FLASH_IRQHandler,              /*    4: FLASH */
    RCC_IRQHandler,                /*    5: RCC */
    EXTI0_IRQHandler,              /*    6: EXTI Line0 */
    EXTI1_IRQHandler,              /*    7: EXTI Line1 */
    EXTI2_IRQHandler,              /*    8: EXTI Line2 */
    EXTI3_IRQHandler,              /*    9: EXTI Line3 */
    EXTI4_IRQHandler,              /*   10: EXTI Line4 */
    DMA1_Stream0_IRQHandler,       /*   11: DMA1 Stream 0 */
    DMA1_Stream1_IRQHandler,       /*   12: DMA1 Stream 1 */
    DMA1_Stream2_IRQHandler,       /*   13: DMA1 Stream 2 */
    DMA1_Stream3_IRQHandler,       /*   14: DMA1 Stream 3 */
    DMA1_Stream4_IRQHandler,       /*   15: DMA1 Stream 4 */
    DMA1_Stream5_IRQHandler,       /*   16: DMA1 Stream 5 */
    DMA1_Stream6_IRQHandler,       /*   17: DMA1 Stream 6 */
    ADC_IRQHandler,                /*   18: ADC1, ADC2 and ADC3s */
    CAN1_TX_IRQHandler,            /*   19: CAN1 TX */
    CAN1_RX0_IRQHandler,           /*   20: CAN1 RX0 */
    CAN1_RX1_IRQHandler,           /*   21: CAN1 RX1 */
    CAN1_SCE_IRQHandler,           /*   22: CAN1 SCE */
    EXTI9_5_IRQHandler,            /*   23: External Line[9:5]s */
    TIM1_BRK_TIM9_IRQHandler,      /*   24: TIM1 Break and TIM9 */
    TIM1_UP_TIM10_IRQHandler,      /*   25: TIM1 Update and TIM10 */
    TIM1_TRG_COM_TIM11_IRQHandler, /*   26: TIM1 Trigger and Commutation and TIM11 */
    TIM1_CC_IRQHandler,            /*   27: TIM1 Capture Compare */
    TIM2_IRQHandler,               /*   28: TIM2 */
    TIM3_IRQHandler,               /*   29: TIM3 */
    TIM4_IRQHandler,               /*   30: TIM4 */
    I2C1_EV_IRQHandler,            /*   31: I2C1 Event */
    I2C1_ER_IRQHandler,            /*   32: I2C1 Error */
    I2C2_EV_IRQHandler,            /*   33: I2C2 Event */
    I2C2_ER_IRQHandler,            /*   34: I2C2 Error */
    SPI1_IRQHandler,               /*   35: SPI1 */
    SPI2_IRQHandler,               /*   36: SPI2 */
    USART1_IRQHandler,             /*   37: USART1 */
    USART2_IRQHandler,             /*   38: USART2 */
    USART3_IRQHandler,             /*   39: USART3 */
    EXTI15_10_IRQHandler,          /*   40: External Line[15:10]s */
    RTC_Alarm_IRQHandler,          /*   41: RTC Alarm (A and B) through EXTI Line */
    OTG_FS_WKUP_IRQHandler,        /*   42: USB OTG FS Wakeup through EXTI line */
    TIM8_BRK_TIM12_IRQHandler,     /*   43: TIM8 Break and TIM12 */
    TIM8_UP_TIM13_IRQHandler,      /*   44: TIM8 Update and TIM13 */
    TIM8_TRG_COM_TIM14_IRQHandler, /*   45: TIM8 Trigger and Commutation and TIM14 */
    TIM8_CC_IRQHandler,            /*   46: TIM8 Capture Compare */
    DMA1_Stream7_IRQHandler,       /*   47: DMA1 Stream7 */
    FSMC_IRQHandler,               /*   48: FSMC */
    SDIO_IRQHandler,               /*   49: SDIO */
    TIM5_IRQHandler,               /*   50: TIM5 */
    SPI3_IRQHandler,               /*   51: SPI3 */
    UART4_IRQHandler,              /*   52: UART4 */
    UART5_IRQHandler,              /*   53: UART5 */
    TIM6_DAC_IRQHandler,           /*   54: TIM6 and DAC1&2 underrun errors */
    TIM7_IRQHandler,               /*   55: TIM7 */
    DMA2_Stream0_IRQHandler,       /*   56: DMA2 Stream 0 */
    DMA2_Stream1_IRQHandler,       /*   57: DMA2 Stream 1 */
    DMA2_Stream2_IRQHandler,       /*   58: DMA2 Stream 2 */
    DMA2_Stream3_IRQHandler,       /*   59: DMA2 Stream 3 */
    DMA2_Stream4_IRQHandler,       /*   60: DMA2 Stream 4 */
    ETH_IRQHandler,                /*   61: Ethernet */
    ETH_WKUP_IRQHandler,           /*   62: Ethernet Wakeup through EXTI line */
    CAN2_TX_IRQHandler,            /*   63: CAN2 TX */
    CAN2_RX0_IRQHandler,           /*   64: CAN2 RX0 */
    CAN2_RX1_IRQHandler,           /*   65: CAN2 RX1 */
    CAN2_SCE_IRQHandler,           /*   66: CAN2 SCE */
    OTG_FS_IRQHandler,             /*   67: USB OTG FS */
    DMA2_Stream5_IRQHandler,       /*   68: DMA2 Stream 5 */
    DMA2_Stream6_IRQHandler,       /*   69: DMA2 Stream 6 */
    DMA2_Stream7_IRQHandler,       /*   70: DMA2 Stream 7 */
    USART6_IRQHandler,             /*   71: USART6 */
    I2C3_EV_IRQHandler,            /*   72: I2C3 event */
    I2C3_ER_IRQHandler,            /*   73: I2C3 error */
    OTG_HS_EP1_OUT_IRQHandler,     /*   74: USB OTG HS End Point 1 Out */
    OTG_HS_EP1_IN_IRQHandler,      /*   75: USB OTG HS End Point 1 In */
    OTG_HS_WKUP_IRQHandler,        /*   76: USB OTG HS Wakeup through EXTI */
    OTG_HS_IRQHandler,             /*   77: USB OTG HS */
    DCMI_IRQHandler,               /*   78: DCMI */
    CRYP_IRQHandler,               /*   79: CRYP crypto */
    HASH_RNG_IRQHandler,           /*   80: Hash and Rng */
    FPU_IRQHandler,                /*   81: FPU */
};

#endif /* STM32F407xx && !ARDUINO */
```


