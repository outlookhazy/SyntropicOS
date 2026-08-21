

# Struct SYN\_FOCEncoderConfig



[**ClassList**](annotated.md) **>** [**SYN\_FOCEncoderConfig**](structSYN__FOCEncoderConfig.md)



_Sensored Encoder Configuration._ 

* `#include <syn_foc_encoder.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**cpr**](#variable-cpr)  <br> |
|  uint8\_t | [**pole\_pairs**](#variable-pole_pairs)  <br> |
|  uint32\_t | [**sample\_rate\_hz**](#variable-sample_rate_hz)  <br> |
|  [**SYN\_FOCEncoderType**](syn__foc__encoder_8h.md#enum-syn_focencodertype) | [**type**](#variable-type)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**zero\_offset\_rad**](#variable-zero_offset_rad)  <br> |












































## Public Attributes Documentation




### variable cpr 

```C++
uint32_t SYN_FOCEncoderConfig::cpr;
```



Counts Per Revolution (4 \* pulses for QEI) 


        

<hr>



### variable pole\_pairs 

```C++
uint8_t SYN_FOCEncoderConfig::pole_pairs;
```



Motor pole pair count (P) 


        

<hr>



### variable sample\_rate\_hz 

```C++
uint32_t SYN_FOCEncoderConfig::sample_rate_hz;
```



Position sampling frequency in Hz 


        

<hr>



### variable type 

```C++
SYN_FOCEncoderType SYN_FOCEncoderConfig::type;
```



Sensored feedback type selection 


        

<hr>



### variable zero\_offset\_rad 

```C++
q16_t SYN_FOCEncoderConfig::zero_offset_rad;
```



Electrical zero position offset (Q16 rad) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc_encoder.h`

