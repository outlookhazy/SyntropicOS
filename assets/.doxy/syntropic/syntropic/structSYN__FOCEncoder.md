

# Struct SYN\_FOCEncoder



[**ClassList**](annotated.md) **>** [**SYN\_FOCEncoder**](structSYN__FOCEncoder.md)



_Sensored Encoder State._ 

* `#include <syn_foc_encoder.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FOCEncoderConfig**](structSYN__FOCEncoderConfig.md) | [**config**](#variable-config)  <br> |
|  int32\_t | [**count**](#variable-count)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**elec\_angle\_rad**](#variable-elec_angle_rad)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**elec\_speed\_rad\_s**](#variable-elec_speed_rad_s)  <br> |
|  uint8\_t | [**hall\_state**](#variable-hall_state)  <br> |
|  int32\_t | [**prev\_count**](#variable-prev_count)  <br> |












































## Public Attributes Documentation




### variable config 

```C++
SYN_FOCEncoderConfig SYN_FOCEncoder::config;
```



Encoder configuration parameters 


        

<hr>



### variable count 

```C++
int32_t SYN_FOCEncoder::count;
```



Current raw pulse count 


        

<hr>



### variable elec\_angle\_rad 

```C++
q16_t SYN_FOCEncoder::elec_angle_rad;
```



Calculated electrical angle (0 to 2\*PI in Q16) 


        

<hr>



### variable elec\_speed\_rad\_s 

```C++
q16_t SYN_FOCEncoder::elec_speed_rad_s;
```



Calculated electrical speed in Q16 rad/s 


        

<hr>



### variable hall\_state 

```C++
uint8_t SYN_FOCEncoder::hall_state;
```



Hall sensor 3-bit state (1 to 6) 


        

<hr>



### variable prev\_count 

```C++
int32_t SYN_FOCEncoder::prev_count;
```



Previous raw pulse count 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc_encoder.h`

