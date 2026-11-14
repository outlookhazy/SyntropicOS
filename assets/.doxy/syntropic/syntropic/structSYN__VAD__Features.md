

# Struct SYN\_VAD\_Features



[**ClassList**](annotated.md) **>** [**SYN\_VAD\_Features**](structSYN__VAD__Features.md)



_Extracted audio frame feature metrics._ 

* `#include <syn_vad.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**energy**](#variable-energy)  <br> |
|  uint32\_t | [**hf\_energy**](#variable-hf_energy)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_speech\_instant**](#variable-is_speech_instant)  <br> |
|  uint32\_t | [**noise\_floor**](#variable-noise_floor)  <br> |
|  uint16\_t | [**zcr**](#variable-zcr)  <br> |












































## Public Attributes Documentation




### variable energy 

```C++
uint32_t SYN_VAD_Features::energy;
```



Normalized short-time frame energy 


        

<hr>



### variable hf\_energy 

```C++
uint32_t SYN_VAD_Features::hf_energy;
```



High-frequency spectral differential energy 


        

<hr>



### variable is\_speech\_instant 

```C++
bool SYN_VAD_Features::is_speech_instant;
```



Raw un-smoothed speech decision for current frame 


        

<hr>



### variable noise\_floor 

```C++
uint32_t SYN_VAD_Features::noise_floor;
```



Current estimated background noise floor 


        

<hr>



### variable zcr 

```C++
uint16_t SYN_VAD_Features::zcr;
```



Zero crossing count across frame 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_vad.h`

