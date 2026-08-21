

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

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_wav.c`

