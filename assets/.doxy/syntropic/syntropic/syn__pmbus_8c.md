

# File syn\_pmbus.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_pmbus.c**](syn__pmbus_8c.md)

[Go to the source code of this file](syn__pmbus_8c_source.md)

_PMBus Protocol Engine implementation._ 

* `#include "syn_pmbus.h"`
* `#include <math.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_pmbus\_encode\_read\_cmd**](#function-syn_pmbus_encode_read_cmd) ([**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t slave\_addr, uint8\_t cmd, [**bool**](syn__defs_8h.md#enum-bool) use\_pec) <br>_Encode a PMBus telemetry read request into an SMBus packet._  |
|  uint16\_t | [**syn\_pmbus\_float\_to\_linear11**](#function-syn_pmbus_float_to_linear11) (float val) <br>_Convert floating-point value to PMBus 16-bit Linear11 format._  |
|  uint16\_t | [**syn\_pmbus\_float\_to\_linear16**](#function-syn_pmbus_float_to_linear16) (float val, uint8\_t vout\_mode) <br>_Convert floating-point output voltage to PMBus 16-bit Linear16 format._  |
|  float | [**syn\_pmbus\_linear11\_to\_float**](#function-syn_pmbus_linear11_to_float) (uint16\_t raw) <br>_Convert PMBus 16-bit Linear11 format (5-bit exponent + 11-bit mantissa) to float._  |
|  float | [**syn\_pmbus\_linear16\_to\_float**](#function-syn_pmbus_linear16_to_float) (uint16\_t raw, uint8\_t vout\_mode) <br>_Convert PMBus 16-bit Linear16 format (unsigned 16-bit mantissa + VOUT\_MODE exponent) to float._  |




























## Public Functions Documentation




### function syn\_pmbus\_encode\_read\_cmd 

_Encode a PMBus telemetry read request into an SMBus packet._ 
```C++
void syn_pmbus_encode_read_cmd (
    SYN_SMBUS_Packet * pkt,
    uint8_t slave_addr,
    uint8_t cmd,
    bool use_pec
) 
```





**Parameters:**


* `pkt` Output SMBus packet. 
* `slave_addr` 7-bit PMBus slave address. 
* `cmd` PMBus command code (e.g., SYN\_PMBUS\_CMD\_READ\_VIN). 
* `use_pec` Enable Packet Error Checking (PEC). 




        

<hr>



### function syn\_pmbus\_float\_to\_linear11 

_Convert floating-point value to PMBus 16-bit Linear11 format._ 
```C++
uint16_t syn_pmbus_float_to_linear11 (
    float val
) 
```





**Parameters:**


* `val` Floating-point value. 



**Returns:**

Encoded 16-bit Linear11 word. 





        

<hr>



### function syn\_pmbus\_float\_to\_linear16 

_Convert floating-point output voltage to PMBus 16-bit Linear16 format._ 
```C++
uint16_t syn_pmbus_float_to_linear16 (
    float val,
    uint8_t vout_mode
) 
```





**Parameters:**


* `val` Floating-point voltage value. 
* `vout_mode` VOUT\_MODE byte (or raw 5-bit signed exponent N). 



**Returns:**

Encoded 16-bit Linear16 raw mantissa word. 





        

<hr>



### function syn\_pmbus\_linear11\_to\_float 

_Convert PMBus 16-bit Linear11 format (5-bit exponent + 11-bit mantissa) to float._ 
```C++
float syn_pmbus_linear11_to_float (
    uint16_t raw
) 
```





**Parameters:**


* `raw` 16-bit raw Linear11 word. 



**Returns:**

Decoded floating-point value. 





        

<hr>



### function syn\_pmbus\_linear16\_to\_float 

_Convert PMBus 16-bit Linear16 format (unsigned 16-bit mantissa + VOUT\_MODE exponent) to float._ 
```C++
float syn_pmbus_linear16_to_float (
    uint16_t raw,
    uint8_t vout_mode
) 
```





**Parameters:**


* `raw` 16-bit raw mantissa word. 
* `vout_mode` VOUT\_MODE byte (or raw 5-bit signed exponent N). 



**Returns:**

Decoded floating-point output voltage. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_pmbus.c`

