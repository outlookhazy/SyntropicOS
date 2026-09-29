

# File syn\_dshot\_telemetry.c



[**FileList**](files.md) **>** [**output**](dir_1e1a1e0352188cb1bf4d84e529f1e02b.md) **>** [**syn\_dshot\_telemetry.c**](syn__dshot__telemetry_8c.md)

[Go to the source code of this file](syn__dshot__telemetry_8c_source.md)

_Zero-Heap Bidirectional DShot (BDShot) Telemetry Decoder Implementation._ 

* `#include "syntropic/output/syn_dshot_telemetry.h"`
* `#include "syntropic/output/syn_dshot.h"`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const int8\_t | [**gcr\_to\_nibble**](#variable-gcr_to_nibble)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dshot\_decode\_gcr\_20bit**](#function-syn_dshot_decode_gcr_20bit) (uint32\_t gcr\_20bit, uint16\_t \* payload\_out) <br>_Decode 20-bit raw GCR bitstream into 16-bit BDShot telemetry payload._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dshot\_parse\_telemetry**](#function-syn_dshot_parse_telemetry) (uint32\_t gcr\_20bit, uint8\_t pole\_pairs, [**SYN\_DShot\_Telemetry**](structSYN__DShot__Telemetry.md) \* telemetry) <br>_Decode complete 20-bit BDShot GCR packet into telemetry measurements._  |




























## Public Static Attributes Documentation




### variable gcr\_to\_nibble 

```C++
const int8_t gcr_to_nibble[32];
```



GCR (Group Code Recording) 5-bit to 4-bit nibble lookup table (-1 = invalid) 


        

<hr>
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
The documentation for this class was generated from the following file `src/syntropic/output/syn_dshot_telemetry.c`

