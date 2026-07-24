

# File syn\_soft\_onewire.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_onewire.c**](syn__soft__onewire_8c.md)

[Go to the source code of this file](syn__soft__onewire_8c_source.md)

_Software bit-bang 1-Wire master implementation._ [More...](#detailed-description)

* `#include "syn_soft_onewire.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_soft\_onewire\_init**](#function-syn_soft_onewire_init) ([**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, uint32\_t delay\_loops) <br>_Initialize a 1-Wire bus instance._  |
|  uint8\_t | [**syn\_soft\_onewire\_read\_byte**](#function-syn_soft_onewire_read_byte) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Read one byte LSB-first from the 1-Wire bus._  |
|  void | [**syn\_soft\_onewire\_read\_rom**](#function-syn_soft_onewire_read_rom) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, uint8\_t rom) <br>_Read an 8-byte ROM code from the bus (after READ ROM command)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_soft\_onewire\_reset**](#function-syn_soft_onewire_reset) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Issue a 1-Wire reset pulse and detect device presence._  |
|  void | [**syn\_soft\_onewire\_write\_byte**](#function-syn_soft_onewire_write_byte) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, uint8\_t byte) <br>_Write one byte LSB-first onto the 1-Wire bus._  |
|  void | [**syn\_soft\_onewire\_write\_rom**](#function-syn_soft_onewire_write_rom) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, const uint8\_t rom) <br>_Write an 8-byte ROM code to the bus (for MATCH ROM command)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**ow\_delay**](#function-ow_delay) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, uint32\_t us) <br>_NOP-loop delay._  |
|  void | [**ow\_drive\_low**](#function-ow_drive_low) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Drive the 1-Wire pin low (open-drain)._  |
|  uint8\_t | [**ow\_read\_bit**](#function-ow_read_bit) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Read a single bit from the 1-Wire bus._  |
|  void | [**ow\_release**](#function-ow_release) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Release the 1-Wire pin (pullup takes over)._  |
|  void | [**ow\_write\_bit**](#function-ow_write_bit) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, uint8\_t bit) <br>_Write a single bit to the 1-Wire bus._  |


























## Detailed Description


Timing slots (standard speed): Write-0 slot : drive LOW 60 µs, release 10 µs recovery Write-1 slot : drive LOW 6 µs, release 64 µs recovery Read slot : drive LOW 6 µs, release, sample at ~9 µs, release 55 µs Reset pulse : drive LOW 480 µs, release, detect presence 60–240 µs, total reset cycle ~960 µs


All timing is approximated via a volatile NOP loop (delay\_loops per µs). All I/O uses syn\_port\_gpio: output = drive low, input = release/read. 


    
## Public Functions Documentation




### function syn\_soft\_onewire\_init 

_Initialize a 1-Wire bus instance._ 
```C++
void syn_soft_onewire_init (
    SYN_SoftOneWire * ow,
    SYN_GPIO_Pin pin,
    uint32_t delay_loops
) 
```





**Parameters:**


* `ow` Bus handle to initialize. Must not be NULL. 
* `pin` GPIO pin for the data line. 
* `delay_loops` NOP loop iterations per µs at your CPU frequency. Use [**SYN\_SOFT\_ONEWIRE\_LOOPS\_PER\_US(F\_CPU)**](syn__soft__onewire_8h.md#define-syn_soft_onewire_loops_per_us) or [**SYN\_SOFT\_ONEWIRE\_LOOPS\_PER\_US(16000000UL)**](syn__soft__onewire_8h.md#define-syn_soft_onewire_loops_per_us) for 16 MHz. 




        

<hr>



### function syn\_soft\_onewire\_read\_byte 

_Read one byte LSB-first from the 1-Wire bus._ 
```C++
uint8_t syn_soft_onewire_read_byte (
    const SYN_SoftOneWire * ow
) 
```





**Parameters:**


* `ow` Initialized bus handle. 



**Returns:**

Byte received. 





        

<hr>



### function syn\_soft\_onewire\_read\_rom 

_Read an 8-byte ROM code from the bus (after READ ROM command)._ 
```C++
void syn_soft_onewire_read_rom (
    const SYN_SoftOneWire * ow,
    uint8_t rom
) 
```





**Parameters:**


* `ow` Initialized bus handle. 
* `rom` Output buffer, must be at least 8 bytes. 




        

<hr>



### function syn\_soft\_onewire\_reset 

_Issue a 1-Wire reset pulse and detect device presence._ 
```C++
bool syn_soft_onewire_reset (
    const SYN_SoftOneWire * ow
) 
```





**Parameters:**


* `ow` Initialized bus handle. 



**Returns:**

true if at least one device acknowledged (presence pulse detected), false if the bus is empty. 





        

<hr>



### function syn\_soft\_onewire\_write\_byte 

_Write one byte LSB-first onto the 1-Wire bus._ 
```C++
void syn_soft_onewire_write_byte (
    const SYN_SoftOneWire * ow,
    uint8_t byte
) 
```





**Parameters:**


* `ow` Initialized bus handle. 
* `byte` Byte to transmit. 




        

<hr>



### function syn\_soft\_onewire\_write\_rom 

_Write an 8-byte ROM code to the bus (for MATCH ROM command)._ 
```C++
void syn_soft_onewire_write_rom (
    const SYN_SoftOneWire * ow,
    const uint8_t rom
) 
```





**Parameters:**


* `ow` Initialized bus handle. 
* `rom` 8-byte ROM code, LSB first. 




        

<hr>
## Public Static Functions Documentation




### function ow\_delay 

_NOP-loop delay._ 
```C++
static void ow_delay (
    const SYN_SoftOneWire * ow,
    uint32_t us
) 
```





**Parameters:**


* `ow` 1-Wire instance. 
* `us` Microseconds to delay. 




        

<hr>



### function ow\_drive\_low 

_Drive the 1-Wire pin low (open-drain)._ 
```C++
static void ow_drive_low (
    const SYN_SoftOneWire * ow
) 
```





**Parameters:**


* `ow` 1-Wire instance. 




        

<hr>



### function ow\_read\_bit 

_Read a single bit from the 1-Wire bus._ 
```C++
static uint8_t ow_read_bit (
    const SYN_SoftOneWire * ow
) 
```





**Parameters:**


* `ow` 1-Wire instance. 



**Returns:**

Bit value (0 or 1). 





        

<hr>



### function ow\_release 

_Release the 1-Wire pin (pullup takes over)._ 
```C++
static void ow_release (
    const SYN_SoftOneWire * ow
) 
```





**Parameters:**


* `ow` 1-Wire instance. 




        

<hr>



### function ow\_write\_bit 

_Write a single bit to the 1-Wire bus._ 
```C++
static void ow_write_bit (
    const SYN_SoftOneWire * ow,
    uint8_t bit
) 
```





**Parameters:**


* `ow` 1-Wire instance. 
* `bit` Bit value (0 or 1). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_onewire.c`

