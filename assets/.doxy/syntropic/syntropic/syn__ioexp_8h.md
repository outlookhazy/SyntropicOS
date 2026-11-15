

# File syn\_ioexp.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_ioexp.h**](syn__ioexp_8h.md)

[Go to the source code of this file](syn__ioexp_8h_source.md)

_Generic I2C GPIO Expander Driver (MCP23017, MCP23008, PCF8574, TCA9555)._ 

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_soft_i2c.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_IOExp**](structSYN__IOExp.md) <br>_Generic I2C GPIO Expander Instance Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_IOExpType**](#enum-syn_ioexptype)  <br>_I2C GPIO Expander IC Type._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ioexp\_init**](#function-syn_ioexp_init) ([**SYN\_IOExp**](structSYN__IOExp.md) \* ioexp, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, [**SYN\_IOExpType**](syn__ioexp_8h.md#enum-syn_ioexptype) type) <br>_Initialize an I2C GPIO Expander instance._  |
|  [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) | [**syn\_ioexp\_read\_pin**](#function-syn_ioexp_read_pin) ([**SYN\_IOExp**](structSYN__IOExp.md) \* ioexp, uint8\_t pin) <br>_Read digital input state from a pin._  |
|  uint16\_t | [**syn\_ioexp\_read\_port**](#function-syn_ioexp_read_port) ([**SYN\_IOExp**](structSYN__IOExp.md) \* ioexp) <br>_Read entire 16-bit input port mask._  |
|  void | [**syn\_ioexp\_set\_pin\_mode**](#function-syn_ioexp_set_pin_mode) ([**SYN\_IOExp**](structSYN__IOExp.md) \* ioexp, uint8\_t pin, [**SYN\_GPIO\_Mode**](syn__defs_8h.md#enum-syn_gpio_mode) mode) <br>_Set pin mode (input vs output)._  |
|  void | [**syn\_ioexp\_write\_pin**](#function-syn_ioexp_write_pin) ([**SYN\_IOExp**](structSYN__IOExp.md) \* ioexp, uint8\_t pin, [**SYN\_GPIO\_State**](syn__defs_8h.md#enum-syn_gpio_state) state) <br>_Write digital output state to a pin._  |
|  void | [**syn\_ioexp\_write\_port**](#function-syn_ioexp_write_port) ([**SYN\_IOExp**](structSYN__IOExp.md) \* ioexp, uint16\_t mask) <br>_Write entire 16-bit output port mask._  |




























## Public Types Documentation




### enum SYN\_IOExpType 

_I2C GPIO Expander IC Type._ 
```C++
enum SYN_IOExpType {
    SYN_IOEXP_MCP23017 = 0,
    SYN_IOEXP_MCP23008 = 1,
    SYN_IOEXP_PCF8574 = 2,
    SYN_IOEXP_TCA9555 = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_ioexp\_init 

_Initialize an I2C GPIO Expander instance._ 
```C++
SYN_Status syn_ioexp_init (
    SYN_IOExp * ioexp,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    SYN_IOExpType type
) 
```





**Parameters:**


* `ioexp` Expander context. 
* `scl` I2C SCL pin. 
* `sda` I2C SDA pin. 
* `i2c_addr` I2C slave address (e.g. 0x20 or 0x27). 
* `type` Expander IC type (MCP23017, MCP23008, PCF8574, TCA9555). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ioexp\_read\_pin 

_Read digital input state from a pin._ 
```C++
SYN_GPIO_State syn_ioexp_read_pin (
    SYN_IOExp * ioexp,
    uint8_t pin
) 
```





**Parameters:**


* `ioexp` Expander context. 
* `pin` Pin index (0 to num\_pins-1). 



**Returns:**

SYN\_GPIO\_HIGH or SYN\_GPIO\_LOW. 





        

<hr>



### function syn\_ioexp\_read\_port 

_Read entire 16-bit input port mask._ 
```C++
uint16_t syn_ioexp_read_port (
    SYN_IOExp * ioexp
) 
```





**Parameters:**


* `ioexp` Expander context. 



**Returns:**

16-bit port value. 





        

<hr>



### function syn\_ioexp\_set\_pin\_mode 

_Set pin mode (input vs output)._ 
```C++
void syn_ioexp_set_pin_mode (
    SYN_IOExp * ioexp,
    uint8_t pin,
    SYN_GPIO_Mode mode
) 
```





**Parameters:**


* `ioexp` Expander context. 
* `pin` Pin index (0 to num\_pins-1). 
* `mode` Mode (SYN\_GPIO\_INPUT, SYN\_GPIO\_OUTPUT, SYN\_GPIO\_INPUT\_PULLUP). 




        

<hr>



### function syn\_ioexp\_write\_pin 

_Write digital output state to a pin._ 
```C++
void syn_ioexp_write_pin (
    SYN_IOExp * ioexp,
    uint8_t pin,
    SYN_GPIO_State state
) 
```





**Parameters:**


* `ioexp` Expander context. 
* `pin` Pin index (0 to num\_pins-1). 
* `state` State (SYN\_GPIO\_HIGH or SYN\_GPIO\_LOW). 




        

<hr>



### function syn\_ioexp\_write\_port 

_Write entire 16-bit output port mask._ 
```C++
void syn_ioexp_write_port (
    SYN_IOExp * ioexp,
    uint16_t mask
) 
```





**Parameters:**


* `ioexp` Expander context. 
* `mask` 16-bit port value. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_ioexp.h`

