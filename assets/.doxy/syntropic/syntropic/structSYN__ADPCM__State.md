

# Struct SYN\_ADPCM\_State



[**ClassList**](annotated.md) **>** [**SYN\_ADPCM\_State**](structSYN__ADPCM__State.md)



_IMA-ADPCM Encoder and Decoder State._ 

* `#include <syn_adpcm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**predicted\_sample**](#variable-predicted_sample)  <br> |
|  int8\_t | [**step\_index**](#variable-step_index)  <br> |












































## Public Attributes Documentation




### variable predicted\_sample 

```C++
int16_t SYN_ADPCM_State::predicted_sample;
```



Current predicted 16-bit PCM sample (-32768 to 32767) 


        

<hr>



### variable step\_index 

```C++
int8_t SYN_ADPCM_State::step_index;
```



Current quantization step index (0 to 88) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_adpcm.h`

