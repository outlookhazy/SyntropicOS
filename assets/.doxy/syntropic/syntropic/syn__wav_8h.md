

# File syn\_wav.h



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_wav.h**](syn__wav_8h.md)

[Go to the source code of this file](syn__wav_8h_source.md)

_Zero-Heap RIFF/WAVE Header Parser._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_WAV\_Info**](structSYN__WAV__Info.md) <br>_Parsed WAV Header Metadata._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_wav\_parse\_header**](#function-syn_wav_parse_header) (const uint8\_t \* buffer, size\_t buffer\_size, [**SYN\_WAV\_Info**](structSYN__WAV__Info.md) \* info) <br>_Parse a RIFF/WAVE header from a memory buffer._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_WAV\_FORMAT\_IMA\_ADPCM**](syn__wav_8h.md#define-syn_wav_format_ima_adpcm)  `0x0011U`<br> |
| define  | [**SYN\_WAV\_FORMAT\_PCM**](syn__wav_8h.md#define-syn_wav_format_pcm)  `0x0001U`<br> |

## Detailed Description


Provides a pure C99 zero-malloc streaming header parser for standard `.wav` audio container files. Supports PCM (Linear) and IMA-ADPCM compressed formats. 


    
## Public Functions Documentation




### function syn\_wav\_parse\_header 

_Parse a RIFF/WAVE header from a memory buffer._ 
```C++
SYN_Status syn_wav_parse_header (
    const uint8_t * buffer,
    size_t buffer_size,
    SYN_WAV_Info * info
) 
```





**Parameters:**


* `buffer` Pointer to input byte array (at least 44 bytes). 
* `buffer_size` Size of available buffer bytes. 
* `info` Pointer to [**SYN\_WAV\_Info**](structSYN__WAV__Info.md) output struct. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM, or SYN\_ERROR on parse fail. 





        

<hr>
## Macro Definition Documentation





### define SYN\_WAV\_FORMAT\_IMA\_ADPCM 

```C++
#define SYN_WAV_FORMAT_IMA_ADPCM `0x0011U`
```



IMA ADPCM compressed audio format 


        

<hr>



### define SYN\_WAV\_FORMAT\_PCM 

```C++
#define SYN_WAV_FORMAT_PCM `0x0001U`
```



WAVE format tags Uncompressed PCM audio format 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_wav.h`

