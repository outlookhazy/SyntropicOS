

# Struct SYN\_Power



[**ClassList**](annotated.md) **>** [**SYN\_Power**](structSYN__Power.md)



_Power monitor instance — ADC + hysteresis + brownout state._ 

* `#include <syn_power.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ADC**](structSYN__ADC.md) \* | [**adc**](#variable-adc)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**brownout**](#variable-brownout)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**errlog**](#variable-errlog)  <br> |
|  [**SYN\_Hysteresis**](structSYN__Hysteresis.md) | [**hyst**](#variable-hyst)  <br> |
|  [**SYN\_PowerCallback**](syn__power_8h.md#typedef-syn_powercallback) | [**on\_brownout**](#variable-on_brownout)  <br> |
|  [**SYN\_PowerCallback**](syn__power_8h.md#typedef-syn_powercallback) | [**on\_restore**](#variable-on_restore)  <br> |
|  [**SYN\_Signal**](structSYN__Signal.md) \* | [**stats**](#variable-stats)  <br> |
|  int32\_t | [**voltage\_mv**](#variable-voltage_mv)  <br> |












































## Public Attributes Documentation




### variable adc 

```C++
SYN_ADC* SYN_Power::adc;
```



ADC channel 
 


        

<hr>



### variable brownout 

```C++
bool SYN_Power::brownout;
```



Currently in brownout? 
 


        

<hr>



### variable ctx 

```C++
void* SYN_Power::ctx;
```



Callback context 
 


        

<hr>



### variable errlog 

```C++
SYN_ErrLog* SYN_Power::errlog;
```



Optional error logging 
 


        

<hr>



### variable hyst 

```C++
SYN_Hysteresis SYN_Power::hyst;
```



Brownout hysteresis state 
 


        

<hr>



### variable on\_brownout 

```C++
SYN_PowerCallback SYN_Power::on_brownout;
```



Brownout callback 
 


        

<hr>



### variable on\_restore 

```C++
SYN_PowerCallback SYN_Power::on_restore;
```



Restore callback 
 


        

<hr>



### variable stats 

```C++
SYN_Signal* SYN_Power::stats;
```



Optional noise stats 
 


        

<hr>



### variable voltage\_mv 

```C++
int32_t SYN_Power::voltage_mv;
```



Last reading in millivolts 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_power.h`

