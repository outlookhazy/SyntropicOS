

# File syn\_adc.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_adc.h**](syn__adc_8h.md)

[Go to the source code of this file](syn__adc_8h_source.md)

_ADC abstraction — oversampling, calibration, unit conversion._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_adc.h"`
* `#include "../dsp/syn_filter.h"`
* `#include "../dsp/syn_signal.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ADC**](structSYN__ADC.md) <br>_ADC channel instance — config + last readings._  |
| struct | [**SYN\_ADC\_Config**](structSYN__ADC__Config.md) <br>_ADC channel configuration._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_ADC\_FilterType**](#enum-syn_adc_filtertype)  <br>_ADC digital filter selector._  |




















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
|  int32\_t | [**syn\_adc\_calibrated**](#function-syn_adc_calibrated) (const [**SYN\_ADC**](structSYN__ADC.md) \* adc) <br>_Get last calibrated value._  |
|  int32\_t | [**syn\_adc\_filtered**](#function-syn_adc_filtered) (const [**SYN\_ADC**](structSYN__ADC.md) \* adc) <br>_Get last filtered value._  |
|  int32\_t | [**syn\_adc\_raw**](#function-syn_adc_raw) (const [**SYN\_ADC**](structSYN__ADC.md) \* adc) <br>_Get last raw value._  |


























## Detailed Description


Wraps the raw ADC port with oversampling, digital filtering, calibration (via LUT or linear scale/offset), and conversion to engineering units (millivolts, etc.).




**
**


```C++
SYN_ADC adc;
SYN_FilterEMA adc_filter;
syn_filter_ema_init(&adc_filter, 128);

SYN_ADC_Config cfg = {
    .channel = 0,
    .oversample = 4,      // 4× oversampling (2 extra bits)
    .filter = &adc_filter,
    .filter_type = SYN_ADC_FILTER_EMA,
    .cal_offset = 0,
    .cal_scale = 1000,    // × 1.000
    .cal_scale_shift = 10, // scale denominator = 1024
};
syn_adc_init(&adc, &cfg);

int32_t mv = syn_adc_read_mv(&adc);
```
 





    
## Public Types Documentation




### enum SYN\_ADC\_FilterType 

_ADC digital filter selector._ 
```C++
enum SYN_ADC_FilterType {
    SYN_ADC_FILTER_NONE = 0,
    SYN_ADC_FILTER_MA = 1,
    SYN_ADC_FILTER_EMA = 2,
    SYN_ADC_FILTER_MEDIAN = 3
};
```




<hr>
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




### function syn\_adc\_calibrated 

_Get last calibrated value._ 
```C++
static inline int32_t syn_adc_calibrated (
    const SYN_ADC * adc
) 
```





**Parameters:**


* `adc` ADC instance. 



**Returns:**

Calibrated reading. 





        

<hr>



### function syn\_adc\_filtered 

_Get last filtered value._ 
```C++
static inline int32_t syn_adc_filtered (
    const SYN_ADC * adc
) 
```





**Parameters:**


* `adc` ADC instance. 



**Returns:**

Filtered reading. 





        

<hr>



### function syn\_adc\_raw 

_Get last raw value._ 
```C++
static inline int32_t syn_adc_raw (
    const SYN_ADC * adc
) 
```





**Parameters:**


* `adc` ADC instance. 



**Returns:**

Raw ADC reading. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_adc.h`

