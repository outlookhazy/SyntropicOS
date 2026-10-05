

# File syn\_dshot\_telemetry.h



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot\_telemetry.h**](syn__dshot__telemetry_8h.md)

[Go to the source code of this file](syn__dshot__telemetry_8h_source.md)

_Zero-Heap Bidirectional DShot (BDShot) Telemetry Decoder._ [More...](#detailed-description)

* `#include "syntropic/common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DShot\_Telemetry**](structSYN__DShot__Telemetry.md) <br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dshot\_decode\_gcr\_20bit**](#function-syn_dshot_decode_gcr_20bit) (uint32\_t gcr\_20bit, uint16\_t \* payload\_out) <br>_Decode 20-bit raw GCR bitstream into 16-bit BDShot telemetry payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dshot\_parse\_telemetry**](#function-syn_dshot_parse_telemetry) (uint32\_t gcr\_20bit, uint8\_t pole\_pairs, [**SYN\_DShot\_Telemetry**](structSYN__DShot__Telemetry.md) \* telemetry) <br>_Decode complete 20-bit BDShot GCR packet into telemetry measurements._  |




























## Detailed Description


BDShot Specifications:
* ESC returns a 20-bit GCR (Group Code Recording 5b/4b) encoded packet on the DShot line.
* 20 GCR bits -&gt; 16 decoded data bits:
  * Bits [15:4]: 12-bit eRPM period value (mantissa & exponent format).
  * Bits [3:0]: 4-bit CRC (computed via `(payload ^ (payload >> 4) ^ (payload >> 8)) & 0x0F`).


* eRPM Calculation:
  * `period_us = (mantissa << exponent)`
  * `eRPM = 60,000,000 / period_us`
  * `RPM = eRPM / (pole_pairs / 2)` 






    
## Public Functions Documentation




### function syn\_dshot\_decode\_gcr\_20bit 

_Decode 20-bit raw GCR bitstream into 16-bit BDShot telemetry payload._ 
```C++
SYN_Status syn_dshot_decode_gcr_20bit (
    uint32_t gcr_20bit,
    uint16_t * payload_out
) 
```





**Parameters:**


* `gcr_20bit` 20-bit raw GCR frame. 
* `payload_out` Pointer to receive 16-bit decoded payload (`[15:4] period`, `[3:0] crc`). 



**Returns:**

SYN\_OK on valid GCR decode, SYN\_ERROR if invalid GCR symbol found. 





        

<hr>



### function syn\_dshot\_parse\_telemetry 

_Decode complete 20-bit BDShot GCR packet into telemetry measurements._ 
```C++
SYN_Status syn_dshot_parse_telemetry (
    uint32_t gcr_20bit,
    uint8_t pole_pairs,
    SYN_DShot_Telemetry * telemetry
) 
```





**Parameters:**


* `gcr_20bit` 20-bit raw GCR frame. 
* `pole_pairs` Motor pole pairs (e.g., 14 poles = 7 pole pairs). Default 7 if 0. 
* `telemetry` Pointer to output telemetry structure. 



**Returns:**

SYN\_OK on valid decode and CRC, SYN\_ERROR on bad GCR/CRC. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/output/syn_dshot_telemetry.h`

