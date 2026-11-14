

# File syn\_cbor\_write.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_cbor\_write.c**](syn__cbor__write_8c.md)

[Go to the source code of this file](syn__cbor__write_8c_source.md)

_CBOR streaming encoder implementation._ 

* `#include "syn_cbor_write.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cbor\_write\_array\_begin**](#function-syn_cbor_write_array_begin) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, size\_t count) <br>_Open a CBOR array with_ `count` _items._ |
|  void | [**syn\_cbor\_write\_bool**](#function-syn_cbor_write_bool) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, [**bool**](syn__defs_8h.md#enum-bool) v) <br>_Write a boolean (0xF4 = false, 0xF5 = true)._  |
|  void | [**syn\_cbor\_write\_bytes**](#function-syn_cbor_write_bytes) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, const uint8\_t \* data, size\_t len) <br>_Write a byte string (major type 2)._  |
|  void | [**syn\_cbor\_write\_float**](#function-syn_cbor_write_float) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, float v) <br>_Write an IEEE 754 single-precision float (major type 7, info=26)._  |
|  void | [**syn\_cbor\_write\_int**](#function-syn_cbor_write_int) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, int64\_t v) <br>_Write a signed integer._  |
|  void | [**syn\_cbor\_write\_map\_begin**](#function-syn_cbor_write_map_begin) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, size\_t count) <br>_Open a CBOR map with_ `count` _key-value pairs._ |
|  void | [**syn\_cbor\_write\_null**](#function-syn_cbor_write_null) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w) <br>_Write a null (0xF6)._  |
|  void | [**syn\_cbor\_write\_text**](#function-syn_cbor_write_text) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, const char \* str, size\_t len) <br>_Write a UTF-8 text string (major type 3)._  |
|  void | [**syn\_cbor\_write\_text\_cstr**](#function-syn_cbor_write_text_cstr) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, const char \* str) <br>_Write a null-terminated UTF-8 text string._  |
|  void | [**syn\_cbor\_write\_uint**](#function-syn_cbor_write_uint) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, uint64\_t v) <br>_Write an unsigned integer._  |
|  void | [**syn\_cbor\_writer\_init**](#function-syn_cbor_writer_init) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, uint8\_t \* buf, size\_t cap) <br>_Initialize a CBOR writer._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**emit\_byte**](#function-emit_byte) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, uint8\_t b) <br>_Emit a single byte to the CBOR output._  |
|  void | [**emit\_head**](#function-emit_head) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, uint8\_t major, uint64\_t val) <br>_Encode a CBOR header (major type + argument)._  |
|  void | [**emit\_raw**](#function-emit_raw) ([**SYN\_CborWriter**](structSYN__CborWriter.md) \* w, const void \* data, size\_t n) <br>_Emit N raw bytes._  |


























## Public Functions Documentation




### function syn\_cbor\_write\_array\_begin 

_Open a CBOR array with_ `count` _items._
```C++
void syn_cbor_write_array_begin (
    SYN_CborWriter * w,
    size_t count
) 
```



Caller must write exactly `count` items after this call.




**Parameters:**


* `w` Writer. 
* `count` Number of items that follow. 




        

<hr>



### function syn\_cbor\_write\_bool 

_Write a boolean (0xF4 = false, 0xF5 = true)._ 
```C++
void syn_cbor_write_bool (
    SYN_CborWriter * w,
    bool v
) 
```





**Parameters:**


* `w` Writer. 
* `v` Value. 




        

<hr>



### function syn\_cbor\_write\_bytes 

_Write a byte string (major type 2)._ 
```C++
void syn_cbor_write_bytes (
    SYN_CborWriter * w,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `w` Writer. 
* `data` Pointer to raw bytes. 
* `len` Byte count. 




        

<hr>



### function syn\_cbor\_write\_float 

_Write an IEEE 754 single-precision float (major type 7, info=26)._ 
```C++
void syn_cbor_write_float (
    SYN_CborWriter * w,
    float v
) 
```





**Parameters:**


* `w` Writer. 
* `v` Float value. 




        

<hr>



### function syn\_cbor\_write\_int 

_Write a signed integer._ 
```C++
void syn_cbor_write_int (
    SYN_CborWriter * w,
    int64_t v
) 
```



Positive values are encoded as major type 0 (uint). Negative values are encoded as major type 1.




**Parameters:**


* `w` Writer. 
* `v` Signed value to encode. 




        

<hr>



### function syn\_cbor\_write\_map\_begin 

_Open a CBOR map with_ `count` _key-value pairs._
```C++
void syn_cbor_write_map_begin (
    SYN_CborWriter * w,
    size_t count
) 
```



Caller must write exactly `count` pairs (key item + value item each) after this call.




**Parameters:**


* `w` Writer. 
* `count` Number of key-value pairs that follow. 




        

<hr>



### function syn\_cbor\_write\_null 

_Write a null (0xF6)._ 
```C++
void syn_cbor_write_null (
    SYN_CborWriter * w
) 
```





**Parameters:**


* `w` Writer. 




        

<hr>



### function syn\_cbor\_write\_text 

_Write a UTF-8 text string (major type 3)._ 
```C++
void syn_cbor_write_text (
    SYN_CborWriter * w,
    const char * str,
    size_t len
) 
```





**Parameters:**


* `w` Writer. 
* `str` Pointer to string data. 
* `len` Byte length (not counting any null terminator). 




        

<hr>



### function syn\_cbor\_write\_text\_cstr 

_Write a null-terminated UTF-8 text string._ 
```C++
void syn_cbor_write_text_cstr (
    SYN_CborWriter * w,
    const char * str
) 
```





**Parameters:**


* `w` Writer. 
* `str` Null-terminated string. 




        

<hr>



### function syn\_cbor\_write\_uint 

_Write an unsigned integer._ 
```C++
void syn_cbor_write_uint (
    SYN_CborWriter * w,
    uint64_t v
) 
```





**Parameters:**


* `w` Writer. 
* `v` Value to encode (CBOR major type 0). 




        

<hr>



### function syn\_cbor\_writer\_init 

_Initialize a CBOR writer._ 
```C++
void syn_cbor_writer_init (
    SYN_CborWriter * w,
    uint8_t * buf,
    size_t cap
) 
```





**Parameters:**


* `w` Writer to initialize. Must not be NULL. 
* `buf` Output buffer. Must not be NULL. 
* `cap` Buffer capacity in bytes. 




        

<hr>
## Public Static Functions Documentation




### function emit\_byte 

_Emit a single byte to the CBOR output._ 
```C++
static void emit_byte (
    SYN_CborWriter * w,
    uint8_t b
) 
```





**Parameters:**


* `w` CBOR writer. 
* `b` Byte to emit. 




        

<hr>



### function emit\_head 

_Encode a CBOR header (major type + argument)._ 
```C++
static void emit_head (
    SYN_CborWriter * w,
    uint8_t major,
    uint64_t val
) 
```



Follows the CBOR additional-info rules for all major types 0-5.




**Parameters:**


* `w` CBOR writer. 
* `major` Major type (0-7). 
* `val` Argument value. 




        

<hr>



### function emit\_raw 

_Emit N raw bytes._ 
```C++
static void emit_raw (
    SYN_CborWriter * w,
    const void * data,
    size_t n
) 
```





**Parameters:**


* `w` CBOR writer. 
* `data` Source data. 
* `n` Number of bytes. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_cbor_write.c`

