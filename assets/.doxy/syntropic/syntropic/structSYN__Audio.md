

# Struct SYN\_Audio



[**ClassList**](annotated.md) **>** [**SYN\_Audio**](structSYN__Audio.md)



_Audio Playback Engine Instance State._ 

* `#include <syn_audio.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  volatile uint8\_t | [**active\_half**](#variable-active_half)  <br> |
|  [**SYN\_Audio\_Config**](structSYN__Audio__Config.md) | [**cfg**](#variable-cfg)  <br> |
|  volatile size\_t | [**fill\_pos**](#variable-fill_pos)  <br> |
|  uint32\_t | [**frames\_played**](#variable-frames_played)  <br> |
|  volatile [**bool**](syn__defs_8h.md#enum-bool) | [**running**](#variable-running)  <br> |
|  uint32\_t | [**underrun\_cnt**](#variable-underrun_cnt)  <br> |












































## Public Attributes Documentation




### variable active\_half 

```C++
volatile uint8_t SYN_Audio::active_half;
```



Active half buffer index (0 or 1) 
 


        

<hr>



### variable cfg 

```C++
SYN_Audio_Config SYN_Audio::cfg;
```



Copy of active configuration 
 


        

<hr>



### variable fill\_pos 

```C++
volatile size_t SYN_Audio::fill_pos;
```



Write position in current half buffer 
 


        

<hr>



### variable frames\_played 

```C++
uint32_t SYN_Audio::frames_played;
```



Total buffer halves submitted 
 


        

<hr>



### variable running 

```C++
volatile bool SYN_Audio::running;
```



True when audio streaming is active 
 


        

<hr>



### variable underrun\_cnt 

```C++
uint32_t SYN_Audio::underrun_cnt;
```



Total buffer underrun count 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_audio.h`

