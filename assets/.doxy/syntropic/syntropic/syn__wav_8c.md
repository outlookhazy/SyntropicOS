

# File syn\_wav.c



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_wav.c**](syn__wav_8c.md)

[Go to the source code of this file](syn__wav_8c_source.md)

_Implementation of RIFF/WAVE Header Parser._ 

* `#include "syn_wav.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_wav\_parse\_header**](#function-syn_wav_parse_header) (const uint8\_t \* buffer, size\_t buffer\_size, [**SYN\_WAV\_Info**](structSYN__WAV__Info.md) \* info) <br>_Parse a RIFF/WAVE header from a memory buffer._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**peek\_u16\_le**](#function-peek_u16_le) (const uint8\_t \* p) <br>_Read unsigned 16-bit little-endian integer from byte stream._  |
|  uint32\_t | [**peek\_u32\_le**](#function-peek_u32_le) (const uint8\_t \* p) <br>_Read unsigned 32-bit little-endian integer from byte stream._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wav\_advance\_chunk**](#function-wav_advance_chunk) (size\_t \* offset, size\_t buffer\_size, uint32\_t chunk\_size, size\_t rem\_payload) <br>_Advance stream offset past current RIFF chunk and optional word-alignment pad byte._  |
|  void | [**wav\_compute\_total\_samples**](#function-wav_compute_total_samples) ([**SYN\_WAV\_Info**](structSYN__WAV__Info.md) \* info) <br>_Compute total decoded sample count from parsed WAV format metadata._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**wav\_parse\_fmt\_chunk**](#function-wav_parse_fmt_chunk) (const uint8\_t \* chunk, uint32\_t chunk\_size, size\_t rem\_payload, [**SYN\_WAV\_Info**](structSYN__WAV__Info.md) \* info) <br>_Parse 16-byte minimum WAVE 'fmt ' chunk fields into info._  |


























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
## Public Static Functions Documentation




### function peek\_u16\_le 

_Read unsigned 16-bit little-endian integer from byte stream._ 
```C++
static inline uint16_t peek_u16_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Pointer to byte stream. 



**Returns:**

Unsigned 16-bit integer value. 





        

<hr>



### function peek\_u32\_le 

_Read unsigned 32-bit little-endian integer from byte stream._ 
```C++
static inline uint32_t peek_u32_le (
    const uint8_t * p
) 
```





**Parameters:**


* `p` Pointer to byte stream. 



**Returns:**

Unsigned 32-bit integer value. 





        

<hr>



### function wav\_advance\_chunk 

_Advance stream offset past current RIFF chunk and optional word-alignment pad byte._ 
```C++
static bool wav_advance_chunk (
    size_t * offset,
    size_t buffer_size,
    uint32_t chunk_size,
    size_t rem_payload
) 
```





**Parameters:**


* `offset` In/out byte offset into buffer. 
* `buffer_size` Total buffer length in bytes. 
* `chunk_size` Declared 32-bit chunk payload size. 
* `rem_payload` Remaining bytes in buffer after 8-byte chunk header. 



**Returns:**

true if chunk and any pad byte fit in buffer, false to stop scanning. 





        

<hr>



### function wav\_compute\_total\_samples 

_Compute total decoded sample count from parsed WAV format metadata._ 
```C++
static void wav_compute_total_samples (
    SYN_WAV_Info * info
) 
```





**Parameters:**


* `info` Populated WAV header info struct. 




        

<hr>



### function wav\_parse\_fmt\_chunk 

_Parse 16-byte minimum WAVE 'fmt ' chunk fields into info._ 
```C++
static bool wav_parse_fmt_chunk (
    const uint8_t * chunk,
    uint32_t chunk_size,
    size_t rem_payload,
    SYN_WAV_Info * info
) 
```





**Parameters:**


* `chunk` Pointer to start of 'fmt ' chunk header. 
* `chunk_size` Declared 32-bit chunk payload size. 
* `rem_payload` Remaining bytes in buffer after 8-byte chunk header. 
* `info` Output WAV info struct. 



**Returns:**

true if 'fmt ' chunk is valid, false if undersized or truncated. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_wav.c`

