

# File syn\_dshot.h



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot.h**](syn__dshot_8h.md)

[Go to the source code of this file](syn__dshot_8h_source.md)

_Zero-Heap DShot (DShot150/300/600) Digital ESC Driver._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DShot\_Packet**](structSYN__DShot__Packet.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DShot\_Speed**](#enum-syn_dshot_speed)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**syn\_dshot\_calc\_crc**](#function-syn_dshot_calc_crc) (uint16\_t payload\_12bit) <br>_Calculate 4-bit DShot CRC for an 12-bit payload (_ `throttle[11:1]` _+_`telemetry[0]` _)._ |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dshot\_encode**](#function-syn_dshot_encode) (uint16\_t throttle, [**bool**](syn__defs_8h.md#enum-bool) telemetry, [**SYN\_DShot\_Packet**](structSYN__DShot__Packet.md) \* packet) <br>_Encode throttle value and telemetry flag into a 16-bit DShot frame._  |
|  uint16\_t | [**syn\_dshot\_us\_to\_throttle**](#function-syn_dshot_us_to_throttle) (uint16\_t us) <br>_Convert standard 1000..2000 us RC pulse width to DShot 48..2047 throttle value._  |




























## Detailed Description


DShot Frame Format:
* 16 bits total:
  * Bits [15:5]: 11-bit Throttle value (0 = Disarmed/Stop, 1..47 = ESC commands, 48..2047 = Throttle command).
  * Bit [4]: Telemetry Request flag (1 = Request telemetry byte via bidirectional DShot).
  * Bits [3:0]: 4-bit CRC (computed via nibble XOR sum: `(payload ^ (payload >> 4) ^ (payload >> 8)) & 0x0F`). 






    
## Public Types Documentation




### enum SYN\_DShot\_Speed 

```C++
enum SYN_DShot_Speed {
    SYN_DSHOT150 = 150,
    SYN_DSHOT300 = 300,
    SYN_DSHOT600 = 600
};
```



DShot Speed Variant. 


        

<hr>
## Public Functions Documentation




### function syn\_dshot\_calc\_crc 

_Calculate 4-bit DShot CRC for an 12-bit payload (_ `throttle[11:1]` _+_`telemetry[0]` _)._
```C++
uint8_t syn_dshot_calc_crc (
    uint16_t payload_12bit
) 
```





**Parameters:**


* `payload_12bit` 12-bit value `(throttle << 1) | (telemetry ? 1 : 0)`. 



**Returns:**

4-bit CRC value. 





        

<hr>



### function syn\_dshot\_encode 

_Encode throttle value and telemetry flag into a 16-bit DShot frame._ 
```C++
SYN_Status syn_dshot_encode (
    uint16_t throttle,
    bool telemetry,
    SYN_DShot_Packet * packet
) 
```





**Parameters:**


* `throttle` 11-bit throttle value (clamped 0..2047). 
* `telemetry` Telemetry request flag. 
* `packet` Pointer to output packet struct. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if packet is NULL. 





        

<hr>



### function syn\_dshot\_us\_to\_throttle 

_Convert standard 1000..2000 us RC pulse width to DShot 48..2047 throttle value._ 
```C++
uint16_t syn_dshot_us_to_throttle (
    uint16_t us
) 
```





**Parameters:**


* `us` Pulse width in microseconds (1000..2000 us). 



**Returns:**

DShot throttle value (0 if us &lt; 1048, 48..2047). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_dshot.h`

