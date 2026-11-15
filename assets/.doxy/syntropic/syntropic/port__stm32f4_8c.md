

# File port\_stm32f4.c



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**stm32f4**](dir_110bdd47148ae3321ee97f39905b089a.md) **>** [**port\_stm32f4.c**](port__stm32f4_8c.md)

[Go to the source code of this file](port__stm32f4_8c_source.md)

_SyntropicOS port layer for STM32F407 (bare-metal, direct register access)._ [More...](#detailed-description)


































































## Detailed Description


Full register-level implementation targeting the STM32F407 Discovery board. Designed for both real hardware and Renode simulation. No vendor HAL/LL dependency — all peripheral access is via CMSIS-style direct register writes.


Peripherals implemented:
* SysTick (tick source, delay)
* GPIO (all ports A–I)
* UART (USART1–3, UART4–5)
* SPI (SPI1–3, master mode, full-duplex)
* I2C (I2C1–3, master mode, 7-bit addressing)
* Flash (erase/read/write via flash controller)
* EXTI (interrupt configuration)
* CAN (CAN1, basic TX/RX/filter)
* ADC (ADC1, single-channel software-triggered)
* Sleep (WFI-based) 




    

------------------------------
The documentation for this class was generated from the following file `src/port/stm32f4/port_stm32f4.c`

