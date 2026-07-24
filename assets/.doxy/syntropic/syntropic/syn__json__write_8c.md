

# File syn\_json\_write.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_json\_write.c**](syn__json__write_8c.md)

[Go to the source code of this file](syn__json__write_8c_source.md)

_Streaming JSON builder implementation._ 

* `#include "syn_json_write.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_json\_arr\_close**](#function-syn_json_arr_close) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Close a JSON array_ `]` _._ |
|  void | [**syn\_json\_arr\_open**](#function-syn_json_arr_open) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Open a JSON array_ `[` _._ |
|  void | [**syn\_json\_init**](#function-syn_json_init) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, char \* buf, size\_t capacity) <br>_Initialize a JSON writer._  |
|  void | [**syn\_json\_key**](#function-syn_json_key) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* key) <br>_Write a bare key_ `"key":` _for nested objects/arrays._ |
|  void | [**syn\_json\_key\_bool**](#function-syn_json_key_bool) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* key, [**bool**](syn__defs_8h.md#enum-bool) val) <br>_Write_ `"key":true` _or_`"key":false` _._ |
|  void | [**syn\_json\_key\_int**](#function-syn_json_key_int) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* key, int32\_t val) <br>_Write_ `"key":123` _._ |
|  void | [**syn\_json\_key\_null**](#function-syn_json_key_null) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* key) <br>_Write_ `"key":null` _._ |
|  void | [**syn\_json\_key\_str**](#function-syn_json_key_str) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* key, const char \* val) <br>_Write_ `"key":"value"` _. Escapes_`"` _and_`\\` _in value._ |
|  void | [**syn\_json\_key\_uint**](#function-syn_json_key_uint) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* key, uint32\_t val) <br>_Write_ `"key":123` _(unsigned)._ |
|  void | [**syn\_json\_obj\_close**](#function-syn_json_obj_close) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Close a JSON object_ `}` _._ |
|  void | [**syn\_json\_obj\_open**](#function-syn_json_obj_open) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Open a JSON object_ `{` _._ |
|  void | [**syn\_json\_val\_bool**](#function-syn_json_val_bool) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, [**bool**](syn__defs_8h.md#enum-bool) val) <br>_Append a boolean value to an array._  |
|  void | [**syn\_json\_val\_int**](#function-syn_json_val_int) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, int32\_t val) <br>_Append an integer value to an array._  |
|  void | [**syn\_json\_val\_str**](#function-syn_json_val_str) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* val) <br>_Append a string value to an array._  |
|  void | [**syn\_json\_val\_uint**](#function-syn_json_val_uint) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, uint32\_t val) <br>_Append an unsigned integer value to an array._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**jw\_comma**](#function-jw_comma) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Insert comma separator if needed._  |
|  void | [**jw\_int**](#function-jw_int) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, int32\_t val) <br>_Write a signed integer._  |
|  void | [**jw\_putc**](#function-jw_putc) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, char ch) <br>_Append a single char to the output buffer._  |
|  void | [**jw\_puts**](#function-jw_puts) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* s) <br>_Append a raw string (no escaping)._  |
|  void | [**jw\_str**](#function-jw_str) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, const char \* s) <br>_Write an escaped JSON string (with surrounding quotes)._  |
|  void | [**jw\_uint**](#function-jw_uint) ([**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w, uint32\_t val) <br>_Write an unsigned integer._  |


























## Public Functions Documentation




### function syn\_json\_arr\_close 

_Close a JSON array_ `]` _._
```C++
void syn_json_arr_close (
    SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 




        

<hr>



### function syn\_json\_arr\_open 

_Open a JSON array_ `[` _._
```C++
void syn_json_arr_open (
    SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 




        

<hr>



### function syn\_json\_init 

_Initialize a JSON writer._ 
```C++
void syn_json_init (
    SYN_JsonWriter * w,
    char * buf,
    size_t capacity
) 
```





**Parameters:**


* `w` Writer instance. 
* `buf` Output buffer. 
* `capacity` Buffer size in bytes. 




        

<hr>



### function syn\_json\_key 

_Write a bare key_ `"key":` _for nested objects/arrays._
```C++
void syn_json_key (
    SYN_JsonWriter * w,
    const char * key
) 
```



Follow with [**syn\_json\_obj\_open()**](syn__json__write_8h.md#function-syn_json_obj_open) or [**syn\_json\_arr\_open()**](syn__json__write_8h.md#function-syn_json_arr_open).




**Parameters:**


* `w` Writer. 
* `key` JSON key. 




        

<hr>



### function syn\_json\_key\_bool 

_Write_ `"key":true` _or_`"key":false` _._
```C++
void syn_json_key_bool (
    SYN_JsonWriter * w,
    const char * key,
    bool val
) 
```





**Parameters:**


* `w` Writer. 
* `key` JSON key. 
* `val` Boolean value. 




        

<hr>



### function syn\_json\_key\_int 

_Write_ `"key":123` _._
```C++
void syn_json_key_int (
    SYN_JsonWriter * w,
    const char * key,
    int32_t val
) 
```





**Parameters:**


* `w` Writer. 
* `key` JSON key. 
* `val` Integer value. 




        

<hr>



### function syn\_json\_key\_null 

_Write_ `"key":null` _._
```C++
void syn_json_key_null (
    SYN_JsonWriter * w,
    const char * key
) 
```





**Parameters:**


* `w` Writer. 
* `key` JSON key. 




        

<hr>



### function syn\_json\_key\_str 

_Write_ `"key":"value"` _. Escapes_`"` _and_`\\` _in value._
```C++
void syn_json_key_str (
    SYN_JsonWriter * w,
    const char * key,
    const char * val
) 
```





**Parameters:**


* `w` Writer. 
* `key` JSON key. 
* `val` String value. 




        

<hr>



### function syn\_json\_key\_uint 

_Write_ `"key":123` _(unsigned)._
```C++
void syn_json_key_uint (
    SYN_JsonWriter * w,
    const char * key,
    uint32_t val
) 
```





**Parameters:**


* `w` Writer. 
* `key` JSON key. 
* `val` Unsigned integer value. 




        

<hr>



### function syn\_json\_obj\_close 

_Close a JSON object_ `}` _._
```C++
void syn_json_obj_close (
    SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 




        

<hr>



### function syn\_json\_obj\_open 

_Open a JSON object_ `{` _._
```C++
void syn_json_obj_open (
    SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 




        

<hr>



### function syn\_json\_val\_bool 

_Append a boolean value to an array._ 
```C++
void syn_json_val_bool (
    SYN_JsonWriter * w,
    bool val
) 
```





**Parameters:**


* `w` Writer. 
* `val` Boolean value. 




        

<hr>



### function syn\_json\_val\_int 

_Append an integer value to an array._ 
```C++
void syn_json_val_int (
    SYN_JsonWriter * w,
    int32_t val
) 
```





**Parameters:**


* `w` Writer. 
* `val` Integer value. 




        

<hr>



### function syn\_json\_val\_str 

_Append a string value to an array._ 
```C++
void syn_json_val_str (
    SYN_JsonWriter * w,
    const char * val
) 
```





**Parameters:**


* `w` Writer. 
* `val` String value. 




        

<hr>



### function syn\_json\_val\_uint 

_Append an unsigned integer value to an array._ 
```C++
void syn_json_val_uint (
    SYN_JsonWriter * w,
    uint32_t val
) 
```





**Parameters:**


* `w` Writer. 
* `val` Unsigned value. 




        

<hr>
## Public Static Functions Documentation




### function jw\_comma 

_Insert comma separator if needed._ 
```C++
static void jw_comma (
    SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` JSON writer. 




        

<hr>



### function jw\_int 

_Write a signed integer._ 
```C++
static void jw_int (
    SYN_JsonWriter * w,
    int32_t val
) 
```





**Parameters:**


* `w` JSON writer. 
* `val` Value to write. 




        

<hr>



### function jw\_putc 

_Append a single char to the output buffer._ 
```C++
static void jw_putc (
    SYN_JsonWriter * w,
    char ch
) 
```





**Parameters:**


* `w` JSON writer. 
* `ch` Character to append. 




        

<hr>



### function jw\_puts 

_Append a raw string (no escaping)._ 
```C++
static void jw_puts (
    SYN_JsonWriter * w,
    const char * s
) 
```





**Parameters:**


* `w` JSON writer. 
* `s` String to append. 




        

<hr>



### function jw\_str 

_Write an escaped JSON string (with surrounding quotes)._ 
```C++
static void jw_str (
    SYN_JsonWriter * w,
    const char * s
) 
```





**Parameters:**


* `w` JSON writer. 
* `s` String to encode. 




        

<hr>



### function jw\_uint 

_Write an unsigned integer._ 
```C++
static void jw_uint (
    SYN_JsonWriter * w,
    uint32_t val
) 
```





**Parameters:**


* `w` JSON writer. 
* `val` Value to write. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_write.c`

