

# File port\_ch32v307.c



[**FileList**](files.md) **>** [**ch32v307**](dir_876381587f01eae594c70ce97acc1e04.md) **>** [**port\_ch32v307.c**](port__ch32v307_8c.md)

[Go to the source code of this file](port__ch32v307_8c_source.md)

_SyntropicOS bare-metal port layer for WCH CH32V307VCT6 (QingKe V4F RISC-V)._ [More...](#detailed-description)


































































## Detailed Description


Full register-level bare-metal driver implementation for CH32V307VCT6. Zero external HAL dependencies — direct peripheral register access.


Implemented peripherals:
* SysTick / STK Timer (1ms system tick, microsecond clock, delay, reset)
* GPIO (Ports A through E, set/reset/read/toggle)
* USART (USART1..3 transmission and reception)
* Flash (Fast Flash page erase and word programming)
* SPI (SPI1 & SPI2 master full-duplex transfers)
* I2C (I2C1 master read/write transactions)
* CAN (CAN1 bxCAN-compatible frame transmission & reception)
* ADC (ADC1 single-channel 12-bit conversions & DMA scan stubs)
* EXTI (External pin interrupt configuration & masking) 




    

------------------------------
The documentation for this class was generated from the following file `src/port/ch32v307/port_ch32v307.c`

