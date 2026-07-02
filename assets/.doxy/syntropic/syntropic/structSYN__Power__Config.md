

# Struct SYN\_Power\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Power\_Config**](structSYN__Power__Config.md)



_Power monitor configuration._ 

* `#include <syn_power.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ADC**](structSYN__ADC.md) \* | [**adc**](#variable-adc)  <br> |
|  int32\_t | [**brownout\_mv**](#variable-brownout_mv)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_PowerCallback**](syn__power_8h.md#typedef-syn_powercallback) | [**on\_brownout**](#variable-on_brownout)  <br> |
|  [**SYN\_PowerCallback**](syn__power_8h.md#typedef-syn_powercallback) | [**on\_restore**](#variable-on_restore)  <br> |
|  int32\_t | [**restore\_mv**](#variable-restore_mv)  <br> |












































## Public Attributes Documentation




### variable adc 

```C++
SYN_ADC* SYN_Power_Config::adc;
```



ADC channel for voltage 
 


        

<hr>



### variable brownout\_mv 

```C++
int32_t SYN_Power_Config::brownout_mv;
```



Low-voltage threshold (mV) 
 


        

<hr>



### variable ctx 

```C++
void* SYN_Power_Config::ctx;
```



User context for callbacks 
 


        

<hr>



### variable on\_brownout 

```C++
SYN_PowerCallback SYN_Power_Config::on_brownout;
```



Called on low-voltage event 
 


        

<hr>



### variable on\_restore 

```C++
SYN_PowerCallback SYN_Power_Config::on_restore;
```



Called when voltage restores 
 


        

<hr>



### variable restore\_mv 

```C++
int32_t SYN_Power_Config::restore_mv;
```



Voltage restore threshold (mV) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_power.h`

