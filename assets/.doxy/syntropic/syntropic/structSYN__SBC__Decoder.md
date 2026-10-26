

# Struct SYN\_SBC\_Decoder



[**ClassList**](annotated.md) **>** [**SYN\_SBC\_Decoder**](structSYN__SBC__Decoder.md)



_SBC Decoder Instance State (~1.5 KB static memory)._ 

* `#include <syn_sbc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int32\_t | [**V**](#variable-v)  <br> |
|  int32\_t | [**audio\_sample**](#variable-audio_sample)  <br> |
|  [**SYN\_SBC\_FrameInfo**](structSYN__SBC__FrameInfo.md) | [**info**](#variable-info)  <br> |
|  int32\_t | [**scale\_factors**](#variable-scale_factors)  <br> |












































## Public Attributes Documentation




### variable V 

```C++
int32_t SYN_SBC_Decoder::V[2][160];
```



Synthesis filter subband delay line 
 


        

<hr>



### variable audio\_sample 

```C++
int32_t SYN_SBC_Decoder::audio_sample[16][2][8];
```



Reconstructed subband samples 
 


        

<hr>



### variable info 

```C++
SYN_SBC_FrameInfo SYN_SBC_Decoder::info;
```



Active frame parameters 
 


        

<hr>



### variable scale\_factors 

```C++
int32_t SYN_SBC_Decoder::scale_factors[2][8];
```



Decoded scale factors 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_sbc.h`

