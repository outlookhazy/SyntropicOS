

# File port\_stm32\_hal.h



[**FileList**](files.md) **>** [**port**](dir_fd8b95245ffcce776715f180c056b450.md) **>** [**stm32\_hal**](dir_beb8b7258a6e20be6ee3b2db46ef4785.md) **>** [**port\_stm32\_hal.h**](port__stm32__hal_8h.md)

[Go to the source code of this file](port__stm32__hal_8h_source.md)

_STM32 HAL GPIO Port Helper Macros._ 

* `#include "syntropic/drivers/syn_gpio.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_stm32\_register\_uart**](#function-syn_port_stm32_register_uart) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, void \* huart) <br>_Register an STM32 HAL UART\_HandleTypeDef pointer (e.g. &huart1) with a SyntropicOS UART instance (0..5)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_PORT\_STM32\_PIN**](port__stm32__hal_8h.md#define-syn_port_stm32_pin) (gpio\_port, gpio\_pin) `/* multi line expression */`<br>_Convert STM32 HAL GPIO\_TypeDef pointer (GPIOA, GPIOB, etc.) and pin (number 0..15 or mask GPIO\_PIN\_0..15) into a packed 16-bit SYN\_GPIO\_Pin handle._  |

## Public Functions Documentation




### function syn\_port\_stm32\_register\_uart 

_Register an STM32 HAL UART\_HandleTypeDef pointer (e.g. &huart1) with a SyntropicOS UART instance (0..5)._ 
```C++
SYN_Status syn_port_stm32_register_uart (
    SYN_UARTInstance instance,
    void * huart
) 
```





**Parameters:**


* `instance` SyntropicOS UART instance index (0..5). 
* `huart` Pointer to STM32 HAL UART\_HandleTypeDef (e.g. &huart1 or &huart3). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if instance &gt;= 6. 





        

<hr>
## Macro Definition Documentation





### define SYN\_PORT\_STM32\_PIN 

_Convert STM32 HAL GPIO\_TypeDef pointer (GPIOA, GPIOB, etc.) and pin (number 0..15 or mask GPIO\_PIN\_0..15) into a packed 16-bit SYN\_GPIO\_Pin handle._ 
```C++
#define SYN_PORT_STM32_PIN (
    gpio_port,
    gpio_pin
) `/* multi line expression */`
```



Examples: [**SYN\_PORT\_STM32\_PIN(GPIOA, 0)**](port__stm32__hal_8h.md#define-syn_port_stm32_pin) -&gt; PA0 [**SYN\_PORT\_STM32\_PIN(GPIOC, GPIO\_PIN\_13)**](port__stm32__hal_8h.md#define-syn_port_stm32_pin) -&gt; PC13 [**SYN\_PORT\_STM32\_PIN(USER\_BTN\_GPIO\_Port, USER\_BTN\_Pin)**](port__stm32__hal_8h.md#define-syn_port_stm32_pin) -&gt; CubeMX pin 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/port/stm32_hal/port_stm32_hal.h`

