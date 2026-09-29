

# Struct SYN\_Audio\_Mixer



[**ClassList**](annotated.md) **>** [**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md)



_Audio Mixer Instance State._ 

* `#include <syn_audio_mixer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_Audio\_Mixer\_Channel**](structSYN__Audio__Mixer__Channel.md) | [**channels**](#variable-channels)  <br> |
|  uint16\_t | [**master\_volume\_q15**](#variable-master_volume_q15)  <br> |












































## Public Attributes Documentation




### variable channels 

```C++
SYN_Audio_Mixer_Channel SYN_Audio_Mixer::channels[SYN_AUDIO_MIXER_MAX_CHANNELS];
```



Input channels 
 


        

<hr>



### variable master\_volume\_q15 

```C++
uint16_t SYN_Audio_Mixer::master_volume_q15;
```



Master gain (Q15) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_audio_mixer.h`

