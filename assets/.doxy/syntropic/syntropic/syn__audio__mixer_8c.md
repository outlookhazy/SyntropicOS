

# File syn\_audio\_mixer.c



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_audio\_mixer.c**](syn__audio__mixer_8c.md)

[Go to the source code of this file](syn__audio__mixer_8c_source.md)

_Implementation of Zero-Heap Multi-Channel Audio Mixer._ 

* `#include "syn_audio_mixer.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_mixer\_init**](#function-syn_audio_mixer_init) ([**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md) \* mixer) <br>_Initialize an Audio Mixer instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_mixer\_play**](#function-syn_audio_mixer_play) ([**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md) \* mixer, uint8\_t channel\_idx, const int16\_t \* pcm\_buf, size\_t sample\_count, uint16\_t volume\_q15, [**bool**](syn__defs_8h.md#enum-bool) loop) <br>_Assign a PCM sample buffer to a mixer channel for playback._  |
|  size\_t | [**syn\_audio\_mixer\_render**](#function-syn_audio_mixer_render) ([**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md) \* mixer, int16\_t \* out\_pcm, size\_t sample\_count) <br>_Render mixed PCM audio into an output sample buffer._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_mixer\_set\_channel\_volume**](#function-syn_audio_mixer_set_channel_volume) ([**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md) \* mixer, uint8\_t channel\_idx, uint16\_t volume\_q15) <br>_Adjust gain for a specific channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_mixer\_set\_master\_volume**](#function-syn_audio_mixer_set_master_volume) ([**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md) \* mixer, uint16\_t volume\_q15) <br>_Adjust master gain across all channels._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_audio\_mixer\_stop**](#function-syn_audio_mixer_stop) ([**SYN\_Audio\_Mixer**](structSYN__Audio__Mixer.md) \* mixer, uint8\_t channel\_idx) <br>_Stop channel playback immediately._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int16\_t | [**saturate\_i32\_to\_i16**](#function-saturate_i32_to_i16) (int32\_t val) <br>_Clamp signed 32-bit value into signed 16-bit range._  |


























## Public Functions Documentation




### function syn\_audio\_mixer\_init 

_Initialize an Audio Mixer instance._ 
```C++
SYN_Status syn_audio_mixer_init (
    SYN_Audio_Mixer * mixer
) 
```





**Parameters:**


* `mixer` Pointer to Audio Mixer instance. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_audio\_mixer\_play 

_Assign a PCM sample buffer to a mixer channel for playback._ 
```C++
SYN_Status syn_audio_mixer_play (
    SYN_Audio_Mixer * mixer,
    uint8_t channel_idx,
    const int16_t * pcm_buf,
    size_t sample_count,
    uint16_t volume_q15,
    bool loop
) 
```





**Parameters:**


* `mixer` Pointer to Audio Mixer instance. 
* `channel_idx` Channel index (0 .. SYN\_AUDIO\_MIXER\_MAX\_CHANNELS - 1). 
* `pcm_buf` Pointer to 16-bit PCM sample array. 
* `sample_count` Sample count in pcm\_buf. 
* `volume_q15` Channel gain (Q15, 32768 = 1.0). 
* `loop` True to restart automatically upon reaching buffer end. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_audio\_mixer\_render 

_Render mixed PCM audio into an output sample buffer._ 
```C++
size_t syn_audio_mixer_render (
    SYN_Audio_Mixer * mixer,
    int16_t * out_pcm,
    size_t sample_count
) 
```



Accumulates active channels, applies Q15 scaling, saturates to [-32768, 32767], and writes to out\_pcm.




**Parameters:**


* `mixer` Pointer to Audio Mixer instance. 
* `out_pcm` Output 16-bit PCM buffer pointer. 
* `sample_count` Output sample count to render. 



**Returns:**

Number of samples actually generated (0 if no active channels or NULL). 





        

<hr>



### function syn\_audio\_mixer\_set\_channel\_volume 

_Adjust gain for a specific channel._ 
```C++
SYN_Status syn_audio_mixer_set_channel_volume (
    SYN_Audio_Mixer * mixer,
    uint8_t channel_idx,
    uint16_t volume_q15
) 
```





**Parameters:**


* `mixer` Pointer to Audio Mixer instance. 
* `channel_idx` Channel index (0 .. SYN\_AUDIO\_MIXER\_MAX\_CHANNELS - 1). 
* `volume_q15` New gain value (Q15). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM / SYN\_ERR\_OUT\_OF\_BOUNDS. 





        

<hr>



### function syn\_audio\_mixer\_set\_master\_volume 

_Adjust master gain across all channels._ 
```C++
SYN_Status syn_audio_mixer_set_master_volume (
    SYN_Audio_Mixer * mixer,
    uint16_t volume_q15
) 
```





**Parameters:**


* `mixer` Pointer to Audio Mixer instance. 
* `volume_q15` New master gain value (Q15). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM. 





        

<hr>



### function syn\_audio\_mixer\_stop 

_Stop channel playback immediately._ 
```C++
SYN_Status syn_audio_mixer_stop (
    SYN_Audio_Mixer * mixer,
    uint8_t channel_idx
) 
```





**Parameters:**


* `mixer` Pointer to Audio Mixer instance. 
* `channel_idx` Channel index (0 .. SYN\_AUDIO\_MIXER\_MAX\_CHANNELS - 1). 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM / SYN\_ERR\_OUT\_OF\_BOUNDS. 





        

<hr>
## Public Static Functions Documentation




### function saturate\_i32\_to\_i16 

_Clamp signed 32-bit value into signed 16-bit range._ 
```C++
static inline int16_t saturate_i32_to_i16 (
    int32_t val
) 
```





**Parameters:**


* `val` Input 32-bit sample value. 



**Returns:**

Clamped 16-bit sample value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_audio_mixer.c`

