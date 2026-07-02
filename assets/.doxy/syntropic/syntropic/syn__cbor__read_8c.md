

# File syn\_cbor\_read.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_cbor\_read.c**](syn__cbor__read_8c.md)

[Go to the source code of this file](syn__cbor__read_8c_source.md)

_CBOR decoder implementation._ 

* `#include "syn_cbor_read.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_CborType**](syn__cbor__read_8h.md#enum-syn_cbortype) | [**syn\_cbor\_peek\_type**](#function-syn_cbor_peek_type) (const [**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Return the type of the next item without consuming it._  |
|  size\_t | [**syn\_cbor\_read\_array\_begin**](#function-syn_cbor_read_array_begin) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Consume an array header; return the number of items._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cbor\_read\_bool**](#function-syn_cbor_read_bool) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Read a boolean (major type 7, info 20 or 21)._  |
|  size\_t | [**syn\_cbor\_read\_bytes**](#function-syn_cbor_read_bytes) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r, uint8\_t \* buf, size\_t cap) <br>_Read a byte string into a caller-provided buffer._  |
|  float | [**syn\_cbor\_read\_float**](#function-syn_cbor_read_float) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Read a float32 value (major type 7, info=26)._  |
|  int64\_t | [**syn\_cbor\_read\_int**](#function-syn_cbor_read_int) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Read a signed integer (major type 0 or 1)._  |
|  size\_t | [**syn\_cbor\_read\_map\_begin**](#function-syn_cbor_read_map_begin) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Consume a map header; return the number of key-value pairs._  |
|  void | [**syn\_cbor\_read\_null**](#function-syn_cbor_read_null) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Consume a null item (major type 7, info=22)._  |
|  size\_t | [**syn\_cbor\_read\_text**](#function-syn_cbor_read_text) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r, char \* buf, size\_t cap) <br>_Read a text string into a caller-provided buffer._  |
|  uint64\_t | [**syn\_cbor\_read\_uint**](#function-syn_cbor_read_uint) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Read an unsigned integer (major type 0)._  |
|  void | [**syn\_cbor\_reader\_init**](#function-syn_cbor_reader_init) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r, const uint8\_t \* buf, size\_t len) <br>_Initialize a CBOR reader._  |
|  void | [**syn\_cbor\_skip**](#function-syn_cbor_skip) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Skip the next complete item (including nested contents)._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**consume\_byte**](#function-consume_byte) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Consume and return one byte from the CBOR stream._  |
|  uint64\_t | [**decode\_arg**](#function-decode_arg) ([**SYN\_CborReader**](structSYN__CborReader.md) \* r, uint8\_t info) <br>_Decode a CBOR argument from the bottom-5-bit info field._  |
|  uint8\_t | [**peek\_byte**](#function-peek_byte) (const [**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Peek at the next byte without consuming it._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SKIP\_MAX\_DEPTH**](syn__cbor__read_8c.md#define-skip_max_depth)  `8u`<br>_Maximum nesting depth for syn\_cbor\_skip._  |

## Public Functions Documentation




### function syn\_cbor\_peek\_type 

_Return the type of the next item without consuming it._ 
```C++
SYN_CborType syn_cbor_peek_type (
    const SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Item type, or SYN\_CBOR\_ERROR on buffer underrun. 





        

<hr>



### function syn\_cbor\_read\_array\_begin 

_Consume an array header; return the number of items._ 
```C++
size_t syn_cbor_read_array_begin (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Item count, or 0 on error. 





        

<hr>



### function syn\_cbor\_read\_bool 

_Read a boolean (major type 7, info 20 or 21)._ 
```C++
bool syn_cbor_read_bool (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Decoded value, or false on error. 





        

<hr>



### function syn\_cbor\_read\_bytes 

_Read a byte string into a caller-provided buffer._ 
```C++
size_t syn_cbor_read_bytes (
    SYN_CborReader * r,
    uint8_t * buf,
    size_t cap
) 
```



Copies at most `cap` bytes. The CBOR item is fully consumed.




**Parameters:**


* `r` Reader. 
* `buf` Output buffer. 
* `cap` Output buffer capacity. 



**Returns:**

Actual byte count in the CBOR item (may exceed `cap`). 





        

<hr>



### function syn\_cbor\_read\_float 

_Read a float32 value (major type 7, info=26)._ 
```C++
float syn_cbor_read_float (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Decoded float, or 0.0f on error. 





        

<hr>



### function syn\_cbor\_read\_int 

_Read a signed integer (major type 0 or 1)._ 
```C++
int64_t syn_cbor_read_int (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Decoded value, or 0 on error. 





        

<hr>



### function syn\_cbor\_read\_map\_begin 

_Consume a map header; return the number of key-value pairs._ 
```C++
size_t syn_cbor_read_map_begin (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Pair count, or 0 on error. 





        

<hr>



### function syn\_cbor\_read\_null 

_Consume a null item (major type 7, info=22)._ 
```C++
void syn_cbor_read_null (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 




        

<hr>



### function syn\_cbor\_read\_text 

_Read a text string into a caller-provided buffer._ 
```C++
size_t syn_cbor_read_text (
    SYN_CborReader * r,
    char * buf,
    size_t cap
) 
```



Copies at most `cap-1` bytes and null-terminates. The CBOR item is fully consumed regardless of buffer capacity.




**Parameters:**


* `r` Reader. 
* `buf` Output buffer. 
* `cap` Output buffer capacity (including space for null terminator). 



**Returns:**

Actual UTF-8 byte count in the CBOR item (may exceed `cap-1`). 





        

<hr>



### function syn\_cbor\_read\_uint 

_Read an unsigned integer (major type 0)._ 
```C++
uint64_t syn_cbor_read_uint (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

Decoded value, or 0 on error. 





        

<hr>



### function syn\_cbor\_reader\_init 

_Initialize a CBOR reader._ 
```C++
void syn_cbor_reader_init (
    SYN_CborReader * r,
    const uint8_t * buf,
    size_t len
) 
```





**Parameters:**


* `r` Reader to initialize. 
* `buf` Input buffer containing encoded CBOR. 
* `len` Buffer length in bytes. 




        

<hr>



### function syn\_cbor\_skip 

_Skip the next complete item (including nested contents)._ 
```C++
void syn_cbor_skip (
    SYN_CborReader * r
) 
```



Handles nested arrays and maps up to 8 levels deep.




**Parameters:**


* `r` Reader. 




        

<hr>
## Public Static Functions Documentation




### function consume\_byte 

_Consume and return one byte from the CBOR stream._ 
```C++
static uint8_t consume_byte (
    SYN_CborReader * r
) 
```





**Parameters:**


* `r` CBOR reader. 



**Returns:**

Consumed byte, or 0 on underrun. 





        

<hr>



### function decode\_arg 

_Decode a CBOR argument from the bottom-5-bit info field._ 
```C++
static uint64_t decode_arg (
    SYN_CborReader * r,
    uint8_t info
) 
```



Reads the argument value associated with `info`. Consumes 0, 1, 2, 4, or 8 additional bytes. Sets `r->ok = false` for reserved info values.




**Parameters:**


* `r` CBOR reader. 
* `info` Bottom 5 bits of the header byte. 



**Returns:**

Decoded argument value. 





        

<hr>



### function peek\_byte 

_Peek at the next byte without consuming it._ 
```C++
static uint8_t peek_byte (
    const SYN_CborReader * r
) 
```





**Parameters:**


* `r` CBOR reader. 



**Returns:**

Next byte, or 0 if at end. 





        

<hr>
## Macro Definition Documentation





### define SKIP\_MAX\_DEPTH 

_Maximum nesting depth for syn\_cbor\_skip._ 
```C++
#define SKIP_MAX_DEPTH `8u`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_cbor_read.c`

