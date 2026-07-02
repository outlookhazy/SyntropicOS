

# File syn\_pack.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pack.h**](syn__pack_8h.md)

[Go to the source code of this file](syn__pack_8h_source.md)

_Binary message packer / unpacker (header-only)._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <string.h>`







































## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_pack\_bytes**](#function-syn_pack_bytes) (uint8\_t \* buf, size\_t \* pos, const uint8\_t \* data, size\_t len) <br>_Pack a raw byte array._  |
|  void | [**syn\_pack\_i16**](#function-syn_pack_i16) (uint8\_t \* buf, size\_t \* pos, int16\_t val) <br>_Pack an int16\_t value (Big Endian)._  |
|  void | [**syn\_pack\_i16\_le**](#function-syn_pack_i16_le) (uint8\_t \* buf, size\_t \* pos, int16\_t val) <br>_Pack an int16\_t value (Little Endian)._  |
|  void | [**syn\_pack\_i32**](#function-syn_pack_i32) (uint8\_t \* buf, size\_t \* pos, int32\_t val) <br>_Pack an int32\_t value (Big Endian)._  |
|  void | [**syn\_pack\_i32\_le**](#function-syn_pack_i32_le) (uint8\_t \* buf, size\_t \* pos, int32\_t val) <br>_Pack an int32\_t value (Little Endian)._  |
|  void | [**syn\_pack\_i8**](#function-syn_pack_i8) (uint8\_t \* buf, size\_t \* pos, int8\_t val) <br>_Pack an int8\_t value (Big Endian)._  |
|  void | [**syn\_pack\_u16**](#function-syn_pack_u16) (uint8\_t \* buf, size\_t \* pos, uint16\_t val) <br>_Pack a uint16\_t value (Big Endian)._  |
|  void | [**syn\_pack\_u16\_le**](#function-syn_pack_u16_le) (uint8\_t \* buf, size\_t \* pos, uint16\_t val) <br>_Pack a uint16\_t value (Little Endian)._  |
|  void | [**syn\_pack\_u32**](#function-syn_pack_u32) (uint8\_t \* buf, size\_t \* pos, uint32\_t val) <br>_Pack a uint32\_t value (Big Endian)._  |
|  void | [**syn\_pack\_u32\_le**](#function-syn_pack_u32_le) (uint8\_t \* buf, size\_t \* pos, uint32\_t val) <br>_Pack a uint32\_t value (Little Endian)._  |
|  void | [**syn\_pack\_u8**](#function-syn_pack_u8) (uint8\_t \* buf, size\_t \* pos, uint8\_t val) <br>_Pack a uint8\_t value (Big Endian)._  |
|  uint16\_t | [**syn\_peek\_u16**](#function-syn_peek_u16) (const uint8\_t \* buf, size\_t pos) <br>_Peek a uint16\_t value (Big Endian) without advancing position._  |
|  uint32\_t | [**syn\_peek\_u32**](#function-syn_peek_u32) (const uint8\_t \* buf, size\_t pos) <br>_Peek a uint32\_t value (Big Endian) without advancing position._  |
|  uint8\_t | [**syn\_peek\_u8**](#function-syn_peek_u8) (const uint8\_t \* buf, size\_t pos) <br>_Peek a uint8\_t value (Big Endian) without advancing position._  |
|  void | [**syn\_unpack\_bytes**](#function-syn_unpack_bytes) (const uint8\_t \* buf, size\_t \* pos, uint8\_t \* out, size\_t len) <br>_Unpack a raw byte array._  |
|  int16\_t | [**syn\_unpack\_i16**](#function-syn_unpack_i16) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack an int16\_t value (Big Endian)._  |
|  int16\_t | [**syn\_unpack\_i16\_le**](#function-syn_unpack_i16_le) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack an int16\_t value (Little Endian)._  |
|  int32\_t | [**syn\_unpack\_i32**](#function-syn_unpack_i32) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack an int32\_t value (Big Endian)._  |
|  int32\_t | [**syn\_unpack\_i32\_le**](#function-syn_unpack_i32_le) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack an int32\_t value (Little Endian)._  |
|  int8\_t | [**syn\_unpack\_i8**](#function-syn_unpack_i8) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack an int8\_t value (Big Endian)._  |
|  uint16\_t | [**syn\_unpack\_u16**](#function-syn_unpack_u16) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack a uint16\_t value (Big Endian)._  |
|  uint16\_t | [**syn\_unpack\_u16\_le**](#function-syn_unpack_u16_le) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack a uint16\_t value (Little Endian)._  |
|  uint32\_t | [**syn\_unpack\_u32**](#function-syn_unpack_u32) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack a uint32\_t value (Big Endian)._  |
|  uint32\_t | [**syn\_unpack\_u32\_le**](#function-syn_unpack_u32_le) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack a uint32\_t value (Little Endian)._  |
|  uint8\_t | [**syn\_unpack\_u8**](#function-syn_unpack_u8) (const uint8\_t \* buf, size\_t \* pos) <br>_Unpack a uint8\_t value (Big Endian)._  |


























## Detailed Description


Endianness-safe serialization for packing sensor telemetry, config blobs, register maps, and protocol payloads.


All functions take a buffer and a position pointer that auto-advances.


Usage: 
```C++
uint8_t buf[32];
size_t pos = 0;

// Pack (big-endian)
syn_pack_u8(buf, &pos, 0x42);
syn_pack_u16(buf, &pos, 0x1234);
syn_pack_i32(buf, &pos, -12345);
syn_pack_bytes(buf, &pos, payload, 4);

// Unpack
pos = 0;
uint8_t  a = syn_unpack_u8(buf, &pos);
uint16_t b = syn_unpack_u16(buf, &pos);
int32_t  c = syn_unpack_i32(buf, &pos);
```
 


    
## Public Static Functions Documentation




### function syn\_pack\_bytes 

_Pack a raw byte array._ 
```C++
static inline void syn_pack_bytes (
    uint8_t * buf,
    size_t * pos,
    const uint8_t * data,
    size_t len
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `data` Array to copy. 
* `len` Number of bytes to copy. 




        

<hr>



### function syn\_pack\_i16 

_Pack an int16\_t value (Big Endian)._ 
```C++
static inline void syn_pack_i16 (
    uint8_t * buf,
    size_t * pos,
    int16_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_i16\_le 

_Pack an int16\_t value (Little Endian)._ 
```C++
static inline void syn_pack_i16_le (
    uint8_t * buf,
    size_t * pos,
    int16_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_i32 

_Pack an int32\_t value (Big Endian)._ 
```C++
static inline void syn_pack_i32 (
    uint8_t * buf,
    size_t * pos,
    int32_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_i32\_le 

_Pack an int32\_t value (Little Endian)._ 
```C++
static inline void syn_pack_i32_le (
    uint8_t * buf,
    size_t * pos,
    int32_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_i8 

_Pack an int8\_t value (Big Endian)._ 
```C++
static inline void syn_pack_i8 (
    uint8_t * buf,
    size_t * pos,
    int8_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_u16 

_Pack a uint16\_t value (Big Endian)._ 
```C++
static inline void syn_pack_u16 (
    uint8_t * buf,
    size_t * pos,
    uint16_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_u16\_le 

_Pack a uint16\_t value (Little Endian)._ 
```C++
static inline void syn_pack_u16_le (
    uint8_t * buf,
    size_t * pos,
    uint16_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_u32 

_Pack a uint32\_t value (Big Endian)._ 
```C++
static inline void syn_pack_u32 (
    uint8_t * buf,
    size_t * pos,
    uint32_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_u32\_le 

_Pack a uint32\_t value (Little Endian)._ 
```C++
static inline void syn_pack_u32_le (
    uint8_t * buf,
    size_t * pos,
    uint32_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_pack\_u8 

_Pack a uint8\_t value (Big Endian)._ 
```C++
static inline void syn_pack_u8 (
    uint8_t * buf,
    size_t * pos,
    uint8_t val
) 
```





**Parameters:**


* `buf` Target buffer. 
* `pos` Cursor position (auto-advanced). 
* `val` Value to pack. 




        

<hr>



### function syn\_peek\_u16 

_Peek a uint16\_t value (Big Endian) without advancing position._ 
```C++
static inline uint16_t syn_peek_u16 (
    const uint8_t * buf,
    size_t pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Source offset byte index. 



**Returns:**

Value at the offset index. 





        

<hr>



### function syn\_peek\_u32 

_Peek a uint32\_t value (Big Endian) without advancing position._ 
```C++
static inline uint32_t syn_peek_u32 (
    const uint8_t * buf,
    size_t pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Source offset byte index. 



**Returns:**

Value at the offset index. 





        

<hr>



### function syn\_peek\_u8 

_Peek a uint8\_t value (Big Endian) without advancing position._ 
```C++
static inline uint8_t syn_peek_u8 (
    const uint8_t * buf,
    size_t pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Source offset byte index. 



**Returns:**

Value at the offset index. 





        

<hr>



### function syn\_unpack\_bytes 

_Unpack a raw byte array._ 
```C++
static inline void syn_unpack_bytes (
    const uint8_t * buf,
    size_t * pos,
    uint8_t * out,
    size_t len
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 
* `out` Output buffer to copy into. 
* `len` Number of bytes to copy. 




        

<hr>



### function syn\_unpack\_i16 

_Unpack an int16\_t value (Big Endian)._ 
```C++
static inline int16_t syn_unpack_i16 (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_i16\_le 

_Unpack an int16\_t value (Little Endian)._ 
```C++
static inline int16_t syn_unpack_i16_le (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_i32 

_Unpack an int32\_t value (Big Endian)._ 
```C++
static inline int32_t syn_unpack_i32 (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_i32\_le 

_Unpack an int32\_t value (Little Endian)._ 
```C++
static inline int32_t syn_unpack_i32_le (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_i8 

_Unpack an int8\_t value (Big Endian)._ 
```C++
static inline int8_t syn_unpack_i8 (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_u16 

_Unpack a uint16\_t value (Big Endian)._ 
```C++
static inline uint16_t syn_unpack_u16 (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_u16\_le 

_Unpack a uint16\_t value (Little Endian)._ 
```C++
static inline uint16_t syn_unpack_u16_le (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_u32 

_Unpack a uint32\_t value (Big Endian)._ 
```C++
static inline uint32_t syn_unpack_u32 (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_u32\_le 

_Unpack a uint32\_t value (Little Endian)._ 
```C++
static inline uint32_t syn_unpack_u32_le (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>



### function syn\_unpack\_u8 

_Unpack a uint8\_t value (Big Endian)._ 
```C++
static inline uint8_t syn_unpack_u8 (
    const uint8_t * buf,
    size_t * pos
) 
```





**Parameters:**


* `buf` Source buffer. 
* `pos` Cursor position (auto-advanced). 



**Returns:**

Unpacked value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pack.h`

