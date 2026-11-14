

# File syn\_audio.h



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_audio.h**](syn__audio_8h.md)

[Go to the source code of this file](syn__audio_8h_source.md)

_Codec-Agnostic PCM Double-Buffered Audio Playback Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Audio**](structSYN__Audio.md) <br>_Audio Playback Engine Instance State._  |
| struct | [**SYN\_Audio\_Config**](structSYN__Audio__Config.md) <br>_Audio Playback Configuration._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_Audio\_OutputFn**](#typedef-syn_audio_outputfn)  <br>_Hardware output callback function signature._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_audio\_feed**](#function-syn_audio_feed) ([**SYN\_Audio**](structSYN__Audio.md) \* audio, const int16\_t \* samples, size\_t count) <br>_Feed decoded PCM audio samples into the stream buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_init**](#function-syn_audio_init) ([**SYN\_Audio**](structSYN__Audio.md) \* audio, const [**SYN\_Audio\_Config**](structSYN__Audio__Config.md) \* cfg) <br>_Initialize an Audio Streaming Engine instance._  |
|  void | [**syn\_audio\_isr\_complete**](#function-syn_audio_isr_complete) ([**SYN\_Audio**](structSYN__Audio.md) \* audio) <br>_Hardware ISR Handler for DMA Transfer-Complete Interrupt._  |
|  void | [**syn\_audio\_isr\_half**](#function-syn_audio_isr_half) ([**SYN\_Audio**](structSYN__Audio.md) \* audio) <br>_Hardware ISR Handler for DMA Half-Transfer Interrupt._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_start**](#function-syn_audio_start) ([**SYN\_Audio**](structSYN__Audio.md) \* audio) <br>_Start audio playback streaming._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_stop**](#function-syn_audio_stop) ([**SYN\_Audio**](structSYN__Audio.md) \* audio) <br>_Stop audio playback streaming._  |




























## Detailed Description


Provides a zero-malloc, hardware-decoupled PCM streaming engine designed to feed audio samples to microcontroller DMA, I2S, SAI, or DAC peripherals.


Uses double-buffering (ping-pong halves) to allow uninterrupted audio decoding while the hardware transfers the alternate half-buffer to the DAC via DMA. 


    
## Public Types Documentation




### typedef SYN\_Audio\_OutputFn 

_Hardware output callback function signature._ 
```C++
typedef void(* SYN_Audio_OutputFn) (const int16_t *samples, size_t count, void *ctx);
```



Called when a buffer half is ready to be submitted to DMA or DAC hardware.




**Parameters:**


* `samples` Pointer to PCM sample array (16-bit linear PCM). 
* `count` Number of samples in the array. 
* `ctx` User context pointer. 




        

<hr>
## Public Functions Documentation




### function syn\_audio\_feed 

_Feed decoded PCM audio samples into the stream buffer._ 
```C++
size_t syn_audio_feed (
    SYN_Audio * audio,
    const int16_t * samples,
    size_t count
) 
```





**Parameters:**


* `audio` Pointer to Audio instance. 
* `samples` Input 16-bit PCM sample array. 
* `count` Number of samples to push. 



**Returns:**

Number of samples actually written to the buffer. 





        

<hr>



### function syn\_audio\_init 

_Initialize an Audio Streaming Engine instance._ 
```C++
SYN_Status syn_audio_init (
    SYN_Audio * audio,
    const SYN_Audio_Config * cfg
) 
```





**Parameters:**


* `audio` Pointer to Audio instance. 
* `cfg` Configuration structure pointer. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_audio\_isr\_complete 

_Hardware ISR Handler for DMA Transfer-Complete Interrupt._ 
```C++
void syn_audio_isr_complete (
    SYN_Audio * audio
) 
```



Call this function from your hardware DMA interrupt when the entire circular buffer transfer completes (`SYN_DMA_EVENT_COMPLETE`).




**Parameters:**


* `audio` Pointer to Audio instance. 




        

<hr>



### function syn\_audio\_isr\_half 

_Hardware ISR Handler for DMA Half-Transfer Interrupt._ 
```C++
void syn_audio_isr_half (
    SYN_Audio * audio
) 
```



Call this function from your hardware DMA interrupt when half of the circular buffer transfer completes (`SYN_DMA_EVENT_HALF_COMPLETE`).




**Parameters:**


* `audio` Pointer to Audio instance. 




        

<hr>



### function syn\_audio\_start 

_Start audio playback streaming._ 
```C++
SYN_Status syn_audio_start (
    SYN_Audio * audio
) 
```





**Parameters:**


* `audio` Pointer to Audio instance. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_audio\_stop 

_Stop audio playback streaming._ 
```C++
SYN_Status syn_audio_stop (
    SYN_Audio * audio
) 
```





**Parameters:**


* `audio` Pointer to Audio instance. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_audio.h`

