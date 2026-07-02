

# File syn\_ringbuf.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_ringbuf.h**](syn__ringbuf_8h.md)

[Go to the source code of this file](syn__ringbuf_8h_source.md)

_Fixed-size, statically-allocated ring buffer._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_RingBuf**](structSYN__RingBuf.md) <br>_Ring buffer control structure._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_ringbuf\_count**](#function-syn_ringbuf_count) (const [**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb) <br>_Return the number of bytes currently stored in the buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ringbuf\_empty**](#function-syn_ringbuf_empty) (const [**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb) <br>_Check if the ring buffer is empty._  |
|  size\_t | [**syn\_ringbuf\_free**](#function-syn_ringbuf_free) (const [**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb) <br>_Return the number of free bytes available for writing._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ringbuf\_full**](#function-syn_ringbuf_full) (const [**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb) <br>_Check if the ring buffer is full._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ringbuf\_get**](#function-syn_ringbuf_get) ([**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, uint8\_t \* byte) <br>_Get a byte from the ring buffer._  |
|  void | [**syn\_ringbuf\_init**](#function-syn_ringbuf_init) ([**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, uint8\_t \* buf, size\_t size) <br>_Initialize a ring buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ringbuf\_peek**](#function-syn_ringbuf_peek) (const [**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, uint8\_t \* byte) <br>_Peek at the next byte without removing it._  |
|  size\_t | [**syn\_ringbuf\_peek\_n**](#function-syn_ringbuf_peek_n) (const [**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, uint8\_t \* data, size\_t len) <br>_Peek at up to_ `len` _bytes without removing them._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_ringbuf\_put**](#function-syn_ringbuf_put) ([**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, uint8\_t byte) <br>_Put a byte into the ring buffer._  |
|  size\_t | [**syn\_ringbuf\_read**](#function-syn_ringbuf_read) ([**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, uint8\_t \* data, size\_t len) <br>_Read multiple bytes from the ring buffer._  |
|  void | [**syn\_ringbuf\_reset**](#function-syn_ringbuf_reset) ([**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb) <br>_Reset the ring buffer to empty._  |
|  size\_t | [**syn\_ringbuf\_write**](#function-syn_ringbuf_write) ([**SYN\_RingBuf**](structSYN__RingBuf.md) \* rb, const uint8\_t \* data, size\_t len) <br>_Write multiple bytes into the ring buffer._  |




























## Detailed Description


Designed for ISR-safe single-producer / single-consumer use. The buffer is caller-owned — you provide the backing array and the [**SYN\_RingBuf**](structSYN__RingBuf.md) struct; the library never allocates memory.


Usage: 
```C++
static uint8_t backing[64];
static SYN_RingBuf rb;
syn_ringbuf_init(&rb, backing, sizeof(backing));

syn_ringbuf_put(&rb, 0xAA);           // producer (e.g., ISR)

uint8_t byte;
if (syn_ringbuf_get(&rb, &byte)) {    // consumer (e.g., main loop)
    // use byte
}
```
 


    
## Public Functions Documentation




### function syn\_ringbuf\_count 

_Return the number of bytes currently stored in the buffer._ 
```C++
size_t syn_ringbuf_count (
    const SYN_RingBuf * rb
) 
```





**Parameters:**


* `rb` Ring buffer. 



**Returns:**

Byte count. 





        

<hr>



### function syn\_ringbuf\_empty 

_Check if the ring buffer is empty._ 
```C++
bool syn_ringbuf_empty (
    const SYN_RingBuf * rb
) 
```





**Parameters:**


* `rb` Ring buffer. 



**Returns:**

true if empty. 





        

<hr>



### function syn\_ringbuf\_free 

_Return the number of free bytes available for writing._ 
```C++
size_t syn_ringbuf_free (
    const SYN_RingBuf * rb
) 
```





**Parameters:**


* `rb` Ring buffer. 



**Returns:**

Free byte count. 





        

<hr>



### function syn\_ringbuf\_full 

_Check if the ring buffer is full._ 
```C++
bool syn_ringbuf_full (
    const SYN_RingBuf * rb
) 
```





**Parameters:**


* `rb` Ring buffer. 



**Returns:**

true if full. 





        

<hr>



### function syn\_ringbuf\_get 

_Get a byte from the ring buffer._ 
```C++
bool syn_ringbuf_get (
    SYN_RingBuf * rb,
    uint8_t * byte
) 
```





**Parameters:**


* `rb` Ring buffer. 
* `byte` [out] The retrieved byte. 



**Returns:**

true if a byte was available, false if the buffer is empty. 





        

<hr>



### function syn\_ringbuf\_init 

_Initialize a ring buffer._ 
```C++
void syn_ringbuf_init (
    SYN_RingBuf * rb,
    uint8_t * buf,
    size_t size
) 
```





**Parameters:**


* `rb` Pointer to the ring buffer control struct. 
* `buf` Pointer to the caller-provided backing array. 
* `size` Size of the backing array in bytes. Must be &gt; 0. One byte is used as a sentinel, so usable capacity is size - 1. 




        

<hr>



### function syn\_ringbuf\_peek 

_Peek at the next byte without removing it._ 
```C++
bool syn_ringbuf_peek (
    const SYN_RingBuf * rb,
    uint8_t * byte
) 
```





**Parameters:**


* `rb` Ring buffer. 
* `byte` [out] The peeked byte. 



**Returns:**

true if a byte was available, false if the buffer is empty. 





        

<hr>



### function syn\_ringbuf\_peek\_n 

_Peek at up to_ `len` _bytes without removing them._
```C++
size_t syn_ringbuf_peek_n (
    const SYN_RingBuf * rb,
    uint8_t * data,
    size_t len
) 
```



Copies bytes starting from the read position into `data` without advancing the tail. Subsequent calls to syn\_ringbuf\_read will return the same bytes.




**Parameters:**


* `rb` Ring buffer. 
* `data` [out] Destination buffer. 
* `len` Maximum number of bytes to peek. 



**Returns:**

Number of bytes actually copied (may be less than `len`). 





        

<hr>



### function syn\_ringbuf\_put 

_Put a byte into the ring buffer._ 
```C++
bool syn_ringbuf_put (
    SYN_RingBuf * rb,
    uint8_t byte
) 
```





**Parameters:**


* `rb` Ring buffer. 
* `byte` Byte to store. 



**Returns:**

true if the byte was stored, false if the buffer is full. 





        

<hr>



### function syn\_ringbuf\_read 

_Read multiple bytes from the ring buffer._ 
```C++
size_t syn_ringbuf_read (
    SYN_RingBuf * rb,
    uint8_t * data,
    size_t len
) 
```



Reads up to `len` bytes into `data`. Returns the number of bytes actually read (may be less than `len` if the buffer doesn't have enough). Uses memcpy internally for efficiency across wrap boundaries.




**Parameters:**


* `rb` Ring buffer. 
* `data` [out] Destination buffer. 
* `len` Maximum number of bytes to read. 



**Returns:**

Number of bytes actually read. 





        

<hr>



### function syn\_ringbuf\_reset 

_Reset the ring buffer to empty._ 
```C++
void syn_ringbuf_reset (
    SYN_RingBuf * rb
) 
```





**Parameters:**


* `rb` Ring buffer. 




        

<hr>



### function syn\_ringbuf\_write 

_Write multiple bytes into the ring buffer._ 
```C++
size_t syn_ringbuf_write (
    SYN_RingBuf * rb,
    const uint8_t * data,
    size_t len
) 
```



Writes up to `len` bytes from `data`. Returns the number of bytes actually written (may be less than `len` if the buffer fills up). Uses memcpy internally for efficiency across wrap boundaries.




**Parameters:**


* `rb` Ring buffer. 
* `data` Source data to write. 
* `len` Number of bytes to write. 



**Returns:**

Number of bytes actually written. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_ringbuf.h`

