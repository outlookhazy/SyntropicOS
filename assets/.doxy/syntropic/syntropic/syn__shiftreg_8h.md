

# File syn\_shiftreg.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_shiftreg.h**](syn__shiftreg_8h.md)

[Go to the source code of this file](syn__shiftreg_8h_source.md)

_Generic Shift Register Driver (74HC595 Output Expander & 74HC165 Input Expander)._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md) <br>_Input Shift Register Context._  |
| struct | [**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) <br>_Output Shift Register Context._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_shiftreg\_in\_get\_bit**](#function-syn_shiftreg_in_get_bit) (const [**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md) \* sr, uint16\_t bit\_index) <br>_Get logical state of a specific sampled input pin._  |
|  uint8\_t | [**syn\_shiftreg\_in\_get\_byte**](#function-syn_shiftreg_in_get_byte) (const [**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md) \* sr, uint8\_t chip\_index) <br>_Get sampled byte for a specific chip in the chain._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_shiftreg\_in\_init**](#function-syn_shiftreg_in_init) ([**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md) \* sr, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) data\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) clock\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) load\_pin, uint8\_t num\_chips) <br>_Initialize an Input Shift Register (74HC165)._  |
|  void | [**syn\_shiftreg\_in\_read**](#function-syn_shiftreg_in_read) ([**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md) \* sr) <br>_Pulse Parallel Load pin and clock in all input bits from hardware._  |
|  void | [**syn\_shiftreg\_in\_set\_bit\_order**](#function-syn_shiftreg_in_set_bit_order) ([**SYN\_ShiftRegIn**](structSYN__ShiftRegIn.md) \* sr, [**bool**](syn__defs_8h.md#enum-bool) msb\_first) <br>_Set bit order mode for input sampling._  |
|  void | [**syn\_shiftreg\_out\_flush**](#function-syn_shiftreg_out_flush) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr) <br>_Atomically flush buffer contents to physical hardware output pins._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_shiftreg\_out\_init**](#function-syn_shiftreg_out_init) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) data\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) clock\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) latch\_pin, uint8\_t num\_chips) <br>_Initialize an Output Shift Register (74HC595)._  |
|  void | [**syn\_shiftreg\_out\_set\_bit**](#function-syn_shiftreg_out_set_bit) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr, uint16\_t bit\_index, [**bool**](syn__defs_8h.md#enum-bool) state) <br>_Set logical state of a single pin in the shift register chain._  |
|  void | [**syn\_shiftreg\_out\_set\_bit\_order**](#function-syn_shiftreg_out_set_bit_order) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr, [**bool**](syn__defs_8h.md#enum-bool) msb\_first) <br>_Set bit order mode._  |
|  void | [**syn\_shiftreg\_out\_set\_enable**](#function-syn_shiftreg_out_set_enable) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr, [**bool**](syn__defs_8h.md#enum-bool) enable) <br>_Enable or disable outputs via Output Enable (OE) pin._  |
|  void | [**syn\_shiftreg\_out\_set\_oe\_pin**](#function-syn_shiftreg_out_set_oe_pin) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) oe\_pin) <br>_Configure optional Output Enable (OE) pin._  |
|  void | [**syn\_shiftreg\_out\_write\_byte**](#function-syn_shiftreg_out_write_byte) ([**SYN\_ShiftRegOut**](structSYN__ShiftRegOut.md) \* sr, uint8\_t chip\_index, uint8\_t val) <br>_Set raw byte for a specific chip in the chain._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SHIFTREG\_MAX\_CHIPS**](syn__shiftreg_8h.md#define-syn_shiftreg_max_chips)  `8`<br> |

## Public Functions Documentation




### function syn\_shiftreg\_in\_get\_bit 

_Get logical state of a specific sampled input pin._ 
```C++
bool syn_shiftreg_in_get_bit (
    const SYN_ShiftRegIn * sr,
    uint16_t bit_index
) 
```





**Parameters:**


* `sr` Shift register context. 
* `bit_index` Pin index (0 to num\_chips\*8 - 1). 



**Returns:**

Logical state (true/false). 





        

<hr>



### function syn\_shiftreg\_in\_get\_byte 

_Get sampled byte for a specific chip in the chain._ 
```C++
uint8_t syn_shiftreg_in_get_byte (
    const SYN_ShiftRegIn * sr,
    uint8_t chip_index
) 
```





**Parameters:**


* `sr` Shift register context. 
* `chip_index` Chip index (0 to num\_chips-1). 



**Returns:**

8-bit byte value. 





        

<hr>



### function syn\_shiftreg\_in\_init 

_Initialize an Input Shift Register (74HC165)._ 
```C++
SYN_Status syn_shiftreg_in_init (
    SYN_ShiftRegIn * sr,
    SYN_GPIO_Pin data_pin,
    SYN_GPIO_Pin clock_pin,
    SYN_GPIO_Pin load_pin,
    uint8_t num_chips
) 
```





**Parameters:**


* `sr` Shift register context. 
* `data_pin` Serial Data In GPIO pin. 
* `clock_pin` Shift Clock GPIO pin. 
* `load_pin` Parallel Load GPIO pin. 
* `num_chips` Number of cascaded 8-bit chips (1 to 8). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_shiftreg\_in\_read 

_Pulse Parallel Load pin and clock in all input bits from hardware._ 
```C++
void syn_shiftreg_in_read (
    SYN_ShiftRegIn * sr
) 
```





**Parameters:**


* `sr` Shift register context. 




        

<hr>



### function syn\_shiftreg\_in\_set\_bit\_order 

_Set bit order mode for input sampling._ 
```C++
void syn_shiftreg_in_set_bit_order (
    SYN_ShiftRegIn * sr,
    bool msb_first
) 
```





**Parameters:**


* `sr` Shift register context. 
* `msb_first` True for MSB-first, false for LSB-first. 




        

<hr>



### function syn\_shiftreg\_out\_flush 

_Atomically flush buffer contents to physical hardware output pins._ 
```C++
void syn_shiftreg_out_flush (
    SYN_ShiftRegOut * sr
) 
```





**Parameters:**


* `sr` Shift register context. 




        

<hr>



### function syn\_shiftreg\_out\_init 

_Initialize an Output Shift Register (74HC595)._ 
```C++
SYN_Status syn_shiftreg_out_init (
    SYN_ShiftRegOut * sr,
    SYN_GPIO_Pin data_pin,
    SYN_GPIO_Pin clock_pin,
    SYN_GPIO_Pin latch_pin,
    uint8_t num_chips
) 
```





**Parameters:**


* `sr` Shift register context. 
* `data_pin` Serial Data GPIO pin. 
* `clock_pin` Shift Clock GPIO pin. 
* `latch_pin` Latch / Storage Clock GPIO pin. 
* `num_chips` Number of cascaded 8-bit chips (1 to 8). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_shiftreg\_out\_set\_bit 

_Set logical state of a single pin in the shift register chain._ 
```C++
void syn_shiftreg_out_set_bit (
    SYN_ShiftRegOut * sr,
    uint16_t bit_index,
    bool state
) 
```





**Parameters:**


* `sr` Shift register context. 
* `bit_index` Pin index (0 to num\_chips\*8 - 1). 
* `state` Logical state. 




        

<hr>



### function syn\_shiftreg\_out\_set\_bit\_order 

_Set bit order mode._ 
```C++
void syn_shiftreg_out_set_bit_order (
    SYN_ShiftRegOut * sr,
    bool msb_first
) 
```





**Parameters:**


* `sr` Shift register context. 
* `msb_first` True for MSB-first, false for LSB-first. 




        

<hr>



### function syn\_shiftreg\_out\_set\_enable 

_Enable or disable outputs via Output Enable (OE) pin._ 
```C++
void syn_shiftreg_out_set_enable (
    SYN_ShiftRegOut * sr,
    bool enable
) 
```





**Parameters:**


* `sr` Shift register context. 
* `enable` True to enable outputs (OE LOW). 




        

<hr>



### function syn\_shiftreg\_out\_set\_oe\_pin 

_Configure optional Output Enable (OE) pin._ 
```C++
void syn_shiftreg_out_set_oe_pin (
    SYN_ShiftRegOut * sr,
    SYN_GPIO_Pin oe_pin
) 
```





**Parameters:**


* `sr` Shift register context. 
* `oe_pin` Output Enable GPIO pin. 




        

<hr>



### function syn\_shiftreg\_out\_write\_byte 

_Set raw byte for a specific chip in the chain._ 
```C++
void syn_shiftreg_out_write_byte (
    SYN_ShiftRegOut * sr,
    uint8_t chip_index,
    uint8_t val
) 
```





**Parameters:**


* `sr` Shift register context. 
* `chip_index` Chip index (0 to num\_chips-1). 
* `val` 8-bit byte value. 




        

<hr>
## Macro Definition Documentation





### define SYN\_SHIFTREG\_MAX\_CHIPS 

```C++
#define SYN_SHIFTREG_MAX_CHIPS `8`
```



Maximum daisy-chained shift register chips (8) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_shiftreg.h`

