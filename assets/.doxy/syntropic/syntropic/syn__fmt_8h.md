

# File syn\_fmt.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_fmt.h**](syn__fmt_8h.md)

[Go to the source code of this file](syn__fmt_8h_source.md)

_Lightweight formatting — no libc printf dependency._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_fmt\_concat**](#function-syn_fmt_concat) (char \* buf, size\_t size, const char \*const \* parts, size\_t n) <br>_Build a string from parts (like snprintf but simpler)._  |
|  size\_t | [**syn\_fmt\_fixed**](#function-syn_fmt_fixed) (char \* buf, size\_t size, int32\_t val, uint8\_t places) <br>_Format a value with a fixed number of decimal places._  |
|  size\_t | [**syn\_fmt\_hex**](#function-syn_fmt_hex) (char \* buf, size\_t size, uint32\_t val, uint8\_t min\_digits) <br>_Format a value as hex._  |
|  size\_t | [**syn\_fmt\_hex\_parse**](#function-syn_fmt_hex_parse) (const char \* hex\_str, uint8\_t \* out\_bin, size\_t max\_bytes) <br>_Parse a hex string into a binary byte array._  |
|  size\_t | [**syn\_fmt\_hexdump**](#function-syn_fmt_hexdump) (char \* buf, size\_t size, const uint8\_t \* data, size\_t len) <br>_Format a hex dump of a byte array._  |
|  size\_t | [**syn\_fmt\_int**](#function-syn_fmt_int) (char \* buf, size\_t size, int32\_t val) <br>_Format a signed integer to decimal string._  |
|  size\_t | [**syn\_fmt\_q16**](#function-syn_fmt_q16) (char \* buf, size\_t size, int32\_t q16\_val, uint8\_t frac\_digits) <br>_Format a Q16.16 fixed-point value._  |
|  size\_t | [**syn\_fmt\_uint**](#function-syn_fmt_uint) (char \* buf, size\_t size, uint32\_t val) <br>_Format an unsigned integer to decimal string._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**syn\_fmt\_parse\_uint**](#function-syn_fmt_parse_uint) (const char \* s) <br>_Parse a decimal integer from a string._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_str\_prefix\_icase**](#function-syn_str_prefix_icase) (const char \* str, const char \* prefix) <br>_Case-insensitive prefix match (ASCII)._  |


























## Detailed Description


Integer-to-string, hex, fixed-point, and hex dump. All functions write to a caller-provided buffer and return the number of chars written (excluding null terminator).




**
**


```C++
char buf[16];
syn_fmt_int(buf, sizeof(buf), -1234);     // "-1234"
syn_fmt_hex(buf, sizeof(buf), 0xDEAD, 4); // "DEAD"
syn_fmt_q16(buf, sizeof(buf), val, 3);    // "3.141"
```
 





    
## Public Functions Documentation




### function syn\_fmt\_concat 

_Build a string from parts (like snprintf but simpler)._ 
```C++
size_t syn_fmt_concat (
    char * buf,
    size_t size,
    const char *const * parts,
    size_t n
) 
```



Concatenate up to `n` string fragments into buf.




**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity. 
* `parts` Array of string pointers. 
* `n` Number of strings. 



**Returns:**

Total characters written. 





        

<hr>



### function syn\_fmt\_fixed 

_Format a value with a fixed number of decimal places._ 
```C++
size_t syn_fmt_fixed (
    char * buf,
    size_t size,
    int32_t val,
    uint8_t places
) 
```



E.g., syn\_fmt\_fixed(buf, 16, 12345, 3) → "12.345"




**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity in bytes. 
* `val` Integer value. 
* `places` Number of decimal places from the right. 



**Returns:**

Number of characters written. 





        

<hr>



### function syn\_fmt\_hex 

_Format a value as hex._ 
```C++
size_t syn_fmt_hex (
    char * buf,
    size_t size,
    uint32_t val,
    uint8_t min_digits
) 
```





**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity in bytes. 
* `val` Value to format. 
* `min_digits` Minimum hex digits (zero-padded). 



**Returns:**

Number of characters written. 





        

<hr>



### function syn\_fmt\_hex\_parse 

_Parse a hex string into a binary byte array._ 
```C++
size_t syn_fmt_hex_parse (
    const char * hex_str,
    uint8_t * out_bin,
    size_t max_bytes
) 
```



Supports both uppercase and lowercase hex characters.




**Parameters:**


* `hex_str` Input hex string. 
* `out_bin` [out] Output binary byte buffer. 
* `max_bytes` Capacity of out\_bin in bytes. 



**Returns:**

Number of binary bytes parsed. 





        

<hr>



### function syn\_fmt\_hexdump 

_Format a hex dump of a byte array._ 
```C++
size_t syn_fmt_hexdump (
    char * buf,
    size_t size,
    const uint8_t * data,
    size_t len
) 
```



Output: "DE AD BE EF" (space-separated hex bytes).




**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity. 
* `data` Byte array. 
* `len` Number of bytes. 



**Returns:**

Number of characters written. 





        

<hr>



### function syn\_fmt\_int 

_Format a signed integer to decimal string._ 
```C++
size_t syn_fmt_int (
    char * buf,
    size_t size,
    int32_t val
) 
```





**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity. 
* `val` Value to format. 



**Returns:**

Number of characters written (excluding null). 





        

<hr>



### function syn\_fmt\_q16 

_Format a Q16.16 fixed-point value._ 
```C++
size_t syn_fmt_q16 (
    char * buf,
    size_t size,
    int32_t q16_val,
    uint8_t frac_digits
) 
```





**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity in bytes. 
* `q16_val` Q16.16 value to format. 
* `frac_digits` Number of fractional decimal digits (1–6). 



**Returns:**

Number of characters written. 





        

<hr>



### function syn\_fmt\_uint 

_Format an unsigned integer to decimal string._ 
```C++
size_t syn_fmt_uint (
    char * buf,
    size_t size,
    uint32_t val
) 
```





**Parameters:**


* `buf` Output buffer. 
* `size` Buffer capacity. 
* `val` Value to format. 



**Returns:**

Number of characters written (excluding null). 





        

<hr>
## Public Static Functions Documentation




### function syn\_fmt\_parse\_uint 

_Parse a decimal integer from a string._ 
```C++
static inline uint32_t syn_fmt_parse_uint (
    const char * s
) 
```





**Parameters:**


* `s` Null-terminated decimal string. 



**Returns:**

Parsed unsigned 32-bit integer. 





        

<hr>



### function syn\_str\_prefix\_icase 

_Case-insensitive prefix match (ASCII)._ 
```C++
static inline bool syn_str_prefix_icase (
    const char * str,
    const char * prefix
) 
```





**Parameters:**


* `str` String to test. 
* `prefix` Prefix to match. 



**Returns:**

true if `str` starts with `prefix` (case-insensitive). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_fmt.h`

