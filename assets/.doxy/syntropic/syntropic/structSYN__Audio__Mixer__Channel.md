

# Struct SYN\_Audio\_Mixer\_Channel



[**ClassList**](annotated.md) **>** [**SYN\_Audio\_Mixer\_Channel**](structSYN__Audio__Mixer__Channel.md)



_State for a single mixer input channel._ 

* `#include <syn_audio_mixer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  const int16\_t \* | [**buf**](#variable-buf)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**loop**](#variable-loop)  <br> |
|  size\_t | [**pos**](#variable-pos)  <br> |
|  size\_t | [**sample\_count**](#variable-sample_count)  <br> |
|  uint16\_t | [**volume\_q15**](#variable-volume_q15)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_Audio_Mixer_Channel::active;
```



True if channel is actively playing 
 


        

<hr>



### variable buf 

```C++
const int16_t* SYN_Audio_Mixer_Channel::buf;
```



Pointer to PCM sample array 
 


        

<hr>



### variable loop 

```C++
bool SYN_Audio_Mixer_Channel::loop;
```



True to loop playback continuously 
 


        

<hr>



### variable pos 

```C++
size_t SYN_Audio_Mixer_Channel::pos;
```



Current read index 
 


        

<hr>



### variable sample\_count 

```C++
size_t SYN_Audio_Mixer_Channel::sample_count;
```



Total sample count in buffer 
 


        

<hr>



### variable volume\_q15 

```C++
uint16_t SYN_Audio_Mixer_Channel::volume_q15;
```



Channel gain (Q15 fixed-point, 32768 = 1.0) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_audio_mixer.h`

