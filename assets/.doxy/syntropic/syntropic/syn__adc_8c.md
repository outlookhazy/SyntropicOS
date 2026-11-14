

# File syn\_adc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_adc.c**](syn__adc_8c.md)

[Go to the source code of this file](syn__adc_8c_source.md)

_General-purpose ADC driver implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_adc.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_adc\_deinit**](#function-syn_adc_deinit) ([**SYN\_ADC**](structSYN__ADC.md) \* adc) <br>_De-initialize an ADC instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_adc\_init**](#function-syn_adc_init) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, const [**SYN\_ADC\_Config**](structSYN__ADC__Config.md) \* cfg) <br>_Initialize an ADC instance._  |
|  uint32\_t | [**syn\_adc\_read\_mv**](#function-syn_adc_read_mv) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, uint8\_t channel) <br>_Read a single channel as calibrated millivolts (0..vref\_mv)._  |
|  uint16\_t | [**syn\_adc\_read\_raw**](#function-syn_adc_read_raw) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, uint8\_t channel) <br>_Read a single channel as raw 12-bit count (0..4095)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_adc\_start\_dma\_scan**](#function-syn_adc_start_dma_scan) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, uint16\_t \* buf, size\_t num\_channels) <br>_Start continuous multi-channel background scanning into SRAM via DMA._  |




























## Public Functions Documentation




### function syn\_adc\_deinit 

_De-initialize an ADC instance._ 
```C++
SYN_Status syn_adc_deinit (
    SYN_ADC * adc
) 
```





**Parameters:**


* `adc` Pointer to [**SYN\_ADC**](structSYN__ADC.md) handle. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_adc\_init 

_Initialize an ADC instance._ 
```C++
SYN_Status syn_adc_init (
    SYN_ADC * adc,
    const SYN_ADC_Config * cfg
) 
```





**Parameters:**


* `adc` Pointer to user-allocated [**SYN\_ADC**](structSYN__ADC.md) handle. 
* `cfg` Configuration parameters. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_adc\_read\_mv 

_Read a single channel as calibrated millivolts (0..vref\_mv)._ 
```C++
uint32_t syn_adc_read_mv (
    SYN_ADC * adc,
    uint8_t channel
) 
```





**Parameters:**


* `adc` Pointer to initialized ADC handle. 
* `channel` Channel index (0..18). 



**Returns:**

Calibrated voltage in millivolts. 





        

<hr>



### function syn\_adc\_read\_raw 

_Read a single channel as raw 12-bit count (0..4095)._ 
```C++
uint16_t syn_adc_read_raw (
    SYN_ADC * adc,
    uint8_t channel
) 
```





**Parameters:**


* `adc` Pointer to initialized ADC handle. 
* `channel` Channel index (0..18). 



**Returns:**

12-bit raw conversion count. 





        

<hr>



### function syn\_adc\_start\_dma\_scan 

_Start continuous multi-channel background scanning into SRAM via DMA._ 
```C++
SYN_Status syn_adc_start_dma_scan (
    SYN_ADC * adc,
    uint16_t * buf,
    size_t num_channels
) 
```





**Parameters:**


* `adc` Pointer to initialized ADC handle. 
* `buf` Destination buffer in SRAM. 
* `num_channels` Number of active channels in scan sequence. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_adc.c`

