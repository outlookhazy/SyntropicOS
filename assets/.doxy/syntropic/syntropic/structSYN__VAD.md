

# Struct SYN\_VAD



[**ClassList**](annotated.md) **>** [**SYN\_VAD**](structSYN__VAD.md)



_Voice Activity Detector instance context._ 

* `#include <syn_vad.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**attack\_counter**](#variable-attack_counter)  <br> |
|  [**SYN\_VAD\_Config**](structSYN__VAD__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  uint32\_t | [**energy\_threshold\_multiplier**](#variable-energy_threshold_multiplier)  <br> |
|  uint8\_t | [**hangover\_counter**](#variable-hangover_counter)  <br> |
|  uint16\_t | [**max\_zcr\_threshold**](#variable-max_zcr_threshold)  <br> |
|  uint16\_t | [**min\_zcr\_threshold**](#variable-min_zcr_threshold)  <br> |
|  uint32\_t | [**noise\_floor**](#variable-noise_floor)  <br> |
|  [**SYN\_VAD\_State**](syn__vad_8h.md#enum-syn_vad_state) | [**state**](#variable-state)  <br> |












































## Public Attributes Documentation




### variable attack\_counter 

```C++
uint8_t SYN_VAD::attack_counter;
```



Speech onset attack accumulator 


        

<hr>



### variable cfg 

```C++
SYN_VAD_Config SYN_VAD::cfg;
```



Configuration 


        

<hr>



### variable energy\_threshold\_multiplier 

```C++
uint32_t SYN_VAD::energy_threshold_multiplier;
```



Noise-to-speech energy scale factor 


        

<hr>



### variable hangover\_counter 

```C++
uint8_t SYN_VAD::hangover_counter;
```



Hangover release countdown 


        

<hr>



### variable max\_zcr\_threshold 

```C++
uint16_t SYN_VAD::max_zcr_threshold;
```



Maximum ZCR threshold 


        

<hr>



### variable min\_zcr\_threshold 

```C++
uint16_t SYN_VAD::min_zcr_threshold;
```



Minimum ZCR threshold 


        

<hr>



### variable noise\_floor 

```C++
uint32_t SYN_VAD::noise_floor;
```



Adaptive background noise floor 


        

<hr>



### variable state 

```C++
SYN_VAD_State SYN_VAD::state;
```



Current smoothed VAD state 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_vad.h`

