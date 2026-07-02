

# Struct SYN\_ADC



[**ClassList**](annotated.md) **>** [**SYN\_ADC**](structSYN__ADC.md)



_ADC channel instance — config + last readings._ 

* `#include <syn_adc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**calibrated**](#variable-calibrated)  <br> |
|  [**SYN\_ADC\_Config**](structSYN__ADC__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  int32\_t | [**filtered**](#variable-filtered)  <br> |
|  int32\_t | [**raw**](#variable-raw)  <br> |
|  [**SYN\_Signal**](structSYN__Signal.md) \* | [**stats**](#variable-stats)  <br> |












































## Public Attributes Documentation




### variable calibrated 

```C++
int32_t SYN_ADC::calibrated;
```



After calibration 
 


        

<hr>



### variable cfg 

```C++
SYN_ADC_Config SYN_ADC::cfg;
```



Configuration 
 


        

<hr>



### variable filtered 

```C++
int32_t SYN_ADC::filtered;
```



After filter 
 


        

<hr>



### variable raw 

```C++
int32_t SYN_ADC::raw;
```



Last raw reading (after oversample) 
 


        

<hr>



### variable stats 

```C++
SYN_Signal* SYN_ADC::stats;
```



If set, calibrated value pushed here 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_adc.h`

