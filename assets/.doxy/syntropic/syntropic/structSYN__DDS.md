

# Struct SYN\_DDS



[**ClassList**](annotated.md) **>** [**SYN\_DDS**](structSYN__DDS.md)



_DDS Generator State Instance._ 

* `#include <syn_dds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**amplitude**](#variable-amplitude)  <br> |
|  uint16\_t | [**duty\_q16**](#variable-duty_q16)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**offset**](#variable-offset)  <br> |
|  uint32\_t | [**phase**](#variable-phase)  <br> |
|  uint32\_t | [**phase\_step**](#variable-phase_step)  <br> |
|  uint32\_t | [**rand\_state**](#variable-rand_state)  <br> |
|  [**SYN\_DDS\_Waveform**](syn__dds_8h.md#enum-syn_dds_waveform) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable amplitude 

```C++
q16_t SYN_DDS::amplitude;
```



Q16.16 Peak Amplitude (e.g. 1.0) 
 


        

<hr>



### variable duty\_q16 

```C++
uint16_t SYN_DDS::duty_q16;
```



PWM Duty cycle (0 to 65536 for 0-100%) 


        

<hr>



### variable offset 

```C++
q16_t SYN_DDS::offset;
```



Q16.16 DC Offset 
 


        

<hr>



### variable phase 

```C++
uint32_t SYN_DDS::phase;
```



32-bit Phase accumulator (0 to 2^32-1) 
 


        

<hr>



### variable phase\_step 

```C++
uint32_t SYN_DDS::phase_step;
```



Phase increment per sample step 
 


        

<hr>



### variable rand\_state 

```C++
uint32_t SYN_DDS::rand_state;
```



PRNG state for noise generator 
 


        

<hr>



### variable type 

```C++
SYN_DDS_Waveform SYN_DDS::type;
```



Waveform type 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_dds.h`

