

# File syn\_adc.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_adc.c**](syn__adc_8c.md)

[Go to the source code of this file](syn__adc_8c_source.md)

_ADC abstraction implementation._ 

* `#include "syn_adc.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_adc\_init**](#function-syn_adc_init) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, const [**SYN\_ADC\_Config**](structSYN__ADC__Config.md) \* cfg) <br>_Initialize an ADC channel with oversampling and calibration._  |
|  int32\_t | [**syn\_adc\_read**](#function-syn_adc_read) ([**SYN\_ADC**](structSYN__ADC.md) \* adc) <br>_Read the ADC (oversample + filter + calibrate)._  |
|  int32\_t | [**syn\_adc\_read\_mv**](#function-syn_adc_read_mv) ([**SYN\_ADC**](structSYN__ADC.md) \* adc) <br>_Read and convert to millivolts._  |
|  void | [**syn\_adc\_set\_calibration**](#function-syn_adc_set_calibration) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, int16\_t offset, uint16\_t scale, uint8\_t shift) <br>_Update calibration at runtime._  |
|  void | [**syn\_adc\_set\_stats**](#function-syn_adc_set_stats) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, [**SYN\_Signal**](structSYN__Signal.md) \* stats) <br>_Attach a signal statistics window._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int16\_t | [**apply\_filter**](#function-apply_filter) ([**SYN\_ADC**](structSYN__ADC.md) \* adc, int16\_t value) <br>_Apply the configured filter to an ADC reading._  |


























## Public Functions Documentation




### function syn\_adc\_init 

_Initialize an ADC channel with oversampling and calibration._ 
```C++
SYN_Status syn_adc_init (
    SYN_ADC * adc,
    const SYN_ADC_Config * cfg
) 
```





**Parameters:**


* `adc` ADC instance. 
* `cfg` Configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_adc\_read 

_Read the ADC (oversample + filter + calibrate)._ 
```C++
int32_t syn_adc_read (
    SYN_ADC * adc
) 
```





**Parameters:**


* `adc` ADC instance. 



**Returns:**

Calibrated value. 





        

<hr>



### function syn\_adc\_read\_mv 

_Read and convert to millivolts._ 
```C++
int32_t syn_adc_read_mv (
    SYN_ADC * adc
) 
```



Uses the port's reference voltage and resolution for conversion.




**Parameters:**


* `adc` ADC instance. 



**Returns:**

Value in millivolts. 





        

<hr>



### function syn\_adc\_set\_calibration 

_Update calibration at runtime._ 
```C++
void syn_adc_set_calibration (
    SYN_ADC * adc,
    int16_t offset,
    uint16_t scale,
    uint8_t shift
) 
```





**Parameters:**


* `adc` ADC instance. 
* `offset` New zero-point offset. 
* `scale` New scale numerator. 
* `shift` New scale shift (denominator = 1 &lt;&lt; shift). 




        

<hr>



### function syn\_adc\_set\_stats 

_Attach a signal statistics window._ 
```C++
void syn_adc_set_stats (
    SYN_ADC * adc,
    SYN_Signal * stats
) 
```



Each [**syn\_adc\_read()**](syn__adc_8h.md#function-syn_adc_read) pushes the calibrated value into the signal window, giving you noise stats (min/max/mean/variance) for free.




**Parameters:**


* `adc` ADC instance. 
* `stats` Initialized [**SYN\_Signal**](structSYN__Signal.md), or NULL to detach. 




        

<hr>
## Public Static Functions Documentation




### function apply\_filter 

_Apply the configured filter to an ADC reading._ 
```C++
static int16_t apply_filter (
    SYN_ADC * adc,
    int16_t value
) 
```





**Parameters:**


* `adc` ADC instance. 
* `value` Raw ADC value. 



**Returns:**

Filtered value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_adc.c`

