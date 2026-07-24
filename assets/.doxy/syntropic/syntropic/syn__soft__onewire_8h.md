

# File syn\_soft\_onewire.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_onewire.h**](syn__soft__onewire_8h.md)

[Go to the source code of this file](syn__soft__onewire_8h_source.md)

_Software bit-bang 1-Wire master driver._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) <br>_1-Wire bus handle. Caller allocates; zero heap._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_soft\_onewire\_init**](#function-syn_soft_onewire_init) ([**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) pin, uint32\_t delay\_loops) <br>_Initialize a 1-Wire bus instance._  |
|  uint8\_t | [**syn\_soft\_onewire\_read\_byte**](#function-syn_soft_onewire_read_byte) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Read one byte LSB-first from the 1-Wire bus._  |
|  void | [**syn\_soft\_onewire\_read\_rom**](#function-syn_soft_onewire_read_rom) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, uint8\_t rom) <br>_Read an 8-byte ROM code from the bus (after READ ROM command)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_soft\_onewire\_reset**](#function-syn_soft_onewire_reset) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow) <br>_Issue a 1-Wire reset pulse and detect device presence._  |
|  void | [**syn\_soft\_onewire\_write\_byte**](#function-syn_soft_onewire_write_byte) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, uint8\_t byte) <br>_Write one byte LSB-first onto the 1-Wire bus._  |
|  void | [**syn\_soft\_onewire\_write\_rom**](#function-syn_soft_onewire_write_rom) (const [**SYN\_SoftOneWire**](structSYN__SoftOneWire.md) \* ow, const uint8\_t rom) <br>_Write an 8-byte ROM code to the bus (for MATCH ROM command)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SOFT\_ONEWIRE\_LOOPS\_PER\_US**](syn__soft__onewire_8h.md#define-syn_soft_onewire_loops_per_us) (freq\_hz) `((uint32\_t)(((uint32\_t)(freq\_hz)) / 4000000UL))`<br>_Compute the delay\_loops value for a given CPU frequency._  |

## Detailed Description


Implements the Dallas/Maxim 1-Wire protocol on any GPIO pin. Timing is controlled by a caller-provided loop count (delay\_loops), analogous to [**syn\_soft\_i2c.h**](syn__soft__i2c_8h.md) — calibrate for your CPU frequency.


Supports standard-speed 1-Wire (15.4 kbps). No parasitic-power or overdrive mode.


Typical use (DS18B20 temperature sensor, protothread-safe): 
```C++
// In a protothread — conversion wait is a yield point, not a busy-wait.
static uint32_t ow_start_ms;
SYN_SoftOneWire ow;

// --- Phase 1: start conversion ---
syn_soft_onewire_init(&ow, PIN_OW_DATA,
                     SYN_SOFT_ONEWIRE_LOOPS_PER_US(F_CPU));
if (syn_soft_onewire_reset(&ow)) {           // ~960 µs busy
    syn_soft_onewire_write_byte(&ow, 0xCC);  // SKIP ROM   (~560 µs)
    syn_soft_onewire_write_byte(&ow, 0x44);  // CONVERT T  (~560 µs)
    ow_start_ms = syn_port_get_tick_ms();
}

// --- Phase 2: yield until conversion done (~750 ms) ---
PT_WAIT_UNTIL(pt, syn_port_get_tick_ms() - ow_start_ms >= 750u);

// --- Phase 3: read result ---
if (syn_soft_onewire_reset(&ow)) {
    syn_soft_onewire_write_byte(&ow, 0xCC);  // SKIP ROM
    syn_soft_onewire_write_byte(&ow, 0xBE);  // READ SCRATCHPAD
    uint8_t lsb = syn_soft_onewire_read_byte(&ow);
    uint8_t msb = syn_soft_onewire_read_byte(&ow);
}
// DO NOT use syn_port_delay_ms(750) — that blocks the entire scheduler.
```





**Note:**

Each byte write/read blocks the CPU for ~560 µs (8 × 70 µs slots). Each reset() blocks for ~960 µs. Hardware PWM and timer ISRs continue to fire normally during these windows. Do not call these functions from a tight control loop that must execute faster than ~2 ms/cycle. 





    
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
## Macro Definition Documentation





### define SYN\_SOFT\_ONEWIRE\_LOOPS\_PER\_US 

_Compute the delay\_loops value for a given CPU frequency._ 
```C++
#define SYN_SOFT_ONEWIRE_LOOPS_PER_US (
    freq_hz
) `((uint32_t)(((uint32_t)(freq_hz)) / 4000000UL))`
```



The NOP loop body compiles to approximately 4 clock cycles on most architectures (load, decrement, compare, branch-not-taken). Dividing the CPU frequency by 4 gives loops per second; dividing again by 1 000 000 gives loops per microsecond.


Use this macro when initialising the bus: 
```C++
syn_soft_onewire_init(&ow, PIN_OW_DATA,
                      SYN_SOFT_ONEWIRE_LOOPS_PER_US(F_CPU));
```



Or with an explicit frequency: 
```C++
// 16 MHz Arduino Uno
syn_soft_onewire_init(&ow, PIN_OW_DATA,
                      SYN_SOFT_ONEWIRE_LOOPS_PER_US(16000000UL));
// 8 MHz AVR / STM32 at 8 MHz
syn_soft_onewire_init(&ow, PIN_OW_DATA,
                      SYN_SOFT_ONEWIRE_LOOPS_PER_US(8000000UL));
```





**Parameters:**


* `freq_hz` CPU frequency in Hz (e.g. F\_CPU, 16000000UL).



**Note:**

The divisor (4) is a conservative estimate for an optimised build. If timing measurements show pulses running consistently long or short, tune by replacing 4 with the actual loop-body cycle count from the generated disassembly. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_soft_onewire.h`

