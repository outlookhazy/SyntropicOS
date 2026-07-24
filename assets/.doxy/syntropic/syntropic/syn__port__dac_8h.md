

# File syn\_port\_dac.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_dac.h**](syn__port__dac_8h.md)

[Go to the source code of this file](syn__port__dac_8h_source.md)

_Platform port: Digital-to-Analog Converter (DAC)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dac\_init**](#function-syn_port_dac_init) (uint8\_t channel) <br>_Initialize a DAC channel._  |
|  uint16\_t | [**syn\_port\_dac\_reference\_mv**](#function-syn_port_dac_reference_mv) (void) <br>_Return the DAC reference voltage in millivolts._  |
|  uint8\_t | [**syn\_port\_dac\_resolution**](#function-syn_port_dac_resolution) (void) <br>_Return the DAC resolution in bits (e.g. 12 for a 12-bit DAC)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_dac\_write**](#function-syn_port_dac_write) (uint8\_t channel, uint16\_t value) <br>_Write a raw value to a DAC channel._  |




























## Detailed Description


Mirrors [**syn\_port\_adc.h**](syn__port__adc_8h.md). Implementors provide these four functions for the target hardware DAC channels. 


    
## Public Functions Documentation




### function syn\_port\_dac\_init 

_Initialize a DAC channel._ 
```C++
SYN_Status syn_port_dac_init (
    uint8_t channel
) 
```





**Parameters:**


* `channel` Platform-specific channel index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_dac\_reference\_mv 

_Return the DAC reference voltage in millivolts._ 
```C++
uint16_t syn_port_dac_reference_mv (
    void
) 
```





**Returns:**

Reference voltage in mV. 





        

<hr>



### function syn\_port\_dac\_resolution 

_Return the DAC resolution in bits (e.g. 12 for a 12-bit DAC)._ 
```C++
uint8_t syn_port_dac_resolution (
    void
) 
```





**Returns:**

Resolution in bits. 





        

<hr>



### function syn\_port\_dac\_write 

_Write a raw value to a DAC channel._ 
```C++
SYN_Status syn_port_dac_write (
    uint8_t channel,
    uint16_t value
) 
```





**Parameters:**


* `channel` Platform-specific channel index. 
* `value` Raw counts (0 .. (1 &lt;&lt; resolution) - 1). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_dac.h`

