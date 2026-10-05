

# File syn\_base64.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_base64.c**](syn__base64_8c.md)

[Go to the source code of this file](syn__base64_8c_source.md)

_RFC 4648 Base64 and Base64URL codec implementation._ 

* `#include "syn_base64.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const char | [**B64\_STD\_TABLE**](#variable-b64_std_table)   = `/* multi line expression */`<br>_Standard RFC 4648 Base64 character encoding table._  |
|  const char | [**B64\_URL\_TABLE**](#variable-b64_url_table)   = `/* multi line expression */`<br>_URL and filename-safe RFC 4648 Base64URL character encoding table._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_base64\_decode**](#function-syn_base64_decode) (const char \* src, size\_t src\_len, uint8\_t \* dst, size\_t dst\_size, size\_t \* out\_len) <br>_Decode an RFC 4648 standard Base64 string into binary data._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_base64\_encode**](#function-syn_base64_encode) (const uint8\_t \* src, size\_t src\_len, char \* dst, size\_t dst\_size, size\_t \* out\_len) <br>_Encode binary data to standard RFC 4648 Base64 string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_base64url\_decode**](#function-syn_base64url_decode) (const char \* src, size\_t src\_len, uint8\_t \* dst, size\_t dst\_size, size\_t \* out\_len) <br>_Decode an RFC 4648 Base64URL string into binary data._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_base64url\_encode**](#function-syn_base64url_encode) (const uint8\_t \* src, size\_t src\_len, char \* dst, size\_t dst\_size, [**bool**](syn__defs_8h.md#enum-bool) with\_padding, size\_t \* out\_len) <br>_Encode binary data to RFC 4648 Base64URL string._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**base64\_decode\_internal**](#function-base64_decode_internal) (const char \* src, size\_t src\_len, uint8\_t \* dst, size\_t dst\_size, size\_t \* out\_len, [**bool**](syn__defs_8h.md#enum-bool) url\_mode) <br>_Internal shared decoder for standard and URL-safe Base64._  |
|  uint8\_t | [**decode\_char**](#function-decode_char) (char c, [**bool**](syn__defs_8h.md#enum-bool) url\_mode) <br>_Decode single ASCII character into 6-bit value or control marker._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**B64\_VAL\_INVALID**](syn__base64_8c.md#define-b64_val_invalid)  `0xFFU`<br>_Marker value for non-base64 character._  |
| define  | [**B64\_VAL\_PAD**](syn__base64_8c.md#define-b64_val_pad)  `0xFEU`<br>_Marker value for base64 padding '=' character._  |
| define  | [**B64\_VAL\_WS**](syn__base64_8c.md#define-b64_val_ws)  `0xFDU`<br>_Marker value for whitespace character._  |

## Public Static Attributes Documentation




### variable B64\_STD\_TABLE 

_Standard RFC 4648 Base64 character encoding table._ 
```C++
const char B64_STD_TABLE[];
```




<hr>



### variable B64\_URL\_TABLE 

_URL and filename-safe RFC 4648 Base64URL character encoding table._ 
```C++
const char B64_URL_TABLE[];
```




<hr>
## Public Functions Documentation




### function syn\_base64\_decode 

_Decode an RFC 4648 standard Base64 string into binary data._ 
```C++
bool syn_base64_decode (
    const char * src,
    size_t src_len,
    uint8_t * dst,
    size_t dst_size,
    size_t * out_len
) 
```



Automatically skips whitespace characters (spaces, tabs, CR, LF). Rejects invalid non-base64 characters and malformed padding.




**Parameters:**


* `src` Input Base64 string (null-terminated or bounded by `src_len`). 
* `src_len` Length of input string (or `0` to determine via `strlen`). 
* `dst` [out] Output binary buffer. 
* `dst_size` Capacity of output buffer in bytes. 
* `out_len` [out] Optional pointer to receive actual binary bytes written. 



**Returns:**

true on success, false if input contains invalid characters or destination is too small. 





        

<hr>



### function syn\_base64\_encode 

_Encode binary data to standard RFC 4648 Base64 string._ 
```C++
bool syn_base64_encode (
    const uint8_t * src,
    size_t src_len,
    char * dst,
    size_t dst_size,
    size_t * out_len
) 
```



Uses alphabet `[A-Za-z0-9+/]` and adds `=` padding when needed. Always null-terminates the output if `dst_size > 0`.




**Parameters:**


* `src` Input binary data. 
* `src_len` Length of input binary data in bytes. 
* `dst` [out] Output string buffer. 
* `dst_size` Capacity of output buffer in bytes (must be &gt;= `syn_base64_calc_encoded_size(src_len)`). 
* `out_len` [out] Optional pointer to receive character length written (excluding null). 



**Returns:**

true on success, false if parameters are invalid or buffer is too small. 





        

<hr>



### function syn\_base64url\_decode 

_Decode an RFC 4648 Base64URL string into binary data._ 
```C++
bool syn_base64url_decode (
    const char * src,
    size_t src_len,
    uint8_t * dst,
    size_t dst_size,
    size_t * out_len
) 
```



Supports both padded (`=`) and unpadded URL-safe strings.




**Parameters:**


* `src` Input Base64URL string. 
* `src_len` Length of input string (or `0` to determine via `strlen`). 
* `dst` [out] Output binary buffer. 
* `dst_size` Capacity of output buffer in bytes. 
* `out_len` [out] Optional pointer to receive actual binary bytes written. 



**Returns:**

true on success, false on invalid input or insufficient buffer capacity. 





        

<hr>



### function syn\_base64url\_encode 

_Encode binary data to RFC 4648 Base64URL string._ 
```C++
bool syn_base64url_encode (
    const uint8_t * src,
    size_t src_len,
    char * dst,
    size_t dst_size,
    bool with_padding,
    size_t * out_len
) 
```



Uses URL-safe alphabet `[A-Za-z0-9-_]`.




**Parameters:**


* `src` Input binary data. 
* `src_len` Length of input binary data in bytes. 
* `dst` [out] Output string buffer. 
* `dst_size` Capacity of output buffer in bytes. 
* `with_padding` If true, appends `=` padding; if false, omits padding (standard JWT format). 
* `out_len` [out] Optional pointer to receive character length written (excluding null). 



**Returns:**

true on success, false if buffer is too small or parameters invalid. 





        

<hr>
## Public Static Functions Documentation




### function base64\_decode\_internal 

_Internal shared decoder for standard and URL-safe Base64._ 
```C++
static bool base64_decode_internal (
    const char * src,
    size_t src_len,
    uint8_t * dst,
    size_t dst_size,
    size_t * out_len,
    bool url_mode
) 
```





**Parameters:**


* `src` Input Base64 string buffer. 
* `src_len` Length of input string (or 0 for strlen). 
* `dst` [out] Output destination binary buffer. 
* `dst_size` Capacity of destination buffer in bytes. 
* `out_len` [out] Pointer to receive decoded byte count. 
* `url_mode` True for URL-safe alphabet, false for standard RFC 4648. 



**Returns:**

True on success, false on decode error or buffer overflow. 





        

<hr>



### function decode\_char 

_Decode single ASCII character into 6-bit value or control marker._ 
```C++
static uint8_t decode_char (
    char c,
    bool url_mode
) 
```





**Parameters:**


* `c` ASCII character to decode. 
* `url_mode` True if decoding in URL-safe mode (- and \_). 



**Returns:**

6-bit value (0-63) or marker constant (B64\_VAL\_PAD, B64\_VAL\_WS, B64\_VAL\_INVALID). 





        

<hr>
## Macro Definition Documentation





### define B64\_VAL\_INVALID 

_Marker value for non-base64 character._ 
```C++
#define B64_VAL_INVALID `0xFFU`
```




<hr>



### define B64\_VAL\_PAD 

_Marker value for base64 padding '=' character._ 
```C++
#define B64_VAL_PAD `0xFEU`
```




<hr>



### define B64\_VAL\_WS 

_Marker value for whitespace character._ 
```C++
#define B64_VAL_WS `0xFDU`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_base64.c`

