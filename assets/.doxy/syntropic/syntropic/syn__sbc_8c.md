

# File syn\_sbc.c



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_sbc.c**](syn__sbc_8c.md)

[Go to the source code of this file](syn__sbc_8c_source.md)

_Bluetooth A2DP Sub-Band Codec (SBC) Decoder implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_sbc.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const int32\_t | [**proto4\_40**](#variable-proto4_40)   = `/* multi line expression */`<br> |
|  const int32\_t | [**proto8\_80**](#variable-proto8_80)   = `/* multi line expression */`<br> |
|  const int32\_t | [**syn\_cos4**](#variable-syn_cos4)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbc\_decode\_frame**](#function-syn_sbc_decode_frame) ([**SYN\_SBC\_Decoder**](structSYN__SBC__Decoder.md) \* dec, const uint8\_t \* in, size\_t in\_len, int16\_t \* pcm\_out, size\_t pcm\_cap, size\_t \* out\_samples) <br>_Decode a single SBC audio frame to 16-bit interleaved PCM samples._  |
|  void | [**syn\_sbc\_decoder\_init**](#function-syn_sbc_decoder_init) ([**SYN\_SBC\_Decoder**](structSYN__SBC__Decoder.md) \* dec) <br>_Initialize an SBC decoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_sbc\_parse\_header**](#function-syn_sbc_parse_header) (const uint8\_t \* data, size\_t len, [**SYN\_SBC\_FrameInfo**](structSYN__SBC__FrameInfo.md) \* info) <br>_Parse an SBC frame header without decoding audio payload._  |




























## Public Static Attributes Documentation




### variable proto4\_40 

```C++
const int32_t proto4_40[40];
```



SBC Proto4 Windowing Coefficients for 4-subband synthesis 


        

<hr>



### variable proto8\_80 

```C++
const int32_t proto8_80[80];
```



SBC Proto8 Windowing Coefficients for 8-subband synthesis (scaled fixed-point Q15) 


        

<hr>



### variable syn\_cos4 

```C++
const int32_t syn_cos4[8][4];
```



IDCT matrix coefficients Q15 for 4 subbands 


        

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_sbc.c`

