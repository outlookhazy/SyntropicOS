

# File syn\_port\_stubs.c



[**FileList**](files.md) **>** [**port\_stubs**](dir_faf66d44489c906207a31f15a9acbcc2.md) **>** [**syn\_port\_stubs.c**](syn__port__stubs_8c.md)

[Go to the source code of this file](syn__port__stubs_8c_source.md)

_Weak default implementations for all port functions._ [More...](#detailed-description)

* `#include "../common/syn_compiler.h"`
* `#include "../common/syn_defs.h"`
* `#include "../util/syn_assert.h"`
* `#include "../port/syn_port_spi.h"`
* `#include "../port/syn_port_i2c.h"`
* `#include "../port/syn_port_flash.h"`
* `#include "../port/syn_port_adc.h"`
* `#include "../port/syn_port_pwm.h"`
* `#include "../system/syn_sleep.h"`
* `#include "../port/syn_port_exti.h"`
* `#include "../port/syn_port_can.h"`
* `#include "../port/syn_port_dma.h"`
* `#include "../port/syn_port_i2c_async.h"`
* `#include "../port/syn_port_spi_async.h"`
* `#include "../common/syn_barrier.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_assert\_failed**](#function-syn_assert_failed) (const char \* file, int line) <br>_Called when an assertion fails._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_adc\_init**](#function-syn_port_adc_init) (uint8\_t channel) <br>_Initialize an ADC channel._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) uint16\_t | [**syn\_port\_adc\_read**](#function-syn_port_adc_read) (uint8\_t channel) <br>_Read a single ADC sample._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) uint16\_t | [**syn\_port\_adc\_reference\_mv**](#function-syn_port_adc_reference_mv) (void) <br>_Get the ADC reference voltage in millivolts._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) uint8\_t | [**syn\_port\_adc\_resolution**](#function-syn_port_adc_resolution) (void) <br>_Get the ADC resolution in bits._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_can\_init**](#function-syn_port_can_init) (uint8\_t port, uint32\_t bitrate) <br>_Initialize CAN peripheral._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_can\_receive**](#function-syn_port_can_receive) (uint8\_t port, uint32\_t \* id, [**bool**](syn__defs_8h.md#enum-bool) \* extended, uint8\_t \* data, uint8\_t \* dlc) <br>_Receive a CAN frame (non-blocking)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_port\_can\_send**](#function-syn_port_can_send) (uint8\_t port, uint32\_t id, [**bool**](syn__defs_8h.md#enum-bool) extended, const uint8\_t \* data, uint8\_t dlc) <br>_Send a CAN frame._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_can\_set\_filter**](#function-syn_port_can_set_filter) (uint8\_t port, uint32\_t id, uint32\_t mask) <br>_Set hardware acceptance filter._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_delay\_ms**](#function-syn_port_delay_ms) (uint32\_t ms) <br>_Blocking delay for the specified number of milliseconds._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_enter\_critical**](#function-syn_port_enter_critical) (void) <br>_Enter a critical section (disable interrupts)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_exit\_critical**](#function-syn_port_exit_critical) (void) <br>_Exit a critical section (re-enable interrupts)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_exti\_clear\_pending**](#function-syn_port_exti_clear_pending) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Clear pending interrupt flag for a pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_exti\_configure**](#function-syn_port_exti_configure) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_EXTI\_Edge**](syn__port__exti_8h.md#enum-syn_exti_edge) edge) <br>_Configure a pin for interrupt generation._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_exti\_disable**](#function-syn_port_exti_disable) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Disable interrupt on a pin (without deconfiguring)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_exti\_enable**](#function-syn_port_exti_enable) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Enable interrupt on a configured pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_flash\_erase**](#function-syn_port_flash_erase) (uint32\_t addr) <br>_Erase a flash sector._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_flash\_read**](#function-syn_port_flash_read) (uint32\_t addr, void \* buf, size\_t len) <br>_Read from flash._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) uint32\_t | [**syn\_port\_flash\_sector\_size**](#function-syn_port_flash_sector_size) (uint32\_t addr) <br>_Get the sector size for the given address._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_flash\_write**](#function-syn_port_flash_write) (uint32\_t addr, const void \* buf, size\_t len) <br>_Write to flash._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) uint32\_t | [**syn\_port\_get\_tick\_ms**](#function-syn_port_get_tick_ms) (void) <br>_Return the current system tick in milliseconds._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_deinit**](#function-syn_port_gpio_deinit) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_De-initialize a GPIO pin, returning it to its reset state._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_init**](#function-syn_port_gpio_init) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_GPIO\_Mode**](syn__defs_8h.md#enum-syn_gpio_mode) mode) <br>_Initialize a GPIO pin with the given mode._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) | [**syn\_port\_gpio\_read**](#function-syn_port_gpio_read) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Read the current logical state of a pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_toggle**](#function-syn_port_gpio_toggle) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin) <br>_Toggle the state of an output pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_gpio\_write**](#function-syn_port_gpio_write) ([**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) state) <br>_Write a logical state to an output pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_deinit**](#function-syn_port_i2c_deinit) (uint8\_t bus) <br>_Deinitialize an I2C bus._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_init**](#function-syn_port_i2c_init) (const [**SYN\_I2C\_Config**](structSYN__I2C__Config.md) \* cfg) <br>_Initialize an I2C bus._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_read**](#function-syn_port_i2c_read) (uint8\_t bus, uint8\_t addr, uint8\_t \* data, size\_t len) <br>_Read data from an I2C device._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_write**](#function-syn_port_i2c_write) (uint8\_t bus, uint8\_t addr, const uint8\_t \* data, size\_t len) <br>_Write data to an I2C device._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_i2c\_write\_read**](#function-syn_port_i2c_write_read) (uint8\_t bus, uint8\_t addr, const uint8\_t \* tx\_data, size\_t tx\_len, uint8\_t \* rx\_data, size\_t rx\_len) <br>_Write then read (register access pattern)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_memory\_barrier**](#function-syn_port_memory_barrier) (void) <br> |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_pwm\_enable**](#function-syn_port_pwm_enable) (uint8\_t channel, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable/disable PWM output._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_pwm\_init**](#function-syn_port_pwm_init) (uint8\_t channel, uint32\_t freq\_hz) <br>_Initialize a PWM channel._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_pwm\_set\_duty**](#function-syn_port_pwm_set_duty) (uint8\_t channel, uint8\_t duty\_pct) <br>_Set PWM duty cycle._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_pwm\_set\_duty\_raw**](#function-syn_port_pwm_set_duty_raw) (uint8\_t channel, uint16\_t duty\_u16) <br>_Set PWM duty cycle with fine resolution._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_pwm\_set\_freq**](#function-syn_port_pwm_set_freq) (uint8\_t channel, uint32\_t freq\_hz) <br>_Set PWM frequency (runtime change)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_sleep**](#function-syn_port_sleep) ([**SYN\_SleepMode**](syn__sleep_8h.md#enum-syn_sleepmode) mode) <br>_Enter low-power mode. Implement for your platform._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) void | [**syn\_port\_sleep\_until**](#function-syn_port_sleep_until) (uint32\_t wake\_tick\_ms) <br> |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_cs\_assert**](#function-syn_port_spi_cs_assert) (uint8\_t bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs\_pin) <br>_Assert (pull low) an SPI chip-select pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_cs\_deassert**](#function-syn_port_spi_cs_deassert) (uint8\_t bus, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) cs\_pin) <br>_Deassert (release high) an SPI chip-select pin._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_deinit**](#function-syn_port_spi_deinit) (uint8\_t bus) <br>_Deinitialize an SPI bus._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_init**](#function-syn_port_spi_init) (const [**SYN\_SPI\_Config**](structSYN__SPI__Config.md) \* cfg) <br>_Initialize an SPI bus._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_spi\_transfer**](#function-syn_port_spi_transfer) (uint8\_t bus, const uint8\_t \* tx\_buf, uint8\_t \* rx\_buf, size\_t len) <br>_Full-duplex SPI transfer._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_port\_system\_reset**](#function-syn_port_system_reset) (void) <br>_Perform a system reset._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_deinit**](#function-syn_port_uart_deinit) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance) <br>_De-initialize a UART peripheral._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_init**](#function-syn_port_uart_init) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint32\_t baudrate) <br>_Initialize a UART peripheral._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_receive**](#function-syn_port_uart_receive) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t \* data, size\_t len, size\_t \* received, uint32\_t timeout\_ms) <br>_Receive bytes into a buffer (blocking)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_receive\_byte**](#function-syn_port_uart_receive_byte) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t \* byte, uint32\_t timeout\_ms) <br>_Receive a single byte (blocking)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_transmit**](#function-syn_port_uart_transmit) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, const uint8\_t \* data, size\_t len, uint32\_t timeout\_ms) <br>_Transmit a buffer of bytes (blocking)._  |
|  [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak) [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_uart\_transmit\_byte**](#function-syn_port_uart_transmit_byte) ([**SYN\_UARTInstance**](syn__defs_8h.md#typedef-syn_uartinstance) instance, uint8\_t byte) <br>_Transmit a single byte (blocking)._  |




























## Detailed Description


Compile this file into your project to catch unimplemented port functions at runtime. Each stub calls [**syn\_assert\_failed()**](syn__port__stubs_8c.md#function-syn_assert_failed) with a descriptive message, making it immediately obvious which function you forgot to implement.


If you provide a strong definition of a function in your platform port file, the linker will silently discard the weak stub.


You can also omit this file entirely and rely on the linker to report unresolved symbols at link time — this is a perfectly valid approach. 


    
## Public Functions Documentation




### function syn\_assert\_failed 

_Called when an assertion fails._ 
```C++
SYN_WEAK  SYN_NORETURN void syn_assert_failed (
    const char * file,
    int line
) 
```



This function is declared weak so you can provide your own implementation. The default (in [**syn\_port\_stubs.c**](syn__port__stubs_8c.md)) enters an infinite loop.




**Parameters:**


* `file` Source file name where the assertion failed. 
* `line` Line number where the assertion failed. 




        

<hr>



### function syn\_port\_adc\_init 

_Initialize an ADC channel._ 
```C++
SYN_WEAK  SYN_Status syn_port_adc_init (
    uint8_t channel
) 
```





**Parameters:**


* `channel` ADC channel number. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_adc\_read 

_Read a single ADC sample._ 
```C++
SYN_WEAK uint16_t syn_port_adc_read (
    uint8_t channel
) 
```





**Parameters:**


* `channel` ADC channel number. 



**Returns:**

Raw ADC value (resolution depends on platform). 





        

<hr>



### function syn\_port\_adc\_reference\_mv 

_Get the ADC reference voltage in millivolts._ 
```C++
SYN_WEAK uint16_t syn_port_adc_reference_mv (
    void
) 
```





**Returns:**

Reference voltage (e.g., 3300 for 3.3V). 





        

<hr>



### function syn\_port\_adc\_resolution 

_Get the ADC resolution in bits._ 
```C++
SYN_WEAK uint8_t syn_port_adc_resolution (
    void
) 
```





**Returns:**

Resolution (e.g., 10, 12, 16). 





        

<hr>



### function syn\_port\_can\_init 

_Initialize CAN peripheral._ 
```C++
SYN_WEAK  bool syn_port_can_init (
    uint8_t port,
    uint32_t bitrate
) 
```





**Parameters:**


* `port` CAN port number (0, 1, ...). 
* `bitrate` Bitrate in bits/sec (e.g. 500000 for 500kbps). 



**Returns:**

true on success. 





        

<hr>



### function syn\_port\_can\_receive 

_Receive a CAN frame (non-blocking)._ 
```C++
SYN_WEAK  bool syn_port_can_receive (
    uint8_t port,
    uint32_t * id,
    bool * extended,
    uint8_t * data,
    uint8_t * dlc
) 
```





**Parameters:**


* `port` CAN port. 
* `id` [out] CAN identifier. 
* `extended` [out] true if 29-bit ID. 
* `data` [out] Frame data buffer (at least 8 bytes). 
* `dlc` [out] Data length code. 



**Returns:**

true if a frame was received. 





        

<hr>



### function syn\_port\_can\_send 

_Send a CAN frame._ 
```C++
SYN_WEAK  bool syn_port_can_send (
    uint8_t port,
    uint32_t id,
    bool extended,
    const uint8_t * data,
    uint8_t dlc
) 
```





**Parameters:**


* `port` CAN port. 
* `id` CAN identifier (11-bit or 29-bit). 
* `extended` true for 29-bit ID. 
* `data` Frame data (up to 8 bytes). 
* `dlc` Data length code (0-8). 



**Returns:**

true if frame was queued for transmission. 





        

<hr>



### function syn\_port\_can\_set\_filter 

_Set hardware acceptance filter._ 
```C++
SYN_WEAK void syn_port_can_set_filter (
    uint8_t port,
    uint32_t id,
    uint32_t mask
) 
```





**Parameters:**


* `port` CAN port. 
* `id` Filter ID. 
* `mask` Filter mask (1 = must match, 0 = don't care). 




        

<hr>



### function syn\_port\_delay\_ms 

_Blocking delay for the specified number of milliseconds._ 
```C++
SYN_WEAK void syn_port_delay_ms (
    uint32_t ms
) 
```



The implementation may busy-wait or yield to an RTOS. The delay must be at least `ms` milliseconds.




**Parameters:**


* `ms` Number of milliseconds to delay. 




        

<hr>



### function syn\_port\_enter\_critical 

_Enter a critical section (disable interrupts)._ 
```C++
SYN_WEAK void syn_port_enter_critical (
    void
) 
```



Calls may be nested; the implementation must track nesting depth and only re-enable interrupts when the outermost critical section exits. 


        

<hr>



### function syn\_port\_exit\_critical 

_Exit a critical section (re-enable interrupts)._ 
```C++
SYN_WEAK void syn_port_exit_critical (
    void
) 
```



Must be called once for each corresponding [**syn\_port\_enter\_critical()**](syn__port__system_8h.md#function-syn_port_enter_critical). 


        

<hr>



### function syn\_port\_exti\_clear\_pending 

_Clear pending interrupt flag for a pin._ 
```C++
SYN_WEAK void syn_port_exti_clear_pending (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_port\_exti\_configure 

_Configure a pin for interrupt generation._ 
```C++
SYN_WEAK  SYN_Status syn_port_exti_configure (
    SYN_GPIO_Pin pin,
    SYN_EXTI_Edge edge
) 
```





**Parameters:**


* `pin` GPIO pin number. 
* `edge` Trigger edge. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_exti\_disable 

_Disable interrupt on a pin (without deconfiguring)._ 
```C++
SYN_WEAK void syn_port_exti_disable (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_port\_exti\_enable 

_Enable interrupt on a configured pin._ 
```C++
SYN_WEAK void syn_port_exti_enable (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` GPIO pin. 




        

<hr>



### function syn\_port\_flash\_erase 

_Erase a flash sector._ 
```C++
SYN_WEAK  SYN_Status syn_port_flash_erase (
    uint32_t addr
) 
```





**Parameters:**


* `addr` Start address of the sector (must be sector-aligned). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_flash\_read 

_Read from flash._ 
```C++
SYN_WEAK  SYN_Status syn_port_flash_read (
    uint32_t addr,
    void * buf,
    size_t len
) 
```





**Parameters:**


* `addr` Source address in flash. 
* `buf` Destination buffer. 
* `len` Number of bytes to read. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_flash\_sector\_size 

_Get the sector size for the given address._ 
```C++
SYN_WEAK uint32_t syn_port_flash_sector_size (
    uint32_t addr
) 
```





**Parameters:**


* `addr` Address within the sector. 



**Returns:**

Sector size in bytes. 





        

<hr>



### function syn\_port\_flash\_write 

_Write to flash._ 
```C++
SYN_WEAK  SYN_Status syn_port_flash_write (
    uint32_t addr,
    const void * buf,
    size_t len
) 
```



Flash must be erased before writing (writes can only clear bits). The implementation should handle any alignment requirements.




**Parameters:**


* `addr` Destination address in flash. 
* `buf` Source data. 
* `len` Number of bytes to write. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_get\_tick\_ms 

_Return the current system tick in milliseconds._ 
```C++
SYN_WEAK uint32_t syn_port_get_tick_ms (
    void
) 
```



This value must be monotonically increasing and should wrap naturally at UINT32\_MAX. Typical sources: SysTick, a hardware timer, or an RTOS tick.




**Returns:**

Milliseconds since system start (or last wrap). 





        

<hr>



### function syn\_port\_gpio\_deinit 

_De-initialize a GPIO pin, returning it to its reset state._ 
```C++
SYN_WEAK  SYN_Status syn_port_gpio_deinit (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin to de-initialize. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_gpio\_init 

_Initialize a GPIO pin with the given mode._ 
```C++
SYN_WEAK  SYN_Status syn_port_gpio_init (
    SYN_GPIO_Pin pin,
    SYN_GPIO_Mode mode
) 
```





**Parameters:**


* `pin` Platform-specific pin identifier. 
* `mode` Desired pin mode (input, output, etc.). 



**Returns:**

SYN\_OK on success, or an error code. 





        

<hr>



### function syn\_port\_gpio\_read 

_Read the current logical state of a pin._ 
```C++
SYN_WEAK  SYN_GPIO_State syn_port_gpio_read (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin to read. 



**Returns:**

SYN\_GPIO\_HIGH or SYN\_GPIO\_LOW. 





        

<hr>



### function syn\_port\_gpio\_toggle 

_Toggle the state of an output pin._ 
```C++
SYN_WEAK  SYN_Status syn_port_gpio_toggle (
    SYN_GPIO_Pin pin
) 
```





**Parameters:**


* `pin` Pin to toggle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_gpio\_write 

_Write a logical state to an output pin._ 
```C++
SYN_WEAK  SYN_Status syn_port_gpio_write (
    SYN_GPIO_Pin pin,
    SYN_GPIO_State state
) 
```





**Parameters:**


* `pin` Pin to write. 
* `state` SYN\_GPIO\_HIGH or SYN\_GPIO\_LOW. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_deinit 

_Deinitialize an I2C bus._ 
```C++
SYN_WEAK  SYN_Status syn_port_i2c_deinit (
    uint8_t bus
) 
```





**Parameters:**


* `bus` I2C bus index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_init 

_Initialize an I2C bus._ 
```C++
SYN_WEAK  SYN_Status syn_port_i2c_init (
    const SYN_I2C_Config * cfg
) 
```





**Parameters:**


* `cfg` I2C configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_i2c\_read 

_Read data from an I2C device._ 
```C++
SYN_WEAK  SYN_Status syn_port_i2c_read (
    uint8_t bus,
    uint8_t addr,
    uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `bus` I2C bus index. 
* `addr` 7-bit device address. 
* `data` Buffer to receive data. 
* `len` Number of bytes to read. 



**Returns:**

SYN\_OK on ACK, SYN\_ERROR on NACK/timeout. 





        

<hr>



### function syn\_port\_i2c\_write 

_Write data to an I2C device._ 
```C++
SYN_WEAK  SYN_Status syn_port_i2c_write (
    uint8_t bus,
    uint8_t addr,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `bus` I2C bus index. 
* `addr` 7-bit device address. 
* `data` Data to write. 
* `len` Number of bytes. 



**Returns:**

SYN\_OK on ACK, SYN\_ERROR on NACK/timeout. 





        

<hr>



### function syn\_port\_i2c\_write\_read 

_Write then read (register access pattern)._ 
```C++
SYN_WEAK  SYN_Status syn_port_i2c_write_read (
    uint8_t bus,
    uint8_t addr,
    const uint8_t * tx_data,
    size_t tx_len,
    uint8_t * rx_data,
    size_t rx_len
) 
```



Sends a write (typically a register address) followed by a repeated start and read. This is the most common I2C transaction pattern.




**Parameters:**


* `bus` I2C bus index. 
* `addr` 7-bit device address. 
* `tx_data` Data to write (e.g., register address). 
* `tx_len` Write length. 
* `rx_data` Buffer for read data. 
* `rx_len` Read length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_memory\_barrier 

```C++
SYN_WEAK void syn_port_memory_barrier (
    void
) 
```




<hr>



### function syn\_port\_pwm\_enable 

_Enable/disable PWM output._ 
```C++
SYN_WEAK void syn_port_pwm_enable (
    uint8_t channel,
    bool enable
) 
```





**Parameters:**


* `channel` PWM channel index. 
* `enable` true to enable, false to disable. 




        

<hr>



### function syn\_port\_pwm\_init 

_Initialize a PWM channel._ 
```C++
SYN_WEAK  SYN_Status syn_port_pwm_init (
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `freq_hz` PWM frequency in Hz. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_pwm\_set\_duty 

_Set PWM duty cycle._ 
```C++
SYN_WEAK void syn_port_pwm_set_duty (
    uint8_t channel,
    uint8_t duty_pct
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `duty_pct` Duty cycle 0–100 (percent). 




        

<hr>



### function syn\_port\_pwm\_set\_duty\_raw 

_Set PWM duty cycle with fine resolution._ 
```C++
SYN_WEAK void syn_port_pwm_set_duty_raw (
    uint8_t channel,
    uint16_t duty_u16
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `duty_u16` Duty cycle 0–65535 (0 = off, 65535 = 100%). 




        

<hr>



### function syn\_port\_pwm\_set\_freq 

_Set PWM frequency (runtime change)._ 
```C++
SYN_WEAK void syn_port_pwm_set_freq (
    uint8_t channel,
    uint32_t freq_hz
) 
```





**Parameters:**


* `channel` PWM channel number. 
* `freq_hz` New frequency in Hz. 




        

<hr>



### function syn\_port\_sleep 

_Enter low-power mode. Implement for your platform._ 
```C++
SYN_WEAK void syn_port_sleep (
    SYN_SleepMode mode
) 
```



For Cortex-M: SYN\_SLEEP\_LIGHT → WFI, SYN\_SLEEP\_DEEP → STOP mode. Must return when an interrupt wakes the MCU.




**Parameters:**


* `mode` Requested sleep depth. 




        

<hr>



### function syn\_port\_sleep\_until 

```C++
SYN_WEAK void syn_port_sleep_until (
    uint32_t wake_tick_ms
) 
```




<hr>



### function syn\_port\_spi\_cs\_assert 

_Assert (pull low) an SPI chip-select pin._ 
```C++
SYN_WEAK  SYN_Status syn_port_spi_cs_assert (
    uint8_t bus,
    SYN_GPIO_Pin cs_pin
) 
```





**Parameters:**


* `bus` SPI bus index. 
* `cs_pin` Chip-select GPIO pin. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_cs\_deassert 

_Deassert (release high) an SPI chip-select pin._ 
```C++
SYN_WEAK  SYN_Status syn_port_spi_cs_deassert (
    uint8_t bus,
    SYN_GPIO_Pin cs_pin
) 
```





**Parameters:**


* `bus` SPI bus index. 
* `cs_pin` Chip-select GPIO pin. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_deinit 

_Deinitialize an SPI bus._ 
```C++
SYN_WEAK  SYN_Status syn_port_spi_deinit (
    uint8_t bus
) 
```





**Parameters:**


* `bus` SPI bus index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_init 

_Initialize an SPI bus._ 
```C++
SYN_WEAK  SYN_Status syn_port_spi_init (
    const SYN_SPI_Config * cfg
) 
```





**Parameters:**


* `cfg` SPI configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_spi\_transfer 

_Full-duplex SPI transfer._ 
```C++
SYN_WEAK  SYN_Status syn_port_spi_transfer (
    uint8_t bus,
    const uint8_t * tx_buf,
    uint8_t * rx_buf,
    size_t len
) 
```



Simultaneously transmits from tx\_buf and receives into rx\_buf. Either buffer can be NULL for half-duplex operation.




**Parameters:**


* `bus` SPI bus index. 
* `tx_buf` Transmit buffer (NULL to send 0x00/0xFF). 
* `rx_buf` Receive buffer (NULL to discard received data). 
* `len` Number of bytes to transfer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_system\_reset 

_Perform a system reset._ 
```C++
SYN_WEAK  SYN_NORETURN void syn_port_system_reset (
    void
) 
```



This function should not return. On Cortex-M, this is typically NVIC\_SystemReset(). 


        

<hr>



### function syn\_port\_uart\_deinit 

_De-initialize a UART peripheral._ 
```C++
SYN_WEAK  SYN_Status syn_port_uart_deinit (
    SYN_UARTInstance instance
) 
```





**Parameters:**


* `instance` UART instance to de-initialize. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_uart\_init 

_Initialize a UART peripheral._ 
```C++
SYN_WEAK  SYN_Status syn_port_uart_init (
    SYN_UARTInstance instance,
    uint32_t baudrate
) 
```





**Parameters:**


* `instance` UART instance number (0, 1, 2, …). 
* `baudrate` Desired baud rate. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_uart\_receive 

_Receive bytes into a buffer (blocking)._ 
```C++
SYN_WEAK  SYN_Status syn_port_uart_receive (
    SYN_UARTInstance instance,
    uint8_t * data,
    size_t len,
    size_t * received,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `instance` UART instance. 
* `data` Buffer to receive into. 
* `len` Maximum number of bytes to receive. 
* `received` [out] Actual number of bytes received. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK on success, SYN\_TIMEOUT if the timeout elapsed. 





        

<hr>



### function syn\_port\_uart\_receive\_byte 

_Receive a single byte (blocking)._ 
```C++
SYN_WEAK  SYN_Status syn_port_uart_receive_byte (
    SYN_UARTInstance instance,
    uint8_t * byte,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `instance` UART instance. 
* `byte` [out] Received byte. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK if a byte was received, SYN\_TIMEOUT otherwise. 





        

<hr>



### function syn\_port\_uart\_transmit 

_Transmit a buffer of bytes (blocking)._ 
```C++
SYN_WEAK  SYN_Status syn_port_uart_transmit (
    SYN_UARTInstance instance,
    const uint8_t * data,
    size_t len,
    uint32_t timeout_ms
) 
```





**Parameters:**


* `instance` UART instance. 
* `data` Pointer to data to transmit. 
* `len` Number of bytes to transmit. 
* `timeout_ms` Timeout in milliseconds (0 = no timeout). 



**Returns:**

SYN\_OK on success, SYN\_TIMEOUT if the timeout elapsed. 





        

<hr>



### function syn\_port\_uart\_transmit\_byte 

_Transmit a single byte (blocking)._ 
```C++
SYN_WEAK  SYN_Status syn_port_uart_transmit_byte (
    SYN_UARTInstance instance,
    uint8_t byte
) 
```





**Parameters:**


* `instance` UART instance. 
* `byte` Byte to transmit. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port_stubs/syn_port_stubs.c`

