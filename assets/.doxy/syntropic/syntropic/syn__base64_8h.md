

# File syn\_base64.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_base64.h**](syn__base64_8h.md)

[Go to the source code of this file](syn__base64_8h_source.md)

_RFC 4648 Base64 and Base64URL encoding and decoding._ [More...](#detailed-description)

* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`
* `#include <string.h>`





































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
|  size\_t | [**syn\_base64\_calc\_encoded\_size**](#function-syn_base64_calc_encoded_size) (size\_t raw\_len) <br>_Calculate the buffer size needed for standard Base64 encoding._  |
|  size\_t | [**syn\_base64\_calc\_max\_decoded\_size**](#function-syn_base64_calc_max_decoded_size) (size\_t b64\_len) <br>_Calculate the maximum binary buffer size needed to decode a Base64 string._  |


























## Detailed Description


Implements pure C99 zero-allocation streaming Base64 (standard and URL-safe). Supports both padded and unpadded Base64URL formats, whitespace-tolerant decoding, and precise buffer length calculation.




**
**


```C++
const uint8_t data[] = "Hello World";
char b64[32];
size_t out_len = 0;
syn_base64_encode(data, sizeof(data) - 1, b64, sizeof(b64), &out_len);
// b64 is "SGVsbG8gV29ybGQ="
```





**
**


```C++
uint8_t bin[32];
size_t bin_len = 0;
if (syn_base64_decode(b64, out_len, bin, sizeof(bin), &bin_len)) {
    // Decoded successfully
}
```
 





    
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




### function syn\_base64\_calc\_encoded\_size 

_Calculate the buffer size needed for standard Base64 encoding._ 
```C++
static inline size_t syn_base64_calc_encoded_size (
    size_t raw_len
) 
```



Includes padding (`=`) and space for the null terminator.




**Parameters:**


* `raw_len` Number of raw binary bytes. 



**Returns:**

Total character buffer capacity needed (including null terminator). 





        

<hr>



### function syn\_base64\_calc\_max\_decoded\_size 

_Calculate the maximum binary buffer size needed to decode a Base64 string._ 
```C++
static inline size_t syn_base64_calc_max_decoded_size (
    size_t b64_len
) 
```





**Parameters:**


* `b64_len` Number of characters in Base64 string. 



**Returns:**

Maximum number of binary bytes after decoding. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_base64.h`

