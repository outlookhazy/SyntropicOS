

# Struct SYN\_VAD\_Config



[**ClassList**](annotated.md) **>** [**SYN\_VAD\_Config**](structSYN__VAD__Config.md)



_VAD configuration descriptor._ 

* `#include <syn_vad.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**attack\_frames**](#variable-attack_frames)  <br> |
|  uint16\_t | [**frame\_length**](#variable-frame_length)  <br> |
|  uint8\_t | [**hangover\_frames**](#variable-hangover_frames)  <br> |
|  uint32\_t | [**initial\_noise\_floor**](#variable-initial_noise_floor)  <br> |
|  uint16\_t | [**sample\_rate\_hz**](#variable-sample_rate_hz)  <br> |
|  [**SYN\_VAD\_Sensitivity**](syn__vad_8h.md#enum-syn_vad_sensitivity) | [**sensitivity**](#variable-sensitivity)  <br> |












































## Public Attributes Documentation




### variable attack\_frames 

```C++
uint8_t SYN_VAD_Config::attack_frames;
```



Number of consecutive speech frames to trigger onset 


        

<hr>



### variable frame\_length 

```C++
uint16_t SYN_VAD_Config::frame_length;
```



Frame size in samples (e.g. 160, 240, 320) 


        

<hr>



### variable hangover\_frames 

```C++
uint8_t SYN_VAD_Config::hangover_frames;
```



Number of hangover frames to sustain speech state 


        

<hr>



### variable initial\_noise\_floor 

```C++
uint32_t SYN_VAD_Config::initial_noise_floor;
```



Initial noise floor estimate 


        

<hr>



### variable sample\_rate\_hz 

```C++
uint16_t SYN_VAD_Config::sample_rate_hz;
```



Audio sample rate in Hz (e.g. 8000, 16000) 


        

<hr>



### variable sensitivity 

```C++
SYN_VAD_Sensitivity SYN_VAD_Config::sensitivity;
```



Sensitivity preset 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_vad.h`

