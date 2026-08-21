

# Struct SYN\_Audio\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Audio\_Config**](structSYN__Audio__Config.md)



_Audio Playback Configuration._ 

* `#include <syn_audio.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**buf\_capacity**](#variable-buf_capacity)  <br> |
|  uint8\_t | [**channels**](#variable-channels)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  size\_t | [**half\_size**](#variable-half_size)  <br> |
|  [**SYN\_Audio\_OutputFn**](syn__audio_8h.md#typedef-syn_audio_outputfn) | [**out\_fn**](#variable-out_fn)  <br> |
|  uint32\_t | [**sample\_rate\_hz**](#variable-sample_rate_hz)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
int16_t* SYN_Audio_Config::buf;
```



Caller-allocated PCM buffer (capacity &gt;= 2 \* half\_size) 


        

<hr>



### variable buf\_capacity 

```C++
size_t SYN_Audio_Config::buf_capacity;
```



Total PCM sample capacity of buffer 
 


        

<hr>



### variable channels 

```C++
uint8_t SYN_Audio_Config::channels;
```



Channel count (1 for mono, 2 for stereo) 
 


        

<hr>



### variable ctx 

```C++
void* SYN_Audio_Config::ctx;
```



User context passed to output callback 
 


        

<hr>



### variable half\_size 

```C++
size_t SYN_Audio_Config::half_size;
```



Sample count per ping-pong buffer half 
 


        

<hr>



### variable out\_fn 

```C++
SYN_Audio_OutputFn SYN_Audio_Config::out_fn;
```



Hardware transfer trigger callback 
 


        

<hr>



### variable sample\_rate\_hz 

```C++
uint32_t SYN_Audio_Config::sample_rate_hz;
```



Audio sample rate in Hz (e.g. 16000, 44100, 48000) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_audio.h`

