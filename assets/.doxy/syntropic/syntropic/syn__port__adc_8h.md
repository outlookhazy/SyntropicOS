

# File syn\_port\_adc.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_adc.h**](syn__port__adc_8h.md)

[Go to the source code of this file](syn__port__adc_8h_source.md)

_ADC port interface — implement these for your platform._ 

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_adc\_init**](#function-syn_port_adc_init) (uint8\_t channel) <br>_Initialize an ADC channel._  |
|  uint16\_t | [**syn\_port\_adc\_read**](#function-syn_port_adc_read) (uint8\_t channel) <br>_Read a single ADC sample._  |
|  uint16\_t | [**syn\_port\_adc\_reference\_mv**](#function-syn_port_adc_reference_mv) (void) <br>_Get the ADC reference voltage in millivolts._  |
|  uint8\_t | [**syn\_port\_adc\_resolution**](#function-syn_port_adc_resolution) (void) <br>_Get the ADC resolution in bits._  |




























## Public Functions Documentation




### function syn\_port\_adc\_init 

_Initialize an ADC channel._ 
```C++
SYN_Status syn_port_adc_init (
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
uint16_t syn_port_adc_read (
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
uint16_t syn_port_adc_reference_mv (
    void
) 
```





**Returns:**

Reference voltage (e.g., 3300 for 3.3V). 





        

<hr>



### function syn\_port\_adc\_resolution 

_Get the ADC resolution in bits._ 
```C++
uint8_t syn_port_adc_resolution (
    void
) 
```





**Returns:**

Resolution (e.g., 10, 12, 16). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_adc.h`

