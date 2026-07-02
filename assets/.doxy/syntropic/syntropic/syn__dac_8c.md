

# File syn\_dac.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_dac.c**](syn__dac_8c.md)

[Go to the source code of this file](syn__dac_8c_source.md)

_DAC driver implementation._ 

* `#include "syn_dac.h"`
* `#include "../util/syn_assert.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dac\_init**](#function-syn_dac_init) ([**SYN\_DAC**](structSYN__DAC.md) \* dac, uint8\_t channel) <br>_Initialize a DAC channel._  |
|  uint16\_t | [**syn\_dac\_max\_raw**](#function-syn_dac_max_raw) (const [**SYN\_DAC**](structSYN__DAC.md) \* dac) <br>_Return the maximum raw value for this DAC (2^resolution - 1)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dac\_write\_mv**](#function-syn_dac_write_mv) (const [**SYN\_DAC**](structSYN__DAC.md) \* dac, uint16\_t mv) <br>_Write a millivolt target to the DAC._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dac\_write\_percent**](#function-syn_dac_write_percent) (const [**SYN\_DAC**](structSYN__DAC.md) \* dac, uint8\_t percent) <br>_Write a percentage of full scale to the DAC._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dac\_write\_raw**](#function-syn_dac_write_raw) (const [**SYN\_DAC**](structSYN__DAC.md) \* dac, uint16\_t raw) <br>_Write a raw count value to the DAC._  |




























## Public Functions Documentation




### function syn\_dac\_init 

_Initialize a DAC channel._ 
```C++
SYN_Status syn_dac_init (
    SYN_DAC * dac,
    uint8_t channel
) 
```





**Parameters:**


* `dac` Handle to initialize. Must not be NULL. 
* `channel` Platform DAC channel index. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on hardware failure. 





        

<hr>



### function syn\_dac\_max\_raw 

_Return the maximum raw value for this DAC (2^resolution - 1)._ 
```C++
uint16_t syn_dac_max_raw (
    const SYN_DAC * dac
) 
```





**Parameters:**


* `dac` Initialized DAC handle (channel field used to query port). 



**Returns:**

Maximum raw count (e.g. 4095 for 12-bit). 





        

<hr>



### function syn\_dac\_write\_mv 

_Write a millivolt target to the DAC._ 
```C++
SYN_Status syn_dac_write_mv (
    const SYN_DAC * dac,
    uint16_t mv
) 
```



Clamps to the reference voltage automatically.




**Parameters:**


* `dac` Initialized DAC handle. 
* `mv` Desired output voltage in millivolts. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dac\_write\_percent 

_Write a percentage of full scale to the DAC._ 
```C++
SYN_Status syn_dac_write_percent (
    const SYN_DAC * dac,
    uint8_t percent
) 
```





**Parameters:**


* `dac` Initialized DAC handle. 
* `percent` 0 = 0 V, 100 = full reference voltage. Values &gt; 100 are clamped to 100. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_dac\_write\_raw 

_Write a raw count value to the DAC._ 
```C++
SYN_Status syn_dac_write_raw (
    const SYN_DAC * dac,
    uint16_t raw
) 
```





**Parameters:**


* `dac` Initialized DAC handle. 
* `raw` Value in range [0, syn\_dac\_max\_raw(dac)]. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_dac.c`

