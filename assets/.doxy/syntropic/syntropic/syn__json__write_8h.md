

# File syn\_json\_write.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_json\_write.h**](syn__json__write_8h.md)

[Go to the source code of this file](syn__json__write_8h_source.md)

_Streaming JSON builder — zero-alloc, caller-provided buffer._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_JsonWriter**](structSYN__JsonWriter.md) <br>_Streaming JSON writer — tracks nesting, commas, and overflow._  |






















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
|  size\_t | [**syn\_json\_len**](#function-syn_json_len) (const [**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Get the number of bytes written._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_json\_ok**](#function-syn_json_ok) (const [**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Check if the writer is in a valid state (no overflow)._  |
|  const char \* | [**syn\_json\_str**](#function-syn_json_str) (const [**SYN\_JsonWriter**](structSYN__JsonWriter.md) \* w) <br>_Get the null-terminated output string._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_JSON\_MAX\_DEPTH**](syn__json__write_8h.md#define-syn_json_max_depth)  `8`<br> |

## Detailed Description


Builds JSON output incrementally into a fixed buffer. Handles auto-commas, nesting, and overflow detection. No parser — this is write-only for building API responses, telemetry payloads, etc.




**
**


```C++
char buf[256];
SYN_JsonWriter w;
syn_json_init(&w, buf, sizeof(buf));
syn_json_obj_open(&w);
  syn_json_key_str(&w, "device", "esp32");
  syn_json_key_int(&w, "uptime", 12345);
  syn_json_key_bool(&w, "wifi", true);
syn_json_obj_close(&w);
// buf = {"device":"esp32","uptime":12345,"wifi":true}
```
 





    
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




### function syn\_json\_len 

_Get the number of bytes written._ 
```C++
static inline size_t syn_json_len (
    const SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 



**Returns:**

Byte count. 





        

<hr>



### function syn\_json\_ok 

_Check if the writer is in a valid state (no overflow)._ 
```C++
static inline bool syn_json_ok (
    const SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 



**Returns:**

true if no overflow occurred. 





        

<hr>



### function syn\_json\_str 

_Get the null-terminated output string._ 
```C++
static inline const char * syn_json_str (
    const SYN_JsonWriter * w
) 
```





**Parameters:**


* `w` Writer. 



**Returns:**

Pointer to the output buffer. 





        

<hr>
## Macro Definition Documentation





### define SYN\_JSON\_MAX\_DEPTH 

```C++
#define SYN_JSON_MAX_DEPTH `8`
```



Maximum nesting depth 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_json_write.h`

