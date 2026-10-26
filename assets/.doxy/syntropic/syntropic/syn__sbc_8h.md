

# File syn\_sbc.h



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_sbc.h**](syn__sbc_8h.md)

[Go to the source code of this file](syn__sbc_8h_source.md)

_Bluetooth A2DP Sub-Band Codec (SBC) Decoder._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_SBC\_Decoder**](structSYN__SBC__Decoder.md) <br>_SBC Decoder Instance State (~1.5 KB static memory)._  |
| struct | [**SYN\_SBC\_FrameInfo**](structSYN__SBC__FrameInfo.md) <br>_SBC Parsed Frame Header Information._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_SBC\_AllocMethod**](#enum-syn_sbc_allocmethod)  <br>_SBC Allocation Method Enums._  |
| enum  | [**SYN\_SBC\_ChannelMode**](#enum-syn_sbc_channelmode)  <br>_SBC Channel Mode Enums._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbc\_decode\_frame**](#function-syn_sbc_decode_frame) ([**SYN\_SBC\_Decoder**](structSYN__SBC__Decoder.md) \* dec, const uint8\_t \* in, size\_t in\_len, int16\_t \* pcm\_out, size\_t pcm\_cap, size\_t \* out\_samples) <br>_Decode a single SBC audio frame to 16-bit interleaved PCM samples._  |
|  void | [**syn\_sbc\_decoder\_init**](#function-syn_sbc_decoder_init) ([**SYN\_SBC\_Decoder**](structSYN__SBC__Decoder.md) \* dec) <br>_Initialize an SBC decoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbc\_parse\_header**](#function-syn_sbc_parse_header) (const uint8\_t \* data, size\_t len, [**SYN\_SBC\_FrameInfo**](structSYN__SBC__FrameInfo.md) \* info) <br>_Parse an SBC frame header without decoding audio payload._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_SBC\_SYNCWORD**](syn__sbc_8h.md#define-syn_sbc_syncword)  `0x9CU`<br>_SBC Frame Synchronization Byte (0x9C)_  |

## Detailed Description


Zero-heap, fixed-point integer implementation of the Bluetooth SIG SBC decoder (A2DP spec v1.3 / Bluetooth Core Spec AVTP). Supports 4 and 8 subbands, 4, 8, 12, 16 blocks, Mono/Dual Channel/Stereo/Joint Stereo, Loudness and SNR bit allocation, and sample rates 16k, 32k, 44.1k, 48k. 


    
## Public Types Documentation




### enum SYN\_SBC\_AllocMethod 

_SBC Allocation Method Enums._ 
```C++
enum SYN_SBC_AllocMethod {
    SYN_SBC_ALLOC_LOUDNESS = 0,
    SYN_SBC_ALLOC_SNR = 1
};
```




<hr>



### enum SYN\_SBC\_ChannelMode 

_SBC Channel Mode Enums._ 
```C++
enum SYN_SBC_ChannelMode {
    SYN_SBC_MODE_MONO = 0,
    SYN_SBC_MODE_DUAL_CHANNEL = 1,
    SYN_SBC_MODE_STEREO = 2,
    SYN_SBC_MODE_JOINT_STEREO = 3
};
```




<hr>
## Public Functions Documentation




### function syn\_sbc\_decode\_frame 

_Decode a single SBC audio frame to 16-bit interleaved PCM samples._ 
```C++
SYN_Status syn_sbc_decode_frame (
    SYN_SBC_Decoder * dec,
    const uint8_t * in,
    size_t in_len,
    int16_t * pcm_out,
    size_t pcm_cap,
    size_t * out_samples
) 
```





**Parameters:**


* `dec` Decoder context pointer. 
* `in` Input SBC frame buffer starting at syncword 0x9C. 
* `in_len` Input frame buffer length. 
* `pcm_out` Output 16-bit PCM buffer (interleaved stereo or mono). 
* `pcm_cap` Capacity of output PCM buffer in samples. 
* `out_samples` Output pointer receiving actual decoded sample count. 



**Returns:**

SYN\_OK on successful frame decode, or error code (&lt;0). 





        

<hr>



### function syn\_sbc\_decoder\_init 

_Initialize an SBC decoder instance._ 
```C++
void syn_sbc_decoder_init (
    SYN_SBC_Decoder * dec
) 
```





**Parameters:**


* `dec` Decoder context pointer. 




        

<hr>



### function syn\_sbc\_parse\_header 

_Parse an SBC frame header without decoding audio payload._ 
```C++
SYN_Status syn_sbc_parse_header (
    const uint8_t * data,
    size_t len,
    SYN_SBC_FrameInfo * info
) 
```





**Parameters:**


* `data` Input byte buffer containing SBC stream. 
* `len` Length of input buffer in bytes. 
* `info` Output frame header structure. 



**Returns:**

SYN\_OK on success, or SYN\_INVALID\_PARAM / SYN\_ERROR if syncword missing or invalid. 





        

<hr>
## Macro Definition Documentation





### define SYN\_SBC\_SYNCWORD 

_SBC Frame Synchronization Byte (0x9C)_ 
```C++
#define SYN_SBC_SYNCWORD `0x9CU`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_sbc.h`

