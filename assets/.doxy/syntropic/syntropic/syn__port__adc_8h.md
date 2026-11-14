

# File syn\_port\_adc.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_adc.h**](syn__port__adc_8h.md)

[Go to the source code of this file](syn__port__adc_8h_source.md)

_Port contract for Analog-to-Digital Converter (ADC) hardware._ 

* `#include "../common/syn_defs.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_adc\_deinit**](#function-syn_port_adc_deinit) (uint8\_t adc\_id) <br>_De-initialize the ADC peripheral._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_adc\_init**](#function-syn_port_adc_init) (uint8\_t adc\_id, uint32\_t channel\_mask) <br>_Initialize the ADC peripheral._  |
|  uint16\_t | [**syn\_port\_adc\_read\_channel**](#function-syn_port_adc_read_channel) (uint8\_t adc\_id, uint8\_t channel) <br>_Read a single analog channel (single-shot polled read)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_adc\_start\_dma\_scan**](#function-syn_port_adc_start_dma_scan) (uint8\_t adc\_id, uint16\_t \* dest, size\_t num\_channels) <br>_Start continuous multi-channel background scan via DMA._  |




























## Public Functions Documentation




### function syn\_port\_adc\_deinit 

_De-initialize the ADC peripheral._ 
```C++
SYN_Status syn_port_adc_deinit (
    uint8_t adc_id
) 
```





**Parameters:**


* `adc_id` ADC instance index. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_adc\_init 

_Initialize the ADC peripheral._ 
```C++
SYN_Status syn_port_adc_init (
    uint8_t adc_id,
    uint32_t channel_mask
) 
```





**Parameters:**


* `adc_id` ADC instance index (0 = ADC1, 1 = ADC2, etc.). 
* `channel_mask` Bitmask of channels to configure for analog input. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_port\_adc\_read\_channel 

_Read a single analog channel (single-shot polled read)._ 
```C++
uint16_t syn_port_adc_read_channel (
    uint8_t adc_id,
    uint8_t channel
) 
```





**Parameters:**


* `adc_id` ADC instance index. 
* `channel` Channel index (0..18). 



**Returns:**

12-bit raw conversion value (0..4095). 





        

<hr>



### function syn\_port\_adc\_start\_dma\_scan 

_Start continuous multi-channel background scan via DMA._ 
```C++
SYN_Status syn_port_adc_start_dma_scan (
    uint8_t adc_id,
    uint16_t * dest,
    size_t num_channels
) 
```





**Parameters:**


* `adc_id` ADC instance index. 
* `dest` Destination buffer in SRAM. 
* `num_channels` Number of active channels in scan sequence. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_adc.h`

