

# File syn\_cbor\_read.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_cbor\_read.h**](syn__cbor__read_8h.md)

[Go to the source code of this file](syn__cbor__read_8h_source.md)

_CBOR decoder — zero-alloc, streaming read from a byte buffer._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CborReader**](structSYN__CborReader.md) <br>_CBOR decoder state. Caller-allocated; zero heap._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_CborType**](#enum-syn_cbortype)  <br>_CBOR item type as seen by the reader._  |




















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
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cbor\_reader\_done**](#function-syn_cbor_reader_done) (const [**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Return true when all bytes have been consumed._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_cbor\_reader\_ok**](#function-syn_cbor_reader_ok) (const [**SYN\_CborReader**](structSYN__CborReader.md) \* r) <br>_Return true if no decode error has occurred._  |


























## Detailed Description


Walks a CBOR-encoded buffer sequentially. Caller peeks the type of the next item, then calls the matching read function. Reading the wrong type sets the error flag and returns a zero/empty value.




**
**


```C++
SYN_CborReader r;
syn_cbor_reader_init(&r, buf, len);

size_t pairs = syn_cbor_read_map_begin(&r);   // A2 -> 2
for (size_t i = 0; i < pairs; i++) {
    uint64_t key = syn_cbor_read_uint(&r);
    if      (key == 1) temperature = syn_cbor_read_float(&r);
    else if (key == 2) humidity    = (uint8_t)syn_cbor_read_uint(&r);
    else    syn_cbor_skip(&r);
}
if (!syn_cbor_reader_ok(&r)) { // handle error }
```
 





    
## Public Types Documentation




### enum SYN\_CborType 

_CBOR item type as seen by the reader._ 
```C++
enum SYN_CborType {
    SYN_CBOR_UINT = 0,
    SYN_CBOR_INT = 1,
    SYN_CBOR_BYTES = 2,
    SYN_CBOR_TEXT = 3,
    SYN_CBOR_ARRAY = 4,
    SYN_CBOR_MAP = 5,
    SYN_CBOR_FLOAT = 6,
    SYN_CBOR_BOOL = 7,
    SYN_CBOR_NULL = 8,
    SYN_CBOR_ERROR = 0xFF
};
```



Returned by [**syn\_cbor\_peek\_type()**](syn__cbor__read_8h.md#function-syn_cbor_peek_type) before consuming an item. 


        

<hr>
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




### function syn\_cbor\_reader\_done 

_Return true when all bytes have been consumed._ 
```C++
static inline bool syn_cbor_reader_done (
    const SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

true if done. 





        

<hr>



### function syn\_cbor\_reader\_ok 

_Return true if no decode error has occurred._ 
```C++
static inline bool syn_cbor_reader_ok (
    const SYN_CborReader * r
) 
```





**Parameters:**


* `r` Reader. 



**Returns:**

true if OK. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_cbor_read.h`

