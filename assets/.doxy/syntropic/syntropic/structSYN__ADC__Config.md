

# Struct SYN\_ADC\_Config



[**ClassList**](annotated.md) **>** [**SYN\_ADC\_Config**](structSYN__ADC__Config.md)



_ADC driver configuration structure._ 

* `#include <syn_adc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**adc\_id**](#variable-adc_id)  <br> |
|  uint32\_t | [**channel\_mask**](#variable-channel_mask)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**use\_dma**](#variable-use_dma)  <br> |
|  uint32\_t | [**vref\_mv**](#variable-vref_mv)  <br> |












































## Public Attributes Documentation




### variable adc\_id 

```C++
uint8_t SYN_ADC_Config::adc_id;
```



Hardware ADC instance index (0 = ADC1) 


        

<hr>



### variable channel\_mask 

```C++
uint32_t SYN_ADC_Config::channel_mask;
```



Active analog channels bitmask 


        

<hr>



### variable use\_dma 

```C++
bool SYN_ADC_Config::use_dma;
```



Enable DMA background scan mode 


        

<hr>



### variable vref\_mv 

```C++
uint32_t SYN_ADC_Config::vref_mv;
```



Reference voltage in millivolts (e.g. 3300) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_adc.h`

