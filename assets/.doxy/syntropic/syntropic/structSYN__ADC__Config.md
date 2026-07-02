

# Struct SYN\_ADC\_Config



[**ClassList**](annotated.md) **>** [**SYN\_ADC\_Config**](structSYN__ADC__Config.md)



_ADC channel configuration._ 

* `#include <syn_adc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**cal\_offset**](#variable-cal_offset)  <br> |
|  uint16\_t | [**cal\_scale**](#variable-cal_scale)  <br> |
|  uint8\_t | [**cal\_scale\_shift**](#variable-cal_scale_shift)  <br> |
|  uint8\_t | [**channel**](#variable-channel)  <br> |
|  void \* | [**filter**](#variable-filter)  <br> |
|  [**SYN\_ADC\_FilterType**](syn__adc_8h.md#enum-syn_adc_filtertype) | [**filter\_type**](#variable-filter_type)  <br> |
|  uint8\_t | [**oversample**](#variable-oversample)  <br> |












































## Public Attributes Documentation




### variable cal\_offset 

```C++
int16_t SYN_ADC_Config::cal_offset;
```



Zero-point offset 
 


        

<hr>



### variable cal\_scale 

```C++
uint16_t SYN_ADC_Config::cal_scale;
```



Scale numerator 
 


        

<hr>



### variable cal\_scale\_shift 

```C++
uint8_t SYN_ADC_Config::cal_scale_shift;
```



Scale denominator = 1&lt;&lt;shift 


        

<hr>



### variable channel 

```C++
uint8_t SYN_ADC_Config::channel;
```



ADC channel number 
 


        

<hr>



### variable filter 

```C++
void* SYN_ADC_Config::filter;
```



Filter instance 
 


        

<hr>



### variable filter\_type 

```C++
SYN_ADC_FilterType SYN_ADC_Config::filter_type;
```



SYN\_ADC\_FilterType 
 


        

<hr>



### variable oversample 

```C++
uint8_t SYN_ADC_Config::oversample;
```



Oversampling count (1,4,16..) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_adc.h`

