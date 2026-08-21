

# File syn\_protobuf.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_protobuf.c**](syn__protobuf_8c.md)

[Go to the source code of this file](syn__protobuf_8c_source.md)

_Zero-Allocation Protocol Buffers Serializer & Deserializer Implementation._ 

* `#include "syn_protobuf.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_decode\_bytes**](#function-syn_pb_decode_bytes) ([**SYN\_PB\_Decoder**](structSYN__PB__Decoder.md) \* dec, const uint8\_t \*\* bytes, size\_t \* len) <br>_Decode a length-delimited byte array field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_decode\_fixed32**](#function-syn_pb_decode_fixed32) ([**SYN\_PB\_Decoder**](structSYN__PB__Decoder.md) \* dec, uint32\_t \* value) <br>_Decode a fixed 32-bit integer field value._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_decode\_tag**](#function-syn_pb_decode_tag) ([**SYN\_PB\_Decoder**](structSYN__PB__Decoder.md) \* dec, [**SYN\_PB\_Tag**](structSYN__PB__Tag.md) \* tag) <br>_Decode the next field tag and wire type._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_decode\_varint**](#function-syn_pb_decode_varint) ([**SYN\_PB\_Decoder**](structSYN__PB__Decoder.md) \* dec, uint64\_t \* value) <br>_Decode a raw varint value._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_decoder\_init**](#function-syn_pb_decoder_init) ([**SYN\_PB\_Decoder**](structSYN__PB__Decoder.md) \* dec, const uint8\_t \* buf, size\_t size) <br>_Initialize a Protobuf decoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_bool**](#function-syn_pb_encode_bool) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, [**bool**](syn__defs_8h.md#enum-bool) value) <br>_Encode a boolean field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_bytes**](#function-syn_pb_encode_bytes) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, const uint8\_t \* bytes, size\_t len) <br>_Encode a raw byte array field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_fixed32**](#function-syn_pb_encode_fixed32) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, uint32\_t value) <br>_Encode a fixed 32-bit integer field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_int32**](#function-syn_pb_encode_int32) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, int32\_t value) <br>_Encode a signed 32-bit integer field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_string**](#function-syn_pb_encode_string) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, const char \* str) <br>_Encode a UTF-8 string field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_tag**](#function-syn_pb_encode_tag) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, uint8\_t wire\_type) <br>_Encode field tag and wire type._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_uint32**](#function-syn_pb_encode_uint32) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint32\_t field\_number, uint32\_t value) <br>_Encode an unsigned 32-bit integer field._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encode\_varint**](#function-syn_pb_encode_varint) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint64\_t value) <br>_Encode a raw varint value._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_encoder\_init**](#function-syn_pb_encoder_init) ([**SYN\_PB\_Encoder**](structSYN__PB__Encoder.md) \* enc, uint8\_t \* buf, size\_t capacity) <br>_Initialize a Protobuf encoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_pb\_skip\_field**](#function-syn_pb_skip_field) ([**SYN\_PB\_Decoder**](structSYN__PB__Decoder.md) \* dec, uint8\_t wire\_type) <br>_Skip unknown field based on wire type._  |




























## Public Functions Documentation




### function syn\_pb\_decode\_bytes 

_Decode a length-delimited byte array field._ 
```C++
SYN_Status syn_pb_decode_bytes (
    SYN_PB_Decoder * dec,
    const uint8_t ** bytes,
    size_t * len
) 
```





**Parameters:**


* `dec` Pointer to decoder instance. 
* `bytes` Output byte array pointer pointer. 
* `len` Output byte array length pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on EOF. 





        

<hr>



### function syn\_pb\_decode\_fixed32 

_Decode a fixed 32-bit integer field value._ 
```C++
SYN_Status syn_pb_decode_fixed32 (
    SYN_PB_Decoder * dec,
    uint32_t * value
) 
```





**Parameters:**


* `dec` Pointer to decoder instance. 
* `value` Output 32-bit value pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on EOF. 





        

<hr>



### function syn\_pb\_decode\_tag 

_Decode the next field tag and wire type._ 
```C++
SYN_Status syn_pb_decode_tag (
    SYN_PB_Decoder * dec,
    SYN_PB_Tag * tag
) 
```





**Parameters:**


* `dec` Pointer to decoder instance. 
* `tag` Output tag structure pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on EOF. 





        

<hr>



### function syn\_pb\_decode\_varint 

_Decode a raw varint value._ 
```C++
SYN_Status syn_pb_decode_varint (
    SYN_PB_Decoder * dec,
    uint64_t * value
) 
```





**Parameters:**


* `dec` Pointer to decoder instance. 
* `value` Output 64-bit value pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on EOF. 





        

<hr>



### function syn\_pb\_decoder\_init 

_Initialize a Protobuf decoder instance._ 
```C++
SYN_Status syn_pb_decoder_init (
    SYN_PB_Decoder * dec,
    const uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `dec` Pointer to decoder instance. 
* `buf` Input byte buffer pointer. 
* `size` Input byte buffer length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL. 





        

<hr>



### function syn\_pb\_encode\_bool 

_Encode a boolean field._ 
```C++
SYN_Status syn_pb_encode_bool (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    bool value
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `value` Boolean value. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_bytes 

_Encode a raw byte array field._ 
```C++
SYN_Status syn_pb_encode_bytes (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    const uint8_t * bytes,
    size_t len
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `bytes` Pointer to input byte array. 
* `len` Length of byte array. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_fixed32 

_Encode a fixed 32-bit integer field._ 
```C++
SYN_Status syn_pb_encode_fixed32 (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    uint32_t value
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `value` Fixed 32-bit integer value. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_int32 

_Encode a signed 32-bit integer field._ 
```C++
SYN_Status syn_pb_encode_int32 (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    int32_t value
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `value` Signed 32-bit integer value. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_string 

_Encode a UTF-8 string field._ 
```C++
SYN_Status syn_pb_encode_string (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    const char * str
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `str` Null-terminated string pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_tag 

_Encode field tag and wire type._ 
```C++
SYN_Status syn_pb_encode_tag (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    uint8_t wire_type
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `wire_type` Protobuf wire type (0, 1, 2, 5). 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_uint32 

_Encode an unsigned 32-bit integer field._ 
```C++
SYN_Status syn_pb_encode_uint32 (
    SYN_PB_Encoder * enc,
    uint32_t field_number,
    uint32_t value
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `field_number` Field tag number. 
* `value` Unsigned 32-bit integer value. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encode\_varint 

_Encode a raw varint value._ 
```C++
SYN_Status syn_pb_encode_varint (
    SYN_PB_Encoder * enc,
    uint64_t value
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `value` 64-bit value to encode. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on overflow. 





        

<hr>



### function syn\_pb\_encoder\_init 

_Initialize a Protobuf encoder instance._ 
```C++
SYN_Status syn_pb_encoder_init (
    SYN_PB_Encoder * enc,
    uint8_t * buf,
    size_t capacity
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `buf` Output byte buffer. 
* `capacity` Maximum buffer size in bytes. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if NULL or zero. 





        

<hr>



### function syn\_pb\_skip\_field 

_Skip unknown field based on wire type._ 
```C++
SYN_Status syn_pb_skip_field (
    SYN_PB_Decoder * dec,
    uint8_t wire_type
) 
```





**Parameters:**


* `dec` Pointer to decoder instance. 
* `wire_type` Wire type of field to skip. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_protobuf.c`

